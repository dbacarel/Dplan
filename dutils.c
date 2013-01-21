#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include "lcscom.h"
#include "lbase.h"
#include "llist.h"
#include "dutils.h"

void Merge(char **elenco, int min, int q, int max)
{
    int i, j, k;
    char **buf;

    buf = (char **) malloc(sizeof(char *) * (max + 1));
    for (i = 0; i < (max + 1); i++)
	buf[i] = (char *) malloc(sizeof(char) * LRECORD + 1);

    i = min;
    j = q + 1;
    k = 0;
    while (i <= q && j <= max) {
	if (strcmp(elenco[i], elenco[j]) < 0) {
	    strcpy(buf[k], elenco[i]);

	    i++;
	} else {
	    strcpy(buf[k], elenco[j]);
	    j++;
	}
	k++;
    }
    while (i <= q) {
	strcpy(buf[k], elenco[i]);
	i++;
	k++;
    }
    while (j <= max) {
	strcpy(buf[k], elenco[j]);
	j++;
	k++;
    }
    for (k = min; k <= max; k++)
	strcpy(elenco[k], buf[k - min]);

    for (i = 0; i < (max + 1); i++)
	free(buf[i]);
    free(buf);

    return;
}

void MergeSort(char **elenco, int min, int max)
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

int is_agenda(char *name, char **array, int size)
{
    int i;

    for (i = 0; i < size; i++)
	if (!strcmp(array[i], name))
	    return i;

    return i;
}
void *enlarge_array(void **array, unsigned int size,
		    unsigned char array_type)
{

    char **p;
    elem_t **r;
    int cont, i;

    i = 0;
    switch (array_type) {

    case ARRAY_NOMI:
	p = (char **) malloc(sizeof(char *) * (size + SLOTS_MORE));
	if (!p)
	    return NULL;

	for (cont = 0; cont < size; cont++) {

	    p[cont] = (char *) array[cont];
	}
	/*if(strlen((char*)array[cont])==0) continue; per adesso non dovrebbe servire */

	for (cont = size; cont < size + SLOTS_MORE; cont++)
	    p[cont] = (char *) malloc(sizeof(char) * LAGENDA + 1);

	return p;
	break;

    case ARRAY_AGENDE:
	r = (elem_t **) malloc(sizeof(elem_t *) * (size + SLOTS_MORE));
	if (!r)
	    return NULL;

	for (cont = 0; cont < size; cont++)
	    r[cont] = (elem_t *) array[cont];
	for (cont = size; cont < size + SLOTS_MORE; cont++)
	    r[cont] = NULL;

	return r;
	break;
    }
    return p;
}

int add_thread(threads_list ** lista, int sd)
{

    threads_list *nodo, *tmp;
    nodo = (threads_list *) malloc(sizeof(threads_list));
    nodo->thrd = sd;
    nodo->next = NULL;

    if (*lista == NULL)
	*lista = nodo;
    else {
	tmp = *lista;
	while (tmp->next)
	    tmp = tmp->next;
	tmp->next = nodo;
    }

    return nodo->thrd;

}

void free_thread_list(threads_list ** lista)
{

    if (*lista == NULL)
	return;
    free_thread_list(&((*lista)->next));
    free(*lista);
    return;
}

