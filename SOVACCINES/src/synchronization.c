/* Grupo SO-007
Rodrigo Branco - FC54457
Vasco Lopes - FC54410 */

#include <semaphore.h>

#include <unistd.h>
#include <sys/types.h> 
#include <fcntl.h>

#include "synchronization.h"

#include <stdlib.h>
#include <stdio.h>

	
// Nomes usados na criacao dos semaforos
#define STR_SEM_MAIN_CLI_FULL 	"sem_main_cli_full"
#define STR_SEM_MAIN_CLI_EMPTY 	"sem_main_cli_empty"
#define STR_SEM_MAIN_CLI_MUTEX 	"sem_main_cli_mutex"
#define STR_SEM_CLI_PRX_FULL 	"sem_cli_prx_full"
#define STR_SEM_CLI_PRX_EMPTY 	"sem_cli_prx_empty"
#define STR_SEM_CLI_PRX_MUTEX 	"sem_cli_prx_mutex"
#define STR_SEM_PRX_SRV_FULL	"sem_prx_srv_full"
#define STR_SEM_PRX_SRV_EMPTY 	"sem_prx_srv_empty"
#define STR_SEM_PRX_SRV_MUTEX 	"sem_prx_srv_mutex"
#define STR_SEM_SRV_CLI_FULL	"sem_srv_cli_full"
#define STR_SEM_SRV_CLI_EMPTY 	"sem_srv_cli_empty"
#define STR_SEM_SRV_CLI_MUTEX 	"sem_srv_cli_mutex"
#define STR_SEM_RESULTS_MUTEX	"sem_results_mutex"


/* Função que cria um novo semáforo com nome name e valor inicial igual a
* value. Pode concatenar o resultado da função getuid() a name, para tornar
* o nome único para o processo.
*/
sem_t * semaphore_create(char* name, int value) {
    sem_t* sem;
    sem = sem_open(name, O_CREAT, 0xFFFFFFFF, value);
    if (sem == SEM_FAILED){
        perror(name);
        exit(1);
    }

    return(sem);
}

/* Função que destroi o semáforo passado em argumento.
*/
void semaphore_destroy(char* name, sem_t* semaphore) {
	if (sem_close(semaphore) == -1){
        perror(name);
    }

    if (sem_unlink(name) == -1){
        perror(name);
    }
}

/* Função que inicia o processo de produzir, fazendo sem_wait nos semáforos
* corretos da estrutura passada em argumento.
*/
void produce_begin(struct prodcons* pc) {
	sem_wait(pc->empty);
    sem_wait(pc->mutex);
}

/* Função que termina o processo de produzir, fazendo sem_post nos semáforos
* corretos da estrutura passada em argumento.
*/
void produce_end(struct prodcons* pc) {
    sem_post(pc->mutex);
	sem_post(pc->full);
}

/* Função que inicia o processo de consumir, fazendo sem_wait nos semáforos
* corretos da estrutura passada em argumento.
*/
void consume_begin(struct prodcons* pc) {
	sem_wait(pc->full);
    sem_wait(pc->mutex);
}

/* Função que termina o processo de consumir, fazendo sem_post nos semáforos
* corretos da estrutura passada em argumento.
*/
void consume_end(struct prodcons* pc) {
    sem_post(pc->mutex);
	sem_post(pc->empty);
}

/* Função que faz wait a um semáforo.
*/
void semaphore_mutex_lock(sem_t* sem) {
	sem_wait(sem);
}

/* Função que faz post a um semáforo.
*/
void semaphore_mutex_unlock(sem_t* sem) {
	sem_post(sem);
}