/*
 * ConsolaDeUMC.c
 *
 *  Created on: 15/7/2016
 *      Author: utnso
 */

#include "UMC.h"

void * lanzar_consola() {
	printf("***CONSOLA DEL PROCESO UMC***\n");
	new_line();

	char * buffer_in = malloc(100);
	int fin = 0;

	while (fin == 0) {
		new_line();
		mostrar_procesos();
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

		if (i > 3) {
			printf("Incorrecta cantidad de argumentos permitidos\n");
			continue;
		}

		char * cmd_in = NULL;
		cmd_in = substrings[0];
		int opt_var = -1;
		int snd_var = -1;

		if (substrings[1] != NULL)
			opt_var = strtol(substrings[1], NULL, 10);

		if (substrings[2] != NULL)
			snd_var = strtol(substrings[2], NULL, 10);

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
				reporte_estructuras(snd_var);
				break;
			case 2:
				reporte_contenido(snd_var);
				break;

			default:
				puts(MSJ_ERROR2);
				continue;
			}
		}

		if (string_equals_ignore_case(cmd_in, "flush")) {
			switch (opt_var) {
			case 1:
				flush_tlb();
				break;
			case 2:
				flush_memory(snd_var);
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

void mostrar_procesos() {
	printf("\n[ ");
	void print_p(t_proceso * p) {
		printf(" %d ", p->pid);
	}
	list_iterate(lista_procesos, (void *) print_p);
	printf(" ]\n");
}

void modificar_retardo(int new_ret) {
	printf("\nModificando Retardo...\n");
	printf("Retardo anterior: %d\n", umc_config->retardo);
	umc_config->retardo = new_ret;
	printf("Retardo actual: %d\n", umc_config->retardo);
}

void reporte_estructuras(int pid) {
	puts("comando dump - estructuras");

	if (pid) {
		t_proceso * proceso = buscar_proceso(pid);

		if (proceso != NULL) {
			reporte_estructuras_del_pid(proceso);
		} else {
			printf("\n*** DUMP_ERROR: NO EXISTE EL PID SOLICITADO ***\n");
		}
	} else {
		//reporte de estructuras de todos los procesos
		int i;
		for (i = 0; i < lista_procesos->elements_count; i++) {
			t_proceso * p = list_get(lista_procesos, i);
			imprimir_tabla_de_paginas(p->tabla_paginas);
		}
	}
}

void reporte_estructuras_del_pid(t_proceso * proceso) {
	printf("\n*** PID: %d ***\n", proceso->pid);

	if (!proceso->tabla_paginas)
		imprimir_tabla_de_paginas(proceso->tabla_paginas);
	//TODO falta persistirlo en el archivo
}

void reporte_contenido(int pid) {
	puts("comando dump - contenido");

	if (pid) {
		if (dump_memory_from_pid(pid) == -1)
			printf("\nError: no existe el pid que se ingreso\n");
	} else {
		puts("dumpeo memoria principal");
		dump_memory(memoria_principal, memoria_size);
	}
}

void flush_tlb() {
	puts("Comando flush - tlb");

	void flush_entry(t_tlb * tlb_entry) {
		tlb_entry->pagina = -1;
		tlb_entry->frame = -1;
		tlb_entry->pid = -1;
	}
	list_iterate(tabla_tlb, (void *) flush_entry);
}

void flush_memory(int pid) {
	printf("Comando flush - memory - pid #%d\n", pid);

	t_proceso * proceso = buscar_proceso(pid);

	if (proceso) {
		void flush_page(t_tabla_pagina * entry) {
			entry->dirtybit = 1;
		}
		list_iterate(proceso->tabla_paginas, (void *) flush_page);
	}
}

void dump_memory(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*) data)[i]);
		fprintf(f_memory_report, "%02X ", ((unsigned char*) data)[i]);
		if (((unsigned char*) data)[i] >= ' '
				&& ((unsigned char*) data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*) data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i + 1) % 8 == 0 || i + 1 == size) {
			printf(" ");
			fprintf(f_memory_report, "", " ");
			if ((i + 1) % 16 == 0) {
				printf("| %s \n", ascii);
				fprintf(f_memory_report, "| %s \n", ascii);
			} else if (i + 1 == size) {
				ascii[(i + 1) % 16] = '\0';
				if ((i + 1) % 16 <= 8) {
					printf(" ");
					fprintf(f_memory_report, "", " ");
				}
				for (j = (i + 1) % 16; j < 16; ++j) {
					printf(" ");
					fprintf(f_memory_report, "", " ");
				}
				printf("| %s \n", ascii);
				fprintf(f_memory_report, "| %s \n", ascii);
			}
		}
	}
}

int dump_memory_from_pid(int pid) {
	t_list * frames_del_proceso = list_create();
	void * copia_memoria = NULL;

	t_proceso * proceso = buscar_proceso(pid);

	if (!proceso)
		return -1;

	void del_proceso_en_mem(t_tabla_pagina * p) {
		if (p->presentbit == 1) {
			list_add(frames_del_proceso, p->frame);
		}
	}
	list_iterate(proceso->tabla_paginas, (void *) del_proceso_en_mem);

	bool de_menor_a_mayor(int f_min, int f_max) {
		return (f_min < f_max);
	}
	list_sort(frames_del_proceso, (void *) de_menor_a_mayor);

	int nro_frames_del_proc = list_size(frames_del_proceso);

	copia_memoria = malloc((nro_frames_del_proc * umc_config->frames_size));

	if (!copia_memoria)
		return -1;

	int j;
	int pointer = 0;
	for (j = 0; j < nro_frames_del_proc; j++) {
		int f = (int) list_get(frames_del_proceso, j);
		memcpy(copia_memoria + pointer,
				memoria_principal + ((f * umc_config->frames_size) - 1), umc_config->frames_size);
		pointer += (umc_config->frames_size - 1);
	}

	dump_memory(copia_memoria, (nro_frames_del_proc * umc_config->frames_size));

	free(copia_memoria);
	free(frames_del_proceso);

	return 0;
}
