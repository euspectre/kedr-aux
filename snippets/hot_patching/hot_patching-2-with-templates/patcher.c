/* [NB] For now, we just don't care of some synchronization issues.
 * */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/slab.h> /* kmalloc for our needs*/
#include <linux/errno.h>
#include <linux/list.h>

#include <linux/kprobes.h>

#include <asm/insn.h> /* instruction decoder machinery */

// Will implement kedr_base+kedr_controller modules interfaces, but will not export them(!)
#include <kedr/base/common.h>

/* ================================================================ */
MODULE_AUTHOR("Eugene A. Shatokhin");
MODULE_LICENSE("GPL");

/* ========== Module Parameters ========== */
/* Name of the module to analyse. It can be passed to 'insmod' as
 * an argument, for example,
 *  /sbin/insmod hp_patcher.ko target_name="module_to_be_analysed"
 * The target module must be already loaded by the time hp_patcher begins to
 * initialize itself.
 */
static char* target_name = ""; /* an empty name will match no module */
module_param(target_name, charp, S_IRUGO);
/* ================================================================ */

/* The module being analysed. */
struct module* target_module = NULL;

/* Non-zero if the module unload notifier has been registered,
 * 0 otherwise.
 */
int module_notifier_registered = 0;
/* ================================================================ */

/*
 * The stuff related to the kprobe-based mechanism of target instrumentation.
 */

/* The required value of kedr_probe.magic (see below) */
#define KEDR_PROBE_MAGIC 0x9ED564F2

/* Each kedr_probe instance corresponds to a call to some target function
 * made by the target module.
 */
struct kedr_probe
{
    /* This is to link the probes for a particular target module
     * into a list.
     */
    struct list_head list;

    /* The kernel probe will be used to break into the target driver
     * in a reasonably SMP-safe way.
     */
    struct kprobe kernel_probe;

    /* A magic number used as a weak guarantee that the structure
     * does not contain garbage. Can be checked, for example, when
     * a pointer to the structure is obtained from the pointer to
     * struct kprobe via container_of().
     */
    unsigned int magic;

    /* 0 if the probe is hit for the first time, non-zero otherwise */
    int was_hit_before;

    /* The addresses of the original and the replacement functions
     * corresponding to the call instruction where the probe is placed.
     */
    void* orig_func;
    void* repl_func;
};

/* The list of kedr_probe instances */
struct list_head kedr_probes;
/* TODO: is it necessary to protect the list with a mutex?
 * See also the notes in init().
 */

/* A spinlock to guarantee proper synchronization when executing probe
 * handlers, removing the probes, etc.
 */
spinlock_t kedr_probe_lock;

/* TODO: Replaces the call address in the appropriate instruction.
 * Must be called with kedr_probe_lock held.
 */
static void
kedr_probe_replace_call(struct kedr_probe* kp);

/* TODO: Restores the call address in the appropriate instruction.
 * Must be called with kedr_probe_lock held.
 */
static void
kedr_probe_restore_call(struct kedr_probe* kp);

/* The pre-handler is where actually call addresses are changed. */
static int
kedr_probe_pre_handler(struct kprobe* p, struct pt_regs* regs)
{
    unsigned long flags;

    struct kedr_probe* kp =
        container_of(p, struct kedr_probe, kernel_probe);
    if (kp->magic != KEDR_PROBE_MAGIC)
    {
        printk(KERN_WARNING "[hp_patcher] "
"kedr_probe_pre_handler() called for a corrupted kedr_probe structure\n");

/* Not sure if there is a preferable way to indicate that something bad
 * happened in the handler, just issue a warning and return for now.
 */
        return 0;
    }

    //<> TODO: remove when debugging is finished
    /*printk (KERN_INFO "[hp_patcher] "
    "Probe hit at 0x%p, call address: 0x%p, was_hit_before=%d\n",
        (void*)p->addr,
        kp->orig_func,
        kp->was_hit_before);*/
    //<>

    spin_lock_irqsave(&kedr_probe_lock, flags);
    if (!kp->was_hit_before)
    {
        kedr_probe_replace_call(kp);
        kp->was_hit_before = 1;
    }
    /* Do nothing if the probe is hit not for the first time */
    spin_unlock_irqrestore(&kedr_probe_lock, flags);
    return 0;
}

