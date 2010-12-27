#define _GNU_SOURCE /*need for WEXITSTATUS and WEXITED macros*/

#include <stdio.h> /*printf*/
#include <unistd.h> /*open*/
#include <stdlib.h> /*exit*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h> /*signals processing*/
#include <sys/signalfd.h> /*signalfd()*/

#include <poll.h> //for poll()
#include <errno.h>

#include <string.h> /*memset, strdup*/

#define TRACEFILE "/sys/kernel/debug/rb_test/trace"
/*
 * Column number in trace line, which represent type of the line.
 * 
 * Columns should be delimited by '\t'.
 */

#define TYPE_COLUMN_NUMBER 2

/*
 * Sizes of the buffer for reading line from the trace file.
 * 
 * At least, READ_BUFFER_SIZE_MIN will be allocated for buffer.
 * If it is insufficient for reading line, buffer will be reallocated
 * with increased size.
 * 
 * If buffer size is less or equal than READ_BUFFER_SIZE_MAX bytes,
 * it may be reused for following reads without freeing (like cache).
 * 
 * If buffer size is greater than READ_BUFFER_SIZE_MAX bytes,
 * then it will be freed after processing line.
 */
#define READ_BUFFER_SIZE_MIN 10
#define READ_BUFFER_SIZE_MAX 20

/*
 * Callback function for filter lines in trace.
 * 
 * Return 1, if line should be processed,
 * return 0, if line should be skipped.
 * 
 * Set 'should_stop' to not 0 for stop trace processing.
 */
typedef int (*global_line_filter)(const char* str, size_t size,
    int *should_stop, void* data);

/*
 * Return type of the line.
 * 
 * On success return 0 and set 'type' to the string, which represent
 * type of the line.
 * 'type_size' is set to the length of this string.
 */
static int get_line_type(const char* str, size_t size,
    const char** type, size_t *type_size);

/*
 * Process only lines, which have needed type.
 * 
 * 'types' is NULL-terminated array of lines' types.
 */
static int
filter_line_type(const char* type, size_t type_size,
    const char** types);

/*
 * Accept all lines until 'Read'(inclusive).
 */
static int
filter_line_type_until_read(const char* str, size_t size,
    int* should_stop, void* unused);

struct child_process
{
    /*
     * File descriptor for write into process'es STDIN.
     * 
     * -1 if process has closed its STDIN.
     */
    int fd_write;
    /*
     * Pid of the process.
     */
    pid_t pid;
    /*
     * Types(NULL-terminated array of strings) of trace lines,
     * which should be passed to the STDIN.
     * NULL, if all trace should be passed.
     */
    char** types;
    /*
     * List organization.
     */
    struct child_process* next;
    struct child_process* prev;
};

/*
 * Create child process, redirect its input,
 * and run 'command_line' in it as in the shell.
 * 
 * On error return NULL.
 * 
 * 'types' represent types of trace lines, which accepted by this process.
 */

static struct child_process* create_child_process(const char* command_line,
    const char* types[]);

/*
 * Destroy child process descriptor.
 * 
 * If need, close writing end of pipe with this process.
 * 
 * NOTE: waiting is not performed.
 */
static void child_process_free(struct child_process* child);

/*
 * Helper for close writing end of pipe with process.
 */
static void child_process_stop_write(struct child_process* child);

/*
 * Helper for testing, whether writing end of pipe with process is open.
 */
static int child_process_is_writeable(struct child_process* child);

/*
 * List of children.
 * 
 * Unordered(in that sence, that order has no sence).
 */
struct children
{
    struct child_process* first_child;
    struct child_process* last_child;
    /*
     * Global filter of trace lines.
     * 
     * NULL if no used.
     */
    global_line_filter filter;
    
    void* filter_data;
};

/*
 * Initialize list of children as empty.
 */
static int
children_init(struct children* children,
    global_line_filter filter, void* filter_data);

// Add child process descriptor to the tail of the list.
static void children_add_child(struct children* children,
    struct child_process* child);

// Remove child process descriptor from the list.
static void
children_del_child(struct children* children, struct child_process* child);

// Macro for iterate child processes in the list
#define children_for_each_child(children, child) \
    for((child) = (children)->first_child; (child) != NULL; (child) = (child)->next)