int add_agenda(char *nome_agenda, params * dati)
{

    char **tmp_nomi_agende;
    elem_t **tmp_agende;
    int i;

    i = 0;
    printf("***************NOME: %s\n", nome_agenda);
    if (is_agenda(nome_agenda, dati->nomi_agende, dati->n_agende)
	< dati->n_agende)
	return 2;

    /*
     * Cerco uno slot vuoto
     */
    for (i = 0; i < dati->n_agende; i++)
	if (strlen(dati->nomi_agende[i]) == 0)
	    break;

    /*
     * no slot liberi, procedo aggiungendo in coda
     */
    if (i == dati->n_agende) {
	if (dati->n_agende == dati->a_size) {

	    tmp_agende = dati->agende;
	    tmp_nomi_agende = dati->nomi_agende;

	    dati->agende =
		enlarge_array((void **) dati->agende, dati->a_size,
			      ARRAY_AGENDE);

	    if (!dati->agende) {
		printf
		    ("add_agenda:Problemi durante l'allocazione di mem\n");
		/*
		 * Riassegno il valore iniziale, in questi casi assumo che
		 *  il prob di malloc sia stato un caso e il client può ritentare
		 */
		dati->agende = tmp_agende;
		return 1;
	    }
	    dati->nomi_agende = enlarge_array((void **) dati->nomi_agende,
					      dati->a_size, ARRAY_NOMI);

	    if (!dati->nomi_agende) {
		printf
		    ("add_agenda:Problemi durante l'allocazione di mem\n");
		/*
		 * Come sopra
		 * NOTA:
		 * nel caso di errore qui, la lunghezza dell'array 'agende' rimarrebe più grande di SLOTS_MORE, nonostante ciò non
		 * dovrebbe rappresentare un problema, visto che il valore 'a_size' non viene aggiornato
		 */
		dati->nomi_agende = tmp_nomi_agende;
		return 1;
	    }

	    dati->a_size += SLOTS_MORE;

	    /*
	     * Il corrispondente nodo nell'aria 'agende' rimane NULL ma
	     * la presenza del nome agenda in 'nomi_agende' indica
	     * che quella agenda è vuota.
	     */
	    free(tmp_agende);
	    free(tmp_nomi_agende);
	}

	strncpy(dati->nomi_agende[dati->n_agende], nome_agenda,
		LAGENDA + 1);

	dati->n_agende++;
    } else {
	/*
	 * Sono in uno slot libero in seguito ad una eliminazione di agenda
	 */
	strncpy(dati->nomi_agende[i], nome_agenda, LAGENDA + 1);
    }

    return 0;

}
void *thread_worker(void *args)
{

    params *param;
    char agenda[LAGENDA + 1], data[LDATA + 1],
	utente[LUTENTE + 1],
	descrizione[LDESCRIZIONE + 1], pattern[LRECORD + 1], record[LRECORD
								    + 1],
	*ptr, **tmp_buf;
    int foo, i, cont;
    message_t *messaggio;	/* magari lo uso anche per il mess di ritorno */
    evento_t *evento;
    elem_t *elem;

    printf("THREAD_WORKER::\n");

    foo = 0;
    i = 0;
    messaggio = (message_t *) malloc(sizeof(message_t));
    messaggio->buffer = NULL;
    param = (params *) args;

    if (receiveMessage(param->client, messaggio) < 0);
    else {
	printf("--->Type: %c\n", messaggio->type);

	printf("--->Buff: %s\n", messaggio->buffer);

	ptr = messaggio->buffer;

	switch (messaggio->type) {

	case MSG_MKAGENDA:
	    printf(">MSG_MKAGENDA: \n");
	    strcpy(agenda, ptr);
	    i = add_agenda(agenda, param);

	    if (i) {
		free(messaggio->buffer);
		messaggio->type = MSG_ERROR;
		if (i == 1) {
		    messaggio->length =
			strlen
			("dserver hasn't been able to add the agenda") + 1;
		    messaggio->buffer = (char *) malloc(sizeof(char)
							*
							messaggio->length);
		    strncpy(messaggio->buffer,
			    "dserver hasn't been able to add the agenda",
			    messaggio->length);
		} else {
		    messaggio->length =
			strlen("Cannot create, agenda already present") +
			1;
		    messaggio->buffer = (char *) malloc(sizeof(char)
							*
							messaggio->length);
		    strncpy(messaggio->buffer,
			    "Cannot create, agenda already present",
			    messaggio->length);
		}

		sendMessage(param->client, messaggio);

	    } else {
		free(messaggio->buffer);
		messaggio->type = MSG_OK;
		messaggio->length = strlen("Created") + 1;
		messaggio->buffer = (char *) malloc(sizeof(char)
						    * messaggio->length);
		strncpy(messaggio->buffer, "Created", messaggio->length);

		sendMessage(param->client, messaggio);

	    }

	    break;

	case MSG_RMAGENDA:
	    printf(">MSG_RMPAGENDA\n");
	    printf("Buffer1 : '%s'\n", ptr);
	    strcpy(agenda, ptr);

	    /*
	     * Cerco il nome dell'agenda all'interno dell'elenco 'nomi_agende'
	     */
	    i = is_agenda(agenda, param->nomi_agende, param->n_agende);

	    /*
	     * In questo caso l'agenda cercata non è stata trovata
	     */
	    if (i == param->n_agende)
		AGENDA_NOT_EXIST
		else {

		/*
		 * Trovata, controllo che sia vuota o meno
		 */
		if (param->agende[i] == NULL) {
		    /*
		     * Marco il corrispondende slot in 'nomi_agende'
		     * così da poterlo ricliclare
		     */
		    strcpy(param->nomi_agende[i], "");	/*ok cosi non si presenta il problema di elinare il nodo in ''agende' */
		    free(messaggio->buffer);
		    messaggio->type = MSG_OK;
		    messaggio->length = strlen("Removed") + 1;
		    messaggio->buffer = (char *) malloc(sizeof(char)
							*
							messaggio->length);
		    strncpy(messaggio->buffer, "Removed",
			    messaggio->length);

		    sendMessage(param->client, messaggio);
		} else {
		    free(messaggio->buffer);
		    messaggio->type = MSG_ERROR;
		    messaggio->length =
			strlen("Agenda not empty, cannot remove") + 1;
		    messaggio->buffer = (char *) malloc(sizeof(char)
							*
							messaggio->length);
		    strncpy(messaggio->buffer,
			    "Agenda not empty, cannot remove",
			    messaggio->length);
		    sendMessage(param->client, messaggio);

		}

		}

	    break;
	case MSG_RMPATTERN:
	    printf(">MSG_RMPATTERN:  \n");
	    printf("Buffer1 : '%s'\n", ptr);
	    strcpy(agenda, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);
	    printf("Buffer2  : '%s'\n", ptr);
	    strcpy(pattern, ptr);

	    i = is_agenda(agenda, param->nomi_agende, param->n_agende);
	    if (i == param->n_agende)
		AGENDA_NOT_EXIST
		else {

		cont = 0;
		free(messaggio->buffer);
		elem = param->agende[i];
		while (elem) {
		    if (matchPattern(pattern, elem->ptev) == 1)
			cont++;

		    elem = elem->next;
		}
		tmp_buf = (char **) malloc(sizeof(char *) * cont);
		for (foo = 0; foo < cont; foo++)
		    tmp_buf[foo] =
			(char *) malloc(sizeof(char) * LRECORD + 1);

		messaggio->buffer =
		    (char *) calloc(sizeof(char) * (LRECORD * cont) +
				    cont + 1, 1);
		elem = param->agende[i];

		cont = 0;
		i = 0;

		while (elem) {
		    if (matchPattern(pattern, elem->ptev) == 1) {
			convertiEvento(elem->ptev, record);

			cont += strlen(record);
			strncpy(tmp_buf[i], record, strlen(record) + 1);
			i += 1;

			/*
			 * strncat(messaggio->buffer, record, LRECORD + 1);
			 strcat(messaggio->buffer, "\0"); Sarebbe un modo per estrarre a 100char alla volta vedere printf e format
			 */
		    }
		    elem = elem->next;
		}
		MergeSort(tmp_buf, 0, i - 1);

		for (foo = 0; foo < i; foo++) {
		    strcat(messaggio->buffer, tmp_buf[foo]);
		    strcat(messaggio->buffer, "\n");
		}

		for (foo = 0; foo < i; foo++)
		    free(tmp_buf[foo]);
		free(tmp_buf);

		messaggio->type = MSG_OK;
		messaggio->length = (LRECORD * i) + i + 1;	/*cont+1 */
		sendMessage(param->client, messaggio);

		}

	    break;
	case MSG_INSERT:
	    printf(">MSG_INSERT: \n");
	    printf("Buffer1 : '%s'\n", ptr);
	    strcpy(agenda, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);
	    printf("Buffer2 : '%s'\n", ptr);
	    strcpy(data, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);
	    printf("Buffer3 : '%s'\n", ptr);
	    strcpy(utente, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);
	    printf("Buffer4 : '%s'\n", ptr);
	    strcpy(descrizione, ptr);

	    i = is_agenda(agenda, param->nomi_agende, param->n_agende);

	    if (i == param->n_agende)
		AGENDA_NOT_EXIST
		else {

		sprintf(record, "%s %s#%s", data, utente, descrizione);
		evento = convertiRecord(record);
		add(&(param->agende[i]), evento);
		free(evento);
		free(messaggio->buffer);
		messaggio->type = MSG_OK;
		messaggio->length = strlen("Success") + 1;
		messaggio->buffer = (char *) malloc(sizeof(char)
						    * messaggio->length);
		strncpy(messaggio->buffer, "Success", messaggio->length);
		sendMessage(param->client, messaggio);

		}

	    break;
	case MSG_EGIORNO:
	    printf(">  MSG_EGIORNO<: \n");
	    printf("Buffer1 : '%s'\n", ptr);
	    strcpy(agenda, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);
	    printf("Buffer2 : '%s'\n", ptr);
	    strcpy(data, ptr);

	    i = is_agenda(agenda, param->nomi_agende, param->n_agende);

	    if (i == param->n_agende)
		AGENDA_NOT_EXIST
		else {

		cont = 0;
		free(messaggio->buffer);
		elem = param->agende[i];
		while (elem) {
		    if (matchData(data, elem->ptev) == 1)
			cont++;

		    elem = elem->next;
		}
		tmp_buf = (char **) malloc(sizeof(char *) * cont);
		for (foo = 0; foo < cont; foo++)
		    tmp_buf[foo] =
			(char *) malloc(sizeof(char) * LRECORD + 1);

		messaggio->buffer = (char *) calloc((LRECORD * cont) +
						    cont + 1,
						    sizeof(char));
		elem = param->agende[i];
		printf("**** %d ****\n", (LRECORD * cont) + cont + 1);

		i = 0;
		cont = 0;
		/*uso var 'i' come cont */

		while (elem) {

		    if (matchData(data, elem->ptev) == 1) {
			convertiEvento(elem->ptev, record);
			cont += strlen(record);
			strncpy(tmp_buf[i], record, strlen(record) + 1);
			i += 1;
		    }
		    elem = elem->next;
		}

		MergeSort(tmp_buf, 0, i - 1);

		for (foo = 0; foo < i; foo++) {
		    strcat(messaggio->buffer, tmp_buf[foo]);
		    strcat(messaggio->buffer, "\n");
		    printf("LEN: %d\n", strlen(messaggio->buffer));
		}

		for (foo = 0; foo < i; foo++)
		    free(tmp_buf[foo]);
		free(tmp_buf);

		messaggio->type = MSG_OK;
		messaggio->length = (LRECORD * i) + i + 1;
		sendMessage(param->client, messaggio);
		}

	    break;
	case MSG_EMESE:
	    printf(">MSG_EMESE: \n");
	    printf("Buffer1 : '%s'\n", ptr);
	    strcpy(agenda, ptr);
	    ptr = &(ptr[strlen(ptr) + 1]);
	    printf("Buffer2 : '%s'\n", ptr);
	    strcpy(data, "**-");
	    strcat(data, ptr);

	    i = is_agenda(agenda, param->nomi_agende, param->n_agende);

	    if (i == param->n_agende)
		AGENDA_NOT_EXIST
		else {

		cont = 0;
		free(messaggio->buffer);
		elem = param->agende[i];

		while (elem) {
		    if (matchData(data, elem->ptev) == 1)
			cont++;

		    elem = elem->next;
		}
		tmp_buf = (char **) malloc(sizeof(char *) * cont);
		for (foo = 0; foo < cont; foo++)
		    tmp_buf[foo] =
			(char *) malloc(sizeof(char) * LRECORD + 1);

		messaggio->buffer = (char *) calloc((LRECORD * cont) +
						    cont + 1,
						    sizeof(char));

		elem = param->agende[i];

		cont = 0;
		i = 0;

		while (elem) {

		    if (matchData(data, elem->ptev) == 1) {
			convertiEvento(elem->ptev, record);

			cont += strlen(record);

			strncpy(tmp_buf[i], record, strlen(record) + 1);
			i += 1;

		    }

		    elem = elem->next;
		}

		printf("***** cont %d**\n", i);
		MergeSort(tmp_buf, 0, i - 1);
		printf("**** %d ****\n", i);

		for (foo = 0; foo < i; foo++) {
		    strcat(messaggio->buffer, tmp_buf[foo]);
		    strcat(messaggio->buffer, "\n");
		}

		for (foo = 0; foo < i; foo++)
		    free(tmp_buf[foo]);
		free(tmp_buf);

		messaggio->type = MSG_OK;
		messaggio->length = (LRECORD * i) + i + 1;
		sendMessage(param->client, messaggio);

		}

	    break;

	}
    }

    free(messaggio->buffer);
    free(messaggio);
    /*
     * rivedere cosa ritornare
     */
    return NULL;

}

