/*
 * elestac_inotify.h
 *
 *  Created on: 28 de jul. de 2016
 *      Author: gabriel
 */

#ifndef ELESTAC_INOTIFY_H_
#define ELESTAC_INOTIFY_H_


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void inotify_getEvent(int fd, const char *target);

#endif /* ELESTAC_INOTIFY_H_ */
