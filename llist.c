/** \file  llist.c
*	\author Daniele Bacarella mat. 408975
*	Si dichiara che il contenuto di questo file e' in ogni sua parte opera
*	originale dell' autore.
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "lbase.h"
#include "llist.h"

int add(elem_t ** agenda, evento_t * ev)
{

    elem_t *nuovo, *tmp;

    if (ev == NULL)
	return 1;

    if ((nuovo = (elem_t *) malloc(sizeof(elem_t))) == NULL)
	return 1;

    if ((nuovo->ptev = (evento_t *) malloc(sizeof(evento_t))) == NULL)
	return 1;

    if (strlen(ev->utente) == 0)
	return 1;


    strncpy(nuovo->ptev->data, ev->data, LDATA + 1);
    strncpy(nuovo->ptev->descrizione, ev->descrizione, LDESCRIZIONE + 1);
    strncpy(nuovo->ptev->utente, ev->utente, LUTENTE + 1);
    nuovo->next = NULL;

    if (*agenda == NULL)
	*agenda = nuovo;
    else {
	tmp = *agenda;
	while (tmp->next != NULL)
	    tmp = tmp->next;

	tmp->next = nuovo;
    }
    return 0;

}

static int trova(char data[], elem_t * agenda, elem_t ** trovati)
{

    int r = 0;


    if (agenda == NULL)
	return 0;

    r = matchData(data, agenda->ptev);

    if (r == 1) {
	add(trovati, agenda->ptev);
	return 1 + trova(data, agenda->next, trovati);
    } else if (r == -1)
	return -1;
    else
	return trova(data, agenda->next, trovati);

    return 0;
}
int cerca(char data[], elem_t * agenda, elem_t ** trovati)
{

    *trovati = NULL;

    return trova(data, agenda, trovati);

}

elem_t *rimuovi(char pattern[], elem_t * agenda)
{

    elem_t *head, *nodo, *tmp;

    head = agenda;

    while (head != NULL && (matchPattern(pattern, head->ptev) == 1)) {

	tmp = head;
	head = head->next;
	free(tmp->ptev);
	free(tmp);
    }

    tmp = head;
    while (tmp != NULL && tmp->next != NULL) {
	if (matchPattern(pattern, tmp->next->ptev) == 1) {
	    nodo = tmp->next;
	    tmp->next = nodo->next;
	    free(nodo->ptev);
	    free(nodo);
	} else
	    tmp = tmp->next;
    }

    return head;
}

int loadAgenda(FILE * ingresso, elem_t ** agenda)
{

    char riga[LRECORD + 2];
    evento_t *nodo;
    int n;

    n = 0;
    if (ingresso == NULL)
	return -1;

    while (fgets(riga, LRECORD + 2, ingresso) != NULL) {
	riga[LRECORD] = '\0';
	nodo = convertiRecord(riga);

	if (nodo == NULL);
	else {

	    add(agenda, nodo);
	    free(nodo);
	    n++;
	}
    }

    return n;

}

int storeAgenda(FILE * uscita, elem_t * agenda)
{

    elem_t *tmp;
    char evento[LRECORD + 1];
    int n;

    n = 0;
    tmp = agenda;
    if (agenda == NULL)
	return -1;
    while (tmp != NULL) {

	if (convertiEvento(tmp->ptev, evento) == -1)
	    continue;


	fputs(evento, uscita);

	fputs("\n", uscita);


	tmp = tmp->next;
	n++;
    }

    return n;
}

static void deallocatore(elem_t * lista)
{
    if (lista == NULL)
	return;
    else {
	deallocatore(lista->next);

	free(lista->ptev);
	lista->ptev = NULL;

	free(lista->next);
	lista->next = NULL;
    }
}

void dealloca_lista(elem_t * lista)
{

    deallocatore(lista);
    free(lista);
    lista = NULL;
}
