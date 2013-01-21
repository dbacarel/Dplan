/** \file  dSutils.h
*	\author Daniele Bacarella mat. 408975
*	Si dichiara che il contenuto di questo file e' in ogni sua parte opera
*	originale dell' autore.
*/

#ifndef DSUTILS_H_
#define DSUTILS_H_

/**
 * Tiene traccia della lunghezza delle strutture dati
 * che memorizzano le info sulle agende
 */
int  a_size;

/**	Traccia l'indice dell'ultima agenda nelle SD	*/
int  offset;

/**	Contiene il numero di threads worker creati	*/
int  n_thread;

/**	T_ID del thread dispatcher, usato dal thread_sigHandler per op di cancel/join	*/
pthread_t tid_disp;

/**	Descrittore del canale di ascolto 	*/
serverChannel_t server_chan;

/**
 * SD per la memorizzazione delle info sulle agende caricate in memoriza principale
 */
char **  nomi_agende;
elem_t **  agende;

/**	Contiene il path della dir nella quale si trovano le agende	*/
char path_dir_agende[UNIX_PATH_MAX];



/**	Struttura nodo per generare una lista per la memorizzazione dei t_id dei threads worker
 *
 * -thrd valore T_ID
 * -next puntatore al nodo successivo
 */
typedef struct threads_list {
	pthread_t thrd;
	struct threads_list * next;
}threads_list;

/**	Lista dei threads worker che verranno generati	*/
threads_list *lista_thread;

/********
 * MACRO
 ********/

/** Indica il numero di "slot" da aggiungere alla SD che verrà ingradita	*/
#define SLOTS_MORE 10

/** Usate come parametri alla funzione enlarge_array per distinguere quale tipo SD trattare	*/
#define ARRAY_NOMI 'N'
#define ARRAY_AGENDE 'A'

/**	Prepara e invia al client un messaggio standard	*/
#define STANDARD_MESSAGES(message,msg_type) {\
				free(messaggio->buffer);\
				messaggio->type = msg_type;\
				messaggio->length = strlen(message) + 1;\
				messaggio->buffer = (char *) malloc(sizeof(char)\
						* messaggio->length);\
				strncpy(messaggio->buffer, message,\
						messaggio->length);\
				sendMessage(client, messaggio);\
				closeConnection(client);}\

#define CLEANUP(res,size){\
	for(cont=0;cont<size;cont++) free(res[cont]);\
	free(res);\
}





/**
 Ordina l'array di stringhe

 \param min indice estremo sx dell'array da ordinare

 \param q valore medio del range

  \param max indice estremo dx dell'array da ordinare

 */
void MergeSort(elem_t ** elenco, int min, int max);

/**
 Controlla che il nome passato come argomento corrisponde al nome di una agenda caricata

 \param name nome dell'agenda

  \retval Indice dell'agenda all'interno dell'array, se è stato trovato
	\retval offset altrimenti

 */
int is_agenda(char *name);

/**
 Genera un array di lunghezza 'size+SLOTS_MORE', dove la macro SLOTS_MORE ha valore 10
 I valori contenuti nel mio vecchio array, vengono copiati nel nuovo.

 Il flag array_type può assumere due valori rappresentati dalle macro ARRAY_NOMI e ARRAY_AGENDE
 La prima macro viene usata per indicare che l'array da espandere è di tipo char**, la seconda
 per array di tipo elem_t**

  \param array_tipe flag di tipo

  \retval puntatore al nuovo array

 */
void * enlarge_array(unsigned char array_type);

/**
 Aggiunge il tid di thread in una lista

 \param sd tid del thread da aggiungere

  \retval tid del thread aggiunto

 */
int add_thread(int sd);


/**	Dealloca la lista dei threads
 * \param lista_threads puntatore alla lista
 * */
void free_thread_list(threads_list ** lista_threads);

/**
 Aggiunge il nome di una agenda nell'array nomi_agende

 \param nome_agenda nome dell'agenda da caricare

  \retval 0 se è stato aggiunto correttamente
	\retval 1 altrimenti
 */
int add_agenda(char *nome_agenda);

/**
 @Funzione thread@
 Comunica con un client eseguendo le sue richieste
 */
void * thread_worker(void *args);

/**
 @Funzione thread@
 Si mette in attesa di una connessione da parte di un client e genera un nuovo thread
 worker per ogni client connesso
 */
void * thread_dispatcher(void *args);

/**
 Dealloca aree di memoria precedentemente occupate per contenere i dati
  \retval 0
 */
int cleanup(void);

/**
 @Funzione thread@
 Usata per l'handling dei segnali inviati al processo, in particolare
 tutti i segnali eccetto SIGTERM sono mascherati, all'arrivo di quest ultimo
 vengono fatti terminare i thread workers e il thread dispatcher per poi eseguire
 operazioni di memorizzazione dei dati su memoria di massa e op di cleanup.
 */
void * thread_sigHandler(void *args);


#endif /* DSUTILS_H_ */

