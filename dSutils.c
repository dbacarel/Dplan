/** \file  dSutils.c
*	\author Daniele Bacarella mat. 408975
*	Si dichiara che il contenuto di questo file e' in ogni sua parte opera
*	originale dell' autore.
*/

#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <bits/signum.h>
#include "lcscom.h"
#include "lbase.h"
#include "llist.h"
#include "dSutils.h"

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
volatile sig_atomic_t sd_busy = 0;

/*	Elimina gli spazi a destra della stringa
 *  Ritorna la stringa modificata
 */
static char *trim_r(char *str)
{
    char *attuale;
    for (attuale = str + strlen(str) - 1;
	 (*attuale == ' ' || *attuale == 't') && (attuale >= str);
	 --attuale)
	*(attuale) = '\0';
    return (str);
}



/*
 * Funzioni per la gestione del race condition
 */
static void lock_SD(void)
{
    pthread_mutex_lock(&mtx);
    while (sd_busy)
	pthread_cond_wait(&cond, &mtx);
    sd_busy = 1;
    pthread_mutex_unlock(&mtx);
}

static void unlock_SD(void)
{
    pthread_mutex_lock(&mtx);
    sd_busy = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
}



/*
 * Implementa il confronto tra due variabili di tipo elem_t
 */
static int events_compare(elem_t * a, elem_t * b)
{

    if (strcmp(a->ptev->data, b->ptev->data) < 0)
	return -1;
    else {
	if (strcmp(a->ptev->data, b->ptev->data) == 0) {
	    /*
	     * Proseguo controllando utente
	     */
	    if (strcmp(a->ptev->utente, b->ptev->utente) < 0
		|| (strlen(a->ptev->utente) < strlen(b->ptev->utente)))
		return -1;
	    else {
		if (strcmp(a->ptev->utente, b->ptev->utente) == 0) {
		    /*
		     * Proseguo controllando le descrizioni
		     */
		    if (strcmp(a->ptev->descrizione, b->ptev->descrizione)
			< 0
			|| (strlen(a->ptev->descrizione) <
			    strlen(b->ptev->descrizione)))
			return -1;
		    if (strcmp(a->ptev->descrizione, b->ptev->descrizione)
			> 0
			|| (strlen(a->ptev->descrizione) <
			    strlen(b->ptev->descrizione)))
			return 1;
		    else
			return 0;
		} else
		    return 1;
	    }

	} else
	    return 1;
    }

}

/*
 * Funzione di supporto a MergeSort
 * Implementa l'algoritmo classico di ordinamento MergeSort
 * Le operazioni di confronto tra elementi di tipo elem_t
 * vengono effettuate tramite la funzione event_compare(a,b)
 */
static void Merge(elem_t ** elenco, int min, int q, int max)
{
    int i, j, k;
    elem_t **buf;

    buf = (elem_t **) malloc(sizeof(elem_t *) * (max + 1));


    i = min;
    j = q + 1;
    k = 0;
    while (i <= q && j <= max) {
	if (events_compare(elenco[i], elenco[j]) < 0) {
	    buf[k] = elenco[i];

	    i++;
	} else {
	    buf[k] = elenco[j];
	    j++;
	}
	k++;
    }
    while (i <= q) {
	buf[k] = elenco[i];
	i++;
	k++;
    }
    while (j <= max) {
	buf[k] = elenco[j];
	j++;
	k++;
    }
    for (k = min; k <= max; k++)
	elenco[k] = buf[k - min];


    free(buf);

    return;
}

void MergeSort(elem_t ** elenco, int min, int max)
{
    int q;

    if (min < max) {
	q = (min + max) / 2;
	MergeSort(elenco, min, q);
	MergeSort(elenco, q + 1, max);
	Merge(elenco, min, q, max);

    }
    return;
}

