/** \file  lbase.c
*	\author Daniele Bacarella mat. 408975
*	Si dichiara che il contenuto di questo file e' in ogni sua parte opera
*	originale dell' autore.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include "lbase.h"
#include "llist.h"

#define E_NULL(var)\
if(var==NULL){\
free(record);\
errno=EINVAL;\
return NULL;\
}



int checkFormat(char *date)
{
    int m, a, tmp;
    char g[3];
    g[2] = '\0';
    if ((tmp = sscanf(date, "%2c-%d-%d", g, &m, &a)) == 3) {

	if (((atoi(g) > 0 && atoi(g) < 32) || strcmp(g, "**") == 0)
	    && (m > 0 && m < 13) && (a >= 2008))
	    return 0;
	else
	    return 1;
    } else
	return -1;

}

evento_t *convertiRecord(char r[])
{

    int x;
    evento_t *record = NULL;
    char *campo1, *campo2, *tmp;
    if (r == NULL) {
	errno = EINVAL;
	return NULL;
    }

    record = (evento_t *) malloc(sizeof(evento_t));
    if (record == NULL) {
	errno = EINVAL;
	return NULL;
    }

    campo1 = strtok(r, "#");
    E_NULL(campo1);
    campo2 = strtok(NULL, "#");
    E_NULL(campo2);

    tmp = strtok(campo1, " ");
    E_NULL(tmp);
    strncpy(record->data, tmp, LDATA + 1);
    tmp = strtok(NULL, " ");
    E_NULL(tmp);
    strncpy(record->utente, tmp, LUTENTE + 1);

    x = strlen(campo2) - LUTENTE - strlen(record->utente);
    if (strlen(record->utente) > LUTENTE || x > LDESCRIZIONE) {
	free(record);
	errno = EINVAL;
	return NULL;
    }

    strncpy(record->descrizione, campo2, LDESCRIZIONE + 1);
    record->descrizione[LDESCRIZIONE] = '\0';


    /*     Applico controlli di formato    */
    if (checkFormat(record->data) < 0) {
	free(record);
	return NULL;
    }


    return record;

}


int convertiEvento(evento_t * e, char r[])
{

    if (e == NULL || r == NULL) {
	errno = EINVAL;
	return -1;
    }

    strncpy(r, e->data, LDATA + 1);
    strcat(r, " ");
    strncat(r, e->utente, LUTENTE + 1);
    strcat(r, "#");
    strncat(r, e->descrizione, LDESCRIZIONE + 1);

    while (strlen(r) < 100)
	strcat(r, " ");


    return 0;
}

int matchData(char b[], evento_t * e)
{

    char *data, *data2;

    data = strchr(e->data, '-');
    data2 = strchr(b, '-');

    if (checkFormat(b))
	return -1;


    if (strcmp(b, e->data) == 0)
	return 1;
    else if ((strcmp(data, data2) == 0)
	     && (strncmp(e->data, "**", 2) == 0
		 || strncmp(b, "**", 2) == 0))
	return 1;
    else
	return 0;

}


int matchPattern(char p[], evento_t * e)
{
    char *tmp;
    if (e == NULL)
	return -1;

    if (p == NULL)
	return 1;

    if (strstr(e->data, p) != NULL)
	return 1;

    if (matchData(p, e) == 1)
	return 1;

    if ((tmp = strstr(e->utente, p)) != NULL)
	return 1;

    if (strstr(e->descrizione, p) != NULL)
	return 1;
    return 0;
}
