#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "lcscom.h"
#include "lbase.h"
#include "dCutils.h"

void printHelp(void)
{
    printf
	("Usage:  dplan [ agenda [ -d gg-mm-aaaa -u utente#descrizione] [ -g gg-mm-aaaa ] [ -m mm-aaaa ] [ -r pattern ]  ] [-cq aname]\n");
    printf("\t-c\tcrea una nuova agenda di nome ‘‘aname’’\n");
    printf
	("\t-q\trimuove una agenda di nome ‘‘aname’’ (solo se vuota)\n");
    printf("\t-d\tspecifica la data\n");
    printf
	("\t-u\tspecifica l’utente che sta effettuando la registrazione e la descrizione dell’evento\n");
    printf
	("\t-g\trichiede gli eventi registrati per un certo giorno su ‘‘agenda’’ e li stampa sullo stdout\n");
    printf
	("\t-m\trichiede gli eventi registrati su ‘‘agenda’’ per un certo mese e li stampa sullo stdout\n");
    printf
	("\t-r\telimina tutti gli eventi di ‘‘agenda’’ che contengono ‘‘pattern’’ come sottostringa in un qualsiasi campo\n");
}

char *getBufferString(int tipo, char *agenda, char **args,
		      unsigned int *length)
{

    char *stringa, *tmp, *g;
    int i;

    i = 0;
    stringa = (char *) malloc(sizeof(char) * (LAGENDA + LRECORD + 2));

    switch (tipo) {
    case MSG_MKAGENDA:
	strncpy(stringa, agenda, strlen(agenda) + 1);
	*length = strlen(agenda) + 1;
	break;

    case MSG_RMAGENDA:
	strncpy(stringa, agenda, strlen(agenda) + 1);
	*length = strlen(agenda) + 1;
	break;

    case MSG_RMPATTERN:
	tmp = stringa;
	strncpy(tmp, agenda, strlen(agenda) + 1);
	i += strlen(tmp) + 1;
	tmp = &(tmp[strlen(tmp) + 1]);
	strncpy(tmp, args[4], strlen(args[4]) + 1);
	i += strlen(tmp) + 1;
	*length = i;
	break;

    case MSG_INSERT:

	/* Agenda */
	tmp = stringa;
	strncpy(tmp, agenda, strlen(agenda) + 1);

	i += strlen(tmp) + 1;

	/* Data */
	tmp = &(tmp[strlen(tmp) + 1]);
	strncpy(tmp, args[0], strlen(args[0]) + 1);

	i += strlen(tmp) + 1;

	/* Utente */
	tmp = &(tmp[strlen(tmp) + 1]);
	g = strtok(args[1], "#");
	strncpy(tmp, g, strlen(g) + 1);

	i += strlen(tmp) + 1;

	/* Descrizione */
	tmp = &(tmp[strlen(tmp) + 1]);
	g = strtok(NULL, "#");
	strncpy(tmp, g, strlen(g) + 1);

	i += strlen(tmp) + 1;

	*length = i;

	return stringa;
	break;

    case MSG_EGIORNO:
	tmp = stringa;
	strncpy(tmp, agenda, strlen(agenda) + 1);
	i += strlen(tmp) + 1;
	tmp = &(tmp[strlen(tmp) + 1]);
	strncpy(tmp, args[5], strlen(args[5]) + 1);
	i += strlen(tmp) + 1;
	*length = i;
	break;

    case MSG_EMESE:
	tmp = stringa;
	strncpy(tmp, agenda, strlen(agenda) + 1);
	i += strlen(tmp) + 1;
	tmp = &(tmp[strlen(tmp) + 1]);
	strncpy(tmp, args[6], strlen(args[6]) + 1);
	i += strlen(tmp) + 1;
	*length = i;
	break;

    }

    return stringa;
}
int checkOpts(short int *flags, int indice)
{

    if (flags[indice])
	return 0;		/* Ok, se già presente non implica un prob */
    if (indice == 0 || indice == 1)
	return (flags[2] || flags[3] || flags[4] || flags[5] || flags[6]);
    else
	return (flags[0] || flags[1] || flags[2] || flags[3] || flags[4]
		|| flags[5] || flags[6]);
}

