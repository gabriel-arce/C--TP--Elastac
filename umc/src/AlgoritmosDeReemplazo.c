/*
 * AlgoritmosDeReemplazo.c
 *
 *  Created on: 7/7/2016
 *      Author: utnso
 */

#include "UMC.h"

//****TLB_LRU****
void run_LRU(t_tlb * reemplazo) {

	//busco si hay libre
	t_tlb * entrada_libre = buscar_en_tlb(-1, -1);

	if (entrada_libre) {
		entrada_libre->referencebit = 0;
		entrada_libre->frame = reemplazo->frame;
		entrada_libre->pagina = reemplazo->pagina;
		entrada_libre->pid = reemplazo->pid;
	} else {
		//else -> no hay ninguna entrada libre en la tlb

		bool comparo_bit(t_tlb * lru, t_tlb * mru) {
			return (lru->referencebit >= mru->referencebit);
		}
		list_sort(tabla_tlb, (void *) comparo_bit);

		t_tlb * target = list_get(tabla_tlb, 0);

		target->referencebit = 0;
		target->frame = reemplazo->frame;
		target->pagina = reemplazo->pagina;
		target->pid = reemplazo->pid;
	}

	free(reemplazo);
}
//****-------****

//****CLOCK****
int clock_algorithm(int page_to_replace, int offset, int bytes, t_proceso * proceso, int read_or_write) {

	int supera, hay_libres, nro_frame;

	supera = supera_limite_frames(proceso->pid);

	if (supera) {
		nro_frame = run_clock(page_to_replace, offset, bytes, proceso, read_or_write);
	} else {
		hay_libres = hay_frames_libres();
		if (hay_libres) {
			//agrego
			nro_frame = agregar_en_frame_libre(page_to_replace, proceso, read_or_write);
		} else {
			nro_frame = run_clock(page_to_replace, offset, bytes, proceso, read_or_write);
		}
	}

	return nro_frame;
}

int run_clock(int page_to_replace, int offset, int bytes, t_proceso * proceso, int read_or_write) {

	int cant_referencias = 0;
	bool page_victim_not_found = true;
	int index = 0;
	int ref = -1;
	t_tabla_pagina * page_ref = NULL;
	int nro_frame;

	cant_referencias = list_size(proceso->referencias);

	while (page_victim_not_found) {

		if (index >= cant_referencias)
			index = 0;

		ref = (int) list_get(proceso->referencias, index);

		page_ref = NULL;
		page_ref = buscar_pagina(ref, proceso->tabla_paginas);

		if ((page_ref->accessedbit) && (page_ref->presentbit)) {
			page_ref->accessedbit = 0;
		} else {

			nro_frame = page_ref->frame;

			swapping(page_to_replace, ref, proceso->pid, page_ref->dirtybit, nro_frame);

			t_mem_frame * frame_target = buscar_frame(page_ref->frame);
			frame_target->pagina = page_to_replace;
			frame_target->pid = proceso->pid;
			frame_target->libre = 0;

			t_tabla_pagina * pagina_reemp = buscar_pagina(page_to_replace,
					proceso->tabla_paginas);
			pagina_reemp->accessedbit = 1;
			pagina_reemp->presentbit = 1;
			pagina_reemp->frame = page_ref->frame;
			pagina_reemp->dirtybit = read_or_write;

			page_ref->accessedbit = 0;
			page_ref->presentbit = 0;
			page_ref->frame = -1;
			page_ref->dirtybit = 0;

			if (tlb_on) {
				eliminar_referencia_en_tlb(ref, proceso->pid);

				t_tlb * reemplazo_tlb = malloc(sizeof(t_tlb));
				reemplazo_tlb->referencebit = 0;
				reemplazo_tlb->frame = pagina_reemp->frame;
				reemplazo_tlb->pagina = page_to_replace;
				reemplazo_tlb->pid = proceso->pid;

				run_LRU(reemplazo_tlb);
			}

			eliminar_referencia(ref, proceso);
			page_victim_not_found = false;
			continue;
		}

		index++;
	}

	return nro_frame;
}
//****-----****

//****CLOCK_MODIFICADO****
int clock_modificado(int page_to_replace, int offset, int bytes, t_proceso * proceso,
		int read_or_write) {

	int supera, hay_libres, nro_frame;

	supera = supera_limite_frames(proceso->pid);

	if (supera) {
		nro_frame = run_clock_modificado(page_to_replace, offset, bytes, proceso, read_or_write);
	} else {
		hay_libres = hay_frames_libres();
		if (hay_libres) {
			//agrego
			nro_frame = agregar_en_frame_libre(page_to_replace, proceso, read_or_write);
		} else {
			nro_frame = run_clock_modificado(page_to_replace, offset, bytes, proceso, read_or_write);
		}
	}

	return nro_frame;
}

