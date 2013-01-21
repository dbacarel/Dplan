/** \file  lcscom.c
*	\author Daniele Bacarella mat. 408975
*	Si dichiara che il contenuto di questo file e' in ogni sua parte opera
*	originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <malloc.h>
#include "lcscom.h"

#define E_NEG(arg)\
if(arg < 0 ) {\
return -1;}

#define E_EOF(arg)\
if(c == 0 ) {\
return SEOF;}


static struct sockaddr_un sockadr;


serverChannel_t createServerChannel(const char *path)
{

    serverChannel_t fd_sock;
    if (strlen(path) > UNIX_PATH_MAX)
	return SOCKNAMETOOLONG;

    strcpy(sockadr.sun_path, path);
    strncat(sockadr.sun_path, SKTNAME, strlen(SKTNAME));
    sockadr.sun_family = AF_UNIX;


    (void) unlink(sockadr.sun_path);
    fd_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    /*
     * In caso di errori le funzioni settano errno
     */
    if (fd_sock < 0)
	return -1;
    if (bind(fd_sock, (struct sockaddr *) &sockadr, sizeof(sockadr)) < 0)
	return -1;
    if (listen(fd_sock, SOMAXCONN) < 0)
	return -1;
    return fd_sock;
}

int closeSocket(serverChannel_t s)
{
    int i;
    i = close(s);		/* setta errno */
    (void) unlink(sockadr.sun_path);
    return i;
}

channel_t acceptConnection(serverChannel_t s)
{

    channel_t fd_client;
    if (s < 0) {
	errno = EINVAL;
	return -1;
    }

/*
 * In caso errore errno settato
 */
    fd_client = accept(s, NULL, 0);
    if (fd_client < 0)
	return -1;
    return fd_client;
}

int receiveMessage(channel_t sc, message_t * msg)
{

    int c;

    if (msg == NULL) {
	errno = EINVAL;
	return -1;
    }
    /*
     * In caso errore errno settato
     */

    /* Get Type */
    c = read(sc, &(msg->type), sizeof(&(msg->type)));
    E_NEG(c);
    E_EOF(c);

    /* Get Size */
    c = read(sc, &(msg->length), sizeof(&(msg->length)));
    E_NEG(c);
    E_EOF(c);

    /* Get Body */

    msg->buffer = (char *) malloc(sizeof(char) * msg->length);

    if (msg->buffer == NULL) {
	errno = ENOMEM;
	perror("LCSCOM: Allocating memory for message's buffer ");
	return -1;
    }
    c = read(sc, msg->buffer, msg->length);



    E_NEG(c);
    E_EOF(c);

    return msg->length;

}

int sendMessage(channel_t sc, message_t * msg)
{

    int c;


    if (msg == NULL && msg->buffer == NULL) {
	errno = EINVAL;
	return -1;
    }

    /*
     * In caso errore errno settato
     */

    c = write(sc, &(msg->type), sizeof(&(msg->type)));
    E_NEG(c);

    c = write(sc, &(msg->length), sizeof(&(msg->length)));
    E_NEG(c);
    c = write(sc, msg->buffer, msg->length);
    E_NEG(c);

    return c;

}

int closeConnection(channel_t sc)
{

    /*
     * In caso errore errno settato
     */
    return close(sc);
}

channel_t openConnection(const char *path)
{


    struct sockaddr_un sock;
    channel_t fd_sock;

    if (strlen(path) > UNIX_PATH_MAX) {
	errno = ENAMETOOLONG;
	return SOCKNAMETOOLONG;
    }

    strcpy(sock.sun_path, TMP);
    strncat(sock.sun_path, SKTNAME, strlen(SKTNAME));

    sock.sun_family = AF_UNIX;

    /*
     * In caso errore errno settato
     */
    fd_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    E_NEG(fd_sock);

    E_NEG(connect(fd_sock, (struct sockaddr *) &sock, sizeof(sockadr)));



    return fd_sock;
}
