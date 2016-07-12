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


int cpuID = 0;						//Numero de CPU

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

void crearSemaforos(){
	mutexListos				= crearMutex();
	mutexCPU					= crearMutex();
	mutexEjecutando		= crearMutex();
	mutexConsolas			= crearMutex();
	semListos						= crearSemaforo(0);
	semCpuDisponible	= crearSemaforo(0);
	semBloqueados			= crearSemaforo(0);
}

/*void destruirPCB(t_pcb *pcb){
	free(pcb->indice_etiquetas);
	free(pcb->indice_stack.args);
	free(pcb);
}*/

void crearListasYColas(){

	cola_listos 				= queue_create();
	cola_bloqueados	= queue_create();
//	cola_ejecutando	= queue_create();


	lista_ejecutando	=	list_create();
	lista_cpu					= list_create();

	lista_semaforos		= list_create();
	lista_io						= list_create();

	t_list *sem_id			= list_map(nucleo->sem_ids, getSemaforo);
	t_list *sem_value	= list_map(nucleo->sem_init, getSemValue);
	t_list *io_id				= list_map(nucleo->io_ids, getIOId);
	t_list *io_value		= list_map(nucleo->io_sleep, getIOSleep);

	char *semaforo		= malloc(NOMBRE_SEMAFORO);
	char *io_nombre	=	malloc(NOMBRE_IO);

	int valor = 0;

	for(int i = 0; i < list_size(nucleo->sem_ids); i++){
		semaforo = list_get(sem_id, i);
		valor			= list_get(sem_value, i);
		list_add(lista_semaforos, crearSemaforoGlobal(semaforo,valor));
	}

	for(int i = 0; i < list_size(nucleo->io_ids); i++){
		io_nombre	= list_get(io_id, i);
		valor				= list_get(io_value, i);
		list_add(lista_semaforos, crearSemaforoGlobal(io_nombre, valor));
	}

	free(semaforo);
	free(io_nombre);

}


void crearClienteUMC(){

	//Crear socket de Nucleo para escuchar
	if((socketNucleo = clienteDelServidor(nucleo->ip_umc, nucleo->puerto_umc)) == -1)
			salirPor("[NUCLEO] No pudo conectarse al swap");

	enviarHandshakeAUMC();
	recibirHandshakeDeUMC();
}

void planificar_consolas(){

	//Crear los hilos para las colas de ejecucion y bloqueados
	pthread_create(&hiloEjecucion, NULL, (void *)mainEjecucion, NULL);
	pthread_create(&hiloBloqueado, NULL, (void *)mainBloqueado, NULL);
	pthread_join(hiloEjecucion, NULL);
	pthread_join(hiloBloqueado, NULL);
}

