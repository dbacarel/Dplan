/** \file  dserver.c
*	\author Daniele Bacarella mat. 408975
*	Si dichiara che il contenuto di questo file e' in ogni sua parte opera
*	originale dell' autore.
*/

#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <mcheck.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <bits/signum.h>
#include "lcscom.h"
#include "lbase.h"
#include "llist.h"
#include "dSutils.h"



/*
 * Semplice macro per controllo dei valori
 * di ritorno delle funzioni
 */
#define E_NEG(a){\
	if(a<0) exit(EXIT_FAILURE);}




int main(int argc, char **argv)
{


	/*	Per gestione dei segnali	*/
    struct sigaction s1;

    /*	Usata per le operazioni di mascheramento dei segnali	*/
    sigset_t set;

    pthread_t tid_handler;

    /*	Usate per la lettura della dir contenente le agende	*/
    DIR *dir;
    struct dirent *dp;

    /*	Usato per memorizzare il pathname dei file agenda	*/
    char path_file[UNIX_PATH_MAX];

    /*	Usate per memorizzare le agende caricate	*/
    elem_t **tmp_agende;

    /*	Usate per memorizzare i nomi delle agende caricate	*/
    char **tmp_nomi_agende;

    /* Uso trascurabile	*/
    int cont, i;

    void *status;
    /*	Per leggere le agende	*/
    FILE *file;


    /*
     * Maschero tutti i segnali tranne SIGTERM
     */
    bzero(&s1, sizeof(s1));


    E_NEG(sigemptyset(&set))
	E_NEG(sigaddset(&set, SIGTERM));
    E_NEG(pthread_sigmask(SIG_BLOCK, &set, NULL))

    /*
     * SIGPIPE verrà ignorato per tutto il tempo
     */
	s1.sa_handler = SIG_IGN;
    E_NEG(sigaction(SIGPIPE, &s1, NULL));


    if (argc == 1) {
	printf("Usage: dserver PATH_DIR_AGENDE\n");
	exit(EXIT_FAILURE);
    }

    if (strlen(argv[1]) > UNIX_PATH_MAX)
	exit(EXIT_FAILURE);

    /*	Copia del path dir agende passato come argomento */
    strcpy(path_dir_agende, argv[1]);

    agende = NULL;
    nomi_agende = NULL;
    status = NULL;
    /*
     * Setto valore iniziale della lunghezza delle strutture dati
     */
    a_size = 10;
    /*
     * Inizializzo var offset
     */
    offset = 0;

    /*
     * Inizializzo tid_disp
     */
    tid_disp = -1;

    /*
     * ***********************************SET STRUTTURE DATI ****************************************
     */

    /*
     * Preparo struttura dati per l'allocazione dell'elenco dei nomi agenda
     */
    nomi_agende = (char **) malloc(sizeof(char *) * a_size);

    if (!nomi_agende) {
	printf("Problemi durante il settaggio delle strutture dati\n");
	exit(EXIT_FAILURE);
    }
    for (i = 0; i < a_size; i++) {
	nomi_agende[i] = (char *) malloc(sizeof(char) * LAGENDA + 1);
	if (!nomi_agende[i]) {
	    printf("Problemi durante il settaggio delle strutture dati\n");
	    CLEANUP(nomi_agende, i);
	    exit(EXIT_FAILURE);
	}
    }

    /*
     * Preparo struttura dati per l'allocazione delle agende
     */
    agende = (elem_t **) malloc(sizeof(elem_t **) * a_size);

    if (agende == NULL) {
	printf("Problemi durante il settaggio delle strutture dati\n");
	free(nomi_agende);
	CLEANUP(nomi_agende, a_size);
	exit(EXIT_FAILURE);
    }
    for (i = 0; i < a_size; i++) {
	agende[i] = NULL;
    }

    /*
     * ******************************************************************************************
     */

    /*
     * Controllo dell'esistenza della dir 'path_dir_agende'
     * in caso negativo viene creata
     */

    if ((dir = opendir(path_dir_agende)) == NULL) {
	if (errno == ENOENT) {
	    if (mkdir(path_dir_agende, S_IRWXU)) {
		perror("");
		exit(EXIT_FAILURE);
	    }
	}

    }

    i = 0;
    while ((dp = readdir(dir)) != NULL) {

	/*
	 * d_type 8 file
	 */
	if (dp->d_type != 8)
	    continue;		/* Ignoro eventuali altri tipi */
	else {
		bzero(path_file,UNIX_PATH_MAX);
	    sprintf(path_file, "%s/%s", path_dir_agende, dp->d_name);


	    file = fopen(path_file, "r");
	    if (file == NULL) continue;

	    if (i == a_size) {
		/*	Aumento size delle strutture	*/

		tmp_agende = agende;
		tmp_nomi_agende = nomi_agende;

		agende = (elem_t **) enlarge_array(ARRAY_AGENDE);

		if (!agende) {
		    printf("Problemi durante l'allocazione di mem\n");
		    cleanup();
		    exit(EXIT_FAILURE);
		}

		nomi_agende = (char **) enlarge_array(ARRAY_NOMI);

		if (!nomi_agende) {
		    printf("Problemi durante l'allocazione di mem\n");
		    free(agende);
		    cleanup();
		    exit(EXIT_FAILURE);
		}

		/*	Aggiorno la lunghezza delle SD	*/
		a_size += SLOTS_MORE;

		free(tmp_agende);
		free(tmp_nomi_agende);

	    }

	    if (loadAgenda(file, &agende[i]) < 0);	/* Nessuna notifica, mi limito ad ignorare l'agenda */
	    else {

		if (strlen(dp->d_name) > 20);	/* Come sopra */
		else {
			/*	Controlli preliminari superati, carico l'agenda	*/
		    strncpy(nomi_agende[i], dp->d_name, LAGENDA + 1);
		    offset++;
		    i++;
		}
	    }
	    fclose(file);
	}

    }


    closedir(dir);



    /*	Creazione del thread_dispatcher	*/
    status = 0;
    if (pthread_create(&tid_disp, NULL, thread_dispatcher, NULL))
	perror("Errore durante la creazione del thread dispatcher");
    sleep(1);

    /*	Creazione del thread_sigHandler	*/
    if (pthread_create(&tid_handler, NULL, thread_sigHandler, NULL))
	perror("Errore durante la creazione del thread dispatcher");

    pthread_join(tid_handler, &status);


    exit(EXIT_SUCCESS);
}