/*
 * Free and remove all child descriptors from children.
 * 
 * Waiting is not performed.
 */
static void children_free(struct children* children);

/*
 * Free and remove all child descriptors from children.
 * 
 * Also wait, while every child is finished.
 * 
 * Return 0, if every child exit with 0 status.
 * Otherwise return 1.
 */
static int children_free_wait(struct children* children);


/*
 * Read from file up to newline symbol.
 * 
 * It is assumed, that newline may appear only at the end of the
 * reading block.
 * 
 * When line is read, its content is passed to the 'process_line' function.
 * 
 * On error, function return -1.
 * If trace is empty, return -2.
 * Otherwise, result of 'process_line' function is returned.
 */

static int
read_line(int fd,
    int (*process_line)(const char* str, size_t len, void* data),
    void* data);

/*
 * Calback for previous function.
 * 
 * Return 0, if line was processed.
 * Return 1, if reading trace should be stopped.
 * Return -1 on error(but with same meaning, as result 1).
 */
static int
children_process_line(const char* str, size_t len, void* data);

/*
 * Auxiliary function for change flags for file descriptor.
 * 
 * Return 0 on success.
 */
static int change_fd_flags(int fd, long mask, long value);

/*
 * Auxiliary function for change status flags for file.
 * 
 * Return 0 on success.
 */

static int change_fl_flags(int fd, long mask, long value);

/*
 * Emulate next behaviour on SIGINT signal:
 * 
 * The first signal arrival emulates EOF of trace file, but do not
 * interrupt program.
 * 
 * Others signals work as usual.
 */

/*
 * Start block of code with behaviour explained above.
 * 
 * Return 0 on success.
 */

static int prepare_processing_sigint(void);

/*
 * End block of code with behaviour explained above.
 */

static void restore_processing_sigint(void);

/*
 * Test, whether signal SIGINT is arrived.
 * 
 * If so, return 1 and restore normal work of signal.
 * Otherwise return 0.
 * 
 * Note: in case function return 1, restore_processing_sigint() call
 * is needed anywhere.
 */

static int test_sigint(void);

/*
 * Return file descriptor, which may be used for watching signal using
 * select/poll mechanizms.
 * 
 * After test_sigint() return 1, this function returns -1.
 * 
 * Note: this function may not change signal disposition.
 */

static int get_signal_fd(void);

/*
 * Auxiliary function for polling the trace, 
 * in the same time watching for signal(if needed).
 * 
 * Return -1 on error.
 * Return 0 otherwise.
 * 
 * Restore signal disposition, if needed.
 */
static int poll_read(int fd);

/*
 * Auxiliary function for polling the pipe with other process, 
 * in the same time watching for signal(if needed).
 * 
 * Return -1 on error.
 * Return 0 otherwise.
 * 
 * Restore signal disposition, if needed.
 */
static int poll_write(int fd);


/*
 * Main.
 */

int main(int argc, char* const argv[], char* const envp[])
{
    int fd_trace;
    int result = 0;
    
    struct children children;
    children_init(&children,
        filter_line_type_until_read, NULL);
    
    if(argc < 2)
    {
        printf("Usage: %s <program(s)> ...\n", argv[0]);
        return 1;
    }

    fd_trace = open(TRACEFILE, O_RDONLY);
    if(fd_trace == -1)
    {
        perror("Cannot open trace file for read:");
        return -1;
    }
    if(change_fd_flags(fd_trace, FD_CLOEXEC, FD_CLOEXEC) == -1)
    {
        printf("Cannot set FD_CLOEXEC flag for opened trace file.\n");
        close(fd_trace);
        return -1;
    }
    
    if(change_fl_flags(fd_trace, O_NONBLOCK, O_NONBLOCK) == -1)
    {
        printf("Cannot set O_NONBLOCK flag for opened trace file.\n");
        close(fd_trace);
        return -1;
    }
    int i;
    for(i = 1; i < argc; i++)
    {
        // Create another process which piped with current
        struct child_process* child = create_child_process(argv[i], NULL);
        if(child == NULL)
        {
            printf("Cannot create child process \"%s\".\n", argv[i]);
            children_free(&children);
            close(fd_trace);
            return -1;
        }
        children_add_child(&children, child);
    }
    if(prepare_processing_sigint())
    {
        printf("Cannot prepare SIGINT processing.\n");
        children_free(&children);
        close(fd_trace);
        return -1;
    }
    
    //while(poll_read(fd_trace) == 0)
    do
    {
        int result;
        //if(test_sigint()) break;
        //nonblocking read
        while(!test_sigint()
            && ((result = read_line(fd_trace, children_process_line,
                &children)) == -2))
        {
            if(poll_read(fd_trace) == -1)
            {
                result = -1;
                break;
            }
        }
        if(test_sigint()) break;//EOF
        if(result == -1)
        {
            printf("Error occures while processing trace. Stop.\n");
            break;
        }
        else if(result == 1)
        {
            break;
        }
        //sleep(4);
        //printf("Next iteration of read.\n");
    }while(1);
    restore_processing_sigint();
    close(fd_trace);
    
    if(result == -1)
    {
        children_free(&children);
        return -1;
    }
    else
        return children_free_wait(&children);
}

