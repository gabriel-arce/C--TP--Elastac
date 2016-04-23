/*
 * elestac_semaforos.h
 *
 *  Created on: 22 de abr. de 2016
 *      Author: gabriel
 */

#ifndef ELESTAC_SEMAFOROS_H_
#define ELESTAC_SEMAFOROS_H_

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t *crearSemaforo(int cantidadInicial);
sem_t *crearMutex();
void destruirSemaforo(sem_t *semaforo);

void waitSemaforo(sem_t *semaforo);
void signalSemaforo(sem_t *semaforo);

#endif /* ELESTAC_SEMAFOROS_H_ */
