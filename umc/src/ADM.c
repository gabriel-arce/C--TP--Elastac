/*
 * ADM.c
 *
 *  Created on: 27/6/2016
 *      Author: utnso
 */

#include "UMC.h"

void inicializar_memoria() {

	memoria_size = umc_config->frames_size * umc_config->cant_frames;

//	memoria_principal = malloc(memoria_size);
//	memset(memoria_principal, 0, memoria_size);
	memoria_principal = (void *) calloc(umc_config->cant_frames, umc_config->frames_size);

//	log_trace(logger, "Se ha creado el espacio de memoria de %d bytes",
//			memoria_size);

	frames_memoria = list_create();
	int indice_frame = 0;

	for (indice_frame = 0; indice_frame < umc_config->cant_frames; indice_frame++) {
		t_mem_frame * frame = malloc(sizeof(t_mem_frame));

		frame->pagina = -1;
		frame->pid = -1;
		frame->libre = 1;

		list_add_in_index(frames_memoria, indice_frame, frame);
	}
}

void inicializar_tabla_paginas(int pags, t_proceso * proceso) {

	int i;
	for (i = 0; i <= pags; i++) {
		t_tabla_pagina * entry = malloc(sizeof(t_tabla_pagina));
		entry->presentbit = 0;
		entry->frame = -1;
		entry->accessedbit = 0;
		entry->dirtybit = 0;

		list_add(proceso->tabla_paginas, entry);
	}
}

int inicializar_proceso(int pid, int paginas) {

	t_proceso * proc = buscar_proceso(pid);

	if (proc) {
		printf("Ya existe un proceso con el pid #%d\n", pid);
		printf("Inicializacion abortada\n");
		return EXIT_FAILURE;
	}

	t_proceso * proceso_nuevo = malloc(sizeof(t_proceso));

	proceso_nuevo->pid = pid;
	proceso_nuevo->tabla_paginas = list_create();
	proceso_nuevo->referencias = list_create();

	inicializar_tabla_paginas(paginas, proceso_nuevo);

	list_add(lista_procesos, proceso_nuevo);

	return EXIT_SUCCESS;
}