/*
 * Implementation of auxiliary functions.
 */

int change_fd_flags(int fd, long mask, long value)
{
    long flags = fcntl(fd, F_GETFD);
    if(flags == -1)
    {
        perror("Cannot get flags for file descriptor:");
        return -1;
    }
    flags = (flags & ~mask) | (value & mask);
    
    if(fcntl(fd, F_SETFD, flags) == -1)
    {
        perror("Cannot set flags to file descriptor:");
        return -1;
    }
    return 0;
}

int change_fl_flags(int fd, long mask, long value)
{
    long flags = fcntl(fd, F_GETFL);
    if(flags == -1)
    {
        perror("Cannot get status flags for file:");
        return -1;
    }
    flags = (flags & ~mask) | (value & mask);
    
    if(fcntl(fd, F_SETFL, flags) == -1)
    {
        perror("Cannot set status flags to file:");
        return -1;
    }
    return 0;
}

static int poll_IO(int fd, short event)
{
    int result;
    struct pollfd pollfds[2] = {
        {
            .fd = -1,//will be fd
            .events = 0, //will be event
        },
        {
            .fd = -1,//will be fd_signals
            .events = POLLIN
        }
    };
    pollfds[0].fd = fd;
    pollfds[0].events = event;
    pollfds[1].fd = get_signal_fd();
    
    // Repeate poll() while interrupted with non-interesting signals
    do
    {
        result = poll(pollfds, (pollfds[1].fd != -1) ? 2 : 1, -1/*forever*/);
        //printf("poll exited.\n");
    }while((result == -1) && (errno == EINTR));

    if(result == -1)
    {
        perror("poll() fail:");
        return -1;
    }
    else if(result == 0)
    {
        printf("Unexpected return value 0 from poll() without timeout.\n");
        return -1;
    }
    return 0;
    /*//result > 0

    if(pollfds[1].revents)
    {
        if(pollfds[1].revents | POLLIN)
        {
            //SIGINT has arrived
            printf("Program was interrupted.\n");
            return 1;
        }
        else
        {
            //SIGINT has arrived, but with strange mask
            printf("Program was interrupted in unknown fashion.\n");
            return 1;
        }
    }
    if(pollfds[0].revents == 0)
    {
        printf("poll() returns %d, but all revents is empty.\n", result);
        return -1;
    }

    if(pollfds[0].revents | POLLIN)
    {
        //printf("Trace became non-empty.\n");
        return 0;//trace is non-empty
    }
    else
    {
        printf("Value of 'revents' for trace: %d.\n", (int)pollfds[0].revents);
        return 1;
    }*/
}

int poll_read(int fd) {return poll_IO(fd, POLLIN);}
int poll_write(int fd) {return poll_IO(fd, POLLOUT);}



