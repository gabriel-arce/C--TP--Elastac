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


void cargarConfiguracion(){

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
	nucleo->stack_size					= getIntProperty(config, "STACK_SIZE");

	config_destroy(config);

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


t_pcb *crearPCB(char *programa){
	t_pcb *pcb = malloc(sizeof(t_pcb));

	const char* PROGRAMA = "#!/usr/bin/ansisop \n begin \n variables a, b, c \n  a = b + 12 \n print b \n textPrint foo\n end";

	//Obtener metadata del programa
	t_metadata_program* metadata = malloc(sizeof(t_metadata_program));
	metadata = metadata_desde_literal(PROGRAMA);

	pcb->pcb_pid	= crearPCBID();
	pcb->pcb_pc	= metadata->instruccion_inicio;
	pcb->pcb_sp	= 0;
	pcb->indice_etiquetas = metadata->etiquetas;
	pcb->paginas_codigo = nucleo->stack_size;
	pcb->indice_codigo.posicion	= metadata->instrucciones_serializado[0].start;
	pcb->indice_codigo.tamanio	= metadata->instrucciones_serializado[0].offset ;
	pcb->indice_stack.args = NULL;
	pcb->indice_stack.retPos = 0;
	pcb->indice_stack.retVar = 0;
	pcb->quantum = 0;

	printf("PCB creado..\n");
	printf("PID: %d\n", pcb->pcb_pid);
	printf("PC: %d\n", pcb->pcb_pc);
	printf("SP: %d\n", pcb->pcb_sp);
	printf("Indice Etiquetas: %s\n", pcb->indice_etiquetas);
	printf("Paginas de Codigo: %d\n", pcb->paginas_codigo);
	printf("Indice de Codigo: %d, %d\n", pcb->indice_codigo.posicion, pcb->indice_codigo.tamanio);

	return pcb;
}

int crearPCBID(){
	if(queue_is_empty(cola_listos))
		return 1;
	return queue_size(cola_listos) + 1;
}

void crearSemaforos(){
	mutexListos				= crearMutex();
	mutexCPU					= crearMutex();
	mutexEjecutando		= crearMutex();
	semListos						= crearSemaforo(0);
	semCpuDisponible	= crearSemaforo(0);
	semBloqueados			= crearSemaforo(0);
}

void destruirPCB(t_pcb *pcb){
	free(pcb);
}

void crearListasYColas(){

	cola_listos 				= queue_create();
	cola_bloqueados	= queue_create();
//	cola_ejecutando	= queue_create();


	lista_ejecutando	=	list_create();
	lista_cpu					= list_create();
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

void salirPor(const char *msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

void crearServerNucleo(){
	 int listener;														//Descriptor de escucha
	 int i;
	 int maximo;													// Número de descriptor maximo
	 int newfd;
	 int nbytes;
	 int reuse;
	 char buffer[MAXIMO_BUFFER];
	 sigset_t * mask, orig_mask;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	//Crear socket de escucha
	listener = crearSocket();

	//descriptor para enlace
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1)
		salirPor("[NUCLEO] No es posible reusar el socket\n");

	//Enlazar
	bindearSocket(listener, PUERTO_NUCLEO);

	//Escuchar
	escucharEn(listener);

	//Nos aseguramos de que pselect no sea interrumpido por señales
	sigemptyset(&mask);
	sigaddset (&mask, SIGUSR1);
	sigaddset (&mask, SIGUSR2);
	sigaddset (&mask, SIGPOLL);

	if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0)
		salirPor("[NUCLEO] Fallo en sigprcomask");

	// añadir listener al conjunto maestro
	FD_SET(listener, &master);

	maximo = listener; // por ahora es éste

	while(1){

		read_fds = master; 								// cópialo

		if (pselect(maximo+1, &read_fds, NULL, NULL, NULL, NULL) == -1)
			salirPor("select");

		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= maximo; i++){

			if (FD_ISSET(i, &read_fds)){
				// ¡¡tenemos datos!!

				if (i == listener){

					if ((newfd = aceptarEntrantes(listener)) == -1)						// gestionar nuevas conexiones
						salirPor("accept");

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
							salirPor("recv");

						close(i); 																						// ¡Hasta luego!
						FD_CLR(i, &master); 																	// eliminar del conjunto maestro

					} else {
					// tenemos datos de algún cliente
						if (FD_ISSET(i, &master))	{

							char *estructura = string_new();
							string_append(&estructura,string_itoa(buffer[0]));	//por identificador del proceso
							string_append(&estructura,SERIALIZADOR);
							string_append(&estructura,buffer);

							pthread_create(&pIDProcesarMensaje, NULL, (void *)hiloProcesarMensaje, (void *)estructura);
							memset(buffer,'\0',sizeof(buffer));

							//Crear socket al UMC
							//socketNucleo = clienteDelServidor(nucleo->ip_umc, nucleo->puerto_umc);
							//Enviar
							//enviarPorSocket(socketNucleo, serializarPCB(pcb_aux));

						}
					}
				}
			}
		}
	}
}

void hiloProcesarMensaje(char *datos){
	char **estructura	= string_split(datos,"##");
	char *buffer 			= string_new();

	string_append(&buffer,estructura[1]);

	int fileDescriptor = atoi(estructura[0]);
	procesarMensaje(fileDescriptor,buffer);

}

void procesarMensaje(int fd, char *buffer){

	t_pcb *pcb_aux;

	switch(fd){
		case CONSOLA:  {
			printf("Consola says.. %s\n", buffer);

			//Crear PCB por consola entrante
			printf("Creando PCB.. \n");
			pcb_aux = malloc(sizeof(t_pcb));
			pcb_aux = crearPCB(buffer);

			//Agregar PCB a la cola de listos
			queue_push(cola_listos, pcb_aux);

			break;
			}
		case CPU: {
			printf("CPU says.. %s\n", buffer);

			//Crea una CPU
			t_clienteCPU *nuevaCPU = malloc(sizeof(t_clienteCPU));
			nuevaCPU->cpuID = 1;
			nuevaCPU->fd = fd;
			nuevaCPU->disponible = 0;

			//Agregar CPU a la lista
			list_add(lista_cpu, nuevaCPU);

			//Signal por CPU nueva
			signalSemaforo(semCpuDisponible);

			break;
			}
	}

	//Planifico
	pthread_create(&pIDPlanificador, NULL, (void *)planificar_consolas, NULL);

}

void crearClienteUMC(){
	if((socketNucleo = clienteDelServidor(nucleo->ip_umc, nucleo->puerto_umc)) == -1)
		salirPor("[NUCLEO] No pudo conectarse al swap");
}

void planificar_consolas(){
	//Si hay al menos una CPU, que planifique
	waitSemaforo(semCpuDisponible);

	//Crear los hilos para las colas de ejecucion y bloqueados
	crearHilosColas();

}

void crearHilosColas(){
	pthread_create(&hiloEjecucion, NULL, (void *)mainEjecucion, NULL);
	pthread_create(&hiloBloqueado, NULL, (void *)mainBloqueado, NULL);
}

void mainEjecucion(){
	while(1){
		waitSemaforo(semListos);
		//waitSemaforo(semCpuDisponible);
		pasarAEjecutar();
	}
}

void mainBloqueado(){
	while(1){
		waitSemaforo(semBloqueados);
		entradaSalida();
	}
}


t_clienteCPU *obtenerCPUDisponible(){

	waitSemaforo(mutexCPU);
	t_clienteCPU *cpuDisponible = list_find(lista_cpu, (void *)CPUestaDisponible);
	signalSemaforo(mutexCPU);

	return cpuDisponible;
}

int CPUestaDisponible(t_clienteCPU *cpu){
	return cpu->disponible == 0;
}

void enviarAEjecutar(t_pcb *pcb, int fd){
	enviarPorSocket(fd, serializarPCB(pcb));			//Enviar al CPU
	pcb->quantum += 1;													//Incrementar quantum
	usleep(nucleo->quantum_sleep);						//Esperar para la ejecucion de la proxima instruccion;
}

void entradaSalida(){
	//Obtener el elemento por encima de la cola
	t_pcb *pcbBloqueado = (t_pcb *)queue_peek(cola_bloqueados);

	usleep(nucleo->io_sleep);

	//Sacar de la cola de bloqueados
	queue_pop(cola_bloqueados);

	//Pasar a la cola de Listos
	pasarAListos(pcbBloqueado);

}

