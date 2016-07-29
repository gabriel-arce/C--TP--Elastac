/*
 * elestac_inotify.c
 *
 *  Created on: 29 de jul. de 2016
 *      Author: gabriel
 */

#include "elestac_inotify.h"

void inotify_getEvent(int fd, const char *target){
	   ssize_t len, i = 0;
	   char action[81+FILENAME_MAX] = {0};
	   char buff[EVENT_SIZE] = {0};

	   len = read (fd, buff, EVENT_BUF_LEN);

	   while (i < len) {
	      struct inotify_event *pevent = (struct inotify_event *)&buff[i];
	      char action[81+FILENAME_MAX] = {0};

	      if (pevent->len)
	         strcpy (action, pevent->name);
	      else
	         strcpy (action, target);

	      if (pevent->mask & IN_ACCESS)
	    	  strcat(action, " fue leido");
	      if (pevent->mask & IN_ATTRIB)
	    	  strcat(action, " cambia atributo");
	      if (pevent->mask & IN_CLOSE_WRITE)
	    	  strcat(action, " abierto para escribir, fue cerrado");
	      if (pevent->mask & IN_CLOSE_NOWRITE)
	         strcat(action, " no abierto para escribir, fue cerrado");
	      if (pevent->mask & IN_CREATE)
	         strcat(action, " se crea el directorio");
	      if (pevent->mask & IN_DELETE)
	         strcat(action, " se elimina el directorio");
	      if (pevent->mask & IN_DELETE_SELF)
	         strcat(action, " se elimina a si mismo");
	      if (pevent->mask & IN_MODIFY)
	         strcat(action, " es modificado");
	      if (pevent->mask & IN_MOVE_SELF)
	         strcat(action, " se movio el fichero/directorio");
	      if (pevent->mask & IN_MOVED_FROM)
	         strcat(action, " fue movido a otro lugar");
	      if (pevent->mask & IN_MOVED_TO)
	         strcat(action, " movido al directorio observador");
	      if (pevent->mask & IN_OPEN)
	         strcat(action, " fue abierto");

	      printf ("%s\n", action);

	      i += sizeof(struct inotify_event) + pevent->len;

	   }
}