struct child_process*
create_child_process(const char* command_line,
    const char* types[])
{
    int n_types;
    struct child_process* child;
    pid_t pid;
    int fd_pipe[2];
    //Allocate child struture
    child = malloc(sizeof(*child));
    if(child == NULL)
    {
        printf("Cannot allocate child struture.\n");
        return NULL;
    }
    /*
     * Set up all fields for correct deleting
     * in case of error in initialization.
     */
    child->fd_write = -1;
    child->types = NULL;
    
    if(types != NULL)
    {
        //count types
        for(n_types = 0; types[n_types] != NULL; n_types++);
        child->types = malloc(sizeof(*child->types) * n_types);
        if(child->types == NULL)
        {
            printf("Cannot allocate array of types strings for child process.\n");
            //free(child);
            child_process_free(child);
            return NULL;
        }

        memset(child->types, 0, sizeof(*child->types) * n_types);

        for(n_types = 0; types[n_types] != NULL; n_types++)
        {
            child->types[n_types] = strdup(types[n_types]);
            if(child->types[n_types] == NULL)
            {
                printf("Cannot allocate type string for child process.\n");
                child_process_free(child);
                return NULL;
            }
        }
    }
    // Create another process which piped with current
    if(pipe(fd_pipe) == -1)
    {
        perror("Cannot create pipe:");
        child_process_free(child);
        return NULL;
    }
    
    if(change_fd_flags(fd_pipe[1], FD_CLOEXEC, FD_CLOEXEC) == -1)
    {
        printf("Cannot set FD_CLOEXEC flag for write end of pipe.\n");
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        child_process_free(child);
        return NULL;
    }
    
    if(change_fl_flags(fd_pipe[1], O_NONBLOCK, O_NONBLOCK) == -1)
    {
        printf("Cannot set O_NONBLOCK flag for write end of pipe.\n");
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        child_process_free(child);
        return NULL;
    }

    // do not terminate when child process close its stdin
    signal(SIGPIPE, SIG_IGN);

    pid = fork();
    if(pid == -1)
    {
        perror("Error when performs fork:");
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        child_process_free(child);
        return NULL;
    }
    if(pid == 0)
    {
        // Child
        close(fd_pipe[1]);
        if(dup2(fd_pipe[0], 0) == -1)
        {
            perror("Cannot redirect input of the child process:");
            close(fd_pipe[0]);
            exit(-1);
        }
        close(fd_pipe[0]);
        
        // For some reason system() do not protect child process against
        // SIGINT for process group(!).
        signal(SIGINT, SIG_IGN);
        exit(system(command_line));

    }
    //Parent
    close(fd_pipe[0]);
    //
    child->pid = pid;
    child->fd_write = fd_pipe[1];
    
    return child;
}

void child_process_free(struct child_process* child)
{
    int n_types;
    if(child == NULL) return;
    child_process_stop_write(child);
    if(child->types != NULL)
    {
        for(n_types = 0; child->types[n_types] != NULL; n_types++)
            free(child->types[n_types]);
        free(child->types);
    }
    free(child);
}

void child_process_stop_write(struct child_process* child)
{
    if(child->fd_write != -1)
    {
        close(child->fd_write);
        child->fd_write = -1;
    }
}

int child_process_is_writeable(struct child_process* child)
{
    return child->fd_write != -1;
}

int children_init(struct children* children,
    global_line_filter filter, void* filter_data)
{
    children->first_child = NULL;
    children->last_child = NULL;
    
    children->filter = filter;
    children->filter_data = filter_data;
}

void children_add_child(struct children* children,
    struct child_process* child)
{
    struct child_process* last_child = children->last_child;
    if(last_child != NULL)
    {
        last_child->next = child;
        child->prev = last_child;
    }
    else
    {
        children->first_child = child;
        child->prev = NULL;
    }
    child->next = NULL;
    children->last_child = child;
}

void
children_del_child(struct children* children, struct child_process* child)
{
    if(child->next)
        child->next->prev = child->prev;
    else
        children->last_child = child->prev;
    if(child->prev)
        child->prev->next = child->next;
    else
        children->first_child = child->next;

    child->next = NULL;
    child->prev = NULL;
}

void children_free(struct children* children)
{
    struct child_process* child;
    while((child = children->first_child) != NULL)
    {
        children_del_child(children, child);
        child_process_free(child);
    }
}

int children_free_wait(struct children* children)
{
    int result = 0;
    struct child_process* child;
    
    children_for_each_child(children, child)
    {
        child_process_stop_write(child);
    }
    
    while((child = children->first_child) != NULL)
    {
        int status;
        pid_t pid = child->pid;
        children_del_child(children, child);
        
        //printf("Wait for child %d...\n", (int)pid);
        waitpid(pid, &status, 0);
        
        if(!WIFEXITED(status) || WEXITSTATUS(status))
            result = 1;
        //printf("Child %d has finished.\n", (int)pid);
        child_process_free(child);
    }
    return result;
}


