/* Grupo SO-007
Rodrigo Branco - FC54457
Vasco Lopes - FC54410 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "process.h"
#include "client.h"
#include "proxy.h"
#include "server.h"

/* Função que inicia um processo através da função fork do SO. O novo
* processo pode ser um cliente, proxy, ou servidor, consoante a variável
* process_code tem valor 0, 1, ou 2 respectivamente, e irá executar a função
* execute_* respetiva, fazendo exit do retorno. O processo pai devolve o pid
* do processo criado.
*/
int launch_process(int process_id, int process_code, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {
    int pid = fork();

    if(pid == -1) {
        perror("Error!");
        exit(1);
    }

    if(pid == 0) {
        if(process_code == 0) {
            process_id = execute_client(process_id, buffers, data, sems);
            exit(process_id);
        }
        else if(process_code == 1) {
            process_id = execute_proxy(process_id, buffers, data, sems);
            exit(process_id);
        }
        else {
            process_id = execute_server(process_id, buffers, data, sems);
            exit(process_id);
        }
    }
    else {
        return(pid);
    }
}


/* Função que espera que um processo termine através da função waitpid. 
* Devolve o retorno do processo, se este tiver terminado normalmente.
*/
int wait_process(int process_id) {
    int status;
    waitpid(process_id, &status, 0);
    if(WIFEXITED(status)) {
        return(WEXITSTATUS(status));
    }
}
