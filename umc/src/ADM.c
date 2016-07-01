/*
 * ADM.c
 *
 *  Created on: 27/6/2016
 *      Author: utnso
 */

#include "UMC.h"

bool pagina_valida(int pid, int pagina) {
	t_proceso * proceso = buscar_proceso(pid);
	if (!proceso)
		return false;

	bool search_page(t_pagina * target) {
		return (target->pagina == pagina);
	}

	return list_any_satisfy(proceso->tabla_paginas, (void *) search_page);
}

t_sesion_cpu * buscar_cpu(int socket) {
	bool buscar_cpu(t_sesion_cpu * sesion) {
		return (sesion->socket_cpu == socket);
	}
	t_sesion_cpu * target = list_find(cpu_conectadas, (void *) buscar_cpu);

	if (target == NULL)
		puts("No se pudo encontrar la cpu");

	return target;
}

t_proceso * buscar_proceso(int pid) {
	bool buscar_elem(t_proceso * elem) {
		return (elem->pid == pid);
	}
	t_proceso * proceso = list_find(lista_procesos, (void *) buscar_elem);

	if (proceso == NULL)
		puts("No se pudo encontrar el proceso");

	return proceso;
}

t_mem_frame * buscar_frame(int pagina, int pid) {
	bool buscar_elem(t_mem_frame * elem) {
		return ((elem->pagina == pagina) && (elem->pid == pid));
	}
	t_mem_frame * frame = list_find(lista_procesos, (void *) buscar_elem);

	if (frame == NULL)
		puts("No se pudo encontrar el frame");

	return frame;
}

void run_LRU_algorithm(t_tlb * entry_to_replace) {

}

void read_with_tlb(t_sesion_cpu * cpu, t_paquete_solicitar_pagina * solicitud) {

	int frame_to_read = -1;

	//busco la pagina en la tlb
	bool buscar_en_tlb(t_tlb * target) {
		return ((target->pagina == solicitud->nro_pagina)
				&& (target->pid == cpu->proceso_activo));
	}
	t_tlb * tlb_entry = list_find(tlb, (void *) buscar_en_tlb);

	if (tlb_entry) {
		//----no es NULL entonces tlb hit
		void * datos = leer_datos(tlb_entry->frame, solicitud->offset,
				solicitud->bytes);
		send(cpu->socket_cpu, datos, solicitud->bytes, 0);
		free(datos);
		tlb_entry->referencebit = 0;
	} else {
		//----tlb miss
		t_tlb * entry_to_replace = malloc(sizeof(t_tlb));
		entry_to_replace->pagina = solicitud->nro_pagina;
		entry_to_replace->pid = cpu->proceso_activo;
		entry_to_replace->referencebit = 0;
		//--busco en memoria
		t_mem_frame * frame = buscar_frame(solicitud->nro_pagina,
				cpu->proceso_activo);
		if (frame) {
			//----esta en memoria
			void * datos = leer_datos(frame->nro_frame, solicitud->offset,
					solicitud->bytes);
			send(cpu->socket_cpu, datos, solicitud->bytes, 0);
			free(datos);
			entry_to_replace->frame = frame->nro_frame;
		} else {
			//----no se encuentra en memoria -> PAGE FAULT
			//--corro el algoritmo de reemplazo
			switch (umc_config->algoritmo) {
			case CLOCK:
				frame_to_read = run_clock(cpu->proceso_activo,
						solicitud->nro_pagina);
				break;
			case CLOCK_MODIFICADO:
				frame_to_read = run_clock_modificado(cpu->proceso_activo,
						solicitud->nro_pagina);
				break;
			}
			//--envio los datos
			void * datos = leer_datos(frame_to_read, solicitud->offset,
					solicitud->bytes);
			send(cpu->socket_cpu, datos, solicitud->bytes, 0);
			free(datos);
		}
		entry_to_replace->frame = frame_to_read;
		//--corro el algoritmo de reemplazo lru en tlb
		run_LRU_algorithm(entry_to_replace);
	}

}

void read_without_tlb(t_sesion_cpu * cpu,
		t_paquete_solicitar_pagina * solicitud) {

}

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

int run_clock(int pid, int page_to_replace) {

}

int run_clock_modificado(int pid, int page_to_replace) {

}

bool supera_limite_frames(int pid) {

	int frames_del_proceso = 0;

	bool es_del_proceso(t_mem_frame * self) {
		return (self->pid == pid);
	}
	frames_del_proceso = list_count_satisfying(marcos_memoria,
			(void *) es_del_proceso);

	if (frames_del_proceso <= umc_config->frame_x_prog) {
		puts(
				"No se puede escribir en memoria porque el proceso se encuetra en su maximo de marcos permitidos");
		return false;
	}

	return true;
}
