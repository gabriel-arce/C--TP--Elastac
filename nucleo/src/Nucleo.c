/*
 ============================================================================
 Name        : Nucleo.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Nucleo.h"


void *cargar_conf(){

	config = config_create(CONFIG_NUCLEO);
	nucleo = malloc(sizeof(t_nucleo));

	printf("Inicializando Nucleo..\n");
	printf("please wait..\n");
	printf("\n");

// Inicializar estructura
	nucleo->puerto_programas = 0;
	nucleo->puerto_cpu = 0;
	nucleo->quantum = 0;
	nucleo->quantum_sleep = 0;

	nucleo->puerto_programas	= getIntProperty(config, "PUERTO_PROG");
	nucleo->puerto_cpu					= getIntProperty(config, "PUERTO_CPU");
	nucleo->puerto_umc				= getIntProperty(config, "PUERTO_UMC");
	nucleo->ip_umc							= getStringProperty(config, "IP_UMC");
	nucleo->quantum						= getIntProperty(config, "QUANTUM");
	nucleo->quantum_sleep			= getIntProperty(config, "QUANTUM_SLEEP");
	nucleo->sem_ids						= getListProperty(config, "SEM_IDS");
	nucleo->sem_init						= getListProperty(config, "SEM_INIT");
	nucleo->io_ids							= getListProperty(config, "IO_IDS");
	nucleo->io_sleep						= getListProperty(config, "IO_SLEEP");
	nucleo->shared_vars				= getListProperty(config, "SHARED_VARS");


	config_destroy(config);

}

int get_quantum(t_nucleo *nucleo){
	return nucleo->quantum;
}

int get_quantum_sleep(t_nucleo *nucleo){
	return nucleo->quantum_sleep;
}

/*
 * Busca en array todas las posiciones con -1 y las elimina, copiando encima
 * las posiciones siguientes.
 * Ejemplo, si la entrada es (3, -1, 2, -1, 4) con *n=5
 * a la salida tendremos (3, 2, 4) con *n=3
 */
void compactaClaves (int *tabla, int *n)
{
	int i,j;

	if ((tabla == NULL) || ((*n) == 0))
		return;

	j=0;
	for (i=0; i<(*n); i++)
	{
		if (tabla[i] != -1)
		{
			tabla[j] = tabla[i];
			j++;
		}
	}

	*n = j;
}


/*
 * Función que devuelve el valor máximo en la tabla.
 * Supone que los valores válidos de la tabla son positivos y mayores que 0.
 * Devuelve 0 si n es 0 o la tabla es NULL */
int dameMaximo (int *tabla, int n)
{
	int i;
	int max;

	if ((tabla == NULL) || (n<1))
		return 0;

	max = tabla[0];
	for (i=0; i<n; i++)
		if (tabla[i] > max)
			max = tabla[i];

	return max;
}



