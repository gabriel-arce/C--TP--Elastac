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

	nucleo->sem_ids  		 = list_create();
	nucleo->sem_init 		 = list_create();
	nucleo->io_ids	 		 = list_create();
	nucleo->io_sleep 		 = list_create();
	nucleo->shared_vars = list_create();

	nucleo->puerto_programas	= getIntProperty(config, "PUERTO_PROG");
	nucleo->puerto_cpu					= getIntProperty(config, "PUERTO_CPU");
	nucleo->quantum						= getIntProperty(config, "QUANTUM");
	nucleo->quantum_sleep			= getIntProperty(config, "QUANTUM_SLEEP");
	nucleo->sem_ids						= obtener_lista(config, "SEM_IDS");
	nucleo->sem_init						= obtener_lista(config, "SEM_INIT");
	nucleo->io_ids							= obtener_lista(config, "IO_IDS");
	nucleo->io_sleep						= obtener_lista(config, "IO_SLEEP");
	nucleo->shared_vars				= obtener_lista(config, "SHARED_VARS");

	config_destroy(config);

	return nucleo;
}

t_list *obtener_lista(t_config *config, char *property){
	char **items = config_get_array_value(config, property);
	t_list *ret  = list_create();
	int cant	 = string_count(config_get_string_value(config, property), ",") +1;

	int i = 0;

	for(i; i < cant; i++)
		list_add(ret, (void *)string_duplicate(items[i]));

	return ret;
}

int string_count(char *text, char *pattern){
	char **chunks = string_split(text, pattern);
	int ret;
	for(ret = 0; chunks[ret]; ret++);
	return ret - 1;
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