void mainEjecucion(){
	while(1){
		waitSemaforo(semListos);							//Si hay consolas
		waitSemaforo(semCpuDisponible);		//Si hay al menos una CPU, que planifique
			puts("Pasando a ejecutar consola..");
			pasarAEjecutar();
			finalizar();
		//}
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

void enviarAEjecutar(t_pcb *pcb, t_clienteCPU *cpu){

	char *serial = serializarPCB(pcb);
	int tamanio  =string_length(serial);

	enviar_header(20, tamanio, cpu->fd);
	enviarPorSocket(cpu->fd,serial);

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
	t_pcb *pcbEjecutando, *pcbActual;;

	waitSemaforo(mutexListos);

	//Obtener una CPU disponible para procesar
	t_clienteCPU *cpuDeEjecucion = obtenerCPUDisponible();

	if(cpuDeEjecucion != NULL){

		//Se obtiene el PCB para la transicion
		pcbEjecutando = (t_pcb *) queue_pop(cola_listos);
		pcbEjecutando->estado = Corriendo;

		//2da version
		//Enviar a Ejecutar
		waitSemaforo(mutexEjecutando);
		puts("Ejecutando consola..");
		enviarAEjecutar(pcbEjecutando, cpuDeEjecucion);
		//pcbEjecutando = pcbActual;
		signalSemaforo(mutexEjecutando);

/*		//Mientras tenga quantum
		while(pcbEjecutando->quantum != nucleo->quantum){
			//Enviar a Ejecutar
			waitSemaforo(mutexEjecutando);
			puts("Ejecutando consola..");
			pcbActual = enviarAEjecutar(pcbEjecutando, cpuDeEjecucion->fd);
			pcbEjecutando = pcbActual;
			signalSemaforo(mutexEjecutando);
		}

		//if(pcbEjecutando->quantum == nucleo->quantum){
		if(pcbEjecutando->estado == FinQuantum){
			pcbEjecutando->quantum = 0;						//Resetear quantum del pcb
			puts("Saliendo de ejecutando..");
			sacarDeEjecutar(pcbEjecutando);				//Sacar de la lista de Ejecutando
			puts("Pasando a listos..");
			pasarAListos(pcbEjecutando);						//Pasar a la cola de Listos
		}*/


	}

	signalSemaforo(mutexListos);
}


void pasarAListos(t_pcb *pcb){

	//Agrega el PCB a la cola de Listos
	puts("Pasando a listos..");
	waitSemaforo(mutexListos);
	queue_push(cola_listos, (void *) pcb);
	signalSemaforo(mutexListos);

	//Incrementa la cantidad de pcb
	signalSemaforo(semListos);

}

void crearServerConsola(){
	 int listener;														//Descriptor de escucha
	 int reuse;
	 int newfd;
	t_pcb *pcb_aux;
	t_header *header;
	 t_paquete_programa *programa;


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

		waitSemaforo(mutexConsolas);
		//Recibir Handshake de Consola
		if (recibir_handshake(newfd) != CONSOLA)
			salirPor("Proceso desconocido por puerto de Consola");

	    puts("[NUCLEO] Recepcion hanshake Consola" );

	    //Recibir Header de Consola
	    if ((header = recibir_header(newfd)) == NULL)
	    	puts("No se pudo recibir header de consola");

	    //Obtener Programa
	    programa = obtener_programa(header, newfd);

	    //Crear PCB
	    puts("Creando PCB.. \n");
		pcb_aux = malloc(sizeof(t_pcb));
		pcb_aux = crearPCB(programa->codigo_programa, newfd, nucleo->stack_size, cola_listos);

		//Agregar PCB a la cola de listos
		queue_push(cola_listos, pcb_aux);

		//Signal por consola nueva
		signalSemaforo(semListos);

		signalSemaforo(mutexConsolas);

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

		if (recibir_handshake(newfd) != CPU)
			puts("Proceso desconocido por puerto de CPU");

	    puts("[NUCLEO] Recepcion hanshake CPU" );

		//Crea una CPU
		t_clienteCPU *nuevaCPU = malloc(sizeof(t_clienteCPU));
		nuevaCPU->cpuID				= obtenerCPUID();
		nuevaCPU->fd 					= newfd;
		nuevaCPU->disponible	= 0;

		//Agregar CPU a la lista
		list_add(lista_cpu, nuevaCPU);

		//Enviar Quantum
		if (enviar_header(28, nucleo->quantum, newfd) == -1)
			puts("[NUCLEO] Envio de Quantum fallido");

		//Enviar Quantum Sleep
		if (enviar_header(28, nucleo->quantum, newfd) == -1)
			puts("[NUCLEO] Envio de Quantum fallido");

		//Crear hilo para CPU entrante
		pthread_create(&pIDCpu, NULL, (void *)accionesDeCPU, nuevaCPU);
		pthread_join(pIDCpu, NULL);

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
		enviarPorSocket(pcb->consola, "Finalizado");
		close(pcb->consola);

		//Sacar de la lista de finalizados
		list_remove(lista_finalizados, i);

		//avisar a UMC que termino

		//Destruyo el pcb creado
		destruirPCB(pcb);
	}

	signalSemaforo(mutexFinalizados);
}

void enviarHandshakeAUMC(){

	//----------Envio el handshake a UMC
	t_header * handshake = malloc(sizeof(t_header));
	int resultado = 0;

	handshake->identificador = (uint8_t) 2;
	handshake->tamanio = (uint32_t) 0;

	void * buffer_handshake = malloc(5);
	memcpy(buffer_handshake, &(handshake->identificador), 1);
	memcpy(buffer_handshake + 1, &(handshake->tamanio), 4);

	if ((resultado = enviarPorSocket(socketNucleo, buffer_handshake)) == -1)
		salirPor("Error en el send del handshake a UMC\n");


//	resultado = send(socketNucleo, buffer_handshake, 5, 0);
	free(handshake);
	free(buffer_handshake);

}

void recibirHandshakeDeUMC(){

	//---------Recibo el tamanio de pagina
	void * buffer_entrada = malloc(5);
	int resultado = 0;

	resultado = recv(socketNucleo, buffer_entrada, 5, MSG_WAITALL);

	if (resultado == -1) {
		printf("Error en el recv del tamanio de pagina desde UMC\n");
		exit(EXIT_FAILURE);
	}

	t_header * head_tamanio_pagina = malloc(sizeof(t_header));
	memcpy(&(head_tamanio_pagina->identificador), buffer_entrada, 1);
	memcpy(&(head_tamanio_pagina->tamanio), buffer_entrada + 1, 4);

	if (head_tamanio_pagina->identificador != (uint8_t) Tamanio_pagina)
		salirPor("Error en el ID de la cabecera de recibirPagina\n");

	tamanio_pagina = head_tamanio_pagina->tamanio;
	printf("El tamanio de pagina es: %d", tamanio_pagina);

	free(buffer_entrada);
	free(head_tamanio_pagina);

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
	t_pcb *pcb		= malloc(sizeof(t_pcb));
	void *buffer	= malloc(header->tamanio);
	bool PCBRetornado = true;

	while(PCBRetornado){
		if ((header = recibir_header(cpu->fd)) == NULL){
			puts("Error");
			continue;}

	   switch(header->identificador){
	  	   case FinalizacionPrograma:{
	  		   void *buffer_aux	= malloc(sizeof(t_pcb));
	  		   recv(cpu->fd, buffer_aux, header->tamanio, 0);
	  		   agregarPCBaFinalizados(lista_finalizados,  convertirPCB(buffer_aux));
	  		   cpu->disponible = 1;
	  		   break;}

	  	   case Wait:{
	  		   ejecutarWait(header->tamanio);
	  		   cpu->disponible = 1; // solo si se bloquea el semaforo
	  		   break; }

	  	   case Signal: {
	  		   break; }

	  	   case EntradaSalida:{
	  		   void *buffer_aux	= malloc(sizeof(t_pcb));
	  		   recv(cpu->fd, buffer_aux, header->tamanio, 0);
	  		   agregarPCBaBloqueados(cola_bloqueados, convertirPCB(buffer_aux));
	  		   cpu->disponible = 1;
	  		   break;}

	  	   case ObtenerValorCompartido:{
	  		   // Enviar a cpu el valor de una variable
	  		   break;}

	  	   case AsignarValorCompartido:{
	  		   //asignar a la variable lo que me manda cpu
	  		   break;}

	  	   case MuereCPU:{
	  		   // terminar hilo de cpu y sacar de la lista de cpu..
	  			pthread_detach(&pIDCpu);
	  		   break;}

	  	   case FinalizacionQuantum:{
	  		   cpu->disponible = 1;
	  		   break;}

	   } //Fin switch


	}//Fin while

}

void agregarPCBaBloqueados(t_queue *cola, t_pcb *pcb){
	puts("Bloqueado por I/O..");
	puts("Pasando a bloqueados..");
	queue_push(cola, pcb);
	signalSemaforo(semBloqueados);
}

void agregarPCBaFinalizados(t_list *lista, t_pcb *pcb){
		puts("Programa ANSISOP finalizado..");
		waitSemaforo(mutexFinalizados);
		puts("Agregando a finalizados..");
		list_add(lista, pcb);
		signalSemaforo(semFinalizados);
		signalSemaforo(mutexFinalizados);
}

char *getSemaforo(char *valor){
	return valor;
}

int getSemValue(char *valor){
	return atoi(valor);
}

t_semNucleo *crearSemaforoGlobal(char *semaforo, int valor){
	t_semNucleo *semNucleo = malloc(sizeof(t_semNucleo));
	semNucleo->id						= semaforo;
	semNucleo->valor					= valor;
	semNucleo->bloqueados	= list_create();
	return semNucleo;
}

char *getIOId(char *valor){
	return valor;
}

int getIOSleep(char *valor){
	return atoi(valor);
}

void ejecutarWait(char *nombreSemaforo){
	int buscarPorNombre(t_semNucleo *semaforo){
		return semaforo->id == nombreSemaforo;
	}

	t_semNucleo *semaforo = list_find(lista_semaforos, (void *)buscarPorNombre);			//obtener el semaforo
	semaforo->valor--;

	list_replace(lista_semaforos, (void *)buscarPorNombre, crearSemaforoGlobal(semaforo->id, semaforo->valor));		//actualizarlo
}