int is_agenda(char *name)
{
    int i;


    for (i = 0; i < offset; i++)
	if (!strcmp(nomi_agende[i], name))
	    break;


    return i;
}
void *enlarge_array(unsigned char array_type)
{

    char **p;
    elem_t **r;
    int cont, i;

    i = 0;

    switch (array_type) {

    case ARRAY_NOMI:
	p = (char **) malloc(sizeof(char *) * (a_size + SLOTS_MORE));
	if (!p)
	    return NULL;

	for (cont = 0; cont < a_size; cont++) {

	    p[cont] = nomi_agende[cont];
	}


	for (cont = a_size; cont < a_size + SLOTS_MORE; cont++)
	    p[cont] = (char *) malloc(sizeof(char) * LAGENDA + 1);

	return p;
	break;

    case ARRAY_AGENDE:
	r = (elem_t **) malloc(sizeof(elem_t *) * (a_size + SLOTS_MORE));
	if (!r)
	    return NULL;

	for (cont = 0; cont < a_size; cont++)
	    r[cont] = agende[cont];
	for (cont = a_size; cont < a_size + SLOTS_MORE; cont++)
	    r[cont] = NULL;

	return r;
	break;
    }

    return p;
}

int add_thread(int sd)
{

    threads_list *nodo, *tmp;
    nodo = (threads_list *) malloc(sizeof(threads_list));
    nodo->thrd = sd;
    nodo->next = NULL;

    if (lista_thread == NULL)
	lista_thread = nodo;
    else {
	tmp = lista_thread;
	while (tmp->next)
	    tmp = tmp->next;
	tmp->next = nodo;

    }

    return nodo->thrd;
}

void free_thread_list(threads_list ** lista_threads)
{

    if (*lista_threads == NULL)
	return;

    free_thread_list(&((*lista_threads)->next));
    free(*lista_threads);
    return;
}


int add_agenda(char *nome_agenda)
{

    char **tmp_nomi_agende;
    elem_t **tmp_agende;
    int i;

    i = 0;

    if (is_agenda(nome_agenda)
	< offset)
	return 2;


    /*
     * Cerco uno slot vuoto
     */

    for (i = 0; i < offset; i++)
	if (strlen(nomi_agende[i]) == 0)
	    break;


    /*
     * no slot liberi, procedo aggiungendo in coda
     */


    if (i == offset) {
	if (offset == a_size) {

	    tmp_agende = agende;
	    tmp_nomi_agende = nomi_agende;

	    agende = enlarge_array(ARRAY_AGENDE);


	    if (!agende) {
		printf
		    ("add_agenda:Problemi durante l'allocazione di mem\n");
		/*
		 * Riassegno il valore iniziale, in questi casi assumo che
		 *  il prob di malloc sia stato un caso e il client può ritentare
		 */
		agende = tmp_agende;

		return 1;
	    }

	    nomi_agende = enlarge_array(ARRAY_NOMI);


	    if (!nomi_agende) {

		printf
		    ("add_agenda:Problemi durante l'allocazione di mem\n");
		/*
		 * Come sopra
		 * NOTA:
		 * nel caso di errore qui, la lunghezza dell'array 'agende' rimarrebe più grande di SLOTS_MORE, nonostante ciò non
		 * dovrebbe rappresentare un problema, visto che il valore 'a_size' non viene aggiornato
		 */
		nomi_agende = tmp_nomi_agende;

		return 1;
	    }

	    a_size += SLOTS_MORE;


	    free(tmp_agende);
	    free(tmp_nomi_agende);
	}

	strncpy(nomi_agende[offset], nome_agenda, LAGENDA + 1);

	offset++;
    } else {
	/*
	 * Sono in uno slot libero in seguito ad una eliminazione di agenda
	 */
	strncpy(nomi_agende[i], nome_agenda, LAGENDA + 1);
    }

    return 0;

}

		/*
		 * THREAD WORKER
		 */