static int fd_signal = -1;
static int is_signal_blocked = 0;

int get_signal_fd(void) {return is_signal_blocked ? fd_signal : -1;}

int prepare_processing_sigint(void)
{
    sigset_t signal_mask;
    
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGINT);
    
    fd_signal = signalfd(-1, &signal_mask, 0);
    if(fd_signal == -1)
    {
        perror("Cannot create file descriptor for signal SIGINT:");
        return -1;
    }
    if(change_fd_flags(fd_signal, FD_CLOEXEC, FD_CLOEXEC) == -1)
    {
        printf("Cannot set FD_CLOEXEC flag for signal file descriptor.\n");
        close(fd_signal);
        return -1;
    }
    if(change_fl_flags(fd_signal, O_NONBLOCK, O_NONBLOCK) == -1)
    {
        printf("Cannot set O_NONBLOCK flag for signal file descriptor.\n");
        close(fd_signal);
        return -1;
    }

    //Otherwise signal will not get into fd_signal...
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    is_signal_blocked = 1;
    return 0;
}

static void restore_processing_sigint(void)
{
    if(is_signal_blocked)
    {
        sigset_t signal_mask;
        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, SIGINT);
        
        sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
        is_signal_blocked = 0;
    }
    close(fd_signal);
}

int test_sigint(void)
{
    struct signalfd_siginfo siginfo;

    /*
     *  Consume signal in the signal file descriptor.
     *  Otherwise, it will 'alive', when be unblocked.
     */

    if(is_signal_blocked
        && read(fd_signal, &siginfo, sizeof(siginfo)) != -1)
    {
        sigset_t signal_mask;
        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, SIGINT);
        sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
        is_signal_blocked = 0;
    }
    return !is_signal_blocked;
}

char* read_buffer = NULL;
int read_buffer_size = 0;

/*
 * Same as read(), but repeat read until at least 1 byte will read
 * or real error occures.
 */
static ssize_t
read_until_error(int fd, void* buffer, size_t size)
{
    ssize_t result;
    do
    {
        result = read(fd, buffer, size);
    }
    while((result == -1)
        && ((errno == EINTR) || (errno == ERESTART)));
    return result;
}

int
read_line(int fd,
    int (*process_line)(const char* str, size_t len, void* data),
    void* data)
{
    int result;
    int size;//number of currently readed characters in the buffer.
    if(read_buffer == NULL)
    {
        read_buffer = malloc(READ_BUFFER_SIZE_MIN);
        if(read_buffer == NULL)
        {
            printf("Cannot allocate buffer for read from file.\n");
            return -1;
        }
        read_buffer_size = READ_BUFFER_SIZE_MIN;
    }
    size = read_until_error(fd, read_buffer, read_buffer_size);
    if(size == -1)
    {
        if(errno == EAGAIN) return -2;
        perror("Error occures when reading from file");
        return -1;
    }
    if(size == 0) return -2;
    
    while(read_buffer[size - 1] != '\n')
    {
        //printf("Current buffer content: \"%.*s\".\n", (int) size, read_buffer);
        size_t size_tmp;
        size_t additional_size = read_buffer_size;
        read_buffer = realloc(read_buffer,
            read_buffer_size + additional_size);
        if(read_buffer == NULL)
        {
            printf("Cannot increase buffer for read from file.\n");
            return -1;
        }
        read_buffer_size = read_buffer_size + additional_size;
        size_tmp = read_until_error(fd, read_buffer + size,
            additional_size);
        if(size_tmp == -1)
        {
            if(errno == EAGAIN)
            {
                printf("Line was partially read from file,"
                    "and next read should block.\n");
                printf("Perhaps, there is another reader from file,"
                    "or file access is not aligned on lines.\n");
                printf("Please, fix this.\n");
                return -1;
            }
            perror("Error occures when reading from file");
            return -1;
        }
        if(size_tmp == 0)
        {
            printf("Line was partially read from file,"
                "and next read encounter EOF.\n");
            printf("Perhaps, there is another reader from file,"
                "or file access is not aligned on lines.\n");
            printf("Please, fix this.\n");
            return -1;
        }
        size += size_tmp;
    }
    result = process_line(read_buffer, size, data);
    if(read_buffer_size >= READ_BUFFER_SIZE_MAX)
    {
        free(read_buffer);
        read_buffer = NULL;
        read_buffer_size = 0;
    }
    return result;
}

