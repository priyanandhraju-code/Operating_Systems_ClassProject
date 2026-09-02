#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "executor.h"
#include "builtin.h"


/*
 * ------------------------------------------------
 * SETUP REDIRECTION
 * ------------------------------------------------
 *
 * Handles:
 *
 *      < input.txt
 *      > output.txt
 *      >> output.txt
 *
 * Returns:
 *
 *       0 -> success
 *      -1 -> error
 */
static int setup_redirection(const command_t *cmd)
{
    int fd;


    /*
     * -----------------------------------------------
     * INPUT REDIRECTION
     * -----------------------------------------------
     */
    if (cmd->input[0] != '\0')
    {
        fd = open(cmd->input, O_RDONLY);

        if (fd < 0)
        {
            perror("open input");
            return -1;
        }


        if (dup2(fd, STDIN_FILENO) < 0)
        {
            perror("dup2 input");
            close(fd);
            return -1;
        }


        close(fd);
    }


    /*
     * -----------------------------------------------
     * OUTPUT REDIRECTION
     * -----------------------------------------------
     */
    if (cmd->output[0] != '\0')
    {
        int flags = O_WRONLY | O_CREAT;


        /*
         * >> means append.
         */
        if (cmd->append)
        {
            flags |= O_APPEND;
        }
        else
        {
            /*
             * > means overwrite.
             */
            flags |= O_TRUNC;
        }


        fd = open(cmd->output, flags, 0644);

        if (fd < 0)
        {
            perror("open output");
            return -1;
        }


        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("dup2 output");
            close(fd);
            return -1;
        }


        close(fd);
    }


    return 0;
}


/*
 * ------------------------------------------------
 * EXECUTE SINGLE COMMAND
 * ------------------------------------------------
 */
int execute_command(command_t *cmd)
{
    pid_t pid;
    int status;


    /*
     * Validate command.
     */
    if (cmd == NULL || cmd->argc == 0)
    {
        return -1;
    }


    /*
     * -----------------------------------------------
     * BUILT-IN
     * -----------------------------------------------
     *
     * Built-ins must execute in the shell process.
     *
     * This is important for commands such as:
     *
     *      cd
     */
    if (is_builtin(cmd))
    {
        int saved_stdin = -1;
        int saved_stdout = -1;
        int result;


        /*
         * Save stdin if input redirection exists.
         */
        if (cmd->input[0] != '\0')
        {
            saved_stdin = dup(STDIN_FILENO);

            if (saved_stdin < 0)
            {
                perror("dup stdin");
                return -1;
            }
        }


        /*
         * Save stdout if output redirection exists.
         */
        if (cmd->output[0] != '\0')
        {
            saved_stdout = dup(STDOUT_FILENO);

            if (saved_stdout < 0)
            {
                perror("dup stdout");

                if (saved_stdin >= 0)
                {
                    close(saved_stdin);
                }

                return -1;
            }
        }


        /*
         * Apply redirection.
         */
        if (setup_redirection(cmd) < 0)
        {
            if (saved_stdin >= 0)
            {
                dup2(saved_stdin, STDIN_FILENO);
                close(saved_stdin);
            }

            if (saved_stdout >= 0)
            {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }

            return -1;
        }


        /*
         * Execute built-in.
         */
        result = execute_builtin(cmd);


        /*
         * Restore stdin.
         */
        if (saved_stdin >= 0)
        {
            dup2(saved_stdin, STDIN_FILENO);
            close(saved_stdin);
        }


        /*
         * Restore stdout.
         */
        if (saved_stdout >= 0)
        {
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }


        return result;
    }


    /*
     * -----------------------------------------------
     * EXTERNAL COMMAND
     * -----------------------------------------------
     */
    pid = fork();


    if (pid < 0)
    {
        perror("fork");
        return -1;
    }


    /*
     * -----------------------------------------------
     * CHILD
     * -----------------------------------------------
     */
    if (pid == 0)
    {
        /*
         * Apply redirection.
         */
        if (setup_redirection(cmd) < 0)
        {
            _exit(1);
        }


        /*
         * Replace child with external command.
         */
        execvp(cmd->argv[0], cmd->argv);


        /*
         * execvp() failed.
         */
        perror("Shellforge");
        _exit(127);
    }


    /*
     * -----------------------------------------------
     * PARENT
     * -----------------------------------------------
     */
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return -1;
    }


    /*
     * Return child exit status.
     */
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }


    /*
     * Child terminated by signal.
     */
    if (WIFSIGNALED(status))
    {
        fprintf(stderr,
                "Process terminated by signal %d\n",
                WTERMSIG(status));

        return -1;
    }


    return -1;
}


