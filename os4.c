#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Struct of a node for allocation list and max list*/
typedef struct node
{
    int key;
    struct node* next;
} node;

/*Struct of a process*/
typedef struct process
{
    int pid;
    node* allocation;
    node* max;
    int state; // state of the process
} process;

int count = 0; //count for safe sequence

/*Funtion BacnkerAlgorithm to find if there is a way to run all processes*/
void  bankersAlgorithm(int m, int n, process* jobs, int* availableResource, int* safeSequence)
{
    node* allocation;
    node* max;
    for (int j = 0; j < n; j++)
    {
        int stop = 0; //Variable stop to check to see if it can be assigned.
        process* process = &(jobs)[j];
        if (process->state == 0)
        {
            allocation = process->allocation;
            max = process->max;
            for (int i = 0; i < m; i++)
            {
                if (((max->key) - (allocation->key)) <= availableResource[i]) // check available resource are more than need resource
                {
                    allocation = allocation->next;
                    max = max->next;
                }
                else
                {
                    stop = 1;
                }
            }
            if (stop == 0) // if all available resource are more than need resource
            {
                safeSequence[count] = process->pid; // save the process pid to safe sequence
                count++;
                allocation = process->allocation;
                for (int i = 0; i < m; i++) // return allocation resource to available resource
                {
                    int temp = availableResource[i];
                    availableResource[i] = allocation->key + temp;
                    allocation = allocation->next;
                }
                process->state = 1; // process done.
            }
        }
    }
}

/*Funtion setJobs to set all process by reading the contents of the file*/
process* setJobs(FILE* input_file, process* jobs, int numberOfResource, int numberOfProcess, int p, int* requestResource)
{
    jobs = (process*)malloc(sizeof(process) * numberOfProcess);

    for (int k = 0; k < numberOfProcess; k++) //repeat process number
    {
        process* process = &(jobs)[k];
        process->state = 0;

        node* allocationNode = (node*)malloc(sizeof(node)); //assign memory for first allocation node (head).
        node* maxNode = (node*)malloc(sizeof(node)); //assign memory for first max node (head).
        process->allocation = allocationNode;
        process->max = maxNode;
        node* temp;

        fscanf(input_file, "%d", &process->pid);
        for (int i = 0; i < numberOfResource; i++)
        {
            fscanf(input_file, "%d", &allocationNode->key);
            if (requestResource != NULL && k + 1 == p) // if after arriving request:
            {
                allocationNode->key += requestResource[i]; // plus request resource
            }
            temp = allocationNode;
            allocationNode = allocationNode->next;
            allocationNode = (node*)malloc(sizeof(node));
            allocationNode->next = NULL;
            temp->next = allocationNode;
        }
        for (int i = 0; i < numberOfResource; i++)
        {
            fscanf(input_file, "%d", &maxNode->key);
            temp = maxNode;
            maxNode = maxNode->next;
            maxNode = (node*)malloc(sizeof(node));
            maxNode->next = NULL;
            temp->next = maxNode;
        }
    }
    return jobs;
}

/*Funtuion setAvailableResource to set available resource by reading file*/
int* setAvailableResource(FILE* input_file, int numberOfResource, int* requestResource)
{
    int* availableResource = (int*)malloc(sizeof(int) * numberOfResource);

    for (int i = 0; i < numberOfResource; i++)
    {
        fscanf(input_file, "%d", &availableResource[i]);
        if (requestResource != NULL)
        {
            availableResource[i] -= requestResource[i];
        }
    }

    return availableResource;
}

/*Funtion clear to free(return) memeory linked list for alloction list and max list*/
void clear(node* head)
{
    node* next_node;
    node* node = head;
    while (node != NULL)
    {
        next_node = node->next;
        free(node);
        node = next_node;
    }
    head = NULL;
}

/*Funtion simulator to control all funtion and fprint output and after arriveing request sequence*/
void simulator(FILE* input_file, process* jobs, int numberOfResource, int numberOfProcess, FILE* output_file, int p, int* requestResource)
{
    int* availableResource;
    int* safeSequence = (int*)malloc(sizeof(int) * numberOfProcess);

    availableResource = setAvailableResource(input_file, numberOfResource, requestResource);

    jobs = setJobs(input_file, jobs, numberOfResource, numberOfProcess, p, requestResource);

    for (int j = 0; j < numberOfProcess; j++) // repeat banker algorithm number or process time, worst case is 5->4->3->2->1
    {
        bankersAlgorithm(numberOfResource, numberOfProcess, jobs, availableResource, safeSequence);
    }

    int safe = 1; // variable safe to check existence of safe sequence
    for (int j = 0; j < numberOfProcess; j++)
    {
        process* process = &(jobs)[j];

        if (process->state == 0) // if one or more process can't run = safe sequence doesn't exist.
        {
            safe = 0;
        }
    }
    if (safe == 1) // if safe sequence exist.
    {
        for (int j = 0; j < numberOfProcess; j++)
        {
            fprintf(output_file, "p%d -> ", safeSequence[j]);
        }
    }
    else
    {
        fprintf(output_file, " unsafe!");
    }

    /*reutrn and free memories*/
    clear(jobs->allocation);
    clear(jobs->max);
    free(jobs);
    free(safeSequence);
    free(availableResource);

    if (requestResource == NULL && safe == 1) // if After arriving request sequence doesn't have output
    {
        int p;
        jobs = NULL;
        requestResource = (int*)malloc(sizeof(int) * numberOfResource);
        fscanf(input_file, "%d", &p);

        for (int i = 0; i < numberOfResource; i++)
        {
            fscanf(input_file, "%d", &requestResource[i]);
        }

        count = 0;
        rewind(input_file); //rewind(reset) input_file cusor
        fprintf(output_file, "\nAfter arriving request: ");

        simulator(input_file, jobs, numberOfResource, numberOfProcess, output_file, p, requestResource);
    }

}

/*Funtion main to call major funtions*/
int main(int argc, char* argv[])
{
    if (argc != 5) // defensive coding
    {
        fprintf(stderr, "%s [input_filename] [output_filename] [the_number_of_resource] [the_number_of_process]\n", argv[0]);
    }
    else
    {
        FILE* input_file = fopen(argv[1], "r");
        FILE* output_file = fopen(argv[2], "w");
        int numberOfResource = (int)atoi(argv[3]);
        int numberOfProcess = atoi(argv[4]);
        process* jobs = NULL;

        fprintf(output_file, "current state: ");
        simulator(input_file, jobs, numberOfResource, numberOfProcess, output_file, 0, NULL);

        fclose(input_file);
        fclose(output_file);
    }
}