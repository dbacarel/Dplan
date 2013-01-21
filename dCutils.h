/** \file  dCutils.h
*	\author Daniele Bacarella mat. 408975
*	Si dichiara che il contenuto di questo file e' in ogni sua parte opera
*	originale dell' autore.
*/


#ifndef DCUTILS_H_
#define DCUTILS_H_

/**
 * Stampa un messaggio di errore ed esce dal programma
 */
#define MESS_EXIT(mess){\
		printf(mess);\
			exit(EXIT_FAILURE);\
}
/**
 * Numero di opzioni
 */
#define NOPTS 7


/**
 * Stampa un messaggio di usage
 */
void printHelp(void);

/**
 Ritorna una stringa formattata da inviare come messaggio al server.
 Valori possibili di 'tipo':
 - 0 Opzione D
 - 1 Opzione U
 - 2 Opzione C
 - 3 Opzione Q
 - 4 Opzione R
 - 5 Opzione G
 - 6 Opzione M

 \param tipo specifica il tipo di messaggio da creare

 \param agenda contiene il nome dell'agenda

 \param args array contenente gli argomenti delle opzioni

 \param length variabile intera da caricare con la lunghezza del messaggio generato

 \retval Messaggio
 */
char * getBufferString(int tipo, char *agenda, char **args,
		unsigned int *length);

/**

 Controlla la compatibilità delle opzioni inserite

 \param flags array dei flags relativi alle opzioni

 \param indice indice dell'opzione da controllare all'interno dell'array

 \retval 0 se controllo ok
 \retval 1 altrimenti
 */

int checkOpts(short int *flags, int indice);

/**

 Controllo formato delle date
 Type:
 - 1 per date xx-xx-xxxx
 - 0 per date xx-xxxx

 \param type tipo di data

 \param date la stringa data

 \retval 0 se controllo ok
 \retval 1 altrimenti
 */

int checkDate(int type, char *date);

/**

 Controllo validità degli argomenti delle opzioni
 Type:
 - 1 per date xx-xx-xxxx
 - 0 per date xx-xxxx

 \param flags array dei flags relativi alle opzioni

 \param argc numero di argomenti passati all'eseguibile

 \param argv stringhe degli argomenti passati all'eseguibile

 \param args array contenente gli argomenti delle opzioni

 \param agenda stringa che sarà riempita con il nome dell'agenda passata come argomento

 \retval 0 se controllo ok
 \retval 1 altrimenti
 */

 int checkArgs(short int *flags, int argc, char **argv, char ** args,
		char **agenda);

/**

 Inizializza alcune variabili di importanza trascurabile
 Viene chiamato per ogni opzione analizzata.
 I dati che genera, serviranno per le operazioni di check

 \param flags array dei flags relativi alle opzioni

 \param indice indice dell'elemento dell'array dai inizializzare

 \param vtipo variabile che sarà inizializzata con il tipo del messaggio da inviare

 \param type intero che indica il tipo di messaggio

  \param args array destinato a contenere gli argomenti delle opzioni passate all'eseuibile

   \param optarg argomento

 \retval 0 se controllo ok
 \retval 1 altrimenti
 */

 void setInfo(short int *flags, int indice, int *vtipo, int type,
		char **args, char *optarg);

#endif /* DCUTILS_H_ */
