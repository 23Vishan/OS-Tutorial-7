#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

const char *DELIMITER = ",";

#define RED "\e[0;31m"
#define reset "\e[0m"

#define TRUE 1
#define FALSE 0

typedef struct Proc
{
    char name[256];
    int priority;
    int pid;
    int runtime;
} Proc;

typedef struct Queue
{
    Proc *process;
    struct Queue *next;
} Queue;

// initialize queue structure
Queue *initialize_queue(Queue *q)
{        
    q = (Queue*)malloc(sizeof(Queue));

    q->process = NULL;
    q->next = NULL;
  
    return q;
}

// initialize proc structure
Proc *initialize_process(Proc *p)
{
    p = (Proc*)malloc(sizeof(Proc));

    strcpy(p->name, "null");
    p->pid = 0;
    p->priority = 0;
    p->runtime = 0;

    return p;
}

// adds a node to queue
void push(Queue *head, Proc* p)
{
    // if head does not have a process
    if (head->process == NULL)
    {
        head->process = p;
        return;
    }
    else
    {
        Queue *current = head;

        // find next empty space
        while (current->next != NULL)
        {
            current = current->next;
        }

        Queue *tmp = initialize_queue(tmp);
        tmp->process = p;

        current->next = tmp;

        return;
    }
}

// remove and returns the first process in queue
Proc *pop(Queue *head)
{
    if (head == NULL)
    {
        return NULL;
    }

    Proc *process = head->process;

    if (head->next != NULL)
    {
        Queue *tmp = head->next;
        
        head->process = tmp->process;
        head->next = tmp->next;
    }
    else
    {
        free(head);
    }

    return process;
}

// remove and return the process with the given name in queue
Proc *delete_name(Queue *head, char *name)
{
    Queue *current = head;
    Queue *prev = NULL;

    // search
    while (current != NULL)
    {   
        // found
        if (strcmp(current->process->name, name) == 0)
        {
            Proc *process = current->process;

            // if not at head
            if (prev != NULL)
            {
                prev ->next = current->next;
            }
            // at head
            else
            {
                pop(head);
            }
        }

        // move to next
        prev = current;
        current = current->next;
    }

    return NULL;
}

// remove and return the process with the given id in queue
Proc *delete_pid(Queue *head, int pid)
{
    Queue *current = head;
    Queue *prev = NULL;

    // search
    while (current != NULL)
    {   
        // found
        if (current->process->pid == pid)
        {
            Proc *process = current->process;

            // if not at head
            if (prev != NULL)
            {
                prev ->next = current->next;
            }
            // at head
            else
            {
                pop(head);
            }
        }

        // move to next
        prev = current;
        current = current->next;
    }

    return NULL;
}

// counts the number of tokens in a string
int count_tokens(char *cmd)
{
    char *cmd_copy = NULL;     // stores command
    cmd_copy = malloc(1024+1); // allocate memory

    // error management
    if (!cmd_copy)
    {
        fprintf(stderr, "Error: Failed to Allocate Memory: %s\n", strerror(errno));
        return 0;
    }

    // copy command
    strcpy(cmd_copy, cmd);

    int token_count = 0;                       // holds number of tokens
    char *token = strtok(cmd_copy, DELIMITER); // first token
    token_count++;                             // increment counter

    // determine number of tokens
    while (token != NULL)
    {
        token_count ++;
        token = strtok(NULL, DELIMITER);
    }

    // free pointer
    free(token);

    // return count
    return token_count;
}

// tokenize string by delimiter
char **parse_cmd(char *cmd)
{
    // holds number of tokens
    int token_count = count_tokens(cmd);

    char **tokens = NULL;                          // stores tokens
    tokens = malloc(sizeof(char *) * token_count); // allocate memory

    // error management
    if (!tokens)
    {
        fprintf(stderr, "Error: Failed to Allocate Memory: %s\n", strerror(errno));
        return NULL;
    }

    // get first token
    char *token = strtok(cmd, DELIMITER);

    // store all tokens
    int i;
    for (i = 0; token != NULL; i++)
    {
        tokens[i] = malloc(sizeof(char) * strlen(token)); // allocate space
        strcpy(tokens[i], token);                         // copy token
        token = strtok(NULL, DELIMITER);                  // get next token
    }

    tokens[i] = NULL; // null terminate the array

    // return all tokens
    return tokens;
}

// adds processes from dispatch file to input queue
void *add_to_queue(Queue *q, char *file)
{
    FILE *dispatch_list = fopen(file, "r");

    if (dispatch_list != NULL)
    {
        char line[1024];
        while (fgets(line, sizeof(line), dispatch_list) != NULL)
        {
            // tokensize line
            char **process = parse_cmd(line);

            // initialize pcb
            Proc *p = initialize_process(p);

            // assign values to pcb
            strcpy(p->name, process[0]);
            p->priority = atoi(process[1]);
            p->runtime = atoi(process[2]);

            // add process to queue
            push(q, p);
        }
    fclose(dispatch_list);
    }
    // error management
    else
    {
        fprintf(stderr, "error opening file\n");
    }
}

// displays contents of a queue
void print_queue(Queue *head)
{
    // exit if end of queue was reached
    if (head == NULL)
    {
        return;
    }
    else
    {
        // print info
        printf("Name: %s\n", head->process->name);
        printf("Priority: %d\n", head->process->priority);
        printf("PID: %d\n", head->process->pid);
        printf("Runtime: %d\n\n", head->process->runtime);

        // move to next node
        print_queue(head->next);
    }
}

// returns and removes the first process in list with the given priority
Proc *find(Queue *head, int priority)
{
    if (head == NULL)
    {
        return NULL;
    }
    else
    {
        if (head->process->priority == priority)
        {
            delete_name(head, head->process->name);
            return head->process;
        }
        find(head->next, priority);
    }
}

int main(void)
{
    Queue *q = initialize_queue(q);
    add_to_queue(q, "processes_q5.txt");
    
    Proc *p = NULL;
    pid_t child = fork();

    // PROCESS 1
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(5);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "systemd", 0, child, 5);
            break;
    }

    // PROCESS 2
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(8);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "bash", 0, child, 8);
            break;
    }

    // PROCESS 3
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(3);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "vim", 1, child, 3);
            break;
    }
    
    // PROCESS 4
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(1);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "emacs", 3, child, 1);
            break;
    }

    // PROCESS 5
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(2);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "chrome", 1, child, 2);
            break;
    }

    // PROCESS 6
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(3);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "chrome", 1, child, 3);
            break;
    }

    // PROCESS 7
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(1);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "chrome", 1, child, 1);
            break;
    }

    // PROCESS 8
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(4);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "gedit", 2, child, 4);
            break;
    }

    // PROCESS 9
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(2);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "eclipse", 2, child, 2);
            break;
    }

    // PROCESS 10
    child = fork();
    switch (child)
    {
        // child
        case 0:            
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;
            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(3);
            kill(child, SIGINT);
            waitpid(child, NULL, WUNTRACED);
            printf("Name: %s, Priority: %d, PID: %d, Runtime: %d\n\n", "cland", 1, child, 3);
            break;
    }

}