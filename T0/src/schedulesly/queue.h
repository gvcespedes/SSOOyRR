#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Estructura para un proceso
typedef struct Process {
    int PID; // ID del proceso
    int CI;
    int CF;
    int NH;
    // Agregar más atributos según sea necesario
} Process;

// Nodo de la lista enlazada para la cola de procesos
typedef struct ProcessNode {
    Process process; // El proceso
    struct ProcessNode* next; // Puntero al siguiente nodo en la cola
} ProcessNode;

// Estructura para la cola de procesos
typedef struct {
    ProcessNode* head; // Puntero al primer nodo de la cola
    ProcessNode* tail; // Puntero al último nodo de la cola
} ProcessQueue;

void initializeQueue(ProcessQueue* queue);
void enqueue(ProcessQueue* queue, Process process);
Process dequeue(ProcessQueue* queue);
void printQueue(ProcessQueue* queue);
int estaEnCola(ProcessQueue* cola, int PID);
Process dequeueLast(ProcessQueue* queue);
bool estaVacia(ProcessQueue* queue);
void freeQueue(ProcessQueue* queue);

#endif