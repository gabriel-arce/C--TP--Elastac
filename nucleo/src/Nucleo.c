/*
 ============================================================================
 Name        : Nucleo.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Nucleo.h"


t_nucleo *cargar_conf(){
	t_config *config = config_create(CONFIG_NUCLEO);
	t_nucleo *nucleo = malloc(sizeof(t_nucleo));

// Inicializar estructura
	nucleo->puerto_programas = 0;
	nucleo->puerto_cpu = 0;
	nucleo->quantum = 0;
	nucleo->quantum_sleep = 0;

	nucleo->puerto_programas	= getIntProperty(config, "PUERTO_PROG");
	nucleo->puerto_cpu					= getIntProperty(config, "PUERTO_CPU");
	nucleo->quantum						= getIntProperty(config, "QUANTUM");
	nucleo->quantum_sleep			= getIntProperty(config, "QUANTUM_SLEEP");
	nucleo->sem_ids						= getListProperty(config, "SEM_IDS");
	nucleo->sem_init						= getListProperty(config, "SEM_INIT");
	nucleo->io_ids							= getListProperty(config, "IO_IDS");
	nucleo->io_sleep						= getListProperty(config, "IO_SLEEP");
	nucleo->shared_vars				= getListProperty(config, "SHARED_VARS");

	config_destroy(config);

	return nucleo;
}

int get_quantum(t_nucleo *nucleo){
	return nucleo->quantum;
}

int get_quantum_sleep(t_nucleo *nucleo){
	return nucleo->quantum_sleep;
}

/*
 * Busca en array todas las posiciones con -1 y las elimina, copiando encima
 * las posiciones siguientes.
 * Ejemplo, si la entrada es (3, -1, 2, -1, 4) con *n=5
 * a la salida tendremos (3, 2, 4) con *n=3
 */
void compactaClaves (int *tabla, int *n)
{
	int i,j;

	if ((tabla == NULL) || ((*n) == 0))
		return;

	j=0;
	for (i=0; i<(*n); i++)
	{
		if (tabla[i] != -1)
		{
			tabla[j] = tabla[i];
			j++;
		}
	}

	*n = j;
}


/*
 * Función que devuelve el valor máximo en la tabla.
 * Supone que los valores válidos de la tabla son positivos y mayores que 0.
 * Devuelve 0 si n es 0 o la tabla es NULL */
int dameMaximo (int *tabla, int n)
{
	int i;
	int max;

	if ((tabla == NULL) || (n<1))
		return 0;

	max = tabla[0];
	for (i=0; i<n; i++)
		if (tabla[i] > max)
			max = tabla[i];

	return max;
}

