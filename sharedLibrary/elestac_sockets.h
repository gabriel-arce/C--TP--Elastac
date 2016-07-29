/*
 * sockets.h
 *
 *  Created on: 22 de abr. de 2016
 *      Author: gabriel
 */

#ifndef ELESTAC_SOCKETS_H_
#define ELESTAC_SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     //memset
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>      //perror
#include <arpa/inet.h>  //INADDR_ANY
#include <unistd.h>     //close  usleep
#include <netdb.h> 		//gethostbyname
#include <netinet/in.h>
#include <fcntl.h> 		//fcntl
#include <commons/string.h>
#include <commons/log.h>


#define COLADECONEXIONES 10			//Cantidad de conexiones

int crearSocket();											//Se crea el Fd del socket
int bindearSocket(int socketFd, int puerto);				//Asocia el socketFd al puerto deseado
int escucharEn(int socketFd/*,int colaDeConecciones*/);		//Se pone empieza a escuchar en el socketFd asociado al puerto
int aceptarEntrantes(int socketFd);							//Se aceptan conexion, pero de a una
int conectarA(int socketFd, char* ipDestino, int puerto);	//se conecta al una direccion a traves del socketFd
int clienteDelServidor(char *ipDestino,int puerto);
int enviarPorSocket(int socketFd,char* paquete);
void limpiarBuffer(char* buffer);

void compactaClaves (int *tabla, int *n);

#endif /* ELESTAC_SOCKETS_H_ */