/*
 * Same as write(), but repeat read until all bytes will be written
 * or real error occures.
 */
static ssize_t
write_until_error(int fd, const void* buffer, size_t size)
{
    size_t bytes_written = 0;
    do
    {
        ssize_t result = write(fd, buffer, size);
        if(result == -1)
        {
            if(errno == EINTR) continue;
            return -1;
        }
        bytes_written += result;
        buffer += result;
        size -= result;
    }
    while(size != 0);
    return bytes_written;
}


int
children_process_line(const char* str, size_t len, void* data)
{
    int result = 0;
    int should_stop = 0;
    int trace_used = 0;//count of trace users
    //Real type of 'data' parameter.
    struct children *children = (struct children *)data;
    struct child_process *child;

    const char* type;
    size_t type_size;
    
    if(children->filter != NULL)
    {
        if(children->filter(str, len, &should_stop,
            children->filter_data) == 0)
        {
            goto out;
        }
        
    }
#define UNKNOWN_TYPE "unknown type"    
    if(get_line_type(str, len, &type, &type_size))
    {
        type = UNKNOWN_TYPE;
        type_size = strlen(type);
    }
#undef UNKNOWN_TYPE

    children_for_each_child(children, child)
    {
        int result;
        if(!child_process_is_writeable(child)) continue;
        if(child->types != NULL)
        {
            if(filter_line_type(type, type_size,
                (const char**)child->types) == 0)
            {
                trace_used++;
                continue;
            }
        }
        while((result
            = write_until_error(child->fd_write, str, len) == -1))
        {
            if(errno == EAGAIN)
            {
                //printf("Polling write...\n");
                test_sigint();
                if(poll_write(child->fd_write) == -1)
                {
                    printf("Error while polling pipe with child process.\n");
                    printf("Writing to this process will stop.\n");
                }
                else
                {
                    //printf("Success.\n");
                    continue;
                }
            }
            if(errno == EPIPE)
            {
                printf("Child process has closed its STDIN.\n");
            }
            else
            {
                perror("Error occure while writting to the pipe with child process");
                printf("Writing to this process will stop.\n");
            }
            child_process_stop_write(child);
            break;
        }
        if(result != -1) trace_used++;
    }
    if(trace_used == 0)
    {
        printf("Trace is not used at all. Stop.\n");
        return 1;
    }
out:
    return should_stop ? 1 : 0;
}

int get_line_type(const char* str, size_t size,
    const char** type, size_t *type_size)
{
    int i, i1;
    int column_number = 0;
    for(i = 0; (i < size) && (column_number < TYPE_COLUMN_NUMBER); i++)
    {
        if(str[i] == '\t') column_number++;
    }
    if(i == size)
    {
        printf("Line '%.*s' has no type information.\n",
            size, str);
        return 1;
    }
    for(i1 = 0; i + i1 < size; i1++)
    {
        if((str[i+i1] == '\t') || (str[i+i1] == '\n')) break;
    }
    *type = str + i;
    *type_size = i1;
    //printf("Type is '%.*s'.\n", *type_size, *type);
    return 0;
}

int
filter_line_type(const char* type, size_t type_size,
    const char** types)
{
    int i;
    
    for(i = 0; types[i] != NULL; i++)
    {
        if((strncmp(type, types[i], type_size) == 0)
            && (types[i][type_size] == '\0')) return 0;
    }
    return 1;
}

int
filter_line_type_until_read(const char* str, size_t size,
    int* should_stop, void* unused)
{
    static char end_marker[] = "Write large";
    const char* type;
    size_t type_len;

    if(get_line_type(str, size, &type, &type_len)) return 0;
    
    if((strncmp(type, end_marker, type_len) == 0)
            && (end_marker[type_len] == '\0')) 
        *should_stop = 1;
    //printf("'should_stop' is %d.\n", *should_stop);
    return 1;
}