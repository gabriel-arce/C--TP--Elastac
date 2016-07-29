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

	pthread_mutex_lock(&mutex_tlb);

	void flush_entry_by_pid(t_tlb * tlb_entry) {
		if (tlb_entry->pid == pid) {
			tlb_entry->pagina = -1;
			tlb_entry->frame = -1;
			tlb_entry->pid = -1;
		}
	}
	list_iterate(tabla_tlb, (void *) flush_entry_by_pid);

	pthread_mutex_unlock(&mutex_tlb);
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

	//pthread_create(&hilo_cliente, NULL, (void *) atiende_swap, (void *) socket_swap);
}

//void atiende_swap(void * args) {
//
//	int socket = (int) args;
//
//	bool keep_running = true;
//
//	while (keep_running) {
//		t_header * header = NULL;
//
//		pthread_mutex_lock(&mutex_swap);
//
//		header = recibir_header(socket);
//
//		if (!header) {
//			keep_running = false;
//			pthread_mutex_unlock(&mutex_swap);
//			continue;
//		}
//
//		switch (header->identificador) {
//		case Respuesta_inicio_programa:
//			respuesta_inicio_swap(header->tamanio);
//			break;
//		default:
//			break;
//		}
//
//		free(header);
//
//		pthread_mutex_unlock(&mutex_swap);
//	}
//}

int inicializar_en_swap(t_paquete_inicializar_programa * paquete) {

	int buffer_size = string_length(paquete->codigo_programa) + 12;

	enviar_header(Inicializar_programa, buffer_size, socket_swap);

	int r = enviar_inicializar_programa(paquete->pid,
			paquete->paginas_requeridas, paquete->codigo_programa, socket_swap);

	return r;
}

void respuesta_inicio_swap(int respuesta_inicio) {
	if ((respuesta_inicio == Respuesta__SI)||(respuesta_inicio > 0)) {
		t_header * header = recibir_header(socket_swap);

		if (header == NULL)
			enviar_respuesta_inicio(socket_nucleo, Respuesta__NO);

		t_paquete_inicializar_programa * paquete = recibir_inicializar_programa(
				header->tamanio, socket_swap);

		if (respuesta_inicio == Respuesta__SI)
			inicializar_proceso(paquete->pid, paquete->paginas_requeridas);

		enviar_respuesta_inicio(socket_nucleo, respuesta_inicio);

		free(header);
		free(paquete->codigo_programa);
		free(paquete);

	} else {
		enviar_respuesta_inicio(socket_nucleo, Respuesta__NO);
	}
}

void finalizar_en_swap(int pid) {
	enviar_header(Finalizar_programa, pid, socket_swap);
}

void * lectura_en_swap(int pagina, int pid) {

	void * datos_solicitados = malloc(umc_config->frames_size);

	void * pedido_swap = malloc(8);
	memcpy(pedido_swap, &pid, 4);
	memcpy(pedido_swap + 4, &pagina, 4);

	if (enviar_header(Solicitar_pagina, 8, socket_swap) <= 0)
			return NULL;

	if (send(socket_swap, pedido_swap, 8, 0) <= 0) {
		free(pedido_swap);
		return NULL;
	}

	free(pedido_swap);

	if (recv(socket_swap, datos_solicitados, umc_config->frames_size, 0) <= 0)
		return NULL;

	char * error_string = string_repeat('?', umc_config->frames_size);

	if (string_equals_ignore_case((char *) datos_solicitados, error_string))
		return NULL;

	free(error_string);
	return datos_solicitados;
}

int escritura_en_swap(int pagina, int frame, int pid) {

	void * pagina_a_escribir = leer_datos(frame, 0, umc_config->frames_size);

	int pedido_size = 8 + umc_config->frames_size;
	void * pedido_swap = malloc(pedido_size);
	memcpy(pedido_swap, &pid, 4);
	memcpy(pedido_swap + 4, &pagina, 4);
	memcpy(pedido_swap + 8, pagina_a_escribir, umc_config->frames_size);

	if (enviar_header(Almacenar_pagina, pedido_size, socket_swap) <= 0)
		return -1;

	if (send(socket_swap, pedido_swap, pedido_size, 0) <= 0)
		return -1;

	free(pagina_a_escribir);
	free(pedido_swap);

	int response = recibir_respuesta_inicio(socket_swap);

	return response;
}

//********************************************************************************************

//********************************  NUCLEO  **************************************************

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
		//escribo en el log el error
		enviar_respuesta_inicio(socket_nucleo, Respuesta__NO);
		free(paquete_init->codigo_programa);
		free(paquete_init);
		return EXIT_FAILURE;
	}

	int respuesta_inicio = recibir_respuesta_inicio(socket_swap);

	respuesta_inicio_swap(respuesta_inicio);

	free(paquete_init->codigo_programa);
	free(paquete_init);
	return EXIT_SUCCESS;
}

