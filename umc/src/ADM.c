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
		return ((elem->pagina == pagina)&&(elem->pid == pid));
	}
	t_mem_frame * frame = list_find(lista_procesos, (void *) buscar_elem);

	if (frame == NULL)
		puts("No se pudo encontrar el frame");

	return frame;
}

int run_LRU_algorithm(t_tlb * entry_to_replace) {

}

int read_with_tlb(t_sesion_cpu * cpu, t_paquete_solicitar_pagina * solicitud) {

	//busco la pagina en la tlb
	bool buscar_en_tlb(t_tlb * target) {
		return ((target->pagina == solicitud->nro_pagina)
				&& (target->pid == cpu->proceso_activo));
	}
	t_tlb * tlb_entry = list_find(tlb, (void *) buscar_en_tlb);

	if (tlb_entry) {
		//----no es NULL entonces tlb hit

	} else {
		//----tlb miss
	}

}

int read_without_tlb(t_sesion_cpu * cpu, t_paquete_solicitar_pagina * solicitud) {

}

