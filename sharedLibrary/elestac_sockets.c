/*
 * sockets.c
 *
 *  Created on: 22 de abr. de 2016
 *      Author: gabriel
 */

#include "elestac_sockets.h"

int crearSocket(){
	int descriptor = -1;

	if ((descriptor = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("[ERROR] Funcion socket\n");
		return descriptor;
	};

	printf("[OK] Funcion SOCKET: Descriptor creado \n");
	return descriptor;
}

int bindearSocket(int socketFd, int puerto){
	int resultadoBind = -1;

	struct sockaddr_in miDireccion;
	miDireccion.sin_family 		= AF_INET;
	miDireccion.sin_port 		= htons(puerto);	//toma el puerto por parametro
	miDireccion.sin_addr.s_addr = INADDR_ANY; 		//Toma la direccion propia de donde se esta corriendo
	memset(&(miDireccion.sin_zero),'\0',8);			//se pone en cero el resto de la estructura

	if ((resultadoBind = bind(socketFd,(struct sockaddr *)&miDireccion, sizeof(struct sockaddr))) == -1){
		perror("[ERROR] Funcion BIND: No se pudo asociar con el puerto\n");
		return resultadoBind;
	};

	printf("[OK] Funcion BIND. Vinculada al puerto [ %d ]\n", puerto);
	return resultadoBind;

}

int escucharEn(int socketFd/*,int colaDeConecciones*/){
	//colaDeConecciones es la cantidad de Conecciones que pueden estar esperando en cola hasta un accept()
	int resultadoListen = -1;

	if ((resultadoListen = listen(socketFd,COLADECONEXIONES)) == -1){
		perror("[ERROR] Funcion LISTEN: No se pudo escuchar con el puerto\n");
		return resultadoListen;
	};

	printf("[OK] Funcion escuchando conexiones\n");
	return resultadoListen;

}

int aceptarEntrantes(int socketFd){
	int socketFdEntrante;
	unsigned int estructFdEntranteSize;
	struct sockaddr_in direccionEntrante;

	estructFdEntranteSize = sizeof(struct sockaddr_in);

	if ((socketFdEntrante = accept(socketFd,
								  (struct sockaddr *)&direccionEntrante,
								   &estructFdEntranteSize)) == -1){
		perror("[ERROR] Funcion accept");
		return socketFdEntrante;
	};

	printf("[OK] ACEPTADA: nueva conexion de %s en socket %d\n", inet_ntoa(direccionEntrante.sin_addr),socketFdEntrante);
	return socketFdEntrante;

}

int conectarA(int socketFd, char* ipDestino, int puertoDestino){
	int resultadoAccept = -1;

	struct sockaddr_in direccionDestino;						// Guardará la dirección de destino
	direccionDestino.sin_family			= AF_INET;				// Ordenación de máquina
	direccionDestino.sin_port			= htons(puertoDestino);	// short, Ordenación de la red
	direccionDestino.sin_addr.s_addr	= inet_addr(ipDestino);
	memset(&(direccionDestino.sin_zero), '\0', 8); 				// Poner a cero el resto de la estructura

	if ((resultadoAccept = connect(socketFd,
							  	  (struct sockaddr *)&direccionDestino,
								   sizeof(struct sockaddr))) == -1){
		perror("[ERROR] En Funcion connect\n");
		return resultadoAccept;
	};

	printf("[OK] Conexión Exitosa a %s\n", inet_ntoa(direccionDestino.sin_addr));
	return resultadoAccept;

}

int clienteDelServidor(char *ipDestino,int puerto){
	int socketCliente = crearSocket();
	conectarA(socketCliente,ipDestino,puerto);
	return socketCliente;
}

int enviarPorSocket(int socketFd,char* paquete){
	char* mensaje = string_new();
	string_append(&mensaje,paquete);
	//string_append(&mensaje,'\0');
	return send(socketFd,paquete,string_length(mensaje)+1,0); //el paquete
}

void limpiarBuffer(char* buffer){

}
