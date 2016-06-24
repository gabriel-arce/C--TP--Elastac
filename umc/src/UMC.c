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

int tlb_habilitada() {
	return (umc_config->entradas_tlb > 0);
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
	//si me da el ok -> armo la tabla de paginas
	//le doy el ok a nucleo

	return EXIT_SUCCESS;
}

int inicializar_en_swap(void * buffer) {

	//le envio a swap el buffer del paquete de inicializar programa
	//espero a que me responda -> es un header id=Respuesta_inicio; size= 1->ok, 0->refused
	//retorno la respuesta....o sea header->size

	return EXIT_SUCCESS;
}

int finalizar_programa(int id_programa) {
	printf("Programa finalizado.\n");

	//--le aviso a swap que libere el programa
	//--luego libero aca: tabla de paginas, libero frames si estaba alocado en memoria, etc...
	//--NOTA: puede que tenga que esperar algun tipo de respuesta de swap para antes avisarle a nucleo
	//que libere el programa

	return EXIT_SUCCESS;
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
			resultado_operacion = solicitar_bytes(socket_cpu);
			break;
		case Almacenar_pagina:
			printf("Almaceno bytes\n");
			resultado_operacion = almacenar_bytes(socket_cpu);
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

int solicitar_bytes(int socket_cpu) {
	t_paquete_solicitar_pagina * solicitud = recibir_solicitud_escritura(
			socket_cpu);

	if (solicitud == NULL)
		return -1;

	return EXIT_SUCCESS;
}

int almacenar_bytes(int socket_cpu) {
	t_paquete_almacenar_pagina * solicitud = recibir_solicitud_escritura(
			socket_cpu);

	if (solicitud == NULL)
		return -1;

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
	bool buscar_cpu(t_sesion_cpu * sesion) {
		return (sesion->socket_cpu == cpu);
	}
	t_sesion_cpu * target = list_find(cpu_conectadas, (void *) buscar_cpu);

	if (target == NULL) {
		puts(
				"No se encontró la cpu conectada para el cambio de proceso activo");
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

t_paquete_solicitar_pagina * recibir_solicitud_lectura(int socket_cpu) {

}

t_paquete_almacenar_pagina * recibir_solicitud_escritura(int socket_cpu) {

}
