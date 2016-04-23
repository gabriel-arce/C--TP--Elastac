/*
 * elestac_semaforos.c
 *
 *  Created on: 22 de abr. de 2016
 *      Author: gabriel
 */

#include "elestac_semaforos.h"

sem_t *crearSemaforo(int cantidadInicial){
	sem_t *semaforo = malloc(sizeof(sem_t));
	if(sem_init(semaforo,0,cantidadInicial) == -1 ){
		exit(EXIT_FAILURE);
	}
	return semaforo;
}

sem_t *crearMutex(){
	return crearSemaforo(1);
}

void destruirSemaforo(sem_t *semaforo){
	sem_destroy(semaforo);
}

void waitSemaforo(sem_t *semaforo){
	if (sem_wait(semaforo) == -1 ){
		exit(EXIT_FAILURE);
	}
}

void signalSemaforo(sem_t *semaforo){
	if (sem_post(semaforo) == -1 ){
		exit(EXIT_FAILURE);
	}
}
