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
            p->pid = atoi(process[2]);
            p->runtime = atoi(process[3]);

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

int main(void)
{
    Queue *q = initialize_queue(q);
    add_to_queue(q, "processes.txt");

    printf("\n");
    print_queue(q);
}