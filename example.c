#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "linenoise.h"


static void *thread_main(void *arg)
{
    printf("thread started\n");

    for(;;) {
        sleep(2);
        printf("thread spamming every two seconds...\n");
        printf("but twice!\n");
    }

    return 0;
}

static void start_thread() {
    pthread_t task_to_be_cancelled_hdl;

    pthread_attr_t attr;
    int ret = pthread_attr_init(&attr);
    if (ret != 0) {
        printf("pthread_attr_init() failed: %s\n", strerror(errno));
    }

    if (ret != 0) {
        printf("pipe() failed: %s\n", strerror(errno));
    }

    ret = pthread_create(&task_to_be_cancelled_hdl, &attr, &thread_main, NULL);
    if (ret != 0) {
        printf("pthread_create() failed: %s\n", strerror(errno));
    }
    printf("pthread created\n");

    ret = pthread_attr_destroy(&attr);
    if (ret != 0) {
        printf("pthread_attr_destroy() failed: %s\n", strerror(errno));
    }
}

void completion(const char *buf, linenoiseCompletions lc) {
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc,"hello");
        linenoiseAddCompletion(lc,"hello there");
    } else if (buf[0] == '\0') {
        linenoiseAddCompletion(lc,"allllll commands here");
    }
}

char *hints(const char *buf, int *color, int *bold) {
    if (!strcasecmp(buf,"hello")) {
        *color = 35;
        *bold = 0;
        return " World";
    }
    return NULL;
}

int main(int argc, char **argv) {

    // this thread will spam stdout, which triggers asnyc output handling
    start_thread();

    int orig_stdout = dup(STDOUT_FILENO);
    // create new pipe and let it be the new stdout, only for background outputs from other threads,
    // which requires special async output logic
    int piped_stdout[2];
    pipe(piped_stdout);
    dup2(piped_stdout[1], STDOUT_FILENO);
    fcntl(piped_stdout[0], F_SETFL, fcntl(piped_stdout[0], F_GETFL, 0) | O_NONBLOCK);

    // manually change the associated stdout FILE pointer
    // so that it does not buffer in block mode, which
    // results in no immediate output when the stdout of
    // the process is a pipe instead of a tty.
    // because printf() uses fprintf() instead of dprintf() in libc
    setvbuf(stdout, NULL, _IOLBF, 4096);
    // and do an immediate flush (in case something is already written by thread)
    fflush(stdout);

    linenoiseState ls;
    char buf[1024];
    struct linenoiseConfig cfg = {
            .fd_in = 0,
            .fd_out = orig_stdout,
            .fd_tty = 0,
            .buf = buf,
            .buf_len = sizeof(buf),
    };
    linenoiseCreateState(&ls, &cfg);
    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(ls, completion);
    linenoiseSetHintsCallback(ls, hints);

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad(ls, "history.txt"); /* Load the history at startup */

    while(1) {
        char *line;
        linenoiseEditStart(ls, "hello> ");
        while(1) {
            struct pollfd fds[] = {
                    {
                            .fd = 0,
                            .events = POLLIN,
                    },
                    {
                            .fd = piped_stdout[0],
                            .events = POLLIN,
                    }
            };
            int retval = poll(fds, sizeof(fds) / sizeof(fds[0]), -1);
            if (retval == -1) {
                if (errno == EINTR) {
                    break;
                }
                perror("poll()");
                exit(1);
            } else if (retval == 0) {
                // Timeout occurred
                static int counter = 0;
                linenoiseHide(ls);
                printf("Async output %d.\n", counter++);
                linenoiseShow(ls);
            } else {
                if (fds[1].revents & POLLIN) {
                    linenoiseHide(ls);
                    // always copy amount of bytes blindly
                    for(;;) {
                        char tmpbuf[256];
                        ssize_t readlen = read(piped_stdout[0], tmpbuf, sizeof(tmpbuf));
                        if (readlen == 0 || (readlen == -1 && errno == EAGAIN)) {
                            break;
                        } else if (readlen == -1) {
                            dprintf(2, "ERROR: read() failed: %s\n", strerror(errno));
                        }
                        ssize_t writelen = write(orig_stdout, tmpbuf, readlen);
                        if (readlen != writelen) {
                            dprintf(2, "ERROR: write() returned %zd, errno: %s\n", writelen, strerror(errno));
                        }
                        fsync(orig_stdout);
                    }
                    linenoiseShow(ls);
                }
                if (fds[0].revents & POLLIN) {
                    line = linenoiseEditFeed(ls);
                    if (line != linenoiseEditMore) {
                        break;
                    }
                }
            }
        }

        linenoiseEditStop(ls);
        if (line == NULL) break; /* Ctrl+D/C. */

        /* Do something with the string. */
        if (line[0] != '\0') {
            linenoiseHistoryAdd(ls, line); /* Add to the history. */
            linenoiseHistorySave(ls, "history.txt"); /* Save the history on disk. */

            printf("User Inputs: %s\n", line);
        }
        free(line);
    }

    linenoiseDeleteState(ls);
    dup2(orig_stdout, STDOUT_FILENO);
    close(piped_stdout[1]); // [0] is closed by dup2()
    close(orig_stdout);

    return 0;
}
