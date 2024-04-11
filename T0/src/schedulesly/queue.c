
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "queue.h"


// Inicializa la cola
void initializeQueue(ProcessQueue* queue) {
    queue->head = NULL;
    queue->tail = NULL;
}


// Retorna si la cola está vacía
bool estaVacia(ProcessQueue* queue) {
    return queue->head == NULL;
}

// Añade un proceso al final de la cola
void enqueue(ProcessQueue* queue, Process process) {
    ProcessNode* newNode = (ProcessNode*)malloc(sizeof(ProcessNode));
    newNode->process = process;
    newNode->next = NULL;
    
    if (queue->tail != NULL) {
        queue->tail->next = newNode;
    }
    
    queue->tail = newNode;
    
    if (queue->head == NULL) {
        queue->head = newNode;
    }
}

// Remueve y retorna el proceso al frente de la cola
Process dequeue(ProcessQueue* queue) {
    if (queue->head == NULL) {
        // Cola vacía
        exit(EXIT_FAILURE); // O manejar de otra manera
    }
    
    ProcessNode* tempNode = queue->head;
    Process process = tempNode->process;
    queue->head = queue->head->next;
    
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    
    free(tempNode);
    return process;
}


// Imprime los procesos en la cola
void printQueue(ProcessQueue* queue) {
    ProcessNode* currentNode = queue->head;
    
    if (currentNode == NULL) {
        printf("La cola está vacía.\n");
        return;
    }
    
    printf("Procesos en la cola:\n");
    while (currentNode != NULL) {
        printf("PID: %d\n", currentNode->process.PID);
        currentNode = currentNode->next;
    }
}


int estaEnCola(ProcessQueue* cola, int PID) {
    ProcessNode* currentNode = cola->head;
    while (currentNode != NULL) {
        if (currentNode->process.PID == PID) {
            return 1;
        }
        currentNode = currentNode->next;
    }
    return 0;
}



Process dequeueLast(ProcessQueue* queue) {
    if (queue->head == NULL) { 
        printf("La cola está vacía.\n");
    }

    if (queue->head == queue->tail) {
        ProcessNode* tempNode = queue->head;
        Process process = tempNode->process;
        free(tempNode);
        queue->head = NULL;
        queue->tail = NULL;
        return process;
    }
    ProcessNode* currentNode = queue->head;
    while (currentNode->next != queue->tail) {
        currentNode = currentNode->next;
    }

    ProcessNode* lastNode = queue->tail;
    Process process = lastNode->process; 
    free(lastNode); 
    currentNode->next = NULL; 
    queue->tail = currentNode; 

    return process;
}


void freeQueue(ProcessQueue* queue) {
    while (queue->head != NULL) {
        dequeue(queue);
    }
}