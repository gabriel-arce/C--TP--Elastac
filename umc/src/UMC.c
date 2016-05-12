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
	char cadFin[] = "fin";

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
		scanf("%[^\n]%*c", buffer_in);
		new_line();

		char ** substrings = string_split(buffer_in, " ");
		char * cmd_in = substrings[0];
		int opt_var = strtol(substrings[1], NULL, 10);
		char * snd_arg = substrings[2];

		//log_trace(logger, "Comando ingresado: %s %d %s", cmd_in, opt_var, snd_arg);

		if (string_equals_ignore_case(cmd_in, cadFin)) {
			fin = 1;
			free(buffer_in);
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
				reporte_estructuras(snd_arg);
				break;
			case 2:
				reporte_contenido(snd_arg);
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
				marcar_paginas(snd_arg);
				break;
			default:
				puts(MSJ_ERROR2);
				continue;
			}
		}

	}

	return EXIT_SUCCESS;

}

int no_es_comando(char * com) {

	char * comandos[3] = { "retardo", "dump", "flush" };

	int i;

	for (i = 0; i < 3; i++) {
		if (string_equals_ignore_case(com, (char *) comandos[i])) {
			return false;
		}
	}

	return true;
}

void inicializar_memoria() {

	memoria_principal = malloc(
			umc_config->frames_size * umc_config->cant_frames);
	memset(memoria_principal, 0, memoria_size);

	log_trace(logger, "Se ha creado el espacio de memoria de %d bytes",
			memoria_size);

	int pos, indice_frame = 0;
	while (pos < memoria_size) {
		memoria_principal[pos] = 'X';
		pos++;
	}

	for (indice_frame = 0; indice_frame < memoria_size; indice_frame++) {
		t_mem_frame * frame = malloc(sizeof(t_mem_frame));

		frame->nro_frame = indice_frame;
		frame->pagina = -1;
		frame->pid = -1;
		frame->libre = true;

		list_add(lista_frames, frame);
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

	if (enviar_handshake(socket_cliente, 3, 0))
		printf("No se pudo enviar el handshake a swap\n");

	t_header * handshake_in = malloc(sizeof(t_header));

	recibir_handshake(socket_cliente, handshake_in);

	if (handshake_in->identificador == 4) {
		//creo el hilo para atender a swap
		printf("Se conecto SWAP.\n");
	} else {
		printf("Se conecto alguien desconocido\n");
	}

	free(handshake_in);

	return EXIT_SUCCESS;

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
	int socket_nuevo = 0;
	int recibido = 1;

	while (recibido > 0) {

		socket_nuevo = accept(socket_servidor,
				(struct sockaddr*) &direccion_cliente, &addrlen);

		if (socket_nuevo == -1) {
			printf("Error en el accept.\n");
		}

		setsockopt(socket_nuevo, SOL_SOCKET, SO_REUSEADDR, &optval,
				sizeof(optval));

		pthread_mutex_lock(&mutex_hilos);
		contador_hilos++;
		pthread_mutex_unlock(&mutex_hilos);

		t_header * handshake_in = malloc(sizeof(t_header));

		recibido = recibir_handshake(socket_nuevo, handshake_in);

		switch (handshake_in->identificador) {
		case 2:
			new_line();
			printf("Se conecto Nucleo \n");
			t_sesion_nucleo * nucleo = malloc(sizeof(t_sesion_nucleo));
			nucleo->socket_nucleo = socket_nuevo;

			if (enviar_handshake(nucleo->socket_nucleo, 3, 0) == -1) {
				printf("No se pudo responder el handshake a nucleo. \n");
			}
			printf("Handshake con nucleo exitoso \n");

			//creo el hilo para atender el nucleo
			break;
		case 5:
			new_line();
			printf("Se conecto una CPU \n");
			t_sesion_cpu * cpu = malloc(sizeof(t_sesion_cpu));
			cpu->socket_cpu = socket_nuevo;
			pthread_mutex_lock(&mutex_lista_cpu);
			cpu->id_cpu = ++id_cpu;
			list_add(cpu_conectadas, cpu);
			pthread_mutex_unlock(&mutex_lista_cpu);

			if (enviar_handshake(cpu->socket_cpu, 3, umc_config->frames_size)
					== -1) {
				printf("No se pudo enviar el handshake a cpu \n");
			}
			printf("Handshake con cpu #%d exitoso \n", cpu->id_cpu);
			//creo el hilo para atender los cpu
			break;
		default:
			printf("Se conecto alguien desconocido.\n");
			break;
		}

		free(handshake_in);
	}

	return EXIT_SUCCESS;
}

void modificar_retardo(int ret) {
	printf("comando retardo %d\n", ret);
	umc_config->retardo = ret;
}

void reporte_estructuras(char * arg) {
	puts("comando dump - estructuras");

	if (string_equals_ignore_case(arg, "todo")) {
		puts("dump - estructuras - todo");
	} else {
		puts("dump - estructuras - proceso en particular");
	}

}

void reporte_contenido(char * arg) {
	puts("comando dump - contenido");

	if (string_equals_ignore_case(arg, "todo")) {
		puts("dump - contenido - todo");
	} else {
		puts("dump - contenido - proceso en particular");
	}
}

void limpiar_tlb() {
	puts("comando flush - tlb");
//list_clean(tlb);
}

void marcar_paginas(char * arg) {
	printf("comando flush - memory - %s\n", arg);
}