void *thread_dispatcher(void *args)
{

    params *param;
    int n_thread;
    void *status;
    pthread_t nuovo_thread;
    serverChannel_t server_chan;
    channel_t client;
    threads_list *lista_threads;

    printf(":: THREAD_DISP ::\n");
    param = (params *) args;
    n_thread = 0;
    lista_threads = NULL;

    /*
       printf("Parametri:\n");
       printf("- a_size: %d\n", param->a_size);
       printf("- n_agende: %d\n", param->n_agende);
       printf("- nomi: %s\n", *(param->nomi_agende));
     */
    server_chan = createServerChannel(TMP);
    if (server_chan < 0)
	perror("Problemi durante creazione del canale socket");
    else {
	while (n_thread < 1) {
	    printf("In attesa di un client %d....\n", n_thread);
	    client = acceptConnection(server_chan);
	    if (client < 0)
		perror("Errore in acceptConnection()");
	    else {

		nuovo_thread = add_thread(&lista_threads, client);
		n_thread++;
		param->client = client;
		if (pthread_create(&nuovo_thread, NULL, thread_worker,
				   (void *) param))
		    perror
			("Errore durante la creazione del thread dispatcher");

		pthread_join(nuovo_thread, &status);
	    }
	}

    }
    free_thread_list(&lista_threads);
    return NULL;
}

/*
 * Aumenta di 10 la dimensione degli arrays
 * Ret
 * 0 Ok
 * 1 Se è stato riscontrato qualche problema
 */

int cleanup(int size, elem_t ** agende, char **nomi)
{

    int i;

    for (i = 0; i < size; i++)
	free(nomi[i]);
    for (i = 0; i < size; i++)
	dealloca_lista(agende[i]);

    free(nomi);
    free(agende);

    return 0;

}
