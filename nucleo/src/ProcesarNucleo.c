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
	 int i;
	 int maximo;													/* Número de descriptor maximo */
	 int newfd;
	 int nbytes;
	 char buffer[2000];

	 fd_set master;				// conjunto maestro de descriptores de fichero
	 fd_set read_fds;			// conjunto temporal de descriptores de fichero para select()

	//Cargar configuracion
	printf("Inicializando Nucleo..\n");
	printf("please wait..\n");
	printf("\n");


/*	// Obtener el directorio actual - TEST
	char *directorio = getcwd(NULL, 0);
	printf("El directorio actual es: %s\n", directorio);
	free(directorio);*/

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

	// añadir listener al conjunto maestro
	FD_SET(listener, &master);

	maximo = listener; // por ahora es éste

	while(1){

		read_fds = master; // cópialo

		if (select(maximo+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(1);
		}

		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= maximo; i++){

			if (FD_ISSET(i, &read_fds)){
				// ¡¡tenemos datos!!
				if (i == listener){
					// gestionar nuevas conexiones
					if ((newfd = aceptarEntrantes(listener)) == -1)	{
						perror("accept");
					} else {
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > maximo)
							maximo = newfd;		// actualizar el máximo
					}
				} else	{
					// gestionar datos de un cliente
					if ((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0){
						// error o conexión cerrada por el cliente
						if (nbytes == 0)
						{
							// conexión cerrada
							printf("[NUCLEO] socket %d desconectado\n", i);
						}
						else
						{
							perror("recv");
						}
						close(i); // ¡Hasta luego!
						FD_CLR(i, &master); // eliminar del conjunto maestro
					}
					else
					{
					// tenemos datos de algún cliente
						if (FD_ISSET(i, &master))
						{
							char buff[2000];
							strcpy(buff,buffer);

							//memset(buffer,'\0',sizeof(buffer));
							printf("%s\n", buffer);

						}
					}
				}
			}
		}
	}

	return EXIT_SUCCESS;
}

