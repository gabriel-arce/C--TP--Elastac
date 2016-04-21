/*
 * Test_config.c
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#include "CUnit/Basic.h"
#include <stdio.h>
#include "../nucleo/src/Nucleo.h"


static FILE *archivo = NULL;
static char *path	 = "../nucleo/src/nucleo.conf";

void test_si_existe_config(){
	//CU_ASSERT_EQUAL(inicializar_conf(),0);
	CU_ASSERT((archivo = fopen(path, "r")) != NULL);
}

void test_si_puerto_programas_tiene_puerto_existente(){
	cargar_conf();
//	CU_ASSERT_EQUAL(nucleo->puerto_programas,0);
}

int main(){
	CU_initialize_registry();

	CU_pSuite prueba = CU_add_suite("Suite de prueba", NULL, NULL);
	CU_add_test(prueba, "Probando que la configuracion existe", test_si_existe_config);
//	CU_add_test(prueba, "Probando que el puerto de programas tenga un puerto existente", test_si_puerto_programas_tiene_puerto_existente);
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}