void escuchar_procesos(){
	 int listener;														//Descriptor de escucha
	 int i;
	 int maximo;													// Número de descriptor maximo
	 int newfd;
	 int nbytes;
	 int reuse;
	 char buffer[MAXIMO_BUFFER];
	 char tipoProceso;
	 char buff[MAXIMO_BUFFER];
	 t_pcb *pcb_aux;

	 fd_set master;				// conjunto maestro de descriptores de fichero
	 fd_set read_fds;			// conjunto temporal de descriptores de fichero para select()

	//Crear socket de escucha
	if( (listener = crearSocket()) == -1 ){
		perror("Nucleo: Error al abrir socket de espera\n");
		exit(1);
	}


	//descriptor para enlace
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1){
		perror("Nucleo: No es posible reusar el socket\n");
		exit(1);
	}

	//Enlazar
	if((bindearSocket(listener, PUERTO_NUCLEO)) == -1){
		perror("Nucleo: No se puede enlazar al puerto: direccion ya esta en uso\n");
		exit(1);
	}

	//Escuchar
	if((escucharEn(listener)) == -1){
		exit(1);
	}

	// añadir listener al conjunto maestro
	FD_SET(listener, &master);

	maximo = listener; // por ahora es éste

	crear_listas();		//Crear las listas

	while(1){

		memset(buff,'\0',sizeof(buff));		//Limpiar buffer
		read_fds = master; 								// cópialo

		if (select(maximo+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(1);
		}

		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= maximo; i++){

			if (FD_ISSET(i, &read_fds)){
				// ¡¡tenemos datos!!

				if (i == listener){

					if ((newfd = aceptarEntrantes(listener)) == -1)						// gestionar nuevas conexiones
						perror("accept");
					else {
						FD_SET(newfd, &master);														// añadir al conjunto maestro
						if (newfd > maximo)
							maximo = newfd;																	// actualizar el máximo
					}

				} else	{

					if ((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0){			// gestionar datos de un cliente
						// error o conexión cerrada por el cliente
						if (nbytes == 0)																			// conexión cerrada
							printf("[NUCLEO] socket %d desconectado\n", i);
						else
							perror("recv");

						close(i); 																						// ¡Hasta luego!
						FD_CLR(i, &master); 																	// eliminar del conjunto maestro

					} else {
					// tenemos datos de algún cliente
						if (FD_ISSET(i, &master))
						{

							strcpy(buff,buffer);
							printf("%s\n", buffer);

							char *estructura = string_new();
							string_append(&estructura,string_itoa(i));
							string_append(&estructura,"##");
							string_append(&estructura,buff);


							//Crear PCB por consola entrante
							printf("Creando PCB.. \n");
							pcb_aux = malloc(sizeof(t_pcb));
							pcb_aux = crear_pcb();
							printf("PCB creado..\n");
							printf("PID: %d\n", pcb_aux->pcb_pid);
							printf("PC: %d\n", pcb_aux->pcb_pc);
							printf("SP: %d\n", pcb_aux->pcb_sp);
							printf("Indice Etiquetas: %d\n", pcb_aux->indice_etiquetas);
							printf("Paginas de Codigo: %d\n", pcb_aux->paginas_codigo);
							printf("Indice de Codigo: %s, %s\n", pcb_aux->indice_codigo.posicion, pcb_aux->indice_codigo.tamanio);

							//Crear socket al UMC
							socketNucleo = clienteDelServidor(nucleo->ip_umc, nucleo->puerto_umc);

							//Enviar
							enviarPorSocket(socketNucleo, serializarPCB(pcb_aux));

						}
					}
				}
			}
		}
	}
}

void planificar_procesos(){
	crear_semaforos();
	crear_listas();
}

t_pcb *crear_pcb(){
	t_pcb *pcb = malloc(sizeof(t_pcb));

	pcb->pcb_pid	= crear_id();
	pcb->pcb_pc	= 0;
	pcb->pcb_sp	= 0;
	pcb->indice_etiquetas = 0;
	pcb->paginas_codigo = 0;
	pcb->indice_codigo.posicion = 0;
	pcb->indice_codigo.tamanio = 0;

	return pcb;
}

int crear_id(){
//	waitSemaforo(mutex);

	if( list_is_empty(lista_pcb))
		return 1;
	return list_size(lista_pcb) + 1;

//	signalSemaforo(mutex);

}

void crear_semaforos(){
	mutex = crearMutex();
}

void destruir_pcb(t_pcb *pcb){
	free(pcb);
}

void crear_listas(){
/*
	cola_pcb 					= queue_create();
	cola_listos 				= queue_create();
	cola_bloqueados	= queue_create();
	cola_ejecutando	= queue_create();
*/

	lista_pcb					=	list_create();
	lista_listos				=	list_create();
	lista_bloqueados	=	list_create();
	lista_ejecutando	=	list_create();

}

char* serializarPCB (t_pcb* pcb)
{
	char* serial = string_new();
	string_append(&serial,"0");											//Tipo de Proceso
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_pid));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_pc));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_sp));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->paginas_codigo));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->indice_etiquetas));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->indice_codigo.posicion));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->indice_codigo.tamanio));

	return serial;
}