/* Allocates memory for struct kedr_probe, initializes the structure and
 * returns a pointer to it if successfull. NULL is returned if there is not
 * enough memory.
 *
 * 'addr' is the address of the 'call' instructon to place the probe at
 * 'orig_func' and 'repl_func' - the address of a function that is supposed
 * to be called by that instruction and the address of the corresponding
 * replacement function, respectively.
 */
static struct kedr_probe*
kedr_probe_create(kprobe_opcode_t* addr, void* orig_func, void* repl_func)
{
    struct kedr_probe* kp;

    BUG_ON(addr == NULL);
    BUG_ON(orig_func == NULL);
    BUG_ON(repl_func == NULL);

    kp = kzalloc(sizeof(struct kedr_probe), GFP_KERNEL);
    if (kp == NULL)
    {
        return NULL;
    }

    /* 'was_hit_before' has been set to 0 already (by kzalloc) */
    kp->kernel_probe.addr = addr;
    kp->kernel_probe.pre_handler = kedr_probe_pre_handler;

    kp->magic = KEDR_PROBE_MAGIC;
    kp->orig_func = orig_func;
    kp->repl_func = repl_func;

    return kp;
}

/* Destroys the kedr_probe instance. */
static void
kedr_probe_destroy(struct kedr_probe* doomed)
{
    if (doomed != NULL)
    {
        doomed->magic = 0; /* just in case */
        kfree(doomed);
    }
    return;
}

/* Creates a kedr_probe instance with the specified parameters (see
 * kedr_probe_create), tries to register the associated kprobe with the
 * kprobe system and, if OK, adds the kedr_probe to the global list.
 *
 * The function returns 0 if successful, an error code otherwise.
 */
static int
kedr_probe_set(void* addr, void* orig_func, void* repl_func)
{
    struct kedr_probe* kp = NULL;
    int result = 0;

    kp = kedr_probe_create((kprobe_opcode_t*)addr,
        orig_func, repl_func);
    if (kp == NULL)
    {
        return -ENOMEM;
    }

    result = register_kprobe(&kp->kernel_probe);
    if (result != 0)
    {
        goto fail;
    }

    INIT_LIST_HEAD(&kp->list);
    list_add_tail(&kp->list, &kedr_probes);

    return 0; /* success */

fail:
    kedr_probe_destroy(kp);
    return result;
}


/* Unregisters and deletes all kedr_probe objects. */
static void
kedr_probe_clear_all(void)
{
    unsigned long flags;
    struct kedr_probe *kp, *tmp;

    list_for_each_entry_safe (kp, tmp, &kedr_probes, list)
    {
        spin_lock_irqsave(&kedr_probe_lock, flags);
        if (kp->was_hit_before)
        {
    /* If the probe has never been hit, the call address must remain
     * unchanged. Only if a hit occured, should we restore the address.
     */
            kedr_probe_restore_call(kp);
        }
        else
        {
    /* This is to prevent call replacement if the probe is hit after
     * kedr_probe_lock is unlocked but before the corresponding kprobe
     * is unregistered.
     */
            kp->was_hit_before = 1;
        }
        spin_unlock_irqrestore(&kedr_probe_lock, flags);

        unregister_kprobe(&kp->kernel_probe);
        list_del(&kp->list);
        kedr_probe_destroy(kp);
    }
    return;
}

/*==========================================================================*/
//List of registered structs 'struct kedr_payload'
struct payload_entry
{
    struct list_head list;
    struct kedr_payload* payload;
};

LIST_HEAD(payload_list);

int
kedr_payload_register(struct kedr_payload *payload)
{
    struct payload_entry *new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
    if(new_entry == NULL)
    {
        pr_err("[hp_patcher] Cannot allocate entry for registered payload.");
        return -ENOMEM;
    }
    new_entry->payload = payload;
    list_add(&new_entry->list, &payload_list);
    return 0;
}

