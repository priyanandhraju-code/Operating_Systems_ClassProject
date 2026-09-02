#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "executor.h"
#include "builtin.h"


/*
 * Execute one command.
 *
 * Built-in commands are executed directly by the shell.
 *
 * External commands are executed using:
 *
 *      fork()
 *      execvp()
 *      waitpid()
 *
 * Returns:
 *      0 or child's exit status -> success
 *     -1                     -> error
 */
int execute_command(command_t *cmd)
{
    pid_t pid;
    int status;


    /*
     * Check whether the command is valid.
     */
    if (cmd == NULL || cmd->argc == 0)
    {
        return -1;
    }


    /*
     * ------------------------------------------------
     * BUILT-IN COMMAND
     * ------------------------------------------------
     *
     * Built-ins such as:
     *
     *      cd
     *      pwd
     *      echo
     *      exit
     *
     * are handled by the shell itself.
     */
    if (is_builtin(cmd))
    {
        return execute_builtin(cmd);
    }


    /*
     * ------------------------------------------------
     * CREATE CHILD PROCESS
     * ------------------------------------------------
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
     * ------------------------------------------------
     * CHILD PROCESS
     * ------------------------------------------------
     */
    if (pid == 0)
    {
        /*
         * Execute the external command.
         *
         * command_t already stores argv as:
         *
         *      argv[0] = command
         *      argv[1] = first argument
         *      ...
         *      argv[argc] = NULL
         *
         * execvp() replaces the child process with
         * the requested external program.
         */
        execvp(cmd->argv[0], cmd->argv);


        /*
         * If execvp() returns, execution failed.
         */
        perror("Shellforge");

        /*
         * Exit only the child process.
         *
         * 127 indicates that the command could
         * not be executed.
         */
        _exit(127);
    }


    /*
     * ------------------------------------------------
     * PARENT PROCESS
     * ------------------------------------------------
     *
     * Wait for the child process to finish.
     */
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return -1;
    }


    /*
     * ------------------------------------------------
     * CHILD EXIT STATUS
     * ------------------------------------------------
     */
    if (WIFEXITED(status))
    {
        /*
         * Return the actual exit status of
         * the external command.
         */
        return WEXITSTATUS(status);
    }


    /*
     * ------------------------------------------------
     * CHILD TERMINATED BY SIGNAL
     * ------------------------------------------------
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
