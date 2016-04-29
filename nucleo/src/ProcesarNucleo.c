/*
 * ProcesarNucleo.c
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#include <elestac_sockets.h>

#include "Nucleo.h"
#define MAX_CLIENTES 10

int main(void) {

	 int listener;														//Descriptor de escucha
	 int socketCliente[MAX_CLIENTES];		//Descriptores de sockets con clientes
	 int numeroClientes = 0;								/* Número clientes conectados */
	 fd_set descriptoresLectura;						/* Descriptores de interes para select() */
	 int i;
	 int maximo;													/* Número de descriptor más grande */
	 int newfd;
	 int nbytes;
	 char buffer[2000];

	//Cargar configuracion
	printf("Inicializando Nucleo..\n");
	printf("please wait..\n");
	printf("\n");

	t_nucleo *nucleo = cargar_conf();

	//Crear socket de escucha
	if( (listener = crearSocket()) == -1 ){
		perror("Nucleo: Error al abrir socket de espera\n");
		exit(1);
	}

	/*
	//descriptor para enlace
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1){
		perror("Nucleo: No es posible reusar el socket\n");
		exit(1);
	}*/

	//Enlazar
	if((bindearSocket(listener, PUERTO_NUCLEO)) == -1){
		perror("Nucleo: No se puede enlazar al puerto: direccion ya esta en uso\n");
		exit(1);
	}

	//Escuchar
	if((escucharEn(listener)) == -1){
		exit(1);
	}

	while(1){

		compactaClaves(socketCliente, &numeroClientes);

		FD_ZERO(&descriptoresLectura);					/* Se inicializa descriptoresLectura */
		FD_SET(listener, &descriptoresLectura);		/* Se añade para select() el socket servidor */

		/* Se añaden para select() los sockets con los clientes ya conectados */
		for (i=0; i<numeroClientes; i++)
			FD_SET (socketCliente[i], &descriptoresLectura);

		/* Se el valor del descriptor más grande. Si no hay ningún cliente,
		 * devolverá 0 */
		maximo = dameMaximo (socketCliente, numeroClientes);

		if (maximo < listener)
			maximo = listener;

		/* Espera indefinida hasta que alguno de los descriptores tenga algo
		 * que decir: un nuevo cliente o un cliente ya conectado que envía un
		 * mensaje */
		if((select (maximo + 1, &descriptoresLectura, NULL, NULL, NULL)) == -1){
			perror("Nucleo: Error en select");
			exit(1);
		}


		/* Se comprueba si algún cliente ya conectado ha enviado algo */
		for (i=0; i<numeroClientes; i++)
		{
			if (FD_ISSET (socketCliente[i], &descriptoresLectura))	{
				/* Se lee lo enviado por el cliente y se escribe en pantalla */
				if ((newfd = aceptarEntrantes(listener)) == -1)
					perror("accept");

				nbytes = recv(i, buffer, sizeof(buffer), 0);
				if(nbytes == 0){
					printf("socket %d desconectado\n", i);
					close(i);
				}

				if(nbytes < 0){
					perror("recv");
					close(i);
				}

				if(nbytes > 0){
					printf("socket aceptado..\n");
				}

				}
		}

		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 * admite */
		if (FD_ISSET (listener, &descriptoresLectura))
			//nuevoCliente (socketServidor, socketCliente, &numeroClientes);
			printf("Cliente aceptado..\n");

	}

	return EXIT_SUCCESS;
}





