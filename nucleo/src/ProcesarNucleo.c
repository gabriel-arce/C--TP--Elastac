/*
 * ProcesarNucleo.c
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#include <elestac_sockets.h>

#include "Nucleo.h"

int main(void) {

	t_nucleo *nucleo = NULL;
	int listener;									//Descriptor de escucha
	int reuse = 1;								//Indicador de reusabilidad del socket
	sigset_t mask;								//Mascara
	sigset_t orig_mask;					//Mascara original
	int fdmax;									//Numero maximo de descriptores a recibir
	int newfd;									//Descriptor de conexion aceptada



	//Cargar configuracion
	printf("Inicializando Nucleo..\n");
	printf("please wait..\n");
	printf("\n");

	nucleo = cargar_conf();

	//Set a 0 los conjuntos maestro y temporal
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	//Crear socket de escucha
	if( (listener = crearSocket()) == -1 ){
		perror("Nucleo: Error al abrir socket de espera\n");
		exit(1);
	}

	printf("socket creado..\n");

	//descriptor para enlace
	if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1){
		perror("Nucleo: No es posible reusar el socket\n");
		exit(1);
	}

	//Enlazar
	if((bindearSocket(listener, PUERTO_NUCLEO)) == -1){
		perror("Nucleo: No se puede enlazar al puerto: direccion ya esta en uso\n");
		exit(1);
	}

	printf("Enlazado al puerto %d\n", PUERTO_NUCLEO);


	//Escuchar
	if((escucharEn(listener)) == -1){
		exit(1);
	}

	printf("Escuchando a consolas o a cpu's..\n");

	//Asegurar a pselect no se bloquee por seniales mientras se este ejecutando
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGPOLL);

	if((sigprocmask(SIG_BLOCK, &mask, &orig_mask)) < 0 )
		perror("Nucleo: No se puede bloquear\n");

	FD_SET(listener, &master);

	while(1){
		read_fds = master;

		if((pselect(fdmax+1, &read_fds, NULL, NULL, NULL, NULL )) == -1){

			//pselect(int nfds, fd_set *readfds, fs_set *writefds, fs_set *exceptfds,
			//const struct timespec *ntimeout, sigset_t *sigmask)

			perror("Nucleo: Error en select");
			exit(1);
		}

		for(int i = 0; i <= fdmax; i++){
			if(FD_ISSET(i, &read_fds)){
					// gestionar nuevas conexiones
					if ((newfd = aceptarEntrantes(listener)) == -1)	{
						perror("accept");
					} else {
						FD_SET(newfd, &master);		// añadir al conjunto maestro
						//agregar(&ptrCabeza,newfd,0);

						if (newfd > fdmax){					// actualizar el máximo
							fdmax = newfd;
						}
					}
				}

			}

	}

	return EXIT_SUCCESS;
}
