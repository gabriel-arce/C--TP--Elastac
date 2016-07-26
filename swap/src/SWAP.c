/*
 ============================================================================
 Name        : SWAP.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <swap.h>

void inicializar_espacio_swap() {
	char * comando = malloc(sizeof(char) * 50);
	sprintf(comando, "dd if=/dev/zero of=swap.data bs=%d count=%d", swap_config->pagina_size,
			swap_config->cant_paginas);
	system(comando);
	free(comando);

	list_swap = list_create();
}

void inicializar_bitmap() {
	bitmap = list_create();

	int i;
	int bit = 0;
	for (i = 0; i < swap_config->cant_paginas; i++) {
		list_add(bitmap, (void *) bit);
	}
}

bool contiguos_necesarios(int posicion_actual, int cantidad_necesaria) {
	bool hay_contiguos = false;

	if ((posicion_actual + cantidad_necesaria) >= swap_config->cant_paginas)
		return hay_contiguos;

	t_list * siguientes = NULL;
	siguientes = list_create();

	int i;
	for (i = 0; i < cantidad_necesaria; i++) {
		list_add(siguientes, list_get(bitmap, posicion_actual));
		posicion_actual++;
	}

	int n = 0;
	void contar_libres(void * bloque) {
		if (((int) bloque) == 0)
			n++;
	}
	list_iterate(siguientes, (void *) contar_libres);

	if (n == cantidad_necesaria)
		hay_contiguos = true;

	free(siguientes);

	return hay_contiguos;
}

int hay_espacios_libres_contiguos(int cantidad_necesaria) {

	int i;
	for (i = 0; i < swap_config->cant_paginas; i++) {
		int bloque = (int) list_get(bitmap, i);

		if (bloque == 1) {
			continue;
		} else {
			if (contiguos_necesarios(i, cantidad_necesaria)) {
				break;
			} else {
				continue;
			}
		}
	}

	if (i >= swap_config->cant_paginas)
		return -1;

	return i;
}

void marcar_en_bitmap(int start, int end, int bit) {
	int i;
	for (i = 0; i < end; i++) {
		list_replace(bitmap, start, (void *) bit);
		start++;
	}
}

void crear_estructuras_control(int pid, int paginas_totales, int start) {
	int i;
	int p = 0;
	for (i = 0; i < paginas_totales; i++) {
		t_swap * swap_reg = malloc(sizeof(t_swap));
		swap_reg->pid = pid;
		swap_reg->pagina = p;
		swap_reg->posicion_en_swap = start;

		list_add(list_swap, swap_reg);
		p++;
		start++;
	}
}

int iniciar_programa(t_paquete_inicializar_programa * paquete_init,
		bool notyetcompacted) {
	int start = 0;

	if ((start = hay_espacios_libres_contiguos(paquete_init->paginas_requeridas))
			!= -1) {
		crear_estructuras_control(paquete_init->pid,
				paquete_init->paginas_requeridas, start);
		marcar_en_bitmap(start, paquete_init->paginas_requeridas, 1);
		almacenar_codigo(paquete_init->codigo_programa, start);

		return Respuesta_inicio_SI;
	} else {
		//compactacion
		if (notyetcompacted) {
			if (compactar() == -1)
				return Respuesta_inicio_NO;

			return iniciar_programa(paquete_init, false);
		} else {
			return Respuesta_inicio_NO;
		}
	}
}

int almacenar_codigo(char * codigo, int posicion_inicial_swap) {
	int fd = open("swap.data", O_RDWR, S_IWRITE | S_IREAD);

	if (fd == -1)
		return -1;

	struct stat buf;

	if ((stat("swap.data", &buf)) == -1) {
		close(fd);
		return -1;
	}

	char * espacio_swap = mmap((caddr_t) 0, buf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED | MAP_NORESERVE, fd, 0);

	if (espacio_swap == MAP_FAILED) {
		close(fd);
		return -1;
	}

	int codigo_length = string_length(codigo);

	int direccion = (posicion_inicial_swap * swap_config->pagina_size);

	memcpy(espacio_swap + direccion, codigo, codigo_length);
	msync(espacio_swap, buf.st_size, 0);

	close(fd);
	munmap(espacio_swap, buf.st_size);

	return 0;
}

void * leer_bytes(int pid, t_paquete_solicitar_pagina * paquete) {
	int fd = open("swap.data", O_RDONLY);

	if (fd == -1)
		return NULL;

	struct stat buf;

	if ((stat("swap.data", &buf)) == -1) {
		close(fd);
		return NULL;
	}

	char * espacio_swap = mmap((caddr_t) 0, buf.st_size, PROT_READ,
	MAP_SHARED, fd, 0);

	if (espacio_swap == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	int direccion_fisica = calcular_direccion_fisica(pid, paquete->nro_pagina,
			paquete->offset);

	char * datos = string_substring(espacio_swap, direccion_fisica,
			paquete->bytes);

	close(fd);
	munmap(espacio_swap, buf.st_size);

	return (void *) datos;
}

int escribir_bytes(int pid, t_paquete_almacenar_pagina * paquete) {

	if ((swap_config->pagina_size - paquete->bytes) < 0)
		return -1;

	int fd = open("swap.data", O_RDWR, S_IWRITE | S_IREAD);

	if (fd == -1)
		return -1;

	struct stat buf;

	if ((stat("swap.data", &buf)) == -1) {
		close(fd);
		return -1;
	}

	void * espacio_swap = mmap((caddr_t) 0, buf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED | MAP_NORESERVE, fd, 0);

	if (espacio_swap == MAP_FAILED) {
		close(fd);
		return -1;
	}

	int direccion_fisica = calcular_direccion_fisica(pid, paquete->nro_pagina,
			paquete->offset);

	memcpy(espacio_swap + direccion_fisica, paquete->buffer, paquete->bytes);
	msync(espacio_swap, buf.st_size, 0);

	close(fd);
	munmap(espacio_swap, buf.st_size);

	return 0;
}

int calcular_direccion_fisica(int pid, int pagina, int offset) {
	int bloque = 0;
	int direccion_fisica = 0;

	bool buscar_en_swap(t_swap * s) {
		return ((s->pid == pid) && (s->pagina == pagina));
	}
	t_swap * swap_reg = list_find(list_swap, (void *) buscar_en_swap);

	if (swap_reg == NULL)
		return -1;

	bloque = swap_reg->posicion_en_swap;

	direccion_fisica = (bloque * swap_config->pagina_size) + offset;

	return direccion_fisica;
}

void fin_programa(int pid) {

	int i = 0;

	finalizar_en_archivo(pid);

	void limpio_todo(t_swap * reg) {
		if (reg->pid == pid) {
			list_replace(bitmap, reg->posicion_en_swap, (void *) 0);
			list_remove_and_destroy_element(list_swap, i, (void *) free);
		} else {
			i++;
		}
	}
	list_iterate(list_swap, (void *) limpio_todo);
}

int finalizar_en_archivo(int pid) {

	int fd = open("swap.data", O_RDWR, S_IWRITE | S_IREAD);

	if (fd == -1)
		return -1;

	struct stat buf;

	if ((stat("swap.data", &buf)) == -1) {
		close(fd);
		return -1;
	}

	char * espacio_swap = mmap((caddr_t) 0, buf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED | MAP_NORESERVE, fd, 0);

	if (espacio_swap == MAP_FAILED) {
		close(fd);
		return -1;
	}

	char * hueco = string_repeat('\0', swap_config->pagina_size);

	bool from_process(t_swap * sw) {
		return (sw->pid == pid);
	}
	t_list * swaps_del_pid = list_filter(list_swap, (void *) from_process);

	int direccion = 0;
	void liberar_espacio(t_swap * s) {
		direccion = (s->posicion_en_swap) * swap_config->pagina_size;
		memcpy(espacio_swap + direccion, hueco, swap_config->pagina_size);
		direccion = 0;
	}
	list_iterate(swaps_del_pid, (void *) liberar_espacio);

	msync(espacio_swap, buf.st_size, 0);

	close(fd);
	munmap(espacio_swap, buf.st_size);

	return 0;
}

int compactar() {
	sleep(swap_config->retardo_acceso);
	int cantidad_de_elemenos = list_swap->elements_count;
	int i = 0;
	void * copia_persistencia = NULL;

	//CREO UN BITMAP NUEVO
	t_list * bitmap_new = list_create();
	int cantidad_bitmap_original = bitmap->elements_count;
	for (i = 0; i < cantidad_bitmap_original; i++) {
		list_add_in_index(bitmap_new, i, (void *) 0);
	}

	//ABRO EL ESPACIO SWAP Y MAPEO
	int fd = open("swap.data", O_RDWR, S_IWRITE | S_IREAD);

	if (fd == -1)
		return -1;

	struct stat buf;

	if ((stat("swap.data", &buf)) == -1) {
		close(fd);
		return -1;
	}

	char * espacio_swap = mmap((caddr_t) 0, buf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED | MAP_NORESERVE, fd, 0);

	if (espacio_swap == MAP_FAILED) {
		close(fd);
		return -1;
	}

	int tamanio_persistencia = swap_config->pagina_size * swap_config->cant_paginas;
	copia_persistencia = malloc(tamanio_persistencia);
	memset(copia_persistencia, 0, swap_config->pagina_size);

	int offset = 0;
	//RECORRO LA LISTA SWAP Y COPIO EL CONTENIDO
	for (i = 0; i < cantidad_de_elemenos; i++) {
		t_swap * reg_swap = list_get(list_swap, i);

		list_replace(bitmap_new, i, (void *) 1);

		int direccion = reg_swap->posicion_en_swap * swap_config->pagina_size;

		memcpy(copia_persistencia + offset, espacio_swap + direccion,
				swap_config->pagina_size);

		reg_swap->posicion_en_swap = i;

		offset += swap_config->pagina_size;
	}

	free(bitmap);
	bitmap = bitmap_new;

	int tamanio_swap = (int) buf.st_size;
	memcpy(espacio_swap, copia_persistencia, tamanio_swap);
	msync(espacio_swap, buf.st_size, 0);

	close(fd);
	munmap(espacio_swap, buf.st_size);

	return 0;
}

void cargar_config(char ** config_path) {

	t_config * config_file = config_create(config_path[1]);

	swap_config = malloc(sizeof(t_swap_config));
	memset(swap_config, 0, sizeof(t_swap_config));

	swap_config->espacio_swap = malloc(sizeof(char) * 10);
	swap_config->espacio_swap = string_new();
	string_append(&swap_config->espacio_swap,
			getStringProperty(config_file, "NOMBRE_SWAP"));

	swap_config->puerto_escucha = getIntProperty(config_file, "PUERTO_ESCUCHA");
	swap_config->cant_paginas = getIntProperty(config_file, "CANTIDAD_PAGINAS");
	swap_config->pagina_size = getIntProperty(config_file, "TAMANIO_PAGINA");
	swap_config->retardo_compactacion = getIntProperty(config_file,
			"RETARDO_COMPACTACION");
	swap_config->retardo_acceso = getIntProperty(config_file,
				"RETARDO_ACCESO");

	config_destroy(config_file);
}

void imprimir_config() {
	printf("PUERTO_ESCUCHA: %d\n", swap_config->puerto_escucha);
	printf("NOMBRE_SWAP: %s\n", swap_config->espacio_swap);
	printf("CANTIDAD_PAGINAS: %d\n", swap_config->cant_paginas);
	printf("TAMANIO_PAGINA: %d\n", swap_config->pagina_size);
	printf("RETARDO_COMPACTACION: %d\n", swap_config->retardo_compactacion);
	printf("RETARDO_ACCESO: %d\n", swap_config->retardo_acceso);
}

void inicializar_semaforos() {
	pthread_mutex_init(&mutex_umc_recv, 0);
}

void iniciar_servidor_UMC() {
	printf("\n");

	int optval = 1;

	socket_SWAP = crearSocket();

	setsockopt(socket_SWAP, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	bindearSocket(socket_SWAP, swap_config->puerto_escucha);

	escucharEn(socket_SWAP);

	socket_UMC = aceptarEntrantes(socket_SWAP);

	int id = recibir_handshake(socket_UMC);

	if (id == -1)
		puts("Se conecto alguien desconocido");

	if (id == ID_UMC)
		printf("Se conecto UMC con el socket: %d\n", socket_UMC);

	printf("\n");
}

void escuchar_solicitudes_UMC() {
	printf("\n");

	bool keep_iterating = true;
	t_header * header = NULL;

	while (keep_iterating) {
		pthread_mutex_lock(&mutex_umc_recv);

		header = recibir_header(socket_UMC);

		if (!header) {
			keep_iterating = false;
			pthread_mutex_unlock(&mutex_umc_recv);
			continue;
		}

		switch (header->identificador) {
		case Inicializar_programa:
			inicializar_programa(header->tamanio);
			break;
		case Finalizar_programa:
			finalizar_programa(header->tamanio);
			break;
		case Solicitar_pagina:
			leer_pagina(header->tamanio);
			break;
		case Almacenar_pagina:
			almacenar_pagina(header->tamanio);
			break;
		default:
			break;
		}

		free(header);

		pthread_mutex_unlock(&mutex_umc_recv);
	}

	printf("\n");
}

int inicializar_programa(int buffer_init_size) {

	t_paquete_inicializar_programa * paquete_inicio_prog =
			recibir_inicializar_programa(buffer_init_size, socket_UMC);

	int respuesta_inicio = Respuesta_inicio_NO;

	if (paquete_inicio_prog == NULL) {
		enviar_respuesta_inicio(socket_UMC, Respuesta_inicio_NO);
		return EXIT_ERROR;
	}

	printf("pid: %d\n", paquete_inicio_prog->pid);
	printf("paginas: %d\n", paquete_inicio_prog->paginas_requeridas);
	printf("length: %d\n", paquete_inicio_prog->programa_length);
	printf("codigo: \n%s\n", paquete_inicio_prog->codigo_programa);

	respuesta_inicio = iniciar_programa(paquete_inicio_prog, true);

	enviar_a_umc_respuesta_inicio(paquete_inicio_prog, respuesta_inicio);

	return EXIT_SUCCESS;
}

void enviar_a_umc_respuesta_inicio(
		t_paquete_inicializar_programa * paquete_inicio_prog,
		int respuesta_inicio) {
	enviar_respuesta_inicio(socket_UMC, respuesta_inicio);

	enviar_header(Inicializar_programa,
			12 + paquete_inicio_prog->programa_length, socket_UMC);

	enviar_inicializar_programa(paquete_inicio_prog->pid,
			paquete_inicio_prog->paginas_requeridas,
			paquete_inicio_prog->codigo_programa, socket_UMC);

	free(paquete_inicio_prog->codigo_programa);
	free(paquete_inicio_prog);
}

int finalizar_programa(int pid) {

	printf("\nFinalizando programa: %d\n", pid);
	fin_programa(pid);

	return EXIT_SUCCESS;
}

int leer_pagina(int buffer_read_size) {

	sleep(swap_config->retardo_acceso);
	t_paquete_solicitar_pagina * paquete_lect_pag = NULL;
	void * buffer = malloc(buffer_read_size);
	int tamanio_paquete = buffer_read_size - 4;
	void * buffer_paquete = malloc(tamanio_paquete);
	int pid = -1;

	int catch_read_error() {
		send(socket_UMC, (void *) " ", 1, 0);
		free(buffer);
		return EXIT_ERROR;
	}

	if (recv(socket_UMC, buffer, buffer_read_size, 0) <= 0) {
		printf("\nError en el recv para lectura de datos\n");
		free(buffer_paquete);
		return catch_read_error();
	}

	memcpy(&(pid), buffer, 4);
	memcpy(buffer_paquete, buffer + 4, tamanio_paquete);

	paquete_lect_pag = deserializar_leer_pagina(buffer_paquete);

	if (paquete_lect_pag == NULL)
		return catch_read_error();

	printf("pid: %d\n", pid);
	printf("pagina: %d\n", paquete_lect_pag->nro_pagina);
	printf("offset: %d\n", paquete_lect_pag->offset);
	printf("bytes: %d\n", paquete_lect_pag->bytes);

	if (!pagina_valida(pid, paquete_lect_pag->nro_pagina,
			paquete_lect_pag->offset, paquete_lect_pag->bytes)) {
		printf("\nERROR: Solicitud de lectura invalida\n");
		free(paquete_lect_pag);
		return catch_read_error();
	}

	void * datos_leidos = leer_bytes(pid, paquete_lect_pag);

	if (datos_leidos == NULL) {
		printf("\nERROR: No se pudo leer los datos solicitados\n");
		free(paquete_lect_pag);
		return catch_read_error();
	}

	send(socket_UMC, datos_leidos, paquete_lect_pag->bytes, 0);

	free(buffer);
	free(paquete_lect_pag);
	free(datos_leidos);

	return EXIT_SUCCESS;
}

int almacenar_pagina(int buffer_write_size) {

	sleep(swap_config->retardo_acceso);
	puts("ALMACENAR_PAGINA");

	t_paquete_almacenar_pagina * paquete_escrt_pag = NULL;
	void * buffer = malloc(buffer_write_size);
	int tamanio_paquete = buffer_write_size - 4;
	void * buffer_paquete = malloc(tamanio_paquete);
	int pid = -1;

	int catch_write_error() {
		send(socket_UMC, (void *) "-1", 2, 0);
		free(buffer);
		return EXIT_ERROR;
	}

	if (recv(socket_UMC, buffer, buffer_write_size, 0) <= 0) {
		printf("\nError en el recv para escritura de datos\n");
		free(buffer_paquete);
		return catch_write_error();
	}

	memcpy(&(pid), buffer, 4);
	memcpy(buffer_paquete, buffer + 4, tamanio_paquete);

	paquete_escrt_pag = deserializar_almacenar_pagina(buffer_paquete);

	if (paquete_escrt_pag == NULL)
		return catch_write_error();

	printf("pagina: %d\n", paquete_escrt_pag->nro_pagina);
	printf("offset: %d\n", paquete_escrt_pag->offset);
	printf("bytes: %d\n", paquete_escrt_pag->bytes);
	printf("buffer: %s\n", (char *) paquete_escrt_pag->buffer);

	if (!pagina_valida(pid, paquete_escrt_pag->nro_pagina,
			paquete_escrt_pag->offset, paquete_escrt_pag->bytes)) {
		printf("\nERROR: Solicitud de escritura invalida\n");
		free(paquete_escrt_pag->buffer);
		free(paquete_escrt_pag);
		return catch_write_error();
	}

	if ((escribir_bytes(pid, paquete_escrt_pag)) == -1) {
		printf("\nERROR: No se pudo escribir los datos solicitados\n");
		free(paquete_escrt_pag->buffer);
		free(paquete_escrt_pag);
		return catch_write_error();
	}

	send(socket_UMC, (void *) "+1", 2, 0);

	free(buffer);
	free(paquete_escrt_pag->buffer);
	free(paquete_escrt_pag);

	return EXIT_SUCCESS;
}

bool pagina_valida(int pid, int pagina, int offset, int bytes) {
	bool del_pid(t_swap * s) {
		return (s->pid == pid);
	}
	bool existe_el_pid = list_any_satisfy(list_swap, (void *) del_pid);

	if (!existe_el_pid)
		return false;

	t_list * paginas_del_pid = list_filter(list_swap, (void *) del_pid);

	if (paginas_del_pid == NULL)
		return false;

	if (offset > swap_config->pagina_size)
		return false;

	if (bytes > (swap_config->pagina_size - offset))
		return false;

	bool match_pagina(t_swap * p) {
		return (p->pagina == pagina);
	}
	bool existe_la_pagina = list_any_satisfy(paginas_del_pid,
			(void *) match_pagina);

	if (!existe_la_pagina)
		return false;

	return true;
}

//-------------------OUT OF BOUNDS-----------------------

void dump_memory(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*) data)[i]);
		if (((unsigned char*) data)[i] >= ' '
				&& ((unsigned char*) data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*) data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i + 1) % 8 == 0 || i + 1 == size) {
			printf(" ");
			if ((i + 1) % 16 == 0) {
				printf("| %s \n", ascii);
			} else if (i + 1 == size) {
				ascii[(i + 1) % 16] = '\0';
				if ((i + 1) % 16 <= 8) {
					printf(" ");

				}
				for (j = (i + 1) % 16; j < 16; ++j) {
					printf(" ");
				}
				printf("| %s \n", ascii);
			}
		}
	}
}

void ver_que_onda() {
	pthread_mutex_lock(&mutex_umc_recv);

	int fd = open("swap.data", O_RDONLY);

	struct stat buf;

	stat("swap.data", &buf);

	void * espacio_swap = mmap((caddr_t) 0, buf.st_size, PROT_READ,
	MAP_SHARED, fd, 0);

	printf("\n*****************************************************\n");
	printf("\nEstado del espacio swap:\n");
	dump_memory(espacio_swap, buf.st_size);
	printf("\n");
	printf("\nEstado de las estructuas de control:\n");
	mostrar_estructuras_swap();
	printf("\n");
	printf("\nEstado del BITMAP:\n");
	mostrar_bitmap();
	printf("\n*****************************************************\n");

	close(fd);
	munmap(espacio_swap, buf.st_size);

	pthread_mutex_unlock(&mutex_umc_recv);
}

void mostrar_estructuras_swap() {
	void show_struct(t_swap * s) {
		printf("%d | %d | %d\n", s->pid, s->pagina, s->posicion_en_swap);
	}
	printf("i | p | s\n");
	list_iterate(list_swap, (void *) show_struct);
	printf("\n");
}

void mostrar_bitmap() {
	void mostrar(void * bit) {
		printf("%d ", (int) bit);
	}
	printf("\n[ ");
	list_iterate(bitmap, (void *) mostrar);
	printf(" ]\n");
}

void signal_handler(int n_singal) {
	switch (n_singal) {
	case SIGUSR1:
		ver_que_onda();
		break;
	default:
		break;
	}
}
