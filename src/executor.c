#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "executor.h"
#include "builtin.h"


/*
 * Apply input/output redirection.
 *
 * Returns:
 *      0  -> success
 *     -1  -> error
 */
static int setup_redirection(const command_t *cmd)
{
    int fd;

    /*
     * -----------------------------------------------
     * INPUT REDIRECTION
     * -----------------------------------------------
     *
     * command < input.txt
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
     *
     * command > output.txt
     * command >> output.txt
     */
    if (cmd->output[0] != '\0')
    {
        int flags = O_WRONLY | O_CREAT;

        if (cmd->append)
        {
            /*
             * Append to existing file.
             */
            flags |= O_APPEND;
        }
        else
        {
            /*
             * Create/truncate file.
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
 * Execute one command.
 *
 * Handles:
 *
 *      - built-in commands
 *      - external commands
 *      - input redirection
 *      - output redirection
 *      - append redirection
 *
 * External commands:
 *
 *      fork()
 *        |
 *        +--> child -> redirection -> execvp()
 *        |
 *        +--> parent -> waitpid()
 *
 * Built-ins:
 *
 *      save shell FDs
 *      redirect
 *      execute built-in
 *      restore shell FDs
 */
int execute_command(command_t *cmd)
{
    pid_t pid;
    int status;


    /*
     * -----------------------------------------------
     * VALIDATE COMMAND
     * -----------------------------------------------
     */
    if (cmd == NULL || cmd->argc == 0)
    {
        return -1;
    }


    /*
     * -----------------------------------------------
     * BUILT-IN COMMAND
     * -----------------------------------------------
     *
     * Built-ins must execute in the shell process.
     *
     * This is especially important for:
     *
     *      cd
     *
     * because cd must change the shell's directory.
     */
    if (is_builtin(cmd))
    {
        int saved_stdin = -1;
        int saved_stdout = -1;
        int result;


        /*
         * Save standard input/output before
         * applying redirection.
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
         * Apply redirection to the shell process.
         */
        if (setup_redirection(cmd) < 0)
        {
            /*
             * Restore anything already saved.
             */
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
         * Execute the built-in.
         */
        result = execute_builtin(cmd);


        /*
         * Restore standard input.
         */
        if (saved_stdin >= 0)
        {
            if (dup2(saved_stdin, STDIN_FILENO) < 0)
            {
                perror("restore stdin");
            }

            close(saved_stdin);
        }


        /*
         * Restore standard output.
         */
        if (saved_stdout >= 0)
        {
            if (dup2(saved_stdout, STDOUT_FILENO) < 0)
            {
                perror("restore stdout");
            }

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


    /*
     * fork() failed.
     */
    if (pid < 0)
    {
        perror("fork");
        return -1;
    }


    /*
     * -----------------------------------------------
     * CHILD PROCESS
     * -----------------------------------------------
     */
    if (pid == 0)
    {
        /*
         * Apply redirection inside the child.
         */
        if (setup_redirection(cmd) < 0)
        {
            _exit(1);
        }


        /*
         * Execute external command.
         */
        execvp(cmd->argv[0], cmd->argv);


        /*
         * execvp() only returns if execution failed.
         */
        perror("Shellforge");

        _exit(127);
    }


    /*
     * -----------------------------------------------
     * PARENT PROCESS
     * -----------------------------------------------
     */
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return -1;
    }


    /*
     * Return child's exit status.
     */
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }


    /*
     * Child was terminated by a signal.
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
