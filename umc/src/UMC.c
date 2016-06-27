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

void cargar_config() {

	t_config * config_file = config_create(CONFIG_PATH);
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
		tlb = list_create();
		int i;
		for (i = 0; i < umc_config->entradas_tlb; i++) {
			t_tlb * new_entry = malloc(sizeof(t_tlb));
			new_entry->frame = -1;
			new_entry->pagina = -1;
			new_entry->pid = -1;
			new_entry->referencebit = 0;

			list_add_in_index(tlb, i, new_entry);
		}
		return true;
	} else {
		return false;
	}
}

void * lanzar_consola() {
	printf("***CONSOLA DEL PROCESO UMC***\n");
	new_line();

	char * buffer_in = malloc(100);
	int fin = 0;

	while (fin == 0) {
		new_line();
		printf("Comandos disponibles:\n");
		printf("(1): retardo x[ms]\n");
		printf("(2): dump #\n");
		printf("	#1- estructuras de memoria\n");
		printf("	#2- contenido de memoria\n");
		printf("(3): flush #\n");
		printf("	#1- tlb\n");
		printf("	#2- memory\n");
		printf("(4): fin \n");
		new_line();

		printf(PROMPT);
		fgets(buffer_in, 100, stdin);
		new_line();

		char ** substrings = string_split(buffer_in, " ");

		int i = 0;

		while (substrings[i] != NULL) {
			i++;
		}

		if (i > 2) {
			printf("Incorrecta cantidad de argumentos permitidos\n");
			continue;
		}

		char * cmd_in = NULL;
		cmd_in = substrings[0];
		int opt_var = -1;

		if (substrings[1] != NULL)
			opt_var = strtol(substrings[1], NULL, 10);

		//log_trace(logger, "Comando ingresado: %s %d %s", cmd_in, opt_var, snd_arg);

		if (string_equals_ignore_case(cmd_in, "fin")) {
			fin = 1;
			printf("Fin de la consola\n");
			continue;
		}

		if (no_es_comando(cmd_in)) {
			puts(MSJ_ERROR1);
			continue;
		}

		if (string_equals_ignore_case(cmd_in, "retardo")) {
			modificar_retardo(opt_var);
			continue;
		}

		if (string_equals_ignore_case(cmd_in, "dump")) {
			switch (opt_var) {
			case 1:
				reporte_estructuras();
				break;
			case 2:
				reporte_contenido();
				break;

			default:
				puts(MSJ_ERROR2);
				continue;
			}
		}

		if (string_equals_ignore_case(cmd_in, "flush")) {
			switch (opt_var) {
			case 1:
				limpiar_tlb();
				break;
			case 2:
				marcar_paginas();
				break;
			default:
				puts(MSJ_ERROR2);
				continue;
			}
		}

	}

	free(buffer_in);

	return EXIT_SUCCESS;

}

int no_es_comando(char * com) {

	char * comandos[4] = { "retardo", "dump", "flush", "fin" };

	int i;

	for (i = 0; i < 4; i++) {
		if (string_equals_ignore_case(com, (char *) comandos[i])) {
			return false;
		}
	}

	return true;
}

void inicializar_memoria() {

	memoria_size = umc_config->frames_size * umc_config->cant_frames;

	memoria_principal = malloc(memoria_size);
	memset(memoria_principal, 0, memoria_size);

	log_trace(logger, "Se ha creado el espacio de memoria de %d bytes",
			memoria_size);

	int indice_frame = 0;

	for (indice_frame = 0; indice_frame < memoria_size; indice_frame++) {
		t_mem_frame * frame = malloc(sizeof(t_mem_frame));

		frame->nro_frame = indice_frame;
		frame->pagina = -1;
		frame->pid = -1;
		frame->libre = true;

		list_add(marcos_memoria, frame);
	}
}

void crear_archivo_reporte() {
	/*
	 * Aca en realidad iria una implementacion diferente con respecto
	 * a la representacion del contenido de la memoria
	 *
	 * TASK: AVERIGUAR ESTRATEGIAS DE DUMP!!
	 *
	 * */
	archivo_reporte = txt_open_for_append(REPORT_PATH);
	txt_write_in_file(archivo_reporte, "Reporte de estructuras:\n");
}

void crear_archivo_log() {
	logger = log_create(LOG_FILE, "UMC log", false, LOG_LEVEL_TRACE);
	log_info(logger, "UMC iniciado.");
}

void * conecta_swap() {
	if ((socket_cliente = clienteDelServidor(umc_config->ip_swap,
			umc_config->puerto_swap)) == -1)
		exit(EXIT_FAILURE);

	void * buffer_out;
	buffer_out = serializar_header((uint8_t) UMC, 0);

	if (send(socket_cliente, buffer_out, 5, 0) == -1) {
		printf("Error en el send\n");
	}

	free(buffer_out);

	return EXIT_SUCCESS;

}

