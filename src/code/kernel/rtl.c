// Real-Time Loading

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <linux/module.h> // Included for all Linux Modules
#include <linux/kernel.h> // Included for KERN_INFO
#include <linux/init.h> // Included for __init and __exit macros

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ronald Pereira");
MODULE_DESCRIPTION("Real-Time Loading Module");

typedef struct
{
    char cmd[100000];
    unsigned int events;
} Command;

int SIZE = 50;

void init()
{
    int i;
    FILE *database;

    database = fopen(".rtldatabase.db", "w");

    fprintf(database, "%d\n", SIZE);

    for(i = 0; i < SIZE; i++)
    fprintf(database, "Empty 0\n");
}

Command *databaseReader()
{
    int i;
    Command *cmd;
    FILE *database;

    database = fopen(".rtldatabase.db", "r");

    if(database == NULL)
    {
        printf("\nERROR: Database was not found. Please, type 'make init' before configuring the database.\n");
        exit(0);
    }

    fscanf(database, "%d", &SIZE);
    cmd = (Command*) calloc(SIZE,sizeof(Command));

    for(i = 0; i < SIZE; i++)
    {
        fscanf(database, "%s %u ", cmd[i].cmd, &(cmd[i].events));

        if(strcmp(cmd[i].cmd, "") == 0)
            strcpy(cmd[i].cmd, "Empty");
    }

    fclose(database);

    return cmd;
}

void showDatabase(Command *cmd)
{
    int i;
    FILE *database;

    database = fopen(".rtldatabase.db", "r");

    printf("\n\n---------- Database commands ----------\n");
    printf("Events\t\t\t\tCommands\n\n");

    for(i = 0; i < SIZE; i++)
    {
        if(!(strcmp(cmd[i].cmd, "Empty") == 0))
            printf("%d\t\t\t\t%s\n\n", cmd[i].events, cmd[i].cmd);
    }

    fclose(database);
}

int commandSearch(Command *cmd, char *command)
{
    int i, firstfit = -1;

    for(i = 0; i < SIZE; i++)
    {
        if(strcmp(cmd[i].cmd, command) == 0)
        {
            printf("Command '%s' was already inserted in the database. Added +1 event to it.\n", cmd[i].cmd);
            return i;
        }

        else if((strcmp(cmd[i].cmd, "Empty") == 0) && firstfit == -1)
        {
            printf("Command '%s' was succesfully inserted in database.\n", command);
            firstfit = i;
        }
    }

    if(firstfit == -1)
        printf("ERROR: Full database, can't insert more commands. Please, delete some commands or change the size of it.\n");

    return firstfit;
}

void databasePrinter(Command *cmd)
{
    FILE *database;
    int i;

    database = fopen(".rtldatabase.db", "w");

    fprintf(database, "%d\n", SIZE);

    for(i = 0; i < SIZE; i++)
        fprintf(database, "%s %u\n", cmd[i].cmd, cmd[i].events);

    fclose(database);
}

Command *deleteCommand(Command *cmd, char *command)
{
    int i;

    for(i = 0; i < SIZE; i++)
    {
        if(strcmp(cmd[i].cmd, command) == 0)
        {
            strcpy(cmd[i].cmd, "Empty");
            cmd[i].events = 0;
            printf("Command '%s' was deleted from the database\n", command);
            return cmd;
        }
    }

    printf("The command was not found in database.\n");

    time_t timestamp;
    time(&timestamp);
    FILE *log;
    log = fopen(".rtllog.txt", "a");
    fprintf(log, "%s - ERROR: Command '%s' couldn't be added to the database. Database was full.\n\n", ctime(&timestamp), command);
    fclose(log);

    return cmd;
}

void logPrinter()
{
    FILE *logs;
    char *str;

    logs = fopen(".rtllog.txt", "r");

    if(logs == NULL)
    {
        printf("Any errors was detected, so the Real-Time Loading log file isn't created.\n");
        return;
    }

    str = (char*) malloc(100000*sizeof(char));

    printf("\n\n---------- Error Logs ----------\n\n");

    while(fscanf(logs, " %[^\n]", str) != EOF)
        printf("%s\n", str);

    free(str);
    fclose(logs);
}

