/*
 ============================================================================
 Name        : UMC.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <UMC.h>

void new_line() {
	printf("\n");
}

void cargar_config(char ** config_path) {

	t_config * config_file = config_create(config_path[1]);
	umc_config = malloc(sizeof(t_umc_config));
	memset(umc_config, 0, sizeof(t_umc_config));

	umc_config->ip_swap = malloc(sizeof(char) * 15);
	umc_config->ip_swap = string_new();
	string_append(&umc_config->ip_swap,
			getStringProperty(config_file, "IP_SWAP"));

	umc_config->puerto_escucha = getIntProperty(config_file, "PUERTO");
	umc_config->puerto_swap = getIntProperty(config_file, "PUERTO_SWAP");
	umc_config->cant_frames = getIntProperty(config_file, "MARCOS");
	umc_config->frames_size = getIntProperty(config_file, "MARCO_SIZE");
	umc_config->frame_x_prog = getIntProperty(config_file, "MARCO_X_PROG");
	umc_config->entradas_tlb = getIntProperty(config_file, "ENTRADAS_TLB");
	umc_config->retardo = getIntProperty(config_file, "RETARDO");
	umc_config->algoritmo = getIntProperty(config_file, "ALGORITMO");

	its_clock_m = (umc_config->algoritmo == CLOCK_MODIFICADO);

	config_destroy(config_file);
}

void imprimir_config() {
	puts("ARCHIVO DE CONFIGURACION:");
	printf("PUERTO: %d\n", umc_config->puerto_escucha);
	printf("IP_SWAP: %s\n", umc_config->ip_swap);
	printf("PUERTO_SWAP: %d\n", umc_config->puerto_swap);
	printf("FRAMES: %d\n", umc_config->frames_size);
	printf("MARCO_SIZE: %d\n", umc_config->frames_size);
	printf("MARCO_X_PROG: %d\n", umc_config->frame_x_prog);
	printf("ENTRADAS_TLB: %d\n", umc_config->entradas_tlb);
	printf("RETARDO: %d\n", umc_config->retardo);

	switch (umc_config->algoritmo) {
	case CLOCK:
		printf("ALGORITMO: CLOCK\n");
		break;
	case CLOCK_MODIFICADO:
		printf("ALGORITMO: CLOCK MODIFICADO\n");
		break;
	default:
		printf("ALGORITMO: desconocido\n");
		break;
	}
	new_line();
}

bool tlb_habilitada() {
	if (umc_config->entradas_tlb > 0) {
		tabla_tlb = list_create();
		int i;
		for (i = 0; i < umc_config->entradas_tlb; i++) {
			t_tlb * new_entry = malloc(sizeof(t_tlb));
			new_entry->frame = -1;
			new_entry->pagina = -1;
			new_entry->pid = -1;
			new_entry->referencebit = 0;

			list_add_in_index(tabla_tlb, i, new_entry);
		}
		return true;
	} else {
		return false;
	}
}

void crear_archivo_log() {
	logger = log_create(LOG_FILE, "UMC log", false, LOG_LEVEL_TRACE);
	log_info(logger, "UMC iniciado.");
}

void inicializar_semaforos() {
	pthread_mutex_init(&mutex_servidor, 0);
	pthread_mutex_init(&mutex_lista_cpu, 0);
	pthread_mutex_init(&mutex_nucleo, 0);
	pthread_mutex_init(&mutex_memoria, 0);
	pthread_mutex_init(&mutex_swap, 0);
}

void enviar_pagina_size(int sock_fd) {
	if (enviar_header(Tamanio_pagina, umc_config->frames_size, sock_fd))
		printf("\nSe ha enviado el tamanio de pagina \n");
}

void flush_tlb_by_certain_pid(int pid) {
	void flush_entry_by_pid(t_tlb * tlb_entry) {
		if (tlb_entry->pid == pid) {
			tlb_entry->pagina = -1;
			tlb_entry->frame = -1;
			tlb_entry->pid = -1;
		}
	}
	list_iterate(tabla_tlb, (void *) flush_entry_by_pid);
}


void signal_handler(int n_singal) {
	switch (n_singal) {
	case SIGUSR1:
		new_line();
		pthread_create(&hiloConsola, NULL, lanzar_consola, NULL);
		pthread_join(hiloConsola, NULL);
		break;
	case SIGUSR2:
		system("clear");
		break;
	default:
		break;
	}
}

void * escucha_conexiones() {

	int optval = 1;

	socket_servidor = crearSocket();
	if (socket_servidor == -1)
		exit(EXIT_FAILURE);
	new_line();

	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	if (bindearSocket(socket_servidor, umc_config->puerto_escucha) == -1)
		exit(EXIT_FAILURE);
	new_line();

	if (escucharEn(socket_servidor) == -1)
		exit(EXIT_FAILURE);
	new_line();

	struct sockaddr_in direccion_cliente;
	unsigned int addrlen = sizeof(struct sockaddr_in);
	int socket_nuevo = -1;

	while (true) {

		pthread_mutex_lock(&mutex_servidor); //LOCK MUTEX SERVIDOR

		socket_nuevo = accept(socket_servidor,
				(struct sockaddr*) &direccion_cliente, &addrlen);

		if (socket_nuevo == -1) {
			printf("Error en el accept.\n");
			pthread_mutex_unlock(&mutex_servidor);
			continue;
		}

		setsockopt(socket_nuevo, SOL_SOCKET, SO_REUSEADDR, &optval,
				sizeof(optval));

		//------Recibo un handshake de la nueva conexion
		int id = recibir_handshake(socket_nuevo);

		if (id <= 0) {
			pthread_mutex_unlock(&mutex_servidor);
			continue;
		}

		//-------Averiguo quien es
		switch (id) {
		case NUCLEO:
			new_line();
			printf("Se conecto Nucleo en el socket: %d\n", socket_nuevo);

			socket_nucleo = socket_nuevo;

			enviar_pagina_size(socket_nuevo);

			pthread_create(&hilo_atiende_nucleo, NULL, atiende_nucleo, (void *) socket_nucleo);

			break;
		case CPU:
			new_line();
			printf("Se conecto una CPU en el socket: %d\n", socket_nuevo);
			pthread_mutex_lock(&mutex_lista_cpu);
			t_sesion_cpu * cpu = malloc(sizeof(t_sesion_cpu));
			cpu->socket_cpu = socket_nuevo;
			cpu->id_cpu = ++id_cpu;
			cpu->proceso_activo = -1;
			list_add(cpu_conectadas, cpu);
			printf("CPU ID: %d\n", cpu->id_cpu);
			printf("CPU socket: %d\n", cpu->socket_cpu);
			pthread_mutex_unlock(&mutex_lista_cpu);

			enviar_pagina_size(socket_nuevo);

			pthread_create(&hilo_atiende_cpu, NULL, atiende_cpu,
					(void *) socket_nuevo);
			contador_hilos++;
			break;
		default:
			printf("Se conecto alguien desconocido.\n");
			break;
		}

		pthread_mutex_unlock(&mutex_servidor); //UNLOCK MUTEX SERVIDOR
	}

	return EXIT_SUCCESS;
}

//******************************  SWAP  **************************************************

void conecta_swap() {
	socket_swap = clienteDelServidor(umc_config->ip_swap,
				umc_config->puerto_swap);

	enviar_handshake(socket_swap, UMC);

	pthread_create(&hilo_cliente, NULL, (void *) atiende_swap, (void *) socket_swap);
}

void atiende_swap(void * args) {

	int socket = (int) args;

	bool keep_running = true;

	while (keep_running) {
		t_header * header = NULL;

		pthread_mutex_lock(&mutex_swap);

		header = recibir_header(socket);

		if (!header) {
			puts("ERROR");
			keep_running = false;
			pthread_mutex_unlock(&mutex_swap);
			continue;
		}

		switch (header->identificador) {
		case Respuesta_inicio_programa:
			respuesta_inicio_swap(header->tamanio);
			break;
		default:
			break;
		}

		free(header);

		pthread_mutex_unlock(&mutex_swap);
	}
}

int inicializar_en_swap(t_paquete_inicializar_programa * paquete) {

	int buffer_size = string_length(paquete->codigo_programa) + 12;

	enviar_header(Inicializar_programa, buffer_size, socket_swap);

	int r = enviar_inicializar_programa(paquete->pid,
			paquete->paginas_requeridas, paquete->codigo_programa, socket_swap);

	return r;
}

void respuesta_inicio_swap(int respuesta_inicio) {

	t_header * header = recibir_header(socket_swap);
	t_paquete_inicializar_programa * paquete = recibir_inicializar_programa(header->tamanio, socket_swap);

	if (respuesta_inicio == Respuesta__SI)
		inicializar_proceso(paquete->pid, paquete->paginas_requeridas);

	enviar_respuesta_inicio(socket_nucleo, respuesta_inicio);

	free(paquete->codigo_programa);
	free(paquete);
}

void finalizar_en_swap(int pid) {
	enviar_header(Finalizar_programa, pid, socket_swap);
}

void * lectura_en_swap(t_paquete_solicitar_pagina * paquete, int pid) {

	int r = enviar_header(Solicitar_pagina, 16, socket_swap);

	if (r < 0)
		return NULL;

	void * buffer_solicitud = malloc(16);

	memcpy(buffer_solicitud, &(pid), 4);
	void * buff_aux = serializar_leer_pagina(paquete->nro_pagina,
			paquete->offset, paquete->bytes);
	memcpy(buffer_solicitud + 4, buff_aux, 12);

	send(socket_swap, buffer_solicitud, 16, 0);

	free(buffer_solicitud);
	free(buff_aux);

	t_header * head = recibir_header(socket_swap);

	if (!head)
		puts("aca se rommpe");

	printf("id: %d", head->identificador);
	printf("t: %d", head->tamanio);

	void * datos_solicitados = malloc(paquete->bytes);
//	r = recv(socket_swap, datos_solicitados, paquete->bytes, 0);
//
//	if (r < 0)
//		return NULL;
//
//	printf("datos: %s", (char *) datos_solicitados);

	return datos_solicitados;
}

void escritura_en_swap(t_paquete_almacenar_pagina * paquete, int pid) {
	void * buffer_solicitud = NULL;

	memcpy(buffer_solicitud, &(pid), 4);
	void * buff_aux = serializar_almacenar_pagina(paquete->nro_pagina,
			paquete->offset, paquete->bytes, paquete->buffer);
	int second_size = 12 + paquete->bytes;
	memcpy(buffer_solicitud + 4, buff_aux, second_size);

	int last_size = 4 + second_size;
	enviar_header(Almacenar_pagina, last_size, socket_swap);
	send(socket_swap, buffer_solicitud, last_size, 0);
}

//********************************************************************************************

//******************************  NUCLEO  **************************************************

void * atiende_nucleo(void * args) {

	int recibido = 1;

	while (recibido > 0) {

		pthread_mutex_lock(&mutex_nucleo);

		t_header * head_in = recibir_header(socket_nucleo);

		if (head_in == NULL) {
			printf("\nSE DESCONECTO NUCLEO 		(...kill me please)\n");
			recibido = -1;
			pthread_mutex_unlock(&mutex_nucleo);
			continue;
		}

		switch (head_in->identificador) {
		case Inicializar_programa:
			new_line();
			printf("Inicializo programa...\n");
			if (inicializar_programa(head_in->tamanio) == EXIT_FAILURE) {
				free(head_in);
				pthread_mutex_unlock(&mutex_nucleo);
				continue;
			}
			break;
		case Finalizar_programa:
			printf("Finalizo programa\n");
			finalizar_programa(head_in->tamanio);
			break;
		default:
			printf("Cabecera desconocida\n");
			break;
		}

		free(head_in);

		pthread_mutex_unlock(&mutex_nucleo);
	}

	return EXIT_SUCCESS;
}

int inicializar_programa(int bytes_to_recv) {

	t_paquete_inicializar_programa * paquete_init =
			recibir_inicializar_programa(bytes_to_recv, socket_nucleo);

	if (!paquete_init) {
		//escribo en el log el error
		enviar_respuesta_inicio(socket_nucleo, Respuesta__NO);
		return EXIT_FAILURE;
	}

	printf("pid: %d\n", paquete_init->pid);
	printf("paginas: %d\n", paquete_init->paginas_requeridas);
	printf("%s", paquete_init->codigo_programa);
	new_line();

	if (inicializar_en_swap(paquete_init) <= 0) {
		printf("\n<<<<<<<ACA explota>>>>>>>\n");
		//escribo en el log el error
		enviar_respuesta_inicio(socket_nucleo, Respuesta__NO);
		free(paquete_init->codigo_programa);
		free(paquete_init);
		return EXIT_FAILURE;
	}

	free(paquete_init->codigo_programa);
	free(paquete_init);
	return EXIT_SUCCESS;
}

void finalizar_programa(int id_programa) {

	//--le aviso a swap que libere el programa
	finalizar_en_swap(id_programa);

	//--luego libero aca: tabla de paginas, libero frames si estaba alocado en memoria, etc...
	t_proceso * proceso = buscar_proceso(id_programa);

	int cantidad_paginas = list_size(proceso->tabla_paginas);

	int nro_pagina;
	for (nro_pagina = 0; nro_pagina < cantidad_paginas; nro_pagina++) {
		t_tabla_pagina * page_entry = list_get(proceso->tabla_paginas,
				nro_pagina);

		if (page_entry->presentbit == 1) {
			//libero memoria en el frame en la cual esta cargado el proceso
			t_mem_frame * marco = buscar_frame_por_pagina(nro_pagina,
					id_programa);

			if (marco == NULL)
				continue;

			marco->libre = 1;
			marco->pagina = -1;
			marco->pid = -1;
		}
	}

	//libero la tabla de paginas
	list_destroy_and_destroy_elements(proceso->tabla_paginas, (void *) free);

	//libero la lista de referencias
	list_destroy_and_destroy_elements(proceso->referencias, (void *) free);

	//Hago un flush en tlb
	flush_tlb_by_certain_pid(proceso->pid);

	//elimino el proceso de la lista de procesos
	list_remove_and_destroy_by_condition(lista_procesos,
			(void *) buscar_proceso, (void *) free);

}


//********************************************************************************************

//***********************************  CPU   *************************************************

void * atiende_cpu(void * args) {

	int socket_cpu = (int) args;
	pthread_mutex_t mutex_cpu;
	pthread_mutex_init(&mutex_cpu, 0);

	int recibido = 1;

	while (recibido > 0) {

		pthread_mutex_lock(&mutex_cpu);

		t_header * head_in = recibir_header(socket_cpu);

		if (head_in == NULL) {
			recibido = -1;
			pthread_mutex_unlock(&mutex_cpu);
			continue;
		}

		switch (head_in->identificador) {
		case Solicitar_pagina:
			printf("Lectura de bytes\n");
			leer_bytes(socket_cpu, head_in->tamanio);
			break;
		case Almacenar_pagina:
			printf("Almaceno bytes\n");
			almacenar_bytes(socket_cpu, head_in->tamanio);
			break;
		case Cambio_proceso_activo:
			printf("Cambio de proceso activo\n");
			cambio_proceso_activo(head_in->tamanio, socket_cpu);
			break;
		default:
			printf("Cabecera desconocida\n");
			break;
		}

		free(head_in);

		pthread_mutex_unlock(&mutex_cpu);
	}

	return EXIT_SUCCESS;
}

int leer_bytes(int socket_cpu, int bytes) {

	t_paquete_solicitar_pagina * solicitud = recibir_solicitud_lectura(bytes,
			socket_cpu);
	if (solicitud == NULL)
		return -1;

	t_sesion_cpu * cpu = buscar_cpu(socket_cpu);
	if (!cpu) {
		free(solicitud);
		return -1;
	}

	printf("PID: %d\n", cpu->proceso_activo);
	printf("Pagina: %d\n", solicitud->nro_pagina);
	printf("Offset: %d\n", solicitud->offset);
	printf("bytes: %d\n", solicitud->bytes);

	bool sol_pag_val = pagina_valida(cpu->proceso_activo,
			solicitud->nro_pagina);
	if (!sol_pag_val) {
		free(solicitud);
		return -1;
	}

	t_tlb * tlb_entry = NULL;
	t_proceso * proceso = NULL;
	proceso = buscar_proceso(cpu->proceso_activo);

	if (tlb_on)
		tlb_entry = buscar_en_tlb(solicitud->nro_pagina, proceso->pid);

	if (tlb_entry) {
		//TLB HIT
		tlb_entry->referencebit = 0;
		t_tabla_pagina * page = buscar_pagina(solicitud->nro_pagina,
				proceso->tabla_paginas);
		page->accessedbit = 1;
	} else {
		//TLB MISS
		if (!esta_en_memoria(solicitud->nro_pagina, proceso, 0)) {
			//PAGE FAULT
			switch (umc_config->algoritmo) {
			case CLOCK:
				clock_algorithm(solicitud->nro_pagina, proceso);
				break;
			case CLOCK_MODIFICADO:
				clock_modificado(solicitud->nro_pagina, proceso, 0);
				break;
			}
		}
	}

	agregar_referencia(solicitud->nro_pagina, proceso);
	free(solicitud);

	return EXIT_SUCCESS;
}

int almacenar_bytes(int socket_cpu, int bytes) {
	t_paquete_almacenar_pagina * solicitud = recibir_solicitud_escritura(bytes,
			socket_cpu);

	if (solicitud == NULL)
		return -1;

	t_sesion_cpu * cpu = buscar_cpu(socket_cpu);
	if (!cpu) {
		free(solicitud);
		return -1;
	}

	printf("PID: %d\n", cpu->proceso_activo);
	printf("Pagina: %d\n", solicitud->nro_pagina);
	printf("Offset: %d\n", solicitud->offset);
	printf("bytes: %d\n", solicitud->bytes);
	printf("Buffer: %s\n", (char *) solicitud->buffer);

	bool sol_pag_val = pagina_valida(cpu->proceso_activo,
			solicitud->nro_pagina);
	if (!sol_pag_val) {
		free(solicitud);
		return -1;
	}

	t_tlb * tlb_entry = NULL;
	t_proceso * proceso = NULL;
	proceso = buscar_proceso(cpu->proceso_activo);

	if (tlb_on)
		tlb_entry = buscar_en_tlb(solicitud->nro_pagina, proceso->pid);

	if (tlb_entry) {
		//TLB HIT
		tlb_entry->referencebit = 0;
		t_tabla_pagina * page = buscar_pagina(solicitud->nro_pagina,
				proceso->tabla_paginas);
		page->accessedbit = 1;
		if (!(page->dirtybit))
			page->dirtybit = 1;
	} else {
		//TLB MISS
		if (!esta_en_memoria(solicitud->nro_pagina, proceso, 1)) {
			//PAGE FAULT
			switch (umc_config->algoritmo) {
			case CLOCK:
				clock_algorithm(solicitud->nro_pagina, proceso);
				break;
			case CLOCK_MODIFICADO:
				clock_modificado(solicitud->nro_pagina, proceso, 1);
				break;
			}
		}
	}

	agregar_referencia(solicitud->nro_pagina, proceso);

	free(solicitud);

	return EXIT_SUCCESS;
}


int cambio_proceso_activo(int pid, int cpu) {

	t_sesion_cpu * target = buscar_cpu(cpu);
	if (target == NULL) {
		puts("No se pudo realizar el cambio de proceso activo");
		return -1;
	}

	if (tlb_on)
		flush_tlb_by_certain_pid(pid);

	target->proceso_activo = pid;

	return EXIT_SUCCESS;
}

//********************************************************************************************