int pagina_valida(t_proceso * proceso, int pagina) {

	t_tabla_pagina * pagina_val = buscar_pagina(pagina, proceso->tabla_paginas);

	if (pagina_val) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

int supera_limite_frames(int pid) {

	int cargados_en_memoria = 1;

	void es_del_proceso(t_mem_frame * f) {
		if (f->pid == pid)
			cargados_en_memoria++;
	}
	list_iterate(frames_memoria, (void *) es_del_proceso);

	return (cargados_en_memoria > umc_config->frame_x_prog);
}

int hay_frames_libres() {
	bool es_libre(t_mem_frame * f) {
		return (f->libre == 1);
	}
	return (int) list_any_satisfy(frames_memoria, (void *) es_libre);
}

int esta_en_memoria(int pagina, t_proceso * proceso, int dirty_bit) {

	int esta;

	t_mem_frame * target = buscar_frame_por_pagina(pagina, proceso->pid);

	if (target) {
		printf("Esta en memoria\n");

		t_tabla_pagina * page = buscar_pagina(pagina, proceso->tabla_paginas);
		page->accessedbit = 1;
		page->presentbit = 1;

		if (!(page->dirtybit))
			page->dirtybit = dirty_bit;

		if (tlb_on) {
			t_tlb * reemplazo_tlb = malloc(
					sizeof(t_tlb));
			reemplazo_tlb->referencebit = 0;
			reemplazo_tlb->frame = page->frame;
			reemplazo_tlb->pagina = target->pagina;
			reemplazo_tlb->pid = proceso->pid;

			run_LRU(reemplazo_tlb);
		}

		esta = true;
	} else {
		//page fault
		esta = false;
	}

	return esta;
}

void incrementar_bit_tlb() {
	void incrementar_bit(t_tlb * entry) {
		entry->referencebit++;
	}
	list_iterate(tabla_tlb, (void *) incrementar_bit);
}

void eliminar_referencia_en_tlb(int pagina, int pid) {
	t_tlb * entrada_tlb = buscar_en_tlb(pagina, pid);

	if (entrada_tlb) {
		entrada_tlb->referencebit = 0;
		entrada_tlb->frame = -1;
		entrada_tlb->pagina = -1;
		entrada_tlb->pid = -1;
	}
}

//*****BUSCADORES*****
t_tabla_pagina * buscar_pagina(int pagina, t_list * tabla) {
	t_tabla_pagina * page = NULL;

	page = list_get(tabla, pagina);

	return page;
}

t_proceso * buscar_proceso(int pid) {
	t_proceso * proceso = NULL;

	bool buscar_proc(t_proceso * p) {
		return (p->pid == pid);
	}
	proceso = list_find(lista_procesos, (void *) buscar_proc);

	return proceso;
}

t_sesion_cpu * buscar_cpu(int socket) {
	t_sesion_cpu * cpu = NULL;

	bool find_cpu(t_sesion_cpu * c) {
		return (c->socket_cpu == socket);
	}
	cpu = list_find(cpu_conectadas, (void *) find_cpu);

	return cpu;
}

t_tlb * buscar_en_tlb(int pagina, int pid) {
	t_tlb * tlb_entry = NULL;

	bool find_tlb(t_tlb * entry) {
		return ((entry->pid == pid)&&(entry->pagina == pagina));
	}
	tlb_entry = list_find(tabla_tlb, (void *) find_tlb);

	return tlb_entry;
}

t_mem_frame * buscar_frame(int f) {
	t_mem_frame * frame = NULL;

	frame = list_get(frames_memoria, f);

	return frame;
}

t_mem_frame * buscar_frame_por_pagina(int pagina, int pid) {
	t_mem_frame * frame = NULL;

	bool find_by_page(t_mem_frame * f) {
		return ((f->pagina == pagina)&&(f->pid == pid));
	}
	frame = list_find(frames_memoria, (void *) find_by_page);

	return frame;
}
//****------------*****

//****MISCELLANEOUS****
void agregar_referencia(int page_referenced, t_proceso * proceso) {

	if (its_clock_m)
		eliminar_referencia(page_referenced, proceso);

	bool esta_en_referencias;

	bool esta_en_lista(int ref) {
		return (ref == page_referenced);
	}
	esta_en_referencias = list_any_satisfy(proceso->referencias,
			(void *) esta_en_lista);

	if (!esta_en_referencias)
		list_add(proceso->referencias, (void *) page_referenced);
}


void eliminar_referencia(int page_referenced, t_proceso * proceso) {
	bool la_referenciada(int ref) {
		return (ref == page_referenced);
	}
	list_remove_by_condition(proceso->referencias, (void *) la_referenciada);
}

int agregar_en_frame_libre(int page_to_add, t_proceso * proceso, int dirty_bit) {

	t_mem_frame * frame_libre = NULL;
	int nro_frame = 0;

	for (nro_frame = 0; nro_frame < umc_config->cant_frames; nro_frame++) {
		frame_libre = list_get(frames_memoria, nro_frame);

		if (frame_libre->libre)
			break;
	}

	frame_libre->libre = false;
	frame_libre->pagina = page_to_add;
	frame_libre->pid = proceso->pid;

	t_tabla_pagina * page = buscar_pagina(page_to_add, proceso->tabla_paginas);
	page->accessedbit = 1;
	page->frame = nro_frame;
	page->presentbit = 1;
	page->dirtybit = dirty_bit;

	if (tlb_on) {
		t_tlb * reemplazo_tlb = malloc(sizeof(t_tlb));
		reemplazo_tlb->referencebit = 0;
		reemplazo_tlb->frame = nro_frame;
		reemplazo_tlb->pagina = page_to_add;
		reemplazo_tlb->pid = proceso->pid;

		run_LRU(reemplazo_tlb);
	}

	return nro_frame;
}


void imprimir_tabla_de_paginas(t_list * tabla) {
	int i = 0;
	printf("#P | #F | P | U | D \n");
	void imprimir_entrada(t_tabla_pagina * entry) {
		printf("%d | %d | %d | %d | %d \n", i, entry->frame, entry->presentbit,
				entry->accessedbit, entry->dirtybit);
		i++;
	}
	list_iterate(tabla, (void *) imprimir_entrada);

	new_line();
}

void imprimir_frames() {

	printf("#F | #P | PID | Libre \n");
	int i;
	void imprimir_frame(t_mem_frame * f) {
		printf("%d | %d | %d | %d \n", i, f->pagina, f->pid, f->libre);
		i++;
	}
	list_iterate(frames_memoria, (void *) imprimir_frame);

	new_line();
}

void imprimir_tlb() {
	printf("#P | #F | PID \n");
	int i;
	void imprimir_entrada_tlb(t_tlb * t) {
		printf("%d | %d | %d \n", t->pagina, t->frame, t->pid);
		i++;
	}
	list_iterate(tabla_tlb, (void *) imprimir_entrada_tlb);

	new_line();
}
//****-------------****

//****LECTURAS Y ESCRITURAS****
void * leer_datos(int frame, int offset, int bytes) {
	void * datos = malloc(bytes);
	int dir_fisica = 0;
	dir_fisica = (frame * umc_config->frames_size) + offset;

	memcpy(datos, memoria_principal + dir_fisica, bytes);

	return datos;
}

void escribir_datos(int frame, t_paquete_almacenar_pagina * solicitud) {
	int dir_fisica = 0;
	dir_fisica = (frame * umc_config->frames_size) + solicitud->offset;

	memcpy(memoria_principal + dir_fisica, solicitud->buffer, solicitud->bytes);
}
//****-------------****
