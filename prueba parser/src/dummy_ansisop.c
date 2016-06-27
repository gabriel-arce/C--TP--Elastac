
#include "dummy_ansisop.h"
#include <parser/metadata_program.h>


static const int CONTENIDO_VARIABLE = 20;




t_posicion * dummy_definirVariable(t_nombre_variable variable) {
	printf("definir la variable %c\n", variable);
	 t_posicion * tipo_posicion = malloc(sizeof(t_posicion));
	 tipo_posicion-> posicion = 10;

	return tipo_posicion;
}

t_posicion * dummy_obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	 t_posicion * tipo_posicion = malloc(sizeof(t_posicion));
	 tipo_posicion-> posicion = 10;
	return tipo_posicion;
}

t_valor_variable dummy_dereferenciar(t_posicion * puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero->posicion, CONTENIDO_VARIABLE);
	return CONTENIDO_VARIABLE;
}

void dummy_asignar(t_posicion * puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero->posicion, variable);
}

void dummy_imprimir(t_valor_variable valor) {
	printf("Imprimir %d\n", valor);
}

void dummy_imprimirTexto(char* texto) {
	printf("ImprimirTexto: %s\n", texto);
}

void dummy_retornar(t_valor_variable valor){

	printf("Retornar: %d\n", valor);
}

void dummy_llamarConRetorno(t_nombre_etiqueta etiqueta, t_posicion donde_retornar){

	printf("llamar con retorno a %s y retornar a %d\n", etiqueta, donde_retornar.posicion);
}

void dummy_irAlLabel(t_nombre_etiqueta etiqueta){

	printf("Ir a label: %s\n", etiqueta);
}