void
kedr_payload_unregister(struct kedr_payload *payload)
{
    struct payload_entry *entry_del;
    list_for_each_entry(entry_del, &payload_list, list)
    {
        if(entry_del->payload == payload)
        {
            list_del(&entry_del->list);
            kfree(entry_del);
            return;
        }
    }
    BUG();
}

int
kedr_target_module_in_init(void)
{
    return target_module->module_init != NULL;
}

// Payload 'init' and 'exit' functions
extern int payload_init(void);
extern void payload_cleanup(void);

/* ================================================================ */
/* Helpers */

/* CALL_ADDR_FROM_OFFSET()
 *
 * Calculate the memory address being the operand of a given instruction
 * (usually, 'call').
 *   'insn_addr' is the address of the instruction itself,
 *   'insn_len' is length of the instruction in bytes,
 *   'offset' is the offset of the destination address from the first byte
 *   past the instruction.
 *
 * For x86_64 architecture, the offset value is sign-extended here first.
 *
 * "Intel x86 Instruction Set Reference" states the following
 * concerning 'call rel32':
 *
 * "Call near, relative, displacement relative to next instruction.
 * 32-bit displacement sign extended to 64 bits in 64-bit mode."
 * *****************************************************************
 *
 * CALL_OFFSET_FROM_ADDR()
 *
 * The reverse of CALL_ADDR_FROM_OFFSET: calculates the offset value
 * to be used in 'call' instruction given the address and length of the
 * instruction and the address of the destination function.
 *
 * */
#ifdef CONFIG_X86_64
#  define CALL_ADDR_FROM_OFFSET(insn_addr, insn_len, offset) \
    (void*)((s64)(insn_addr) + (s64)(insn_len) + (s64)(s32)(offset))

#else /* CONFIG_X86_32 */
#  define CALL_ADDR_FROM_OFFSET(insn_addr, insn_len, offset) \
    (void*)((u32)(insn_addr) + (u32)(insn_len) + (u32)(offset))
#endif

#define CALL_OFFSET_FROM_ADDR(insn_addr, insn_len, dest_addr) \
    (u32)(dest_addr - (insn_addr + (u32)insn_len))
/* ================================================================ */

/* ================================================================ */

/* Decode and process the instruction ('c_insn') at
 * the address 'kaddr' - see the description of do_process_area for details.
 *
 * Check if we get past the end of the buffer [kaddr, end_kaddr)
 *
 * The function returns the length of the instruction in bytes.
 * 0 is returned in case of failure.
 */