void enviar_pagina_size(int sock_fd) {

	void * buffer_out;
	buffer_out = serializar_header((uint8_t) Tamanio_pagina,
			umc_config->frames_size);

	if (send(sock_fd, buffer_out, 5, 0) == -1) {
		printf("No se pudo responder el handshake a nucleo. \n");
	}
	printf("Se ha enviado el tamanio de pagina \n");

	free(buffer_out);
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

			pthread_t hilo_atiende_nucleo;
			pthread_create(&hilo_atiende_nucleo, NULL, atiende_nucleo, NULL);

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

			pthread_t hilo_atiende_cpu;
			pthread_create(&hilo_atiende_cpu, NULL, atiende_cpu,
					(void *) socket_nuevo);
			break;
		default:
			printf("Se conecto alguien desconocido.\n");
			break;
		}

		pthread_mutex_unlock(&mutex_servidor); //UNLOCK MUTEX SERVIDOR
	}

	return EXIT_SUCCESS;
}

void * atiende_nucleo() {

	int recibido = 1;

	while (recibido > 0) {

		pthread_mutex_lock(&mutex_nucleo);

		t_header * head_in = recibir_header(socket_nucleo);

		if (head_in == NULL) {
			recibido = -1;
			pthread_mutex_unlock(&mutex_nucleo);
			continue;
		}

		switch (head_in->identificador) {
		case Inicializar_programa:
			new_line();

			void * buffer_stream = malloc(head_in->tamanio);

			if ((recibido = recv(socket_nucleo, buffer_stream, head_in->tamanio,
			MSG_WAITALL)) == -1) {
				printf("Error en el recv de +++inicializar_programa+++ \n");
				break;
			}
			if (recibido <= 0)
				break;

			t_paquete_inicializar_programa * iniciar_prog_pack =
					deserializar_iniciar_prog(buffer_stream);

			printf("pid: %d\n", iniciar_prog_pack->pid);
			printf("paginas: %d\n", iniciar_prog_pack->paginas_requeridas);
			printf("%s", iniciar_prog_pack->codigo_programa);
			new_line();
			printf("Inicializo programa...\n");
			//inicializar_programa(iniciar_prog_pack);

			free(iniciar_prog_pack);

			break;
		case Finalizar_programa:
			printf("Finalizo programa\n");
			pthread_mutex_lock(&mutex_nucleo);
			finalizar_programa(head_in->tamanio);
			pthread_mutex_unlock(&mutex_nucleo);
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

int inicializar_programa(t_paquete_inicializar_programa * paquete) {

	//mando a swap
	void * buffer = serializar_iniciar_prog(paquete->pid,
			paquete->paginas_requeridas, paquete->codigo_programa);
	int reply = inicializar_en_swap(buffer, 12 + paquete->programa_length);
	free(buffer);

	if (reply == -1)
		return Respuesta__NO;
	//else: si me da el ok -> armo la tabla de paginas
	t_proceso * new_process = malloc(sizeof(t_proceso));
	new_process->pid = paquete->pid;
	new_process->tabla_paginas = list_create();

	int i;
	for (i = 0; i < paquete->paginas_requeridas; i++) {
		t_pagina * page_entry = malloc(sizeof(t_pagina));
		page_entry->pagina = i;
		page_entry->frame = -1;
		page_entry->dirtybit = 0;
		page_entry->presentbit = 0;

		list_add(new_process->tabla_paginas, page_entry);
	}

	list_add(lista_procesos, new_process);

	//le doy el ok a nucleo
	int result = enviar_respuesta_inicio(socket_nucleo, 1);
	//si no se pudo enviar la respuesta a nucleo lo mejor
	//seria enviarle a swap que finalice el programa (libere memoria)
	//en lugar de volver a tratar de enviarla

	if (result <= 0)
		return Respuesta__NO;

	return Respuesta__SI;
}

int inicializar_en_swap(void * buffer, int buffer_size) {

	//primero le envio el header del inicializar programa a swap
	enviar_header(Inicializar_programa, buffer_size, socket_cliente);
	//luego le envio el buffer del paquete de inicializar programa
	int r = send(socket_cliente, buffer, buffer_size, 0);
	if (r <= 0)
		return Respuesta__NO;
	//espero a que me responda
	int respuesta = recibir_respuesta_inicio(socket_cliente);
	//retorno la respuesta

	return respuesta;
}

int finalizar_programa(int id_programa) {
	printf("Programa finalizado.\n");

	//--le aviso a swap que libere el programa
	finalizar_en_swap(id_programa);
	//--luego libero aca: tabla de paginas, libero frames si estaba alocado en memoria, etc...
	bool buscar_proceso(t_proceso * target) {
		return (target->pid == id_programa);
	}
	t_proceso * proceso = list_find(lista_procesos, (void *) buscar_proceso);

	if (proceso == NULL)
		return -1;

	int cantidad_paginas = list_size(proceso->tabla_paginas);

	int i;
	for (i = 0; i < cantidad_paginas; i++) {
		t_pagina * page_entry = list_get(proceso->tabla_paginas, i);

		if (page_entry->presentbit == 1) {
			//libero memoria en el frame en la cual esta cargado el proceso
			bool buscar_frame(t_mem_frame * frame) {
				return ((frame->pagina == page_entry->pagina)
						&& (frame->pid == id_programa));
			}
			t_mem_frame * marco = list_find(marcos_memoria,
					(void *) buscar_frame);

			if (marco == NULL)
				continue;

			marco->libre = 1;
			marco->pagina = -1;
			marco->pid = -1;
		}
	}

	//libero la tabla de paginas
	void lib_entry(t_pagina * target) {
		free(target);
	}
	list_destroy_and_destroy_elements(proceso->tabla_paginas,
			(void *) lib_entry);

	//elimino el proceso de la lista de procesos
	void process_destroyer(t_proceso * self) {
		free(self);
	}
	list_remove_and_destroy_by_condition(lista_procesos,
			(void *) buscar_proceso, (void *) process_destroyer);

	return EXIT_SUCCESS;
}

void finalizar_en_swap(int pid) {
	void * buffer = serializar_fin_prog(pid);
	int r = send(socket_cliente, buffer, 5, 0);

	if (r == -1)
		puts("Error en el send de +++Finalizar_programa+++");
}

void * atiende_cpu(void * args) {

	int socket_cpu = (int) args;

	int recibido = 1;
	void * buffer_in = malloc(5);

	while (recibido > 0) {

		t_header * head_in = recibir_header(socket_cpu);
		int resultado_operacion = -1; //**

		if (head_in == NULL)
			continue; //quizas aca le pueda responder con el mismo formato en el que usare para avisarle el error **

		switch (head_in->identificador) {
		case Solicitar_pagina:
			printf("Lectura de bytes\n");
			resultado_operacion = leer_bytes(socket_cpu, head_in->tamanio);
			break;
		case Almacenar_pagina:
			printf("Almaceno bytes\n");
			resultado_operacion = almacenar_bytes(socket_cpu, head_in->tamanio);
			break;
		case Cambio_proceso_activo:
			printf("Cambio de proceso activo\n");
			resultado_operacion = cambio_proceso_activo(head_in->tamanio,
					socket_cpu);
			if (resultado_operacion == -1)
				recibido = -1;
			break;
		default:
			printf("Cabecera desconocida\n");
			break;
		}

		//--luego del switch compruebo si resultado_operacion == -1
		//--y handleo el error avisandole a cpu que se re pudrio

		free(head_in);
	}

	free(buffer_in);

	return EXIT_SUCCESS;
}

int leer_bytes(int socket_cpu, int bytes) {
	t_paquete_solicitar_pagina * solicitud = recibir_solicitud_lectura(bytes,
			socket_cpu);

	if (solicitud == NULL)
		return -1;

	t_sesion_cpu * cpu = buscar_cpu(socket_cpu);
	if (!cpu)
		return -1;

	bool sol_pag_val = pagina_valida(cpu->proceso_activo, solicitud->nro_pagina);
	if(!sol_pag_val)
		return -1;

	if(tlb_on) {
		read_with_tlb(cpu, solicitud);
	} else {
		read_without_tlb(cpu, solicitud);
	}

	printf("Pagina: %d\n", solicitud->nro_pagina);
	printf("Offset: %d\n", solicitud->offset);
	printf("bytes: %d\n", solicitud->bytes);

	return EXIT_SUCCESS;
}

int almacenar_bytes(int socket_cpu, int bytes) {
	t_paquete_almacenar_pagina * solicitud = recibir_solicitud_escritura(bytes,
			socket_cpu);

	if (solicitud == NULL)
		return -1;

	printf("Pagina: %d\n", solicitud->nro_pagina);
	printf("Offset: %d\n", solicitud->offset);
	printf("bytes: %d\n", solicitud->bytes);
	printf("Buffer: %s\n", (char *) solicitud->buffer);

	return EXIT_SUCCESS;
}

void modificar_retardo(int ret) {
	printf("Retardo modificado\n");
	printf("Retardo anterior: %d\n", umc_config->retardo);
	umc_config->retardo = ret;
	printf("Retardo actual: %d\n", umc_config->retardo);
}

void reporte_estructuras() {
	puts("comando dump - estructuras");
}

void reporte_contenido() {
	puts("comando dump - contenido");
}

void limpiar_tlb() {
	puts("comando flush - tlb");
//list_clean(tlb);
}

void marcar_paginas() {
	printf("comando flush - memory\n");
}

int cambio_proceso_activo(int pid, int cpu) {

	t_sesion_cpu * target = buscar_cpu(cpu);
	if (target == NULL) {
		puts("No se pudo realizar el cambio de proceso activo");
		return -1;
	}

	target->proceso_activo = pid;
	return EXIT_SUCCESS;
}

t_paquete_inicializar_programa * recibir_inicializar_programa(
		int bytes_a_recibir) {
	int recibido = -1;
	void * buffer = malloc(bytes_a_recibir);

	recibido = recv(socket_nucleo, buffer, bytes_a_recibir, 0);

	if (recibido <= 0) {
		puts("Error en el recv de ++inicializar_programa++");
		return NULL;
	}

	t_paquete_inicializar_programa * paquete = deserializar_iniciar_prog(
			buffer);

	free(buffer);
	return paquete;
}
