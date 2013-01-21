/** \file  dplan.c
*	\author Daniele Bacarella mat. 408975
*	Si dichiara che il contenuto di questo file e' in ogni sua parte opera
*	originale dell' autore.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lcscom.h"
#include "lbase.h"
#include "dCutils.h"

#define CLEANUP(var)\
{\
	for(tmp=0;tmp<7;tmp++)\
	if(flags[tmp]) free(var[tmp]);\
	exit(EXIT_FAILURE);\
	}


int main(int argc, char **argv)
{

    int arg, tmp, tipo;
    /* flags[i]=1 se opzione i-esima digitata
     * flags[i]=0 altrimenti
     * */
    short int flags[NOPTS];	/* 0 dflag, 1 uflag, 2 cflag, 3 qflag, 4 rflag, 5 gflag, 6 mflag */

    /* Conterrà l'argomento dell' i-esima opzione */
    char *args[NOPTS];

    /*	Nome dell'agenda	*/
    char *agenda;
    channel_t chan;
    message_t *mess;


    /* Init SD	*/
    for (tmp = 0; tmp < NOPTS; tmp++)
	flags[tmp] = 0;
    for (tmp = 0; tmp < NOPTS; tmp++)
	args[tmp] = NULL;

    if (argc == 1 || argc == 2) {
	printHelp();
	exit(EXIT_SUCCESS);
    }

    while ((arg = getopt(argc, argv, ":cqd:u:g:m:r:")) != -1) {
	switch (arg) {
	case 'c':
	    if (checkOpts(flags, 2))
		MESS_EXIT("dplan: Formato scorretto del comando\n")
		    else
		setInfo(flags, 2, &tipo, MSG_MKAGENDA, args, NULL);

	    break;
	case 'd':
	    if (checkOpts(flags, 0))
		MESS_EXIT("dplan: Formato scorretto del comando\n")
		    else
		setInfo(flags, 0, &tipo, MSG_INSERT, args, optarg);
	    break;
	case 'q':
	    if (checkOpts(flags, 3))
		MESS_EXIT("dplan: Formato scorretto del comando\n")
		    else
		setInfo(flags, 3, &tipo, MSG_RMAGENDA, args, NULL);
	    break;
	case 'r':
	    if (checkOpts(flags, 4))
		MESS_EXIT("dplan: Formato scorretto del comando\n")
		    else
		setInfo(flags, 4, &tipo, MSG_RMPATTERN, args, optarg);
	    break;
	case 'g':
	    if (checkOpts(flags, 5))
		MESS_EXIT("dplan: Formato scorretto del comando\n")
		    else
		setInfo(flags, 5, &tipo, MSG_EGIORNO, args, optarg);
	    break;
	case 'm':
	    if (checkOpts(flags, 6))
		MESS_EXIT("dplan: Formato scorretto del comando\n")
		    else
		setInfo(flags, 6, &tipo, MSG_EMESE, args, optarg);
	    break;
	case 'u':
	    if (checkOpts(flags, 1))
		MESS_EXIT("dplan: Formato scorretto del comando\n")
		    else
		setInfo(flags, 1, &tipo, MSG_INSERT, args, optarg);
	    break;
	default:
	    printf("dplan: -%c: Opzione errata\n", optopt);
	    exit(EXIT_FAILURE);
	    ;

	}

    }

    /*
     * Mi assicuro che le opts -d e -u non siano usate separatamente
     */
    if (flags[0] || flags[1]) {
	if (!(flags[0] && flags[1])) {
	    printf("dplan: Formato scorretto del comando\n");
	    exit(EXIT_FAILURE);
	}
    }


    /*
     * Controlli sugli argomenti
     */
    if (checkArgs(flags, argc, argv, args, &agenda))
	exit(EXIT_FAILURE);

    chan = -1;
    tmp = -1;
    while (chan < 0 && (++tmp < 5)) {
	chan = openConnection(TMP);
	sleep(1);
    }

    if (tmp == 5)
	exit(EXIT_FAILURE);


    /*
     * Preparo il messaggio da inviare al server
     */
    mess = (message_t *) calloc(1, sizeof(message_t));
    mess->type = tipo;
    mess->buffer = getBufferString(tipo, agenda, args, &(mess->length));



    sendMessage(chan, mess);

    /*
     * Libero memoria per buffer
     */
    free(mess->buffer);

    /*
     * Azzero valori in mess in modo da poterla riutilizzare
     */
    mess->buffer = NULL;
    mess->length = 0;
    mess->type = '0';


    /*
     * In attesa di una risposta dal server
     */
    receiveMessage(chan, mess);




    if (tipo == MSG_EGIORNO || tipo == MSG_EMESE) {
	if (mess->type == MSG_OK) {
	    printf("dplan: %s: %s\n", agenda, mess->buffer);
	    free(mess->buffer);
	    while (receiveMessage(chan, mess) >= 0) {
		printf("dplan: %s: %s\n", agenda, mess->buffer);
		free(mess->buffer);
	    }

	} else {
	    printf("dplan: %s: %s\n", agenda, mess->buffer);
	    free(mess->buffer);
	}

    } else {
	if (tipo == MSG_RMPATTERN) {

	    if (mess->type == MSG_OK)
		printf("dplan: %s: %s\n", args[4], mess->buffer);
	    else
		printf("dplan: %s: %s\n", agenda, mess->buffer);
	} else if (tipo == MSG_INSERT)
	    printf("dplan: %s: %s\n", args[0], mess->buffer);
	else
	    printf("dplan: %s: %s\n", agenda, mess->buffer);

	free(mess->buffer);
    }


    free(mess);

    closeConnection(chan);

    exit(EXIT_SUCCESS);

}
