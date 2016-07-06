

#include <stdio.h>
#include <commons/log.h>
#include <string.h>
#include <stdint.h>


#include "dummy_ansisop.h"

#include <parser/metadata_program.h>
#include <parser/parser.h>


static const char* DEFINICION_VARIABLES = "variables a, b, c";
static const char* FINALIZACION = "end";
static const char* ASIGNACION = "a = b + 12";
static const char* IMPRIMIR = "print b";
static const char* IMPRIMIR_TEXTO = "textPrint foo\n";
static const char* CODIGO_COMPLETO = "#!/usr/bin/ansisop \n begin \n variables a, b, c \n  a = b + 12 \n print b \n textPrint foo\n end";
static const char* SALTO = "jnz b inicio_for";
static const char* FUNCTION = "function doble";
static const char* RETORNAR = "return a";



FILE *file;


AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		 = dummy_definirVariable,
		.AnSISOP_obtenerPosicionVariable = dummy_obtenerPosicionVariable,
		.AnSISOP_dereferenciar			 = dummy_dereferenciar,
		.AnSISOP_asignar				 = dummy_asignar,
		.AnSISOP_irAlLabel 				 = dummy_irAlLabel,
		.AnSISOP_llamarConRetorno        = dummy_llamarConRetorno,
		.AnSISOP_retornar				 = dummy_retornar,
		.AnSISOP_imprimir				 = dummy_imprimir,
		.AnSISOP_imprimirTexto			 = dummy_imprimirTexto,
		.AnSISOP_finalizar				 = dummy_finalizar,

};
AnSISOP_kernel kernel_functions = { };

void correrDefinirVariables() {
	printf("Ejecutando '%s'\n", DEFINICION_VARIABLES);
	analizadorLinea(strdup(DEFINICION_VARIABLES), &functions, &kernel_functions);
	printf("================\n");
}

void correrAsignar() {
	printf("Ejecutando '%s'\n", ASIGNACION);
	analizadorLinea(strdup(ASIGNACION), &functions, &kernel_functions);
	printf("================\n");
}

void correrImprimir() {
	printf("Ejecutando '%s'\n", IMPRIMIR);
	analizadorLinea(strdup(IMPRIMIR), &functions, &kernel_functions);
	printf("================\n");
}

void correrImprimirTexto() {
	printf("Ejecutando '%s\n'", IMPRIMIR_TEXTO);
	analizadorLinea(strdup(IMPRIMIR_TEXTO), &functions, &kernel_functions);
	printf("================\n");
}


void leerArchivo(){
	int contadorCaracteres;

file = fopen("/home/utnso/workspace/prueba parser/src/ansisop_script", "r");

if (file) {
    while ((contadorCaracteres = getc(file)) != EOF)
        putchar(contadorCaracteres);
    fclose(file);
}
}


void correrFinalizar() {
	printf("Ejecutando '%s'\n", FINALIZACION);
	analizadorLinea(strdup(IMPRIMIR_TEXTO), &functions, &kernel_functions);
	printf("================\n");
}

void correrSalto(){
	printf("Ejecutando '%s'\n", SALTO);
	analizadorLinea(strdup(SALTO), &functions, &kernel_functions);
	printf("================\n");
}

void correrFunction(){
	printf("Ejecutando '%s'\n", FUNCTION);
	analizadorLinea(strdup(FUNCTION), &functions, &kernel_functions);
	printf("================\n");
}

void correrRetornar(){
	printf("Ejecutando '%s'\n", RETORNAR);
	analizadorLinea(strdup(RETORNAR), &functions, &kernel_functions);
	printf("================\n");
}

int main(int argc, char **argv) {
	correrAsignar();
	correrDefinirVariables();
	correrImprimir();
	correrImprimirTexto();
	correrFinalizar();
	correrSalto();
	correrRetornar();
	//correrFunction();			//TODO rompe al ejecutar
	printf(" El numero de instrucciones del codigo completo es '%d'\n",metadata_desde_literal(CODIGO_COMPLETO)->instrucciones_size);
	t_intructions * instrucciones = metadata_desde_literal(CODIGO_COMPLETO)->instrucciones_serializado;
	printf("primera instruccion inicio:%d\n", instrucciones->start);
	printf("primera instruccion offset:%d\n", instrucciones->offset);
	printf("segunda instruccion inicio:%d\n", (instrucciones+1)->start);

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------
//lo que hay que saber es que el parser tiene dos funciones importantes:
//			metadata_desde_literal que recibe el codigo completo y se usara en el nucleo para crear el PCB
//			analizadorLinea que recibe una sola instruccion y llama a las funciones primitivas que tenemos que crear
