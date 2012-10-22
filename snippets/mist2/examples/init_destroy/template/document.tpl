/* Global definitions */

<$global: join "\n"$>

/* init() and destroy() for core functionality */
static int init_core(void)
{
    /* ... */
    return 0;
}
static void destroy_core(void)
{
    /* ... */
}

/* Per-'code' init() and destroy(). */
<$with code$>/* For '<$.name$>'. */
<$if .init$>static int <$.name$>_init(void)
{
<$.init: indent "    " $>
}
<$endif$><$if .destroy$>static void <$.name$>_destroy(void)
{
<$.destroy: indent "    " $>
}
<$endif$><$endwith: join "\n"$>

/* 
 * Declate init() and destroy() functions, which performs all
 * initialization and finalization correspondingly.
 */

int init(void)
{
    int result;
    
    result = init_core();
    if(result) goto err_core;

    /* Call per-'code' init() */
<$with code$><$if .init$>    result = <$.name$>_init();
    if(result) goto <$.name$>_err;

<$endif$><$endwith: join$>/* Need for supress "Unused label" warning */
    if(0) goto err_fake;

    return 0;
    /* Need for supress "Unreachable code" warning */
err_fake:
/* Call per-'code' destroy() on fail in reverse order.*/
<$with code$><$if .destroy$>    <$.name$>_destroy();
<$endif$><$if .init$><$.name$>_err:
<$endif$><$endwith: rjoin$>
/* Call core destroy() on fail */
    destroy_core();
err_core:
    return result;
}

void destroy()
{
<$with code$><$if .destroy$>    <$.name$>_destroy();
<$endif$><$endwith: rjoin$>
    destroy_core();
}