void config()
{
    int i, position, qtd, option = 1;
    Command *cmd;
    FILE *database;
    char *command, *startcommand, *waitingstartcommand, *killcommand;

    command = (char*) calloc(100000,sizeof(char));
    startcommand = (char*) calloc(100000,sizeof(char));
    waitingstartcommand = (char*) calloc(100000,sizeof(char));
    killcommand = (char*) calloc(100000,sizeof(char));

    while(option != 0)
    {
        cmd = databaseReader();

        printf("\n\n---------- Configuration menu ----------\n\nInsert the option:\n1 - Show the database saved commands\n2 - Load all processes\n3 - Insert commands\n4 - Delete a command\n5 - Clear the database and change the maximum size (actual size = %d processes)\n6 - Show the error logs\n0 - Exit the configuration program\n\n> ", SIZE);
        scanf(" %d", &option);

        if(option == 1)
        {
            showDatabase(cmd);
            printf("Press enter to continue...");
            getchar();
            getchar();
            fflush(stdin);
        }

        else if(option == 2)
        {
            cmd = databaseReader();

            for(i = 0; i < SIZE; i++)
            {
                if(!(strcmp(cmd[i].cmd, "Empty") == 0))
                {
                    startcommand[0] = '\0';
                    waitingstartcommand[0] = '\0';
                    killcommand[0] = '\0';

                    printf("'%s' is starting\n", cmd[i].cmd);
                    strcat(startcommand, cmd[i].cmd);
                    strcat(startcommand, " &");
                    system(startcommand);

                    strcat(waitingstartcommand, "until pids=$(pidof ");
                    strcat(waitingstartcommand, cmd[i].cmd);
                    strcat(waitingstartcommand, "); do   sleep 0.1; done");
                    system(waitingstartcommand);

                    strcat(killcommand, "killall ");
                    strcat(killcommand, cmd[i].cmd);

                    printf("Killing process %s\n", killcommand);
                    system(killcommand);
                    cmd[i].events++;
                }
            }

            databasePrinter(cmd);
            printf("Press enter to continue...");
            getchar();
            getchar();
            fflush(stdin);
        }

        else if(option == 3)
        {

            printf("\nInsira quantos comandos deseja inserir:\n> ");
            scanf("%d", &qtd);

            for(i = 0; i < qtd; i++)
            {
                database = fopen(".rtldatabase.db", "r+w");

                startcommand[0] = '\0';
                killcommand[0] = '\0';
                fseek(database, SEEK_SET, 0);
                printf("Insira o comando %d:\n> ", i+1);
                scanf(" %[^\n]", command);
                position = commandSearch(cmd, command);

                if(position == -1)
                {
                    time_t timestamp;
                    time(&timestamp);
                    FILE *log;
                    log = fopen(".rtllog.txt", "a");
                    fprintf(log, "%s - ERROR: Command '%s' couldn't be added to the database. Database was full.\n\n", ctime(&timestamp), command);
                    fclose(log);
                }

                else
                {
                    strcpy(cmd[position].cmd, command);
                    cmd[position].events++;
                }

                databasePrinter(cmd);
                fclose(database);

                printf("Press enter to continue...");
                getchar();
                getchar();
                fflush(stdin);
            }
        }

        else if(option == 4)
        {
            showDatabase(cmd);
            printf("Insert the command you want to delete:\n> ");
            scanf("%s", command);
            cmd = deleteCommand(cmd, command);
	        databasePrinter(cmd);
            printf("Press enter to continue...");
            getchar();
            getchar();
            fflush(stdin);
        }

        else if(option == 5)
        {
            database = fopen(".rtldatabase.db", "w");

            printf("Insert the new size of the database:\n> ");
            scanf(" %d", &SIZE);

            while(SIZE < 1)
            {
                printf("Invalid size.\nInsert the new size of the database:\n> ");
                scanf(" %d", &SIZE);
            }

            fprintf(database,"%d\n", SIZE);

            for(i = 0; i < SIZE; i++)
                fprintf(database, "Empty 0\n");

            cmd = databaseReader();

            databasePrinter(cmd);

            fclose(database);
        }

        else if(option == 6)
        {
            logPrinter();
            printf("Press enter to continue...");
            getchar();
            getchar();
            fflush(stdin);
        }

        else if(option != 0)
        {
            printf("Invalid option.\n");
            printf("Press enter to continue...");
            getchar();
            getchar();
            fflush(stdin);
        }
    }

    free(command);
    free(startcommand);
    free(waitingstartcommand);
    free(killcommand);
}

void rtl()
{
    int i;
    char *startcommand, *waitingstartcommand, *killcommand;
    Command *cmd;

    startcommand = (char*) calloc(100000,sizeof(char));
    waitingstartcommand = (char*) calloc(100000,sizeof(char));
    killcommand = (char*) calloc(100000,sizeof(char));

    cmd = databaseReader();

    for(i = 0; i < SIZE; i++)
    {
        if(!(strcmp(cmd[i].cmd, "Empty") == 0))
        {
            startcommand[0] = '\0';
            waitingstartcommand = '\0';
            killcommand[0] = '\0';

            printf("'%s' is starting\n", cmd[i].cmd);
            strcat(startcommand, cmd[i].cmd);
            strcat(startcommand, " &");
            system(startcommand);

            strcat(waitingstartcommand, "until pids=$(pidof ");
            strcat(waitingstartcommand, cmd[i].cmd);
            strcat(waitingstartcommand, "); do   sleep 0.1; done");
            system(waitingstartcommand);

            strcat(killcommand, "killall ");
            strcat(killcommand, cmd[i].cmd);

            printf("Killing process %s\n", killcommand);
            system(killcommand);
            cmd[i].events++;
        }
    }

    free(startcommand);
    free(waitingstartcommand);
    free(killcommand);
    databasePrinter(cmd);
}

static int __init rtl_init(int argv, char *argc[])
{
    if(argc[1] == NULL)
    {
        printk(KERN_INFO "\nExecuting Real-Time Loading. Please wait...\n");
        rtl();
        printk(KERN_INFO "\nReal-Time Loading ended\n");
    }

    else if(strcmp(argc[1], "init") == 0)
    {
        printf("\n\n");
        printf("*********************\n");
        printf("*                   *\n");
        printf("* Real-Time Loading *\n");
        printf("*                   *\n");
        printf("*********************\n");

        printf("\nInitializing Real-Time Loading database file...\n");
        init();
        printf("Real-Time Loading ready!\n");
    }

    else if(strcmp(argc[1], "config") == 0)
    {
        printf("\n\n");
        printf("*********************\n");
        printf("*                   *\n");
        printf("* Real-Time Loading *\n");
        printf("*                   *\n");
        printf("*********************\n");

        printf("\nWelcome to Real-Time Loading configuration.\n");
        config();
    }

	return 0;
}

static void __exit rtl_cleanup()
{
    printk(KERN_INFO "Cleaning up module\n");
}

module_init(rtl_init);
module_exit(rtl_cleanup);