/*
 * ------------------------------------------------
 * EXECUTE PIPELINE
 * ------------------------------------------------
 *
 * Example:
 *
 *      ls | grep .c
 *
 * For each command:
 *
 *      create pipe
 *      fork child
 *      connect stdin/stdout using dup2()
 *      close unused descriptors
 *
 * Finally:
 *
 *      wait for all children
 *
 * Return:
 *
 *      exit status of the last command
 */
int execute_pipeline(pipeline_t *pipeline)
{
    int previous_read = -1;
    pid_t pids[MAX_COMMANDS];
    int pid_count = 0;


    /*
     * Validate pipeline.
     */
    if (pipeline == NULL ||
        pipeline->command_count <= 0)
    {
        return -1;
    }


    /*
     * A single command does not need a pipeline.
     */
    if (pipeline->command_count == 1)
    {
        return execute_command(&pipeline->commands[0]);
    }


    /*
     * -----------------------------------------------
     * CREATE EACH PIPELINE COMMAND
     * -----------------------------------------------
     */
    for (int i = 0; i < pipeline->command_count; i++)
    {
        int pipefd[2] = {-1, -1};


        /*
         * Every command except the last one needs
         * a pipe for its output.
         */
        if (i < pipeline->command_count - 1)
        {
            if (pipe(pipefd) == -1)
            {
                perror("pipe");

                if (previous_read != -1)
                {
                    close(previous_read);
                }

                return -1;
            }
        }


        /*
         * -------------------------------------------
         * FORK
         * -------------------------------------------
         */
        pid_t pid = fork();


        if (pid < 0)
        {
            perror("fork");

            if (previous_read != -1)
            {
                close(previous_read);
            }

            if (pipefd[0] != -1)
            {
                close(pipefd[0]);
            }

            if (pipefd[1] != -1)
            {
                close(pipefd[1]);
            }

            return -1;
        }


        /*
         * -------------------------------------------
         * CHILD PROCESS
         * -------------------------------------------
         */
        if (pid == 0)
        {
            /*
             * If this is not the first command,
             * connect previous pipe to stdin.
             *
             * previous pipe:
             *
             *      previous command
             *              |
             *              ▼
             *         previous_read
             */
            if (previous_read != -1)
            {
                if (dup2(previous_read, STDIN_FILENO) == -1)
                {
                    perror("dup2 stdin");
                    _exit(1);
                }
            }


            /*
             * If this is not the last command,
             * connect stdout to the new pipe.
             */
            if (i < pipeline->command_count - 1)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                {
                    perror("dup2 stdout");
                    _exit(1);
                }
            }


            /*
             * Close previous pipe descriptor.
             */
            if (previous_read != -1)
            {
                close(previous_read);
            }


            /*
             * Close both ends of the new pipe.
             *
             * dup2() has already copied the required
             * descriptor to stdin/stdout.
             */
            if (pipefd[0] != -1)
            {
                close(pipefd[0]);
            }

            if (pipefd[1] != -1)
            {
                close(pipefd[1]);
            }


            /*
             * Apply explicit redirection.
             *
             * This allows commands such as:
             *
             *      cat < input.txt | grep hello
             *
             * or:
             *
             *      ls | grep .c > output.txt
             */
            if (setup_redirection(&pipeline->commands[i]) < 0)
            {
                _exit(1);
            }


            /*
             * Execute the command.
             */
            if (is_builtin(&pipeline->commands[i]))
            {
                int result =
                    execute_builtin(&pipeline->commands[i]);

                _exit(result == 0 ? 0 : result);
            }


            execvp(pipeline->commands[i].argv[0],
                   pipeline->commands[i].argv);


            /*
             * execvp() failed.
             */
            perror("Shellforge");
            _exit(127);
        }


        /*
         * -------------------------------------------
         * PARENT PROCESS
         * -------------------------------------------
         */

        pids[pid_count++] = pid;


        /*
         * Parent no longer needs the previous pipe.
         */
        if (previous_read != -1)
        {
            close(previous_read);
            previous_read = -1;
        }


        /*
         * Parent closes the write end of the
         * newly-created pipe.
         *
         * The read end becomes the input for
         * the next command.
         */
        if (i < pipeline->command_count - 1)
        {
            close(pipefd[1]);
            previous_read = pipefd[0];
        }
    }


    /*
     * -----------------------------------------------
     * PARENT: CLOSE REMAINING PIPE
     * -----------------------------------------------
     */
    if (previous_read != -1)
    {
        close(previous_read);
    }


    /*
     * -----------------------------------------------
     * WAIT FOR ALL CHILDREN
     * -----------------------------------------------
     */
    int last_status = 0;


    for (int i = 0; i < pid_count; i++)
    {
        int status;


        if (waitpid(pids[i], &status, 0) == -1)
        {
            perror("waitpid");
            continue;
        }


        /*
         * Save the status of the LAST command.
         */
        if (i == pid_count - 1)
        {
            if (WIFEXITED(status))
            {
                last_status = WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status))
            {
                last_status = -1;
            }
        }
    }


    return last_status;
}