static unsigned int
do_process_insn(struct insn* c_insn, void* kaddr, void* end_kaddr,
    void** from_funcs, void** to_funcs, unsigned int nfuncs)
{
    /* ptr to the 32-bit offset argument in the instruction */
    u32* offset = NULL;

    /* address of the function being called */
    void* addr = NULL;

    static const unsigned char op_call = 0xe8; /* 'call <offset>' */
    static const unsigned char op_jmp  = 0xe9; /* 'jmp  <offset>' */

    int i;

    int result = 0;

    BUG_ON(from_funcs == NULL || to_funcs == NULL);

    /* Decode the instruction and populate 'insn' structure */
    kernel_insn_init(c_insn, kaddr);
    insn_get_length(c_insn);

    if (c_insn->length == 0)
    {
        return 0;
    }

    if (kaddr + c_insn->length > end_kaddr)
    {
    /* Note: it is OK to stop at 'end_kaddr' but no further */
        printk( KERN_WARNING "[hp_patcher] "
    "Instruction decoder stopped past the end of the section.\n");
    }

/* This call may be overkill as insn_get_length() probably has to decode
 * the instruction completely.
 * Still, to operate safely, we need insn_get_opcode() before we can access
 * c_insn->opcode.
 * The call is cheap anyway, no re-decoding is performed.
 */
    insn_get_opcode(c_insn);
    if (c_insn->opcode.value != op_call &&
        c_insn->opcode.value != op_jmp)
    {
        /* Not a 'call' instruction, nothing to do. */
        return c_insn->length;
    }

/* [NB] For some reason, the decoder stores the argument of 'call' and 'jmp'
 * as 'immediate' rather than 'displacement' (as Intel manuals name it).
 * May be it is a bug, may be it is not.
 * Meanwhile, I'll call this value 'offset' to avoid confusion.
 */

    /* Call this before trying to access c_insn->immediate */
    insn_get_immediate(c_insn);
    if (c_insn->immediate.nbytes != 4)
    {
        printk( KERN_WARNING "[hp_patcher] At 0x%p: "
    "opcode: 0x%x, "
    "immediate field is %u rather than 32 bits in size; "
    "insn.length = %u, insn.imm = %u, off_immed = %d\n",
            kaddr,
            (unsigned int)c_insn->opcode.value,
            8 * (unsigned int)c_insn->immediate.nbytes,
            c_insn->length,
            (unsigned int)c_insn->immediate.value,
            insn_offset_immediate(c_insn));

        return c_insn->length;
    }

    offset = (u32*)(kaddr + insn_offset_immediate(c_insn));
    addr = CALL_ADDR_FROM_OFFSET(kaddr, c_insn->length, *offset);

    /* Check if one of the functions of interest is called */
    for (i = 0; i < nfuncs; ++i)
    {
        if (addr == from_funcs[i])
        {
        /* Change the address of the function to be called */
            BUG_ON(to_funcs[i] == NULL);

            printk( KERN_INFO "[hp_patcher] At 0x%p: "
        "setting a probe to change call address 0x%p to 0x%p\n",
                kaddr,
                from_funcs[i],
                to_funcs[i]
            );
/*
(unsigned int)(*offset),
(unsigned int)CALL_OFFSET_FROM_ADDR(
    kaddr, c_insn->length, to_funcs[i])
*/
            result = kedr_probe_set(kaddr,
                from_funcs[i], to_funcs[i]);
            if (result != 0)
            {
                printk( KERN_INFO "[hp_patcher] "
            "Failed to set a probe at 0x%p, error code is %d\n",
                kaddr, -result);
            }
/*
            *offset = CALL_OFFSET_FROM_ADDR(
                kaddr,
                c_insn->length,
                to_funcs[i]
            );
*/
            break;
        }
    }

    return c_insn->length;
}

/* Process the instructions in [kbeg, kend) area.
 * Each 'call' instruction calling one of the target functions will be
 * changed so as to call the corresponding replacement function instead.
 * The addresses of target and replacement fucntions are given in
 * 'from_funcs' and 'to_funcs', respectively, the number of the elements
 * to process in these arrays being 'nfuncs'.
 * For each i=0..nfuncs-1, from_funcs[i] corresponds to to_funcs[i].
 */
static void
do_process_area(void* kbeg, void* kend, 
    void** from_funcs, void** to_funcs, unsigned int nfuncs)
{
    struct insn c_insn; /* current instruction */
    void* pos = NULL;
    
    BUG_ON(kbeg == NULL);
    BUG_ON(kend == NULL);
    BUG_ON(kend < kbeg);
        
    for (pos = kbeg; pos + 4 < kend; )
    {
        unsigned int len;
        unsigned int k;

/* 'pos + 4 < kend' is based on another "heuristics". 'call' and 'jmp' 
 * instructions we need to instrument are 5 bytes long on x86 and x86-64 
 * machines. So if there are no more than 4 bytes left before the end, they
 * cannot contain the instruction of this kind, we do not need to check 
 * these bytes. 
 * This allows to avoid "decoder stopped past the end of the section"
 * conditions (see do_process_insn()). There, the decoder tries to chew 
 * the trailing 1-2 zero bytes of the section (padding) and gets past 
 * the end of the section.
 * It seems that the length of the instruction that consists of zeroes
 * only is 3 bytes (it is a flavour of 'add'), i.e. shorter than that 
 * kind of 'call' we are instrumenting.
 *
 * [NB] The above check automatically handles 'pos == kend' case.
 */
       
        len = do_process_insn(&c_insn, pos, kend,
            from_funcs, to_funcs, nfuncs);
        if (len == 0)   
        {
            printk( KERN_WARNING "[hp_patcher] "
                "do_process_insn() returned 0\n");
            WARN_ON(1);
            break;
        }

        if (pos + len > kend)
        {
            break;
        }
        
/* If the decoded instruction contains only zero bytes (this is the case,
 * for example, for one flavour of 'add'), skip to the first nonzero byte
 * after it. 
 * This is to avoid problems if there are two or more sections in the area
 * being analysed. Such situation is very unlikely - still have to find 
 * the example. Note that ctors and dtors seem to be placed to the same 
 * '.text' section as the ordinary functions ('.ctors' and '.dtors' sections
 * probably contain just the lists of their addresses or something similar).
 * 
 * As we are not interested in instrumenting 'add' or the like, we can skip 
 * to the next instruction that does not begin with 0 byte. If we are 
 * actually past the last instruction in the section, we get to the next 
 * section or to the end of the area this way which is what we want in this
 * case.
 */
        for (k = 0; k < len; ++k)
        {
            if (*((unsigned char*)pos + k) != 0) 
            {
                break;
            }
        }
        pos += len;
        
        if (k == len) 
        {
            /* all bytes are zero, skip the following 0s */
            while (pos < kend && *(unsigned char*)pos == 0)
            {
                ++pos;
            }
        }
    }
    
    return;
}

