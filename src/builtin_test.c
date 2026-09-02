#include <stdio.h>

#include "builtin.h"

int main(void)
{
    command_t cmd;

    /* ==================== PWD ==================== */

    printf("========== PWD TEST ==========\n");

    cmd.argc = 1;
    cmd.argv[0] = "pwd";
    cmd.argv[1] = NULL;

    if (execute_builtin(&cmd) == 0)
    {
        printf("pwd: SUCCESS\n");
    }


    /* ==================== ECHO ==================== */

    printf("\n========== ECHO TEST ==========\n");

    cmd.argc = 3;
    cmd.argv[0] = "echo";
    cmd.argv[1] = "Hello";
    cmd.argv[2] = "ShellForge";
    cmd.argv[3] = NULL;

    if (execute_builtin(&cmd) == 0)
    {
        printf("echo: SUCCESS\n");
    }


    /* ==================== CD ==================== */

    printf("\n========== CD TEST ==========\n");

    cmd.argc = 2;
    cmd.argv[0] = "cd";
    cmd.argv[1] = "/tmp";
    cmd.argv[2] = NULL;

    if (execute_builtin(&cmd) == 0)
    {
        printf("cd: SUCCESS\n");
    }


    /* Verify cd using pwd */

    printf("\n========== PWD AFTER CD ==========\n");

    cmd.argc = 1;
    cmd.argv[0] = "pwd";
    cmd.argv[1] = NULL;

    execute_builtin(&cmd);


    /* ==================== EXIT ==================== */

    printf("\n========== EXIT TEST ==========\n");

    cmd.argc = 1;
    cmd.argv[0] = "exit";
    cmd.argv[1] = NULL;

    if (execute_builtin(&cmd) == 1)
    {
        printf("exit: SUCCESS\n");
    }

    return 0;
}