void *thread_worker(void *args)
{

    char agenda[LAGENDA + 1], data[LDATA + 1],
	utente[LUTENTE + 1],
	descrizione[LDESCRIZIONE + 1], pattern[LRECORD + 1], record[LRECORD
								    + 1],
	*ptr;
    int foo, i, cont;
    message_t *messaggio;
    evento_t *evento;
    elem_t *elem, *elem_tmp, **tmp_buf;
    channel_t client;


    foo = 0;
    i = 0;

    messaggio = (message_t *) malloc(sizeof(message_t));
    messaggio->buffer = NULL;
    client = *((channel_t *) args);

    /* In caso di errori in ricezione, provo ad inviare un messaggio di errore al client	*/
    if (receiveMessage(client, messaggio) < 0) STANDARD_MESSAGES("Problems experienced during request processing",MSG_ERROR)
    else {

	ptr = messaggio->buffer;

	lock_SD();
	switch (messaggio->type) {

	case MSG_MKAGENDA:
	    strcpy(agenda, ptr);
	    i = add_agenda(agenda);

	    if (i) {

		/*
		 * i=1 errore generico
		 * i=2 agenda già presente
		 */
		if (i == 1) STANDARD_MESSAGES("dserver hasn't been able to add the agenda", MSG_ERROR)
		else STANDARD_MESSAGES("Cannot create, agenda already present", MSG_ERROR)

	    } else STANDARD_MESSAGES("Created", MSG_OK)


	    break;

	case MSG_RMAGENDA:

	    strcpy(agenda, ptr);

	    /*
	     * Cerco il nome dell'agenda all'interno dell'elenco 'nomi_agende'
	     */
	    i = is_agenda(agenda);

	    /*
	     * In questo caso l'agenda cercata non è stata trovata
	     */
	    if (i == offset)
		STANDARD_MESSAGES("Agenda not existent", MSG_ERROR)
		    else {

		/*
		 * Trovata, controllo che sia vuota o meno
		 */

		if (agende[i] == NULL) {

		    chdir(path_dir_agende);

		    remove(nomi_agende[i]);

				/*
				 * Marco il corrispondende slot in 'nomi_agende'
				 * così da poterlo ricliclare
			     */
		    strcpy(nomi_agende[i], "");

		    STANDARD_MESSAGES("Removed", MSG_OK)

		} else
			STANDARD_MESSAGES("Agenda not empty, cannot remove",MSG_ERROR);


		}

	    break;
	case MSG_RMPATTERN:

	    strcpy(agenda, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);

	    strcpy(pattern, ptr);

	    i = is_agenda(agenda);

	    if (i == offset)
		STANDARD_MESSAGES("Agenda not existent", MSG_ERROR)
		    else {

		agende[i] = rimuovi(pattern, agende[i]);
		STANDARD_MESSAGES("Success ", MSG_OK)

		}

	    break;
	case MSG_INSERT:

	    strcpy(agenda, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);

	    strcpy(data, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);

	    strcpy(utente, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);

	    strcpy(descrizione, ptr);

	    i = is_agenda(agenda);

	    if (i == offset)
		STANDARD_MESSAGES("Agenda not existent", MSG_ERROR)
		    else {

		strncpy(record, data, strlen(data) + 1);
		strcat(record, " ");
		strncat(record, utente, strlen(utente) + 1);
		strcat(record, "#");
		strncat(record, descrizione, strlen(descrizione) + 1);

		evento = convertiRecord(record);

		add(&(agende[i]), evento);


		free(evento);
		STANDARD_MESSAGES("Success ", MSG_OK)


		}

	    break;
	case MSG_EGIORNO:
	case MSG_EMESE:
	    strcpy(agenda, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);

	    if(messaggio->type==MSG_EGIORNO) strcpy(data, ptr);
	    else  {
	    	strcpy(data, "**-");
	    	strcat(data, ptr);
	    }


	    i = is_agenda(agenda);

	    if (i == offset)
		STANDARD_MESSAGES("Agenda not existent", MSG_ERROR)
		    else {

		cont = 0;

		cont = cerca(data, agende[i], &elem);
		elem_tmp = elem;


		if (cont == 0)
		    STANDARD_MESSAGES("No events registered", MSG_OK)
			else {
				/*	match some events */
		    free(messaggio->buffer);
		    /*	preparo array per contenere gli eventi trovati	*/
		    tmp_buf = (elem_t **) malloc(sizeof(elem_t *) * cont);
		    for (foo = 0; foo < cont; foo++) {
			tmp_buf[foo] = elem_tmp;
			elem_tmp = elem_tmp->next;

		    }


		    messaggio->buffer =
			(char *) calloc(LRECORD + 1, sizeof(char));

		    i = 0;


		    /* Ordino l'array	*/
		    MergeSort(tmp_buf, 0, cont - 1);

		    /*	Invio uno ad uno gli eventi al client	*/
		    for (foo = 0; foo < cont; foo++) {

			convertiEvento(tmp_buf[foo]->ptev,
				       messaggio->buffer);

			strcat(messaggio->buffer, "\0");

			messaggio->buffer = trim_r(messaggio->buffer);
			messaggio->type = MSG_OK;
			messaggio->length = strlen(messaggio->buffer) + 1;
			sendMessage(client, messaggio);

		    }
		    closeConnection(client);

		    /* Free della memoria occupata da elem*/
		    while (elem) {
			elem_tmp = elem;
			elem = elem->next;
			free(elem_tmp->ptev);
			free(elem_tmp);
		    }
		    free(tmp_buf);

		    }
		}

	    break;

	}
	unlock_SD();
    }

    free(messaggio->buffer);
    free(messaggio);

    pthread_exit((void *) 0);

}