int run_clock_modificado(int page_to_replace, int offset, int bytes, t_proceso * proceso,
		int read_or_write) {

	bool page_victim_not_found = true;
	int res_paso1;
	int res_paso2;
	int nro_frame = -1;

	while (page_victim_not_found) {
		res_paso1 = paso_1(page_to_replace, offset, bytes, proceso, read_or_write);

		if (res_paso1 >= 0) {
			page_victim_not_found = false;
			nro_frame = res_paso1;
			continue;
		} else {
			res_paso2 = paso_2(page_to_replace, offset, bytes, proceso, read_or_write);
			if (res_paso2 >= 0) {
				page_victim_not_found = false;
				nro_frame = res_paso2;
				continue;
			} else {
				continue;
			}
		}
	}

	return nro_frame;
}

int paso_1(int page_to_replace, int offset, int bytes, t_proceso * proceso, int read_or_write) {

	int cant_referencias = list_size(proceso->referencias);
	int i;
	int ref = -1;
	t_tabla_pagina * page_ref = NULL;
	int nro_frame = -1;

	for (i = 0; i < cant_referencias; i++) {
		ref = (int) list_get(proceso->referencias, i);
		page_ref = buscar_pagina(ref, proceso->tabla_paginas);

		if ((page_ref->presentbit) && (page_ref->accessedbit == 0)
				&& (page_ref->dirtybit == 0)) {

			nro_frame = page_ref->frame;

			swapping(page_to_replace, ref, proceso->pid, page_ref->dirtybit, nro_frame);

			t_mem_frame * frame_target = buscar_frame(page_ref->frame);
			frame_target->libre = 0;
			frame_target->pagina = page_to_replace;
			frame_target->pid = proceso->pid;

			t_tabla_pagina * page_rplc = buscar_pagina(page_to_replace,
					proceso->tabla_paginas);
			page_rplc->frame = page_ref->frame;
			page_rplc->accessedbit = 1;
			page_rplc->dirtybit = read_or_write;
			page_rplc->presentbit = 1;

			page_ref->accessedbit = 0;
			page_ref->dirtybit = 0;
			page_ref->frame = -1;
			page_ref->presentbit = 0;

			eliminar_referencia(ref, proceso);

			if (tlb_on) {
				eliminar_referencia_en_tlb(ref, proceso->pid);

				t_tlb * reemplazo_tlb = malloc(
						sizeof(t_tlb));
				reemplazo_tlb->referencebit = 0;
				reemplazo_tlb->frame = page_rplc->frame;
				reemplazo_tlb->pagina = page_to_replace;
				reemplazo_tlb->pid = proceso->pid;

				run_LRU(reemplazo_tlb);
			}

			break;
		}
	}

	if (i >= cant_referencias)
		return -1;

	return nro_frame;
}

int paso_2(int page_to_replace, int offset, int bytes, t_proceso * proceso, int read_or_write) {

	int cant_referencias = list_size(proceso->referencias);
	int i;
	int ref = -1;
	t_tabla_pagina * page_ref = NULL;
	int nro_frame = -1;

	for (i = 0; i < cant_referencias; i++) {
		ref = (int) list_get(proceso->referencias, i);
		page_ref = buscar_pagina(ref, proceso->tabla_paginas);

		if ((page_ref->presentbit) && (page_ref->accessedbit == 0)
				&& (page_ref->dirtybit == 1)) {

			nro_frame = page_ref->frame;

			swapping(page_to_replace, ref, proceso->pid, page_ref->dirtybit, nro_frame);

			t_mem_frame * frame_target = buscar_frame(page_ref->frame);
			frame_target->libre = 0;
			frame_target->pagina = page_to_replace;
			frame_target->pid = proceso->pid;

			t_tabla_pagina * page_rplc = buscar_pagina(page_to_replace,
					proceso->tabla_paginas);
			page_rplc->frame = page_ref->frame;
			page_rplc->accessedbit = 1;
			page_rplc->dirtybit = read_or_write;
			page_rplc->presentbit = 1;

			page_ref->accessedbit = 0;
			page_ref->dirtybit = 0;
			page_ref->frame = -1;
			page_ref->presentbit = 0;

			eliminar_referencia(ref, proceso);

			if (tlb_on) {
				eliminar_referencia_en_tlb(ref, proceso->pid);

				t_tlb * reemplazo_tlb = malloc(
						sizeof(t_tlb));
				reemplazo_tlb->referencebit = 0;
				reemplazo_tlb->frame = page_rplc->frame;
				reemplazo_tlb->pagina = page_to_replace;
				reemplazo_tlb->pid = proceso->pid;

				run_LRU(reemplazo_tlb);
			}

			break;
		} else {
			page_ref->accessedbit = 0;
		}
	}

	if (i >= cant_referencias)
		return -1;

	return nro_frame;
}

//****----------------****