void finalizar_programa(int id_programa) {

	//--le aviso a swap que libere el programa
	finalizar_en_swap(id_programa);

	usleep(umc_config->retardo * 1000);
	//--luego libero aca: tabla de paginas, libero frames si estaba alocado en memoria, etc...
	t_proceso * proceso = buscar_proceso(id_programa);

	int cantidad_paginas = list_size(proceso->tabla_paginas);

	char * hueco = string_repeat('\0', umc_config->frames_size);
	int direccion = 0;

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

			direccion = page_entry->frame * umc_config->frames_size;
			memcpy(memoria_principal, (void *) hueco, umc_config->frames_size);

			marco->libre = 1;
			marco->pagina = -1;
			marco->pid = -1;
		}
	}

	free(hueco);

	//libero la tabla de paginas
	list_destroy_and_destroy_elements(proceso->tabla_paginas, (void *) free);

	//libero la lista de referencias
	free(proceso->referencias);

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

	pthread_mutex_lock(&mutex_memoria);

	int catch_read_error() {
		send(socket_cpu, (void *) " ", 1, 0);
		pthread_mutex_unlock(&mutex_memoria);
		return -1;
	}

	t_paquete_solicitar_pagina * solicitud = recibir_solicitud_lectura(bytes,
			socket_cpu);
	if (solicitud == NULL)
		return catch_read_error();

	t_sesion_cpu * cpu = buscar_cpu(socket_cpu);
	if (!cpu) {
		free(solicitud);
		return catch_read_error();
	}

	printf("pid: %d\n", cpu->proceso_activo);
	printf("pagina: %d\n", solicitud->nro_pagina);
	printf("offset: %d\n", solicitud->offset);
	printf("bytes: %d\n", solicitud->bytes);

	t_proceso * proceso = buscar_proceso(cpu->proceso_activo);
	if (proceso == NULL) {
		free(solicitud);
		return catch_read_error();
	}

	bool es_valida = pagina_valida(proceso,
			solicitud->nro_pagina, solicitud->offset, solicitud->bytes);
	if (!es_valida) {
		free(solicitud);
		return catch_read_error();
	}

	t_tlb * tlb_entry = NULL;
	int frame_to_read = -1;

	if (tlb_on)
		tlb_entry = buscar_en_tlb(solicitud->nro_pagina, proceso->pid);

	if (tlb_entry) {
		//TLB HIT
		printf("\nTLB HIT\n");
		tlb_entry->referencebit = 0;
		t_tabla_pagina * page = buscar_pagina(solicitud->nro_pagina,
				proceso->tabla_paginas);
		page->accessedbit = 1;

		frame_to_read = page->frame;
	} else {
		//TLB MISS
		printf("\nTLB MISS\n");
		frame_to_read = esta_en_memoria(solicitud->nro_pagina, proceso, 0);
		if (frame_to_read == -1) {
			//PAGE FAULT
			printf("PAGE FAULT\n");
			switch (umc_config->algoritmo) {
			case CLOCK:
				frame_to_read = clock_algorithm(solicitud->nro_pagina, solicitud->offset, solicitud->bytes, proceso, 0);
				break;
			case CLOCK_MODIFICADO:
				frame_to_read = clock_modificado(solicitud->nro_pagina, solicitud->offset, solicitud->bytes, proceso, 0);
				break;
			}
		}
	}

	if (frame_to_read == -1) {
		free(solicitud);
		return catch_read_error();
	}

	void * datos_leidos = leer_datos(frame_to_read, solicitud->offset, solicitud->bytes);

	printf("\nDATOS QUE LE ENVIO A CPU: %s\n", (char *) datos_leidos);

	if ((send(cpu->socket_cpu, datos_leidos, solicitud->bytes, 0)) < 0) {
		free(solicitud);
		free(datos_leidos);
		return catch_read_error();
	}

	agregar_referencia(solicitud->nro_pagina, proceso);

	free(datos_leidos);
	free(solicitud);

	pthread_mutex_unlock(&mutex_memoria);

	return EXIT_SUCCESS;
}

int almacenar_bytes(int socket_cpu, int bytes) {

	pthread_mutex_lock(&mutex_memoria);

	int catch_write_error() {
		enviar_header(Almacenar_pagina, 0, socket_cpu);
		pthread_mutex_unlock(&mutex_memoria);
		return -1;
	}

	t_paquete_almacenar_pagina * solicitud = recibir_solicitud_escritura(bytes,
			socket_cpu);

	if (solicitud == NULL)
		return catch_write_error();

	t_sesion_cpu * cpu = buscar_cpu(socket_cpu);
	if (!cpu) {
		free(solicitud);
		return catch_write_error();
	}

	printf("PID: %d\n", cpu->proceso_activo);
	printf("Pagina: %d\n", solicitud->nro_pagina);
	printf("Offset: %d\n", solicitud->offset);
	printf("bytes: %d\n", solicitud->bytes);
	printf("Buffer: %s\n", (char *) solicitud->buffer);

	t_proceso * proceso = buscar_proceso(cpu->proceso_activo);

	if (!proceso) {
		free(solicitud);
		return catch_write_error();
	}

	bool es_valida = pagina_valida(proceso,
			solicitud->nro_pagina, solicitud->offset, solicitud->bytes);
	if (!es_valida) {
		free(solicitud);
		return catch_write_error();
	}

	t_tlb * tlb_entry = NULL;
	int frame_to_write = -1;

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

		frame_to_write = page->frame;
	} else {
		//TLB MISS
		frame_to_write = esta_en_memoria(solicitud->nro_pagina, proceso, 1);
		if (frame_to_write == -1) {
			//PAGE FAULT
			switch (umc_config->algoritmo) {
			case CLOCK:
				frame_to_write = clock_algorithm(solicitud->nro_pagina, solicitud->offset, solicitud->bytes, proceso, 1);
				break;
			case CLOCK_MODIFICADO:
				frame_to_write = clock_modificado(solicitud->nro_pagina, solicitud->offset, solicitud->bytes, proceso, 1);
				break;
			}
		}
	}

	if (frame_to_write == -1) {
		free(solicitud);
		return catch_write_error();
	}

	escribir_datos(frame_to_write, solicitud);

	if (enviar_header(Almacenar_pagina, 1, socket_cpu) < 0) {
		free(solicitud);
		return catch_write_error();
	}

	agregar_referencia(solicitud->nro_pagina, proceso);

	free(solicitud);

	pthread_mutex_unlock(&mutex_memoria);

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