/* Changes the address of the function called by the instruction at
 * 'insn_addr'. If the original address differs from 'from_addr', the function
 * issues a warning and does nothing more. Otherwise, the address is changed
 * to 'to_addr'.
 *
 * [NB] Currently (as of kernel 2.6.31), struct kprobe stores a copy of
 * the original call instruction without any changes. For instance, no fixup
 * is applied to the offset of the function to be called.
 * As a result, the address of the original instruction is also necessary to
 * calculate the offset of the replacement function properly.
 * 'insn_addr' is the address of the stored copy of the instruction,
 * 'orig_insn_addr' is the address of the original.
 */
static void
change_call_address(kprobe_opcode_t* insn_addr,
    kprobe_opcode_t* orig_insn_addr,
    void* from_addr, void* to_addr)
{
/* Decoding the instruction again may be not very good performance-wise,
 * but still, it is more simple than inventing a way to pass all the data
 * from do_process_insn() here.
 *
 * Anyway, decoding it here should make no harm.
 */
    struct insn c_insn;
    void* kaddr = NULL;
    void* kaddr_orig = NULL;

    /* ptr to the 32-bit offset argument in the instruction */
    u32* offset = NULL;

    /* address of the function being called */
    void* addr = NULL;

    kaddr = (void*)insn_addr;
    kaddr_orig = (void*)orig_insn_addr;

    /* Decode the instruction and populate 'insn' structure */
    kernel_insn_init(&c_insn, kaddr);

    insn_get_length(&c_insn);
    if (c_insn.length == 0)
    {
        printk(KERN_INFO "[hp_patcher] "
    "Warning: the decoder failed to process the instruction at 0x%p\n",
            kaddr);
        return;
    }

    insn_get_immediate(&c_insn);
    if (c_insn.immediate.nbytes != 4)
    {
        printk(KERN_INFO "[hp_patcher] "
    "Warning: instruction at 0x%p: "
    "size of the 'immediate' field is %u rather than 4 bytes\n",
            kaddr,
            c_insn.immediate.nbytes);
        return;
    }

    offset = (u32*)(kaddr + insn_offset_immediate(&c_insn));

    /* Note that the original address is used to calculate 'addr'
     * while the address of the stored instruction is used to find the
     * position of the 'offset' ('displacement') field.
     */
    addr = CALL_ADDR_FROM_OFFSET(kaddr_orig, c_insn.length, *offset);

    //<>
    /*insn_get_opcode(&c_insn);
    printk(KERN_INFO "[hp_patcher] "
"instruction at 0x%p  (stored copy): opcode is 0x%x (should be 0xe8), "
"offset of offset is %d byte(s), "
"offset value is 0x%x"
"\n",
        kaddr,
        c_insn.opcode.value,
        (int)insn_offset_immediate(&c_insn),
        (unsigned int)(*offset));*/
    //<>

    if (addr != from_addr)
    {
        printk(KERN_INFO "[hp_patcher] "
    "Warning: instruction at 0x%p (stored copy): "
    "the address of the original function is 0x%p rather than 0x%p\n",
            kaddr,
            addr,
            from_addr);
        return;
    }

    /* Note 'kaddr_orig' rather than 'kaddr' here */
    *offset = CALL_OFFSET_FROM_ADDR(
        kaddr_orig,
        c_insn.length,
        to_addr);
    return;
}

