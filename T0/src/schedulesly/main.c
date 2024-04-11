#include <stdio.h>	
#include <stdlib.h> 
#include <stdbool.h> 
#include "../file_manager/manager.h"
#include "queue.h"

typedef struct {
    int TI;
} ProcessGroup;

int max(int a, int b);
void simulateSO(ProcessGroup groups[], int nGroups, ProcessQueue waitQueue, ProcessQueue runningQueue, ProcessQueue finishedQueue, int qstart, int qdelta, int qmin, InputFile *input_file);
void simulacion_procesos(int CI, int NH, ProcessQueue waitQueue, ProcessQueue runningQueue, ProcessQueue finishedQueue, int qstart, int qdelta, int qmin, InputFile *input_file, int linea, int argumento, int PID, int PPID, int GID, int T_SO, bool leerDeArchivo);

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <archivo_de_entrada>\n", argv[0]);
        return 1;
    }
    freopen("output.txt", "w", stdout);
    char *file_name = (char *)argv[1];
    InputFile *input_file = read_file(file_name); 
    int qstart = atoi(input_file->lines[0][0]);
    int qdelta = atoi(input_file->lines[0][1]);
    int qmin = atoi(input_file->lines[0][2]);
    ProcessGroup groups[input_file->len - 1];
    ProcessQueue waitQueue, runningQueue, finishedQueue;
    initializeQueue(&finishedQueue);
    initializeQueue(&waitQueue);
    initializeQueue(&runningQueue);
    for (int i = 1; i < input_file->len; ++i) {
        int TI = atoi(input_file->lines[i][0]);
        groups[i - 1].TI = TI;
    }
    simulateSO(groups, input_file->len - 1, waitQueue, runningQueue, finishedQueue, qstart, qdelta, qmin, input_file);

    input_file_destroy(input_file);

}

int ID = 1;
int SO_TIME = 0;
int GrupoTerminados = 0;
int aux = 1;

void simulateSO(ProcessGroup groups[], int nGroups, ProcessQueue waitQueue, ProcessQueue runningQueue, ProcessQueue finishedQueue, int qstart, int qdelta, int qmin, InputFile *input_file) {
    int currentTime = SO_TIME;
    int activeGroups = GrupoTerminados;
    while (activeGroups < nGroups) {
        for (int i = 0; i < nGroups; i++) {
            if (groups[i].TI == currentTime) {
                if (currentTime > 0) {
                    printf("IDLE %d\n", currentTime);
                }
                activeGroups++;
                simulacion_procesos(0, 0, waitQueue, runningQueue, finishedQueue, qstart, qdelta, qmin, input_file, i+1, aux, ID, 0, activeGroups, currentTime, true);
            }
        }

        currentTime++;
    }
}


int Tiempo_SO = 0;

void simulacion_procesos(int CI, int NH, ProcessQueue waitQueue, ProcessQueue runningQueue, ProcessQueue finishedQueue, int qstart, int qdelta, int qmin, InputFile *input_file, int linea, int argumento, int PID, int PPID, int GID, int T_SO, bool leerDeArchivo) {
    if (leerDeArchivo) {
        CI = atoi(input_file->lines[linea][argumento++]);
        NH = atoi(input_file->lines[linea][argumento++]);
    }
    Process nuevoProceso;
    nuevoProceso.PID = PID;
    nuevoProceso.CI = CI;
    nuevoProceso.NH = NH;
    int tiempoRestanteEjecucion = CI;
    if (!estaEnCola(&runningQueue, PID)) {
        argumento -= 2;
        printf("ENTER %d %d %d TIME %d LINE %d ARG %d\n", PID, PPID, GID, T_SO, linea, argumento);
        argumento += 2;
        enqueue(&runningQueue, nuevoProceso);
    }
    
    if (!estaEnCola(&waitQueue, PID)) {
        tiempoRestanteEjecucion = (tiempoRestanteEjecucion <= qstart) ? tiempoRestanteEjecucion : qstart;
        printf("RUN %d %d\n", PID, tiempoRestanteEjecucion);
        T_SO += tiempoRestanteEjecucion;
        nuevoProceso.CI -= tiempoRestanteEjecucion;
    }
    if (nuevoProceso.CI > 0) {
        qstart = max(qstart - qdelta, qmin);
        simulacion_procesos(nuevoProceso.CI, NH, waitQueue, runningQueue, finishedQueue, qstart, qdelta, qmin, input_file, linea, argumento, PID, PPID, GID, T_SO, false);
    } else {
        if (NH > 0) {
            printf("WAIT %d\n", PID);
            nuevoProceso.NH = 0;
            enqueue(&waitQueue, nuevoProceso);
            dequeueLast(&runningQueue);
            printQueue(&runningQueue);
            printQueue(&waitQueue);
            int nuevo_PID = PID + 1;
            simulacion_procesos(0, 0, waitQueue, runningQueue, finishedQueue, qstart-tiempoRestanteEjecucion, qdelta, qmin, input_file, linea, argumento, nuevo_PID, PID, GID, T_SO, true);

        } else {
            printf("END %d TIME %d\n", PID, T_SO);
            Tiempo_SO = T_SO;
            argumento++;
            dequeueLast(&runningQueue);
            enqueue(&finishedQueue, nuevoProceso);
            if (estaVacia(&runningQueue) && !estaVacia(&waitQueue)) {
                Process ultimoProceso = dequeueLast(&waitQueue);
                printf("RESUME %d\n", ultimoProceso.PID);
                int CF = atoi(input_file->lines[linea][argumento]);
                ultimoProceso.CF = CF;
                enqueue(&runningQueue, ultimoProceso);
                simulacion_procesos(ultimoProceso.CF, 0, waitQueue, runningQueue, finishedQueue, qstart, qdelta, qmin, input_file, linea, argumento, ultimoProceso.PID, ultimoProceso.PID, ultimoProceso.PID, T_SO, false);
            }
        }
        freeQueue(&runningQueue);
        freeQueue(&waitQueue);
        freeQueue(&finishedQueue);
    }
}



int max(int a, int b) {
    return (a > b) ? a : b;
}