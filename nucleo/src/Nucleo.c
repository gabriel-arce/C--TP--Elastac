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
#include <elestac_inotify.h>

int cpuID = 0;						//Numero de CPU

void cargarConfiguracion(char ** config_path){


	config = config_create(config_path[1]);
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

	//Crear hilo unicamente para notificaciones
	pthread_create(&pIDInotify, NULL, (void *)crearObserverConfiguracion, config_path[1]);

}

void crearObserverConfiguracion(char *ruta){
	int fd;
	int observer;

	if ((fd = inotify_init()) < 0)
		puts("[INOTIFY] Error al iniciar Inotify");

	if ((observer = inotify_add_watch(fd, ruta, IN_MODIFY | IN_CREATE | IN_DELETE)) < 0)
		puts("[INOTIFY] Error al crear observer");

	while(1){
		inotify_getEvent(fd, observer);
	}


	/*
		inotify_rm_watch(fd, observer);
		close(fd);
	*/
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

void crearSemaforos(){
	mutexListos				= crearMutex();
	mutexCPU					= crearMutex();
	mutexEjecutando		= crearMutex();
	mutexConsolas			= crearMutex();
	mutexFinalizados		=crearMutex();
	semListos						= crearSemaforo(0);
	semCpuDisponible	= crearSemaforo(0);
	semBloqueados			= crearSemaforo(0);
	semFinalizados			= crearSemaforo(0);
	mutex_pid					= crearMutex();

}

/*void destruirPCB(t_pcb *pcb){
	free(pcb->indice_etiquetas);
	free(pcb->indice_stack.args);
	free(pcb);
}*/

void crearListasYColas(){

	cola_listos 				= queue_create();

	lista_bloqueados		=	list_create();
	lista_ejecutando		=	list_create();
	lista_cpu						=	list_create();
	lista_semaforos			=	list_create();
	lista_io							=	list_create();
	lista_finalizados			=	list_create();
	lista_sharedValues	=	list_create();

	semaforos = list_create();
	dispositivos = list_create();

	//Recorro del config los semaforos
	int i;
	for(i = 0; i < list_size(nucleo->sem_ids); i++){
		char *semaforo = list_get(nucleo->sem_ids, i);
		char * valor_char = list_get(nucleo->sem_init, i);
		int valor = atoi(valor_char);


		list_add(lista_semaforos, crearSemaforoGlobal(semaforo,valor,0));
		/*t_semaforo * s = malloc(sizeof(t_semaforo));
		s->id = string_duplicate(semaforo);
		s->sem = crearSemaforo(valor);
		s->bloqueados = queue_create();
		list_add(semaforos, (void *) s);
	*/
	}

	//Recorro del config los dispositivos
	for(i = 0; i < list_size(nucleo->io_ids); i++){
		char * io_nombre	= list_get(nucleo->io_ids, i);
		char * io_sleep_char  = list_get(nucleo->io_sleep, i);
		int io_sleep = atoi(io_sleep_char);

		t_dispositivo * d = malloc(sizeof(t_dispositivo));
		d->id = string_duplicate(io_nombre);
		d->io_sleep = io_sleep;
		pthread_mutex_init(&(d->mutex_disp), 0);
		d->bloqueados = queue_create();
		list_add(dispositivos, (void *) d);
	}

	//Recorro del config las shared vars
	for(i = 0; i < list_size(nucleo->shared_vars); i++){
		char * sharedNombre = list_get(nucleo->shared_vars, i);
		list_add(lista_sharedValues, crearSharedGlobal(sharedNombre));
		printf("Shared: %s\n", sharedNombre);
	}

}

void crearClienteUMC(){
	if((socketUMC = clienteDelServidor(nucleo->ip_umc, nucleo->puerto_umc)) == -1)
			salirPor("[NUCLEO] No pudo conectarse a UMC");

	enviar_handshake(socketUMC, NUCLEO);

	t_header * head = recibir_header(socketUMC);

	if (head->identificador == Tamanio_pagina)
		tamanio_pagina = head->tamanio;
}

void planificar_consolas(){

	//Crear los hilos para las colas de ejecucion y bloqueados
	pthread_create(&hiloEjecucion, NULL, (void *)mainEjecucion, NULL);

	//pthread_create(&hiloBloqueado, NULL, (void *)mainBloqueado, NULL);

	pthread_join(hiloEjecucion, NULL);

	//pthread_join(hiloBloqueado, NULL);
}

void mainEjecucion(){
	while(1){

		//Atrapar senial por cambio de quantum
		//signal(SIGINT, interrupcionConsola);

		waitSemaforo(semListos);							//Si hay consolas
		waitSemaforo(semCpuDisponible);		//Si hay al menos una CPU, que planifique
			puts("Pasando a ejecutar consola..");
			pasarAEjecutar();
			//finalizar();
		//}
	}
}

/*void mainBloqueado(){
	while(1){
		waitSemaforo(semBloqueados);
		entradaSalida();
		signalSemaforo(semBloqueados);
	}
}
*/

t_clienteCPU *obtenerCPUDisponible(){

	waitSemaforo(mutexCPU);
	t_clienteCPU *cpuDisponible = list_find(lista_cpu, (void *)CPUestaDisponible);
	signalSemaforo(mutexCPU);

	return cpuDisponible;
}

int CPUestaDisponible(t_clienteCPU *cpu){
	return cpu->disponible == Si;
}

void enviarAEjecutar(t_pcb *pcb, t_clienteCPU *cpu){

	int tamanio  =calcular_size_pcb(pcb);

	enviar_header(20, tamanio, cpu->fd);
	enviar_pcb(pcb, cpu->fd);

}

/*void entradaSalida(){
	//Obtener el elemento por encima de la cola
	t_pcb *pcbBloqueado = (t_pcb *)queue_peek(cola_bloqueados);

	//Buscar el dispositivo

	usleep(nucleo->io_sleep);

	//Sacar de la cola de bloqueados
	queue_pop(cola_bloqueados);

	//Pasar a la cola de Listos
	pasarAListos(pcbBloqueado);

}
*/
void pasarAEjecutar(){
	t_pcb *pcbEjecutando;

	waitSemaforo(mutexListos);

	//Obtener una CPU disponible para procesar
	t_clienteCPU *cpuDeEjecucion = obtenerCPUDisponible();

	cpuDeEjecucion->disponible = No;

	if(cpuDeEjecucion != NULL){

		//Se obtiene el PCB para la transicion
		pcbEjecutando = (t_pcb *) queue_pop(cola_listos);

		//2da version
		//Enviar a Ejecutar
		waitSemaforo(mutexEjecutando);
		puts("Ejecutando consola..");
		enviarAEjecutar(pcbEjecutando, cpuDeEjecucion);
		signalSemaforo(mutexEjecutando);
	}

	signalSemaforo(mutexListos);
}


void pasarAListos(t_pcb *pcb){

	//Agrega el PCB a la cola de Listos
	waitSemaforo(mutexListos);
	printf("\nPasando a listos el pcb con pid: %d\n", pcb->pcb_pid);
	queue_push(cola_listos, (void *) pcb);
	signalSemaforo(mutexListos);

	//Incrementa la cantidad de pcb
	signalSemaforo(semListos);

}

void crearServerConsola(){
	 int listener = -1;														//Descriptor de escucha
	 int reuse = 1;
	 int newfd = -1;

	//Crear socket de escucha
	listener = crearSocket();
	if (listener == -1)
		exit(EXIT_FAILURE);

	//descriptor para enlace
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1)
		salirPor("[NUCLEO] No es posible reusar el socket\n");

	//Enlazar
	bindearSocket(listener, nucleo->puerto_programas);

	//Escuchar
	escucharEn(listener);

	while(1){

		waitSemaforo(mutexConsolas);

		if ((newfd = aceptarEntrantes(listener)) == -1)
			salirPor("accept");

		setsockopt(newfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

		//Recibir Handshake de Consola
		if (recibir_handshake(newfd) != CONSOLA)
			salirPor("Proceso desconocido por puerto de Consola");

	    puts("[NUCLEO] Recepcion hanshake Consola" );

	    pthread_t initProgramThread;
	    pthread_create(&initProgramThread, NULL, (void *) inicializar_programa, (void *) newfd);
	    //JOIN??
		signalSemaforo(mutexConsolas);

	}
}

void inicializar_programa(void * fd) {
	int socket_consola = (int) fd;
	int paginas_necesarias = 0;

	//recibo el header de iniciar_ansisop
	t_header * header = recibir_header(socket_consola);

	//recibo el programa
	void * buffer = malloc(header->tamanio);
	recv(socket_consola, buffer, header->tamanio, 0);
	t_paquete_programa * paquete_programa = deserializar_ansisop(buffer);

	//mando a iniciar a UMC
	paginas_necesarias = calcular_cantidad_paginas(paquete_programa->programa_length);
	int pid =generar_pid();
	enviar_header(Inicializar_programa, 12 + paquete_programa->programa_length, socketUMC);
	enviar_inicializar_programa(pid, paginas_necesarias, paquete_programa->codigo_programa, socketUMC);

	//espero la respuesta
	int respuesta = recibir_respuesta_inicio(socketUMC);

	if (respuesta){
		//si es SI  --> creo el pcb
		//			--> mando el pcb a LISTOS
		t_pcb * new_pcb = crearPCB(paquete_programa->codigo_programa, pid, socket_consola, tamanio_pagina);
//		imprimir_pcb(new_pcb);
//		printf("\n\n");
		pasarAListos(new_pcb);
	} else {
		//si es NO -> finalizo la consola
		enviar_header(Fin_programa, 0, socket_consola);
	}

	//se muere el hilo?
	//por ahi lo necesito vivo por si se muere consola para que me avise que se re pudrio
}

int calcular_cantidad_paginas(int codigo_length) {
	int paginas_codigo = 0;
	int paginas_stack = 0;
	int paginas_totales = 0;

	paginas_codigo = codigo_length / tamanio_pagina;
	if ((codigo_length % tamanio_pagina) > 0)
		paginas_codigo++;

	paginas_stack = nucleo->stack_size / tamanio_pagina;
	if ((nucleo->stack_size % tamanio_pagina) > 0)
		paginas_stack++;

	paginas_totales = paginas_codigo + paginas_stack;

	return paginas_totales;
}

int generar_pid() {
	int pid;
	waitSemaforo(mutex_pid);
		pid_global++;
		pid = pid_global;
	signalSemaforo(mutex_pid);

	return pid;
}

void crearServerCPU(){
	 int listener;														//Descriptor de escucha
	 int reuse;
	 int newfd;
	 int hiloCPU;
	 void *respuestaHilo;

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

		if (recibir_handshake(newfd) != CPU)
			puts("Proceso desconocido por puerto de CPU");

	    puts("[NUCLEO] Recepcion hanshake CPU" );

		//Crea una CPU
		t_clienteCPU *nuevaCPU = malloc(sizeof(t_clienteCPU));
		nuevaCPU->cpuID				= obtenerCPUID();
		nuevaCPU->fd 					= newfd;
		nuevaCPU->disponible	= Si;

		//Agregar CPU a la lista
		list_add(lista_cpu, nuevaCPU);



		//Crear hilo para CPU entrante
		hiloCPU = pthread_create(&pIDCpu, NULL, (void *)accionesDeCPU, nuevaCPU);

/*		hiloCPU = pthread_join(pIDCpu, respuestaHilo);
		if(respuestaHilo == PTHREAD_CANCELED)
			printf("El hilo de CPU %d fue terminado", nuevaCPU->cpuID);*/


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
	puts("Saliendo de ejecutando..");
	waitSemaforo(mutexEjecutando);

	signalSemaforo(mutexEjecutando);
}

int obtenerCPUID(){
	return ++cpuID;
}

void finalizar(){

	t_pcb *pcb;

	waitSemaforo(mutexFinalizados);
	waitSemaforo(semFinalizados);

	int i;
	for(i = 0; !list_is_empty(lista_finalizados); i++){

		//Por cada elemento de la lista de finalizados
		pcb = list_get(lista_finalizados, i);

		//Envio un mensaje de finalizacion a la consola
		enviar_header(13, pcb->pcb_pid, pcb->consola);
		close(pcb->consola);

		//Sacar de la lista de finalizados
		list_remove(lista_finalizados, i);

		//Avisar a UMC que termino
		enviar_header(13, pcb->pcb_pid, socketUMC);

		//Destruyo el pcb creado
		destruirPCB(pcb);
	}

	signalSemaforo(mutexFinalizados);
}

t_paquete_programa *obtener_programa(t_header *header, int fd){
	void *buffer2 = malloc(header->tamanio);
	t_paquete_programa *programa;

	if (recv(fd, buffer2, header->tamanio, 0) < 0)
		salirPor("No se pudo obtener el codigo del programa");

	programa = deserializar_ansisop(buffer2);

	printf("Codigo programa: %s\n", programa->codigo_programa);
	return programa;
}

void accionesDeCPU(t_clienteCPU *cpu){

	t_header *header;
	int recibido;
	sem_t *mutex_acciones = crearMutex();
	void *buffer;

	fd_set readset;

	//Enviar Quantum
	if (enviar_header(28, nucleo->quantum, cpu->fd) == -1)
		puts("[NUCLEO] Envio de Quantum fallido");

	//Enviar Quantum Sleep
	if (enviar_header(29, nucleo->quantum_sleep, cpu->fd) == -1)
		puts("[NUCLEO] Envio de Quantum fallido");

	//Signal por CPU nueva
	signalSemaforo(semCpuDisponible);

	FD_ZERO(&readset);
	FD_SET(cpu->fd, &readset);

	while(1){
		if ((recibido = pselect(cpu->fd + 1, &readset, NULL, NULL, NULL, NULL)) < 0){
			//Error
			printf("Fin hilo de acciones del CPU %d", cpu->fd);
			pthread_cancel(pIDCpu);
			break;
		} else if( FD_ISSET(cpu->fd, &readset)){
			if ((header = recibir_header(cpu->fd)) != NULL){
				switch(header->identificador){
						case FinalizacionPrograma:			{ ejecutarFinalizacionPrograma(cpu, header); break;}
						case Wait:											{ ejecutarWait(header->tamanio, cpu); break; }
						case Signal: 										{ ejecutarSignal(header->tamanio, cpu); break; }
						case EntradaSalida:							{ ejecutarEntradaSalida(cpu, header); break;}
						case ObtenerValorCompartido:	{ ejecutarObtenerValorCompartido(cpu->fd, header->tamanio); break;}
						case AsignarValorCompartido:		{ ejecutarAsignarValorCompartido(cpu->fd, header->tamanio); break;}
						case MuereCPU:								{ ejecutarMuerteCPU(cpu); break;}
						case FinalizacionQuantum:				{ ejecutaFinalizacionDeQuantum(cpu); break;}
						case abortarPrograma:					{ ejecutarFinalizacionPrograma(cpu, header); break;}
						case imprimir_texto:							{ ejecutarImprimirTexto(cpu->fd, header->tamanio); break;}
						case imprimir_variable:					{ ejecutarImprimirVariable(cpu->fd, header->tamanio); break;}
				} //Fin switch
				free(header);
			}
		}
	}


/*
	do{

			if( FD_ISSET(cpu->fd, &readset)){
				if ((header = recibir_header(cpu->fd)) != NULL){
					switch(header->identificador){
							case FinalizacionPrograma:			{ ejecutarFinalizacionPrograma(cpu, header); break;}
							case Wait:											{ ejecutarWait(header->tamanio, cpu); break; }
							case Signal: 										{ ejecutarSignal(header->tamanio, cpu); break; }
							case EntradaSalida:							{ ejecutarEntradaSalida(cpu, header); break;}
							case ObtenerValorCompartido:	{ ejecutarObtenerValorCompartido(cpu->fd, header->tamanio); break;}
							case AsignarValorCompartido:		{ ejecutarAsignarValorCompartido(cpu->fd, header->tamanio); break;}
							case MuereCPU:								{ ejecutarMuerteCPU(cpu); break;}
							case FinalizacionQuantum:				{ ejecutaFinalizacionDeQuantum(cpu); break;}
							case abortarPrograma:					{ ejecutarFinalizacionPrograma(cpu, header); break;}
							case imprimir_texto:							{ ejecutarImprimirTexto(cpu->fd, header->tamanio); break;}
							case imprimir_variable:					{ ejecutarImprimirVariable(cpu->fd, header->tamanio); break;}
					} //Fin switch
					free(header);

				} else{
					printf("Fin hilo de acciones del CPU %d", cpu->fd);
					pthread_cancel(pIDCpu);
					result = -1;
					break;
				}
			}
	} while (result == -1 && errno == EINTR);
*/

/*	if(result > 0){
		if( FD_ISSET(cpu->fd, &readset)){
			result = recv(cpu->fd, buffer, MAXIMO_BUFFER, 0);
			if(result == 0){
				printf("Fin hilo de acciones del CPU %d", cpu->fd);
				pthread_cancel(pIDCpu);
				recibido = -1;
				free(header);
				//signalSemaforo(mutex_acciones);
			} else {
				puts("recibi accion");
			}
		}
	}*/


/* 	while(recibido > 0){
		waitSemaforo(mutex_acciones);

		if ((header = recibir_header(cpu->fd)) == NULL){
			printf("Fin hilo de acciones del CPU %d", cpu->fd);
			pthread_cancel(pIDCpu);
			recibido = -1;
			free(header);
			signalSemaforo(mutex_acciones);
			break;
		}

		switch(header->identificador){
				case FinalizacionPrograma:			{ ejecutarFinalizacionPrograma(cpu, header); break;}
				case Wait:											{ ejecutarWait(header->tamanio, cpu); break; }
				case Signal: 										{ ejecutarSignal(header->tamanio, cpu); break; }
				case EntradaSalida:							{ ejecutarEntradaSalida(cpu, header); break;}
				case ObtenerValorCompartido:	{ ejecutarObtenerValorCompartido(cpu->fd, header->tamanio); break;}
				case AsignarValorCompartido:		{ ejecutarAsignarValorCompartido(cpu->fd, header->tamanio); break;}
				case MuereCPU:								{ ejecutarMuerteCPU(cpu); break;}
				case FinalizacionQuantum:				{ ejecutaFinalizacionDeQuantum(cpu); break;}
				case abortarPrograma:					{ ejecutarFinalizacionPrograma(cpu, header); break;}
				case imprimir_texto:							{ ejecutarImprimirTexto(cpu->fd, header->tamanio); break;}
				case imprimir_variable:					{ ejecutarImprimirVariable(cpu->fd, header->tamanio); break;}
			} //Fin switch

		free(header);
		signalSemaforo(mutex_acciones);*/

	//}//Fin while
}

void agregarPCBaBloqueados(t_queue *cola, t_pcb *pcb, t_clienteCPU *cpu){
	puts("Bloqueado por I/O..");
	puts("Pasando a bloqueados..");

	queue_push(cola, pcb);

	cpu->disponible = Si;

	signalSemaforo(semCpuDisponible);
	signalSemaforo(semBloqueados);
}

void agregarPCBaFinalizados(t_list *lista, t_pcb *pcb, t_clienteCPU *cpu){
		puts("Programa ANSISOP finalizado..");
		waitSemaforo(mutexFinalizados);
		puts("Agregando a finalizados..");
		list_add(lista, pcb);


		signalSemaforo(semCpuDisponible);
		signalSemaforo(semFinalizados);
		signalSemaforo(mutexFinalizados);
}

char *getSemaforo(char *valor){
	return valor;
}

int getSemValue(char *valor){
	return atoi(valor);
}

t_semNucleo *crearSemaforoGlobal(char *semaforo, int valor, int io_sleep){
	t_semNucleo *semNucleo = malloc(sizeof(t_semNucleo));
	semNucleo->id						= string_duplicate(semaforo);
	semNucleo->valor					= crearSemaforo(valor);
	semNucleo->io_sleep			= io_sleep;
	semNucleo->bloqueados	= list_create();
	return semNucleo;
}

t_variableCompartida *crearSharedGlobal(char *sharedNombre){
	t_variableCompartida *variable = malloc(sizeof(t_variableCompartida));
	variable->id = string_duplicate(sharedNombre);
	variable->valor = 0;
	variable->id = sharedNombre;
	return variable;
}

char *getIOId(char *valor){
	return valor;
}

int getIOSleep(char *valor){
	return atoi(valor);
}

char *getSharedValue(char *valor){
	return valor;
}

void ejecutarWait(int tamanio_buffer, t_clienteCPU *cpu){

	char* nombreSemaforo;
	int valorSemaforo;

	nombreSemaforo = recibir_wait_identificador_semaforo(tamanio_buffer, cpu->fd);

	bool busco_sem(t_semaforo * s) {
		return string_equals_ignore_case(s->id, nombreSemaforo);
	}
	t_semaforo * semaforo = list_find(semaforos, busco_sem);

	t_pth_sems_bloqueados * pth_args = malloc(sizeof(t_pth_sems_bloqueados));
	pth_args->semaforo = semaforo;
	t_pcb * pcb = NULL;

	if ((sem_getvalue(semaforo->sem, &valorSemaforo)) < 0) {
		//cpu->disponible = No;
		enviar_header(31, 0, cpu->fd);

		//recibir el pcb y enviar a bloqueados
		t_header * header = recibir_header(cpu->fd);
		t_pcb * pcb = recibir_pcb(cpu->fd, header->tamanio);

		//enviar el pcb a bloqueados
//		list_add(lista_bloqueados, pcb);
		queue_push(semaforo->bloqueados, pcb);
	} else {
		pcb = NULL;
		enviar_header(32, 0, cpu->fd);
	}

/*	param->pcb = pcb;
	param->semaforo = semaforo;
	param->tiempo = 0;
	pthread_create(&PiDBloqueado, NULL, (void *)crearHiloBloqueados, &param);*/

	cpu->disponible = Si;
	signalSemaforo(semCpuDisponible);

	pth_args->pcb = pcb;
	pthread_create(&PiDBloqueado, NULL, ejecutar_sem, (void *) pth_args);

	//
//	t_semNucleo * semaforo = obtenerSemaforoPorID(nombreSemaforo);
//	t_header *header;
//	t_pcb *pcb;
//	t_parametrosHiloBloqueados * param = malloc(sizeof(t_parametrosHiloBloqueados));
//	if ((sem_getvalue(semaforo->valor, &valorSemaforo)) < 0){
//		//cpu->disponible = No;
//		enviar_header(31, 0, cpu->fd);
//
//		//recibir el pcb y enviar a bloqueados
//		header = recibir_header(cpu->fd);
//		pcb = recibir_pcb(cpu->fd, header->tamanio);
//
//		cpu->disponible = Si;
//		signalSemaforo(semCpuDisponible);
//
//		//enviar el pcb a bloqueados
//		list_add(lista_bloqueados, pcb);
//	} else {
//		cpu->disponible = Si;
//		signalSemaforo(semCpuDisponible);
//		pcb =NULL;
//		enviar_header(32, 0, cpu->fd);
//	}
//
//	param->pcb = pcb;
//	param->semaforo = semaforo;
//	param->tiempo = 0;
//	pthread_create(&PiDBloqueado, NULL, (void *)crearHiloBloqueados, &param);
//
}

t_semNucleo *obtenerSemaforoPorID(char *nombreSemaforo){
	int i, j;
	t_semNucleo *semaforo;

	for(i = 0; i < list_size(lista_semaforos); i++){
		semaforo = list_get(lista_semaforos, i);
		if(strcmp(semaforo->id, nombreSemaforo) == 0){
			break;
		}
	}

	return semaforo;
}

void ejecutarSignal(int tamanio_buffer, t_clienteCPU * cpu){
	char* nombreSemaforo;
	nombreSemaforo = recibir_signal_identificador_semaforo(tamanio_buffer, cpu->fd);

	bool buscar_sem_to_V(t_semaforo * s) {
		return string_equals_ignore_case(s->id, nombreSemaforo);
	}
	t_semaforo * semaforo = list_find(semaforos, (void *) buscar_sem_to_V);

//	t_semNucleo *semaforo = obtenerSemaforoPorID(nombreSemaforo);
	signalSemaforo(semaforo->sem);
}

//t_pcb *recibir_pcb(t_clienteCPU *cpu, uint32_t tamanio){
//    void *buffer_aux	= malloc(sizeof(t_pcb));
//
//    recv(cpu->fd, buffer_aux, tamanio, 0);
//    return convertirPCB(buffer_aux);
//}

void ejecutarObtenerValorCompartido(int fd, int tamanio_buffer){

	char * nombreVariable;
	t_variableCompartida *variable;

	nombreVariable = recibir_obtener_valor_compartido(tamanio_buffer, fd);

   // buscar por id variable y tomar valor
	int i;
	for(i = 0; i<list_size(lista_sharedValues); i++){
		variable = list_get(lista_sharedValues, i);
		if (string_equals_ignore_case(variable->id,nombreVariable))
			break;
	}

	if(string_equals_ignore_case(variable->id, nombreVariable))
		enviar_header(0,variable->valor,fd);
	else
		enviar_header(1,0,fd);

}


void ejecutarAsignarValorCompartido(int fd, int tamanio_buffer){
	char * nombreVariable;
	t_valor_variable valor;
	t_paquete_asignar_valor_compartido * paquete;
	t_variableCompartida *variable;

	paquete = recibir_asignar_valor_compartido(tamanio_buffer, fd);

	nombreVariable	= paquete->nombre;
	valor 			= paquete->valor;

	//buscar por id variable y pisar valor
	int i;
	for(i = 0; i < list_size(lista_sharedValues); i++){
		variable = list_get(lista_sharedValues, i);
		if (string_equals_ignore_case(variable->id,nombreVariable)){
			variable->valor = valor;
			break;
		}
	}

	if(!string_equals_ignore_case(variable->id,nombreVariable))
		enviar_header(0, 1,fd);
	else
		enviar_header(0,0,fd);
}

void ejecutarFinalizacionPrograma(t_clienteCPU *cpu, t_header *header){

	puts("ejecutando finalizacion de programa");
	t_header * header_pcb = recibir_header(cpu->fd);

	   t_pcb *pcb = recibir_pcb(cpu->fd, header_pcb->tamanio);


	   agregarPCBaFinalizados(lista_finalizados,  pcb, cpu);

	   finalizar();

	   cpu->disponible = Si;
	   signalSemaforo(semCpuDisponible);


}

void ejecutarEntradaSalida(t_clienteCPU *cpu, t_header * header){

	t_paquete_entrada_salida * paquete = NULL;
	paquete = recibir_entrada_salida(header->tamanio, cpu->fd);

	//TODO catchear el error en caso de que dispositivo == null
	t_header * head = recibir_header(cpu->fd);
	t_pcb * pcb_recibido = recibir_pcb(cpu->fd, head->tamanio);

	t_pth_disp_bloqueados * thread_args = malloc(sizeof(t_pth_disp_bloqueados));
	thread_args->pcb = pcb_recibido;
	thread_args->paquete = paquete;
	thread_args->cpu = cpu;

	pthread_create(&PiDBloqueado, NULL, (void *) ejecutar_io, (void *) thread_args);

//	t_paquete_entrada_salida * paquete;
//	t_semNucleo *semaforo;
//
//	t_pcb * pcb  = malloc(sizeof(t_pcb));
//	t_parametrosHiloBloqueados * param = malloc(sizeof(t_parametrosHiloBloqueados));
//
//	paquete = recibir_entrada_salida(header->tamanio, cpu->fd);
//
//	//recibir el pcb y enviar a bloqueados
//	header = recibir_header(cpu->fd);
//	pcb = recibir_pcb(cpu->fd, header->tamanio);
//	cpu->disponible = Si;
//	signalSemaforo(semCpuDisponible);
//
//	semaforo = obtenerSemaforoPorID(paquete->nombre);
//
//	param->pcb = pcb;
//	param->semaforo = semaforo;
//	param->tiempo = paquete->tiempo;
//
//	pthread_create(&PiDBloqueado, NULL, (void *)crearHiloBloqueados, param);
	//pthread_join(PiDBloqueado, NULL);


/*
	   t_pcb *pcb = recibir_pcb(cpu, header->tamanio);

	   agregarPCBaBloqueados(cola_bloqueados, pcb, cpu);
*/

}

void ejecutarMuerteCPU(t_clienteCPU *cpu){
	   // terminar hilo de cpu y sacar de la lista de cpu..
	int i;
	t_clienteCPU *cpuAux = malloc(sizeof(t_clienteCPU));

	for(i = 0; i < list_size(lista_cpu); i++){
		cpuAux = (t_clienteCPU *) list_get(lista_cpu, i);
		if(cpuAux->cpuID == cpu->cpuID){
			list_remove_and_destroy_element(lista_cpu, i, (void*)destruirCPU);
			break;
		}
	};

	pthread_detach(&pIDCpu);
}

void  ejecutarSemaforoBloqueado(){

}

void  interrupcionConsola(int interrupcion){
	signal(interrupcion, SIG_IGN);
	puts("Ingrese un nuevo quantum: ");
}

void destruirCPU(t_clienteCPU *cpu){
	free(cpu);
}

void crearHiloBloqueados(t_parametrosHiloBloqueados * parametros){


	waitSemaforo(parametros->semaforo->valor);

	if(parametros->semaforo->io_sleep != 0){

//		usleep(parametros->semaforo->io_sleep * parametros->tiempo * 1000);
		signalSemaforo(parametros->semaforo->valor);
	}
	int i=0;

	for(i=0; i < list_size(lista_bloqueados); i++){
		t_pcb * pcbAux= list_get(lista_bloqueados, i);

		if(pcbAux->pcb_pid == parametros->pcb->pcb_pid){
			list_remove(lista_bloqueados, i);
			break;
		}

	}

	pasarAListos(parametros->pcb);

}

void ejecutarImprimirTexto(int socket, int tamanio_buffer){
	char * texto;

	texto = recibir_texto(tamanio_buffer, socket);

	t_header * header = recibir_header(socket);

	enviar_texto(texto, header->tamanio);
}

void ejecutarImprimirVariable(int socket, int valor){

	t_header * header = recibir_header(socket);

	enviar_valor_de_variable(valor, header->tamanio);
}

void ejecutaFinalizacionDeQuantum(t_clienteCPU * cpu){
	t_header * header = recibir_header(cpu->fd);
	t_pcb * pcb = recibir_pcb(cpu->fd,header->tamanio);

	cpu->disponible = Si;
	signalSemaforo(semCpuDisponible);

	pasarAListos(pcb);
}

void ejecutar_io(void * args) {
	t_pth_disp_bloqueados * args_disp = args;

	//TODO catchear el error en caso de que paquete == null
	bool busco_disp(t_dispositivo * d) {
		return string_equals_ignore_case(d->id, args_disp->paquete->nombre);
	}
	t_dispositivo * dispositivo = list_find(dispositivos, (void *) busco_disp);

	agregarPCBaBloqueados(dispositivo->bloqueados, args_disp->pcb, args_disp->cpu);


	pthread_mutex_lock(&(dispositivo->mutex_disp));

	usleep(dispositivo->io_sleep * args_disp->paquete->tiempo * 1000);

	t_pcb * pcb = queue_pop(dispositivo->bloqueados);
	pasarAListos(pcb);

	pthread_mutex_unlock(&(dispositivo->mutex_disp));
}

void ejecutar_sem(void * args) {

}