static void
kedr_probe_replace_call(struct kedr_probe* kp)
{
    //<>
    /*printk(KERN_INFO "[hp_patcher] "
"instruction at 0x%p: original offset is 0x%x\n",
        kp->kernel_probe.ainsn.insn,
        *(u32*)(kp->kernel_probe.addr + 1));*/
    //<>
    change_call_address(kp->kernel_probe.ainsn.insn,
        kp->kernel_probe.addr,
        kp->orig_func,
        kp->repl_func);
    return;
}

static void
kedr_probe_restore_call(struct kedr_probe* kp)
{
    change_call_address(kp->kernel_probe.ainsn.insn,
        kp->kernel_probe.addr,
        kp->repl_func,
        kp->orig_func);
    return;
}

/* Instruments the target module, i.e. sets kprobes to replace the calls to
 * the target functions with calls to the replacement functions.
 */
static void
instrument_target_module(struct module* mod)
{
    struct payload_entry *payload_entry;

    BUG_ON(mod == NULL);
    BUG_ON(mod->module_core == NULL);

    if (mod->module_init != NULL)
    {
        printk( KERN_INFO "[hp_patcher] Module \"%s\", "
        "processing \"init\" area\n",
            module_name(mod));

        list_for_each_entry(payload_entry, &payload_list, list)
        {
            struct kedr_repl_table *repl_table = &payload_entry->payload->repl_table;
            do_process_area(mod->module_init,
                mod->module_init + mod->init_text_size,
                repl_table->orig_addrs,
                repl_table->repl_addrs,
                repl_table->num_addrs);
        }
    }

    printk( KERN_INFO "[hp_patcher] Module \"%s\", "
        "processing \"core\" area\n",
        module_name(mod));

    list_for_each_entry(payload_entry, &payload_list, list)
    {
        struct kedr_repl_table *repl_table = &payload_entry->payload->repl_table;
        do_process_area(mod->module_core,
            mod->module_core + mod->core_text_size,
            repl_table->orig_addrs,
            repl_table->repl_addrs,
            repl_table->num_addrs);
    }
    return;
}

/* Roll back what was changed in the target module during
 * the instrumentation.
 */
static void
uninstrument_target_module(struct module* mod)
{
    if (target_module == NULL)
    {
        /* Nothing to do */
        return;
    }

    printk(KERN_INFO
    "[hp_patcher] Uninstrumenting module \"%s\".\n",
        module_name(mod));

    kedr_probe_clear_all();
    return;
}

/* ================================================================== */
/* A callback function to catch unloading of the target module.
 * Sets target_module pointer among other things. */
static int
detector_notifier_call(struct notifier_block *nb,
    unsigned long mod_state, void *vmod)
{
    struct module *mod = (struct module *)vmod;

    // We only need to handle the case where the module is going
    // to unload (the target module should have been loaded before
    // this patcher module)

    if (mod_state == MODULE_STATE_GOING &&
        mod == target_module)
    {
        /* if the target module has already been unloaded,
         * target_module is NULL, so (mod == target_module) will
         * be false. */
        struct payload_entry *payload_entry;

        printk(KERN_INFO
        "[hp_patcher] Module '%s' is going to unload.\n",
            module_name(mod));

        uninstrument_target_module(mod);
        //notify payloads about unloading of target module
        list_for_each_entry(payload_entry, &payload_list, list)
        {
            if(payload_entry->payload->target_unload_callback)
                payload_entry->payload->target_unload_callback(target_module);
        }

        target_module = NULL;
    }
    return 0;
}

/* ================================================================ */
// struct for watching for loading/unloading of modules.
struct notifier_block detector_nb = {
    .notifier_call = detector_notifier_call,
    .next = NULL,
    .priority = 3, /*Some number*/
};
/* ================================================================ */

