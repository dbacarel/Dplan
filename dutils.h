typedef struct params {

	int a_size;
	int n_agende;
	char ** nomi_agende;
	elem_t ** agende;
	channel_t client;
}params;

typedef struct threads_list {
	pthread_t thrd;
	struct threads_list * next;
}threads_list;

#define SLOTS_MORE 10
#define ARRAY_NOMI 'N'
#define ARRAY_AGENDE 'A'

#define AGENDA_NOT_EXIST {\
				free(messaggio->buffer);\
				messaggio->type = MSG_ERROR;\
				messaggio->length = strlen("Agenda not existent") + 1;\
				messaggio->buffer = (char *) malloc(sizeof(char)\
						* messaggio->length);\
				strncpy(messaggio->buffer, "Agenda not existent",\
						messaggio->length);\
				sendMessage(param->client, messaggio);}\


/**
 Ordina le stringhe contenute dell'array che si trovano nel range min,max

 \param min indice estremo sx dell'array da ordinare

 \param q valore medio del range

  \param max indice estremo dx dell'array da ordinare

 */
void Merge(char **elenco, int min, int q, int max);


/**
 Ordina l'array di stringhe

 \param min indice estremo sx dell'array da ordinare

 \param q valore medio del range

  \param max indice estremo dx dell'array da ordinare

 */
void MergeSort(char ** elenco, int min, int max);
/**
 Controlla che il nome passato come argomento corrisponde al nome di una agenda caricata

 \param name nome dell'agenda

 \param array array che contiene i nomi delle agende caricate

  \param size lunghezza dell'array

  \retval Indice dell'agenda all'interno dell'array, se è stato trovato
	\retval size altrimenti

 */
int is_agenda(char *name, char **array, int size);
/**
 Genera un array di lunghezza 'size+SLOTS_MORE', dove la macro SLOTS_MORE ha valore 10
 I valori contenuti nel mio vecchio array, vengono copiati nel nuovo.

 Il flag array_type può assumere due valori rappresentati dalle macro ARRAY_NOMI e ARRAY_AGENDE
 La prima macro viene usata per indicare che l'array da espandere è di tipo char**, la seconda
 per array di tipo elem_t**

 \param array array da espandere

 \param size lunghezza dell'array

  \param array_tipe flag di tipo

  \retval puntatore al nuovo array

 */
void * enlarge_array(void **array, unsigned int size, unsigned char array_type);
/**
 Aggiunge il tid di thread in una lista

 \param lista lista dei threads

 \param sd tid del thread da aggiungere

  \retval tid del thread aggiunto

 */
int add_thread(threads_list **lista, int sd);
/**
 Dealloca la lista dei threads

 \param lista lista dei threads

 */
void free_thread_list(threads_list **lista);
/**
 Aggiunge il nome di una agenda nell'array

 \param nome_agenda nome dell'agenda da caricare

 \param dati struttura contenente varie dati tra cui l'array dei nomi delle agende caricate

  \retval 0 se è stato aggiunto correttamente
	\retval 1 altrimenti
 */
int add_agenda(char *nome_agenda, params *dati);
/**
 Comunica con un client eseguendo le sue richieste

 \param args Struttura contenente i dati necessari per eseguire le richieste

 */
void * thread_worker(void *args);
/**
 Si mette in attesa di una connessione da parte di un client e genera un nuovo thread per
 ogni client connesso

 \param args Struttura contenente i dati necessari per eseguire le richieste

 */
void * thread_dispatcher(void *args);
/**
 Dealloca aree di memoria precedentemente occupate per contenere i dati

 \param agende array contenente i contenuti delle agende caricate

 \param nomi array contenente i nomi delle agende caricate

 \param size lunghezza degli arrays

  \retval 0
 */
int cleanup(int size, elem_t ** agende, char ** nomi);