/*
 * Controlla formato di una data in base ad un pattern
 * Type
 * 1 -> date pattern gg-mm-aaaa
 * 0 -> date pattern mm-aaaa
 *
 * Return
 * 0 match
 * 1 altrimenti
 */
int checkDate(int type, char *date)
{
    int a, tmp;
    char g[3], m[3];
    g[2] = '\0';
    m[3] = '\0';
    if (type) {
	if ((tmp = sscanf(date, "%2c-%2c-%d", g, m, &a)) == 3) {

	    if (((atoi(g) > 0 && atoi(g) < 32) || strcmp(g, "**") == 0)
		&& (atoi(m)
		    > 0 && atoi(m) < 13) && (a >= 2000))
		return 0;
	    else
		printf("dplan: %s: Data non corretta\n", date);
	} else
	    printf("dplan: %s: Data non corretta\n", date);
    } else {
	if ((tmp = sscanf(date, "%2c-%d", g, &a)) == 2) {

	    if ((atoi(g) > 0 && atoi(g) < 13) && (a >= 2000))
		return 0;
	    else
		printf("dplan: %s: Data non corretta\n", date);
	} else
	    printf("dplan: %s: Data non corretta\n", date);

    }
    return 1;

}

int checkArgs(short int *flags, int argc, char **argv, char **args,
	      char **agenda)
{

    int i;
    char *buf;
    char tmp;
    char tmp2[110];

    *agenda = NULL;
    buf = NULL;

    /*
     * Ciclo sugli argomenti in argv per estrarre il nome 'agenda'
     */

    for (i = 1; i < argc; i++) {

	if (sscanf(argv[i], "-%c", &tmp)) {
	    if (flags[2] || flags[3]);
	    else
		i += 1;
	} else {

	    if (strlen(argv[i]) > LAGENDA) {
		printf("dplan: %s: Agenda max 20 caratteri\n", argv[i]);
		return -1;
	    } else {
		*agenda = argv[i];
		break;
	    }
	}
    }

    if (*agenda == NULL) {
	MESS_EXIT("dplan: Formato scorretto del comando\n");
    }


    for (i = 0; i < NOPTS; i++) {

	if (flags[i]) {
	    switch (i) {
	    case 2:		/* aname */
		return 0;
		break;
	    case 0:		/* gg-mm-aaaa */
		if (checkDate(1, args[i]))
		    return -1;
		break;
	    case 3:		/* aname */
		return 0;
		break;
	    case 4:		/* pattern */

		break;
	    case 5:		/* gg-mm-aaaa */
		if (checkDate(1, args[i]))
		    return -1;
		break;
	    case 6:		/* mm-aaaa */
		if (checkDate(0, args[i]))
		    return -1;
		break;
	    case 1:		/* utente#descrizione */
		strncpy(tmp2, args[i], 110);
		buf = strtok(tmp2, "#");

		if (strlen(buf) <= LUTENTE) {
		    buf = strtok(NULL, "#");
		    if (!buf) {
			printf
			    ("dplan: %s: Formato evento non corretto (manca #)\n",
			     args[i]);
			return -1;
		    }
		    if (!(strlen(buf) <= LDESCRIZIONE)) {
			printf
			    ("dplan: %s: Formato evento non corretto (Descrizione max 80 caratteri)\n",
			     args[i]);
			return -1;
		    }
		    /*
		     * Mi assicuro che ci sia una sola occorrenza di '#' o al più un'ulteriore alla fine
		     */
		    if (strtok(NULL, "#"))
			return -1;
		} else {
		    printf
			("dplan: %s: Formato evento non corretto (Utente max 8 caratteri)\n",
			 buf);
		    return -1;
		}
		break;

	    }

	}

    }


    return 0;
}
void setInfo(short int *flags, int indice, int *vtipo, int type,
	     char **args, char *optarg)
{
    flags[indice] = 1;
    *vtipo = type;

    if (optarg)
	args[indice] = optarg;
}
