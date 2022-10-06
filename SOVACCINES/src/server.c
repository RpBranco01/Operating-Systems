/* Grupo SO-007
Rodrigo Branco - FC54457
Vasco Lopes - FC54410 */

#include "server.h"

#include <stdlib.h>



/* Função principal de um Servidor. Deve executar um ciclo infinito onde em 
* cada iteração do ciclo lê uma operação dos proxies e se a mesma tiver id 
* diferente de -1 e se data->terminate ainda for igual a 0, processa-a e
* escreve a resposta para os clientes. Operações com id igual a -1 são 
* ignoradas (op inválida) e se data->terminate for igual a 1 é porque foi 
* dada ordem de terminação do programa, portanto deve-se fazer return do
* número de operações processadas. Para efetuar estes passos, pode usar os
* outros métodos auxiliares definidos em server.h.
*/
int execute_server(int server_id, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {
    int* counter = create_dynamic_memory(sizeof(int));

    while(1) {
        struct operation* op = create_dynamic_memory(sizeof(struct operation));
        
        server_receive_operation(op, buffers, data, sems);

        if((op->id != -1) && (*(data->terminate) == 0)) {
            server_process_operation(op, server_id, counter);
            server_send_answer(op, buffers, data, sems);
        }

        destroy_dynamic_memory(op);

        if(*(data->terminate) == 1) {
            int rtrn = *counter;
            destroy_dynamic_memory(counter);
            return(rtrn);
        }
    }
}


/* Função que lê uma operação do buffer de memória partilhada entre
* proxies e servidores, efetuando a necessária sincronização antes e
* depois de ler. Quando o processo acorda da sincronização, e antes de
* tentar ler a operação, deve verificar se data->terminate tem valor 1.
* Em caso afirmativo, retorna imediatamente da função.
*/
void server_receive_operation(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {
    if(*(data->terminate) == 1) {
        return;
    }
    consume_begin(sems->prx_srv);
    read_rnd_access_buffer(buffers->prx_srv, data->buffers_size, op);
    consume_end(sems->prx_srv);
}


/* Função que processa uma operação, alterando o seu campo server para o id
* passado como argumento, alterando o estado da mesma para 'S' (served), e 
* incrementando o contador de operações.
*/
void server_process_operation(struct operation* op, int server_id, int* counter) {
    op->server = server_id;
    op->status = 'S';
    *counter = *counter + 1;
}


/* Função que escreve uma operação no buffer de memória partilhada entre
* servidores e clientes, efetuando a necessária sincronização antes e
* depois de escrever.
*/
void server_send_answer(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {
    produce_begin(sems->srv_cli);
    write_circular_buffer(buffers->srv_cli, data->buffers_size, op);
    produce_end(sems->srv_cli);
}