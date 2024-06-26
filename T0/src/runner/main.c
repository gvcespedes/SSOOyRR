#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../file_manager/manager.h"
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>

int signal_received = 0; // indica si se recibió una señal ctrl + z
int amount_pr; // guarda el valor amount entregado en consola
int max_pr; // guarda el valor max entregado en consola

typedef struct  Process
{
	int pid;
	double begin; // tiempo (clocks) en el que empezó a correr el proceso
	int time; // tiempo que corrió el proceso, en segundos 
	int exit_code;
	char* path; // path del proceso
	int cant_args; // cantidad de argumentos que recibirá el proceso
	char** argv; // listado de argumentos que recibe el proceso
	int status;
	int is_fork_first; // 0 si no ha hecho fork antes
	int is_finished; // 1 si terminó de ejecutarse
	int is_max_time; // 0 en caso de que no se cumple el time max; 1 proceso tiene 10 segundos terminar
	int timeout; // para caso de wait_all, indica el tiempo que dbee esperar antes de hacer sigkill
	int wait_ejecutado; // para caso de wait_all, indica si este ya fue ejecutado
	int interrupted; //1 si fue interrumpido por alguna condición
} Process;

void sighandler(int signalNumber) {
	signal_received = 1;
	printf("Received signal: %d\n", signalNumber);
}

int main(int argc, char const *argv[])
{
	/*Rescato valores de consola*/
	int amount_pr = atoi(argv[3]);
	if (argc == 5){
		max_pr = atoi(argv[4]);
	}
	printf("soy el amount %d\n", amount_pr);
	printf("soy el max %d\n", max_pr);
	
	/*Lectura del input*/
	char *file_name = (char *)argv[1];
	InputFile *input_file = read_file(file_name);
	// hacer el output

	/*Mostramos el archivo de input en consola*/

	printf("Cantidad de lineas: %d\n", input_file->len);
	Process* process_array[input_file->len];
	for (int i = 0; i < input_file->len; ++i)
	{
		Process* p = calloc(1, sizeof(Process));
		p->path = input_file->lines[i][1];
		p->argv = &(input_file->lines[i][2]);
		p->is_finished = 0;
		p->is_fork_first = 0;
		p->interrupted = 0;
		p->cant_args = atoi(input_file->lines[i][0]); //cant. argumentos que tiene el proceso
		if (p->cant_args == -1){
			p->timeout = atoi(input_file->lines[i][2]);
			p->wait_ejecutado = 0;
		}
		process_array[i] = p;
	}

	int signal_just_received = 0;
	int max_alcanzado = 0;
	int progs_running = 0;
	double timeout_inicio;
	double time_since_max;
	clock_t start_runner = clock();
	double signal_clock;
	int loop_largo = 1;
	int contador_finished = 0;
	int wait_iniciado = 0;
	int tiempo_timeout = 0;
	while (loop_largo == 1) {

		/*manejo de señal ctrl + z*/
		signal(SIGTSTP, sighandler); //recibe señal ctrl + z
		if (signal_received == 1 && signal_just_received == 0){
			signal_just_received = 1;
			signal_clock = clock();
			for (int l = 0; l < input_file->len; ++l){
				if (process_array[l]->cant_args > -1 && process_array[l]->is_fork_first == 1 && process_array[l]->is_finished == 0 && process_array[l]->is_max_time == 0){
					process_array[l]->is_max_time = 1;
					process_array[l]->interrupted = 1;
					kill(process_array[l]->pid, SIGINT);
				}
			}
		}
		if (signal_just_received == 1 && (clock() - signal_clock)/CLOCKS_PER_SEC >= 10){
			for (int l = 0; l < input_file->len; ++l){
				if (process_array[l]->cant_args > -1 && process_array[l]->is_fork_first == 1 && process_array[l]->is_finished == 0 && process_array[l]->is_max_time == 1){
					process_array[l]->interrupted = 1;
					kill(process_array[l]->pid, SIGKILL);
				}
			}
		}
		
		/*manejo de arg max en programas corriendo*/
		if (max_pr > 0 && max_pr <= ((double)clock() - start_runner)/CLOCKS_PER_SEC && max_alcanzado == 0){
			max_alcanzado = 1;
			wait_iniciado = 0;
			time_since_max = clock();
			for (int l = 0; l < input_file->len; ++l){
				if (process_array[l]->cant_args > -1 && process_array[l]->is_fork_first == 1 && process_array[l]->is_finished == 0 && process_array[l]->is_max_time == 0){
					process_array[l]->is_max_time = 1;
					process_array[l]->interrupted = 1;
					kill(process_array[l]->pid, SIGINT);
				}
			}
		}
		if (max_alcanzado == 1 && ((double)clock() - time_since_max)/CLOCKS_PER_SEC >= 10){
			for (int l = 0; l < input_file->len; ++l){
				if (process_array[l]->cant_args > -1 && process_array[l]->is_max_time == 1 && process_array[l]->is_finished == 0){
					process_array[l]->interrupted = 1;
					kill(process_array[l]->pid, SIGTERM);
				}
			}
		}

		/*manejo de wait*/
		if (wait_iniciado == 1 && ((double)clock() - timeout_inicio)/CLOCKS_PER_SEC >= tiempo_timeout){
			for (int l = 0; l < input_file->len; ++l){
				if (process_array[l]->cant_args > -1 && process_array[l]->is_fork_first == 1 && process_array[l]->is_finished == 0 && process_array[l]->is_max_time == 0){
					process_array[l]->interrupted = 1;
					kill(process_array[l]->pid, SIGKILL);
				}
			}
			wait_iniciado = 0;
		}		

	 	for (int k = 0; k < input_file->len; ++k){
			if (process_array[k]->cant_args == -1 && process_array[k]->wait_ejecutado == 0 && wait_iniciado == 0){
				process_array[k]->wait_ejecutado = 1;
				wait_iniciado = 1;
				timeout_inicio = clock();
				tiempo_timeout = process_array[k]->timeout;
				contador_finished ++;//moverlo a cuando termine el wait
			}
			if (process_array[k]->cant_args > -1 && process_array[k]->is_fork_first == 0 && progs_running < amount_pr && wait_iniciado == 0){
				progs_running ++;
				pid_t pid = fork();
				process_array[k]->begin = (double)clock(); //tiempo en que inicia el child
				process_array[k]->is_fork_first = 1; //marca que ya hizo fork
				if (pid == 0){ //si es el hijo, córrelo
					execve(process_array[k]->path, process_array[k]->argv, NULL);
				}
				else { //si es el padre, guarda el pid
					process_array[k]->pid = pid;
				}
			} 
			if (process_array[k]->cant_args > -1 && process_array[k]->is_fork_first == 1 && process_array[k]->is_finished == 0) {
				int status;
				int wait_result = waitpid(process_array[k]->pid, &status, WNOHANG);
				if (wait_result > 0) {
					process_array[k]->status = status;
					process_array[k]->exit_code = WEXITSTATUS(status);
					process_array[k]->time = round(((double)clock() - process_array[k]->begin)/CLOCKS_PER_SEC);
					process_array[k]->is_finished = 1;
					contador_finished ++;
					progs_running --;
				}
			}
			if (contador_finished == input_file->len) {
				loop_largo = 0;
				for (int l = 0; l < input_file->len; ++l){
					if (process_array[l]->cant_args > -1){
						if (process_array[l]->interrupted == 1){
							process_array[l]->exit_code = process_array[l]->status;
						}
					}
				}
				break;
			}
	 	}
	}

	// Output file
	FILE *output_file = fopen("output.csv", "w");
	for (int l = 0; l < input_file->len; ++l){
		if (process_array[l]->cant_args > -1){
			fprintf(output_file,"%s,%d,%d\n", process_array[l]->path, process_array[l]->time, process_array[l]->exit_code);
		}
	}

	for (int i = 0; i < input_file->len; ++i){
		free(process_array[i]);
	}

	input_file_destroy(input_file);
	fclose(output_file);
}
