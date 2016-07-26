/*
 * ADM.h
 *
 *  Created on: 6/7/2016
 *      Author: utnso
 */

#ifndef ADM_H_
#define ADM_H_

typedef struct {
	int pagina;
	int pid;
	int libre;
} t_mem_frame;

typedef struct {
	int frame;
	int presentbit;
	int accessedbit; //bit de uso
	int dirtybit; //bit de modificado
} t_tabla_pagina;

typedef struct {
	int pagina;
	int frame;
	int pid;
	int referencebit;
} t_tlb;

typedef struct {
	int pid;
	t_list * tabla_paginas;
	t_list * referencias;
} t_proceso;

typedef struct {
	int socket_cpu;
	int id_cpu;
	int proceso_activo;
} t_sesion_cpu;

t_list * tabla_tlb;
t_list * frames_memoria;
t_list * lista_procesos;
int its_clock_m;

//---ESTRUCTURAS Y VALIDACIONES
void inicializar_memoria();
void inicializar_tabla_paginas(int pags, t_proceso * proceso);
int inicializar_proceso(int pid, int paginas);
int pagina_valida(t_proceso * proceso, int pagina);
int supera_limite_frames(int pid);
int hay_frames_libres();
int esta_en_memoria(int pagina, t_proceso * proceso, int dirty_bit);
void incrementar_bit_tlb();
void eliminar_referencia_en_tlb(int pagina, int pid);

//---BUSCADORES
t_tabla_pagina * buscar_pagina(int pagina, t_list * tabla);
t_proceso * buscar_proceso(int pid);
t_sesion_cpu * buscar_cpu(int socket);
t_tlb * buscar_en_tlb(int pagina, int pid);
t_mem_frame * buscar_frame(int frame);
t_mem_frame * buscar_frame_por_pagina(int pagina, int pid);

//---LRU
void run_LRU(t_tlb * reemplazo);

//---CLOCK
int clock_algorithm(int page_to_replace, t_proceso * proceso);
int run_clock(int page_to_replace, t_proceso * proceso);

//---CLOCK MODIFICADO
int clock_modificado(int page_to_replace, t_proceso * proceso, int read_or_write);
int run_clock_modificado(int page_to_replace, t_proceso * proceso, int read_or_write);
int paso_1(int page_to_replace, t_proceso * proceso, int read_or_write);
int paso_2(int page_to_replace, t_proceso * proceso, int read_or_write);

//MISCELLANEOUS
void agregar_referencia(int page_referenced, t_proceso * proceso);
void eliminar_referencia(int page_referenced, t_proceso * proceso);
int agregar_en_frame_libre(int page_to_add, t_proceso * proceso, int dirty_bit);
void imprimir_tabla_de_paginas(t_list * tabla);
void imprimir_frames();
void imprimir_tlb();

//EN MEMORIA FISICA
void * leer_datos(int frame, int offset, int bytes);
void escribir_datos(int frame, t_paquete_almacenar_pagina * solicitud);

#endif /* ADM_H_ */