void pasarAEjecutar(){
	t_pcb *pcbEjecutando;

	waitSemaforo(mutexListos);

	//Obtener una CPU disponible para procesar
	t_clienteCPU *cpuDeEjecucion = obtenerCPUDisponible();

	if(cpuDeEjecucion != NULL){

		//Se obtiene el PCB para la transicion
		pcbEjecutando = (t_pcb *) queue_pop(cola_listos);

		//Mientras tenga quantum
		while(pcbEjecutando->quantum != nucleo->quantum){
			//Enviar a Ejecutar
			waitSemaforo(mutexEjecutando);
			enviarAEjecutar(pcbEjecutando, cpuDeEjecucion->fd);
			signalSemaforo(mutexEjecutando);
		}

		if(pcbEjecutando->quantum == nucleo->quantum){
			pcbEjecutando->quantum = 0;						//Resetear quantum del pcb
			sacarDeEjecutar(pcbEjecutando);				//Sacar de la lista de Ejecutando
			pasarAListos(pcbEjecutando);						//Pasar a la cola de Listos
		}


	}

	signalSemaforo(mutexListos);
}


void pasarAListos(t_pcb *pcb){

	//Agrega el PCB a la cola de Listos
	waitSemaforo(mutexListos);
	queue_push(cola_listos, (void *) pcb);
	signalSemaforo(mutexListos);

	//Incrementa la cantidad de pcb
	signalSemaforo(semListos);

}

void crearServerConsola(){
	 int listener;														//Descriptor de escucha
	 int nbytes;
	 int reuse;
	 int newfd;
	 char buffer[MAXIMO_BUFFER];
	t_pcb *pcb_aux;

	//Crear socket de escucha
	listener = crearSocket();

	//descriptor para enlace
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1)
		salirPor("[NUCLEO] No es posible reusar el socket\n");

	//Enlazar
	bindearSocket(listener, nucleo->puerto_programas);

	//Escuchar
	escucharEn(listener);

	while(1){

		if ((newfd = aceptarEntrantes(listener)) == -1)
			salirPor("accept");

		if ((nbytes = recv(newfd, buffer, sizeof(buffer), 0)) < 0){
			printf("[NUCLEO] No se pudo recibir informacion desde el socket %d\n", listener);
			}
		if (nbytes == 0){
				printf("[NUCLEO] Conexion con socket de consola nro. %d cerrada.\n", listener);
		} else {
			//Crear PCB por consola entrante
			printf("Creando PCB.. \n");
			pcb_aux = malloc(sizeof(t_pcb));
			pcb_aux = crearPCB(buffer);

			//Agregar PCB a la cola de listos
			queue_push(cola_listos, pcb_aux);
			}

	}
}

void crearServerCPU(){
	 int listener;														//Descriptor de escucha
	 int reuse;
	 int newfd;

	//Crear socket de escucha
	listener = crearSocket();

	//descriptor para enlace
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1)
		salirPor("[NUCLEO] No es posible reusar el socket\n");

	//Enlazar
	bindearSocket(listener, nucleo->puerto_cpu);

	//Escuchar
	escucharEn(listener);

	while(1){

		if ((newfd = aceptarEntrantes(listener)) == -1)
			salirPor("accept");

		//Crea una CPU
		t_clienteCPU *nuevaCPU = malloc(sizeof(t_clienteCPU));
		nuevaCPU->cpuID				= 1;
		nuevaCPU->fd 					= newfd;
		nuevaCPU->disponible	= 0;

		//Agregar CPU a la lista
		list_add(lista_cpu, nuevaCPU);

		//Signal por CPU nueva
		signalSemaforo(semCpuDisponible);

	}
}

void destruirSemaforos(){
	destruirSemaforo(mutexCPU);
	destruirSemaforo(mutexEjecutando);
	destruirSemaforo(mutexListos);
	destruirSemaforo(semBloqueados);
	destruirSemaforo(semListos);
	destruirSemaforo(semCpuDisponible);
}

void sacarDeEjecutar(t_pcb *pcb){
	waitSemaforo(mutexEjecutando);

	signalSemaforo(mutexEjecutando);
}
