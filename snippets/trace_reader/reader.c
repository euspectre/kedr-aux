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

#define TRACEFILE "/sys/kernel/debug/rb_test/trace"

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
 * Auxiliary function for polling the trace.
 * 
 * Return 0 on success.
 */
static int poll_trace(int fd_trace, int fd_signals);


/*
 * Create child process, redirect its input,
 * and run 'command_line' in it as in the shell.
 * 
 * Return file descriptor for writting into child process input.
 * 
 * Set 'process_pid' to the pit of the process created(for wait).
 * 
 * On error return -1.
 */

static int create_process_piped(const char* command_line,
    pid_t *process_pid);

/*
 * Set next behaviour on SIGINT signal:
 *
 * First signal arrival is ignored, but marked in signal file descriptor.
 * 
 * Second signal arrival terminate program.
 * 
 * Return file descriptor, which may be used for polling SIGINT.
 */

static int prepare_processing_sigint(void);

/*
 * Main.
 */

int main(int argc, char* const argv[], char* const envp[])
{
    int status;
    char buffer[80];
    //debug
    //char buffer[10];
    ssize_t size;

    pid_t child_pid;

    int fd_trace;
    int fd_redirect;
    int fd_signal;
    
    if(argc != 2)
    {
        printf("Usage: %s <program>\n", argv[0]);
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

    // Create another process which piped with current
    fd_redirect = create_process_piped(argv[1], &child_pid);
    if(fd_redirect == -1)
    {
        printf("Cannot create process.\n");
        close(fd_trace);
        return -1;
    }

    fd_signal = prepare_processing_sigint();
    if(fd_signal == -1)
    {
        printf("Cannot prepare SIGINT processing.\n");
        close(fd_redirect);
        close(fd_trace);
        return -1;
    }
    
    while(poll_trace(fd_trace, fd_signal) == 0)
    {
        ssize_t size = read(fd_trace, buffer, sizeof(buffer));
        if(size <= 0) continue;/*trace is empty again*/
        
        if(write(fd_redirect, buffer, size) == -1)
        {
            printf("Child process closes stdin.\n");
            break;
        }
    }
    close(fd_signal);
    close(fd_redirect);
    close(fd_trace);
    
    if(waitpid(child_pid, &status, 0) == -1)
    {
        perror("Error occures while waiting child:");
        return -1;
    }
    
    return status;
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
        perror("Cannot set flags file descriptor:");
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
        perror("Cannot set status flags for file file:");
        return -1;
    }
    return 0;
}

int poll_trace(int fd_trace, int fd_signals)
{
    int result;
    struct pollfd pollfds[2] = {
        {
            .fd = -1,//will be fd_trace
            .events = POLLIN
        },
        {
            .fd = -1,//will be fd_signals
            .events = POLLIN
        }
    };
    pollfds[0].fd = fd_trace;
    pollfds[1].fd = fd_signals;
    
    // Repeate poll() while interrupted with non-interesting signals
    do
    {
        result = poll(pollfds, 2, -1/*forever*/);
    }while((result == -1) && (errno == -EAGAIN));

    if((result != -1)
        && (pollfds[0].revents | POLLIN)
        ) return 0;//trace is non-empty

    if((pollfds[1].revents | POLLIN)
        || ((result == -1) && (errno == -EINTR))
        )
    {
        //SIGINT has arrived
        printf("Program was stopped.\n");
        return 1;
    }


    if(result == -1)
    {
        perror("poll() fail:");
        return -1;
    }
    
    return -1;// some error in fds
}

int create_process_piped(const char* command_line,
    pid_t *process_pid)
{
    pid_t pid;
    int fd_pipe[2];
    // Create another process with piped with current
    if(pipe(fd_pipe) == -1)
    {
        perror("Cannot create pipe:");
        return -1;
    }
    
    // do not terminate when child process close its stdin
    signal(SIGPIPE, SIG_IGN);

    pid = fork();
    if(pid == -1)
    {
        perror("Error when performs fork:");
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        return -1;
    }
    if(pid == 0)
    {
        // Child
        close(fd_pipe[1]);
        if(dup2(fd_pipe[0], 0) == -1)
        {
            perror("Cannot redirect input of the child process:");
            close(fd_pipe[0]);
            return -1;
        }
        close(fd_pipe[0]);
        
        /*if(execl("/bin/sh", 
            "/bin/sh",
            "-c",
            command_line,
            NULL) == -1)
        {
            perror("Failed to execute execl():");
            return -1;
        }*/
        // For some reason system() do not protect child process against
        // SIGINT for process group(!).
        signal(SIGINT, SIG_IGN);
        exit(system(command_line));
        /*if(execl("./child",
            "./child",
            command_line,
            NULL) == -1)
        {
            perror("Failed to execute execl():");
            return -1;
        }*/
    }
    //Parent
    close(fd_pipe[0]);
    //
    *process_pid = pid;
    return fd_pipe[1];
}

static void on_sigint(int sig)
{
}
static struct sigaction action_sigint =
{
    .sa_handler = on_sigint,
    .sa_mask = 0,
    .sa_flags = SA_RESETHAND //reset handler after first signal arrived
};


int prepare_processing_sigint(void)
{
    int fd_signal;
    sigset_t signal_mask;
    
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGINT);
    
    fd_signal = signalfd(-1, &signal_mask, 0);
    if(fd_signal == -1)
    {
        perror("Cannot create file descriptor for signal SIGINT:");
        return -1;
    }
    
    //Change reaction on SIGINT
    if(sigaction(SIGINT, &action_sigint, NULL) == -1)
    {
        perror("Cannot set action for signal SIGINT:");
        close(fd_signal);
        return -1;
    }
}