static void
patcher_cleanup_module(void)
{
    /* We need to obtain mutex_lock to be sure the target module
     * will not disappear after we unregister the notifier but before
     * we complete restoring the call addresses in it.
     */
    if (mutex_lock_interruptible(&module_mutex) != 0)
    {
        printk(KERN_INFO
        "[hp_patcher] failed to lock module_mutex\n");
    }
    else
    {
        if (module_notifier_registered)
        {
            unregister_module_notifier(&detector_nb);
        }

        /* if the target module is still loaded, uninstrument it */
        if (target_module != NULL)
        {
            struct payload_entry *payload_entry;

            uninstrument_target_module(target_module);

            // Notify payloads about disconnecting from target module.
            // Disconnecting is not the same as unloading, but nevertheless...
            list_for_each_entry(payload_entry, &payload_list, list)
            {
                if(payload_entry->payload->target_unload_callback)
                    payload_entry->payload->target_unload_callback(target_module);
            }

        }

        mutex_unlock(&module_mutex);
    }
    payload_cleanup();

    printk(KERN_INFO "[hp_patcher] Cleanup complete\n");
    return;
}

/* ================================================================ */
static int __init
patcher_init_module(void)
{
    int result;

    printk(KERN_INFO "[hp_patcher] Initializing\n");

    //<>
/*  printk(KERN_INFO
        "[hp_patcher] Address of __kmalloc is 0x%p\n",
        (void*)&__kmalloc);
    printk(KERN_INFO
        "[hp_patcher] Address of kfree is 0x%p\n",
        (void*)&kfree);
    printk(KERN_INFO
        "[hp_patcher] Address of kmem_cache_alloc is 0x%p\n",
        (void*)&kmem_cache_alloc);
    printk(KERN_INFO
        "[hp_patcher] Address of kmem_cache_free is 0x%p\n",
        (void*)&kmem_cache_free);   */
    //<>

    INIT_LIST_HEAD(&kedr_probes);

    printk(KERN_INFO "[hp_patcher] Before initialize payload");
    result = payload_init();
    printk(KERN_INFO "[hp_patcher] After initialize payload");
    if(result)
    {
        pr_err("[hp_patcher] Cannot register payload.");
        payload_cleanup();
        return result;//not "goto fail" because it call payload_cleanup()
    }

    /* When looking for the target module, module_mutex must be locked */
    result = mutex_lock_interruptible(&module_mutex);
    if (result != 0)
    {
        printk(KERN_INFO
        "[hp_patcher] failed to lock module_mutex\n");
        goto fail;
    }

    /* Check if the target is already loaded */
    target_module = find_module(target_name);
    if (target_module == NULL)
    {
        printk(KERN_INFO
        "[hp_patcher] target module \"%s\" is not currently loaded\n",
            target_name);

        result = -EINVAL;
        goto unlock_and_fail;
    }

    result = register_module_notifier(&detector_nb);
    if (result < 0)
    {
        goto unlock_and_fail;
    }
    module_notifier_registered = 1;

    // Notify payloads about connecting to the target module.
    // Connecting is not the same as loading, but nevertheless...
    {
        struct payload_entry *payload_entry;
        list_for_each_entry(payload_entry, &payload_list, list)
        {
            if(payload_entry->payload->target_load_callback)
                payload_entry->payload->target_load_callback(target_module);
        }
    }
    /* Instrument the target */
    /* TODO: would it be beneficial to do the instrumentation in
     * a  workqueue rather than wait here until it is complete?
     * Not sure.
     * + sync issues=?
     */
    instrument_target_module(target_module);
    mutex_unlock(&module_mutex);

/* [NB] Because of 'module_mutex', the target module cannot trigger unload
 * notification before its instrumentation is complete.
 * Anyway, another mutex still may be provided to protect the list of the
 * kedr_probe instances - just in case.
 */

    return 0; /* success */

unlock_and_fail:
    mutex_unlock(&module_mutex);

fail:
    patcher_cleanup_module();
    return result;
}

static void __exit
patcher_exit_module(void)
{
    patcher_cleanup_module();
    return;
}

module_init(patcher_init_module);
module_exit(patcher_exit_module);
/* ================================================================ */