void *thread_dispatcher(void *args)
{

    int i;
    void *status;
    pthread_t nuovo_thread;
    /*	channel di trasmissione con il client connesso	*/
    channel_t client;


    /* Inizializzo le variabili riguardo le info sui thread generati*/
    n_thread = 0;
    lista_thread = NULL;

    /*	Creo il socket	*/
    server_chan = createServerChannel(TMP);

    if (server_chan < 0)
	perror("Problemi durante creazione del canale socket");
    else {
	while (1) {

		/* In attesa di una connessione di un client	*/
	    client = acceptConnection(server_chan);
	    if (client < 0)
		perror("Errore in acceptConnection()");
	    else {

		if (pthread_create(&nuovo_thread, NULL, thread_worker,
				   (void *) &client))
		    perror
			("Errore durante la creazione del thread dispatcher");
		else {

		    add_thread(nuovo_thread);
		    n_thread++;
		}

	    }

	}
	for (i = 0; i < n_thread; i++)
	    pthread_join(lista_thread[i].thrd, &status);

    }

    free_thread_list(&lista_thread);

    pthread_exit((void *) 0);
}

void *thread_sigHandler(void *args)
{
    int status, i, sig;
    FILE *fd;
    sigset_t set;
    threads_list *tmp;

	/*********************************************************
	* Maschero tutti i signali tranne quelli SIGTERM
	*/
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_SETMASK, &set, NULL);

    /***********************************
     *In attesa di un segnale di SIGTERM
     */
    sigwait(&set, &sig);




				/**************************************************
				 * Termino il thread dispatcher, in questo modo
				 * nessuno altra richiesta da client sarà possibile
				 *
				 */

    if (pthread_cancel(tid_disp))


				/****************************
				 * Join del thread dispatcher
				 */

    pthread_join(tid_disp, (void *) &status);

				/*************************
				 * Join dei threads worker
				 */

    tmp = lista_thread;
    while (tmp) {
	pthread_join(tmp->thrd, (void *) &status);
	tmp = tmp->next;
    }
    tmp = lista_thread;



				/***************************
				 * Chiudo/Cancello il socket
				 */
    closeSocket(server_chan);

				/*****************************************
				 * Memorizzo le agende in memoria di massa
				 */
    chdir(path_dir_agende);
    for (i = 0; i < offset; i++) {

	if (strlen(nomi_agende[i])) {
	    fd = fopen(nomi_agende[i], "w");

	    storeAgenda(fd, agende[i]);

	    fclose(fd);
	}
    }



    pthread_exit((void *) 0);

}



int cleanup(void)
{

    int i;

    for (i = 0; i < a_size; i++)
	free(nomi_agende[i]);
    for (i = 0; i < a_size; i++)
	dealloca_lista(agende[i]);

    free(nomi_agende);

    free(agende);


    return 0;

}
