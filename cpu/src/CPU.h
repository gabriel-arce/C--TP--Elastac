/*
 * CPU.h
 *
 *  Created on: 27/4/2016
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <elestac_config.h>

#define CONFIG_PATH "../CPU/src/CPU.conf"

typedef struct {
	int puerto_nucleo;
	char * ip_nucleo;
	int puerto_UMC;
	char * ip_UMC;
} t_CPU_config;

t_CPU_config * cargar_config();

#endif /* CPU_H_ */
