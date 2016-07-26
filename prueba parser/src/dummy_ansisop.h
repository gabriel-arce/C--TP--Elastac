
#ifndef DUMMY_ANSISOP_H_
#define DUMMY_ANSISOP_H_

	#include <parser/parser.h>
	#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

	typedef struct {
	int posicion ;
	char size;
	}t_posicion;

	t_posicion * dummy_definirVariable(t_nombre_variable variable);

	t_posicion * dummy_obtenerPosicionVariable(t_nombre_variable variable);
	t_valor_variable dummy_dereferenciar(t_posicion * puntero);
	void dummy_asignar(t_posicion * puntero, t_valor_variable variable);

	void dummy_imprimir(t_valor_variable valor);
	void dummy_imprimirTexto(char* texto);

	void dummy_retornar(t_valor_variable valor);
	void dummy_llamarConRetorno(t_nombre_etiqueta etiqueta, t_posicion donde_retornar);
	void dummy_irAlLabel(t_nombre_etiqueta etiqueta);
	void dummy_finalizar();

#endif

