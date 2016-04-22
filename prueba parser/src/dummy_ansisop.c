
#include "dummy_ansisop.h"
#include <parser/metadata_program.h>

static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;

t_puntero dummy_definirVariable(t_nombre_variable variable) {
	printf("definir la variable %c\n", variable);
	return POSICION_MEMORIA;
}

t_puntero dummy_obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	return POSICION_MEMORIA;
}

t_valor_variable dummy_dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	return CONTENIDO_VARIABLE;
}

void dummy_asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
}

void dummy_imprimir(t_valor_variable valor) {
	printf("Imprimir %d\n", valor);
}

void dummy_imprimirTexto(char* texto) {
	printf("ImprimirTexto: %s", texto);
}
