/* Grupo SO-007
Rodrigo Branco - FC54457
Vasco Lopes - FC54410 */

#include "main.h"
#include "process.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
	//init data structures
	struct main_data *data = create_dynamic_memory(sizeof(struct main_data));
	struct communication_buffers *buffers = create_dynamic_memory(sizeof(struct communication_buffers));
	buffers->main_cli = create_dynamic_memory(sizeof(struct rnd_access_buffer));
	buffers->cli_prx = create_dynamic_memory(sizeof(struct circular_buffer));
	buffers->prx_srv = create_dynamic_memory(sizeof(struct rnd_access_buffer));
	buffers->srv_cli = create_dynamic_memory(sizeof(struct circular_buffer));
	struct semaphores *sems = create_dynamic_memory(sizeof(struct semaphores));
	sems->main_cli = create_dynamic_memory(sizeof(struct prodcons));
	sems->cli_prx = create_dynamic_memory(sizeof(struct prodcons));
	sems->prx_srv = create_dynamic_memory(sizeof(struct prodcons));
	sems->srv_cli = create_dynamic_memory(sizeof(struct prodcons));

	//execute main code
	main_args(argc, argv, data);
	create_dynamic_memory_buffers(data);
	create_shared_memory_buffers(data, buffers);
	create_semaphores(data, sems);
	launch_processes(buffers, data, sems);
	user_interaction(buffers, data, sems);
	
	//release final memory
	destroy_dynamic_memory(data);
	destroy_dynamic_memory(buffers->main_cli);
	destroy_dynamic_memory(buffers->cli_prx);
	destroy_dynamic_memory(buffers->prx_srv);
	destroy_dynamic_memory(buffers->srv_cli);
	destroy_dynamic_memory(buffers);
	destroy_dynamic_memory(sems->main_cli);
	destroy_dynamic_memory(sems->cli_prx);
	destroy_dynamic_memory(sems->prx_srv);
	destroy_dynamic_memory(sems->srv_cli);
	destroy_dynamic_memory(sems);
}

///////////////////////////////////////////////////////////////////////////////

/* Função que lê os argumentos da aplicação, nomeadamente o número
* máximo de operações, o tamanho dos buffers de memória partilhada
* usados para comunicação, e o número de clientes, de proxies e de
* servidores. Guarda esta informação nos campos apropriados da
* estrutura main_data.
*/
void main_args(int argc, char *argv[], struct main_data *data) {
	data->max_ops = atoi(argv[1]);
	data->buffers_size = atoi(argv[2]);
	data->n_clients = atoi(argv[3]);
	data->n_proxies = atoi(argv[4]);
	data->n_servers = atoi(argv[5]);
}

/* Função que reserva a memória dinâmica necessária para a execução
* do sovaccines, nomeadamente para os arrays *_pids e *_stats da estrutura 
* main_data. Para tal, pode ser usada a função create_dynamic_memory do memory.h.
*/
void create_dynamic_memory_buffers(struct main_data *data) {
	data->client_pids = create_dynamic_memory(data->n_clients * sizeof(int));
	data->proxy_pids = create_dynamic_memory(data->n_proxies * sizeof(int));
	data->server_pids = create_dynamic_memory(data->n_servers * sizeof(int));

	data->client_stats = create_dynamic_memory(data->n_clients * sizeof(int));
	data->proxy_stats = create_dynamic_memory(data->n_proxies * sizeof(int));
	data->server_stats = create_dynamic_memory(data->n_servers * sizeof(int));
}

/* Função que reserva a memória partilhada necessária para a execução do
* sovaccines. É necessário reservar memória partilhada para todos os buffers da
* estrutura communication_buffers, incluindo os buffers em si e respetivos
* pointers, assim como para o array data->results e variável data->terminate.
* Para tal, pode ser usada a função create_shared_memory do memory.h.
*/
void create_shared_memory_buffers(struct main_data *data, struct communication_buffers *buffers) {
	buffers->main_cli->ptr = create_shared_memory(STR_SHM_MAIN_CLI_PTR, data->buffers_size * sizeof(int));
	buffers->main_cli->ops = create_shared_memory(STR_SHM_MAIN_CLI_BUFFER, data->buffers_size * sizeof(struct operation));

	buffers->cli_prx->ptr = create_shared_memory(STR_SHM_CLI_PRX_PTR, 2 * sizeof(int));
	buffers->cli_prx->ops = create_shared_memory(STR_SHM_CLI_PRX_BUFFER, data->buffers_size * sizeof(struct operation));

	buffers->prx_srv->ptr = create_shared_memory(STR_SHM_PRX_SRV_PTR, data->buffers_size * sizeof(int));
	buffers->prx_srv->ops = create_shared_memory(STR_SHM_PRX_SRV_BUFFER, data->buffers_size * sizeof(struct operation));

	buffers->srv_cli->ptr = create_shared_memory(STR_SHM_SRV_CLI_PTR, 2 * sizeof(int));
	buffers->srv_cli->ops = create_shared_memory(STR_SHM_SRV_CLI_BUFFER, data->buffers_size * sizeof(struct operation));

	data->results = create_shared_memory(STR_SHM_RESULTS, data->max_ops * sizeof(struct operation));;

	data->terminate = create_shared_memory(STR_SHM_TERMINATE, sizeof(int));
}

/* Função que inicializa os semáforos da estrutura semaphores. Semáforos
* *_full devem ser inicializados com valor 0, semáforos *_empty com valor
* igual ao tamanho dos buffers de memória partilhada, e os *_mutex com valor
* igual a 1. Para tal pode ser usada a função semaphore_create.
*/
void create_semaphores(struct main_data *data, struct semaphores *sems) {
	sems->main_cli->full = semaphore_create(STR_SEM_MAIN_CLI_FULL, 0);
	sems->main_cli->empty = semaphore_create(STR_SEM_MAIN_CLI_EMPTY, data->buffers_size);
	sems->main_cli->mutex = semaphore_create(STR_SEM_MAIN_CLI_MUTEX, 1);

	sems->cli_prx->full = semaphore_create(STR_SEM_CLI_PRX_FULL, 0);
	sems->cli_prx->empty = semaphore_create(STR_SEM_CLI_PRX_EMPTY, data->buffers_size);
	sems->cli_prx->mutex = semaphore_create(STR_SEM_CLI_PRX_MUTEX, 1);

	sems->prx_srv->full = semaphore_create(STR_SEM_PRX_SRV_FULL, 0);
	sems->prx_srv->empty = semaphore_create(STR_SEM_PRX_SRV_EMPTY, data->buffers_size);
	sems->prx_srv->mutex = semaphore_create(STR_SEM_PRX_SRV_MUTEX, 1);

	sems->srv_cli->full = semaphore_create(STR_SEM_SRV_CLI_FULL, 0);
	sems->srv_cli->empty = semaphore_create(STR_SEM_SRV_CLI_EMPTY, data->buffers_size);
	sems->srv_cli->mutex = semaphore_create(STR_SEM_SRV_CLI_MUTEX, 1);

	sems->results_mutex = semaphore_create(STR_SEM_RESULTS_MUTEX, 1);
}

/* Função que inicia os processos dos clientes, proxies e
* servidores. Para tal, pode usar a função launch_process,
* guardando os pids resultantes nos arrays respetivos
* da estrutura data.
*/
void launch_processes(struct communication_buffers *buffers, struct main_data *data, struct semaphores *sems) {
	for(int i = 0; i < data->n_clients; i++) {
		data->client_pids[i] = launch_process(i, 0, buffers, data, sems);
	}

	for(int i = 0; i < data->n_proxies; i++) {
		data->proxy_pids[i] = launch_process(i, 1, buffers, data, sems);
	}

	for(int i = 0; i < data->n_servers; i++) {
		data->server_pids[i] = launch_process(i, 2, buffers, data, sems);
	}
}

/* Função que faz interação do utilizador com o sistema, podendo receber 4 comandos:
* op - cria uma nova operação, através da função create_request
* read - verifica o estado de uma operação através da função read_answer
* stop - termina o execução do sovaccines através da função stop_execution
* help - imprime informação sobre os comandos disponiveis
*/
void user_interaction(struct communication_buffers *buffers, struct main_data *data, struct semaphores *sems) {
	int* op_counter = create_dynamic_memory(sizeof(int));

	while(1) {
		printf("\nMENU:\n");
		printf("-> op\n");
		printf("-> read\n");
		printf("-> stop\n");
		printf("-> help\n");
		printf("\nEscreva a opção: ");

		char command[4 + 1];
		scanf("%s", command);

		if(!strcmp(command, "op")) {
			create_request(op_counter, buffers, data, sems);
		}
		else if(!strcmp(command, "read")) {
			read_answer(data, sems);
		}
		else if(!strcmp(command, "stop")) {
			destroy_dynamic_memory(op_counter);
			stop_execution(data, buffers, sems);
		}
		else if(!strcmp(command, "help")) {
			printf("op – criar um pedido de aquisição de vacinas\n");
			printf("read – consultar o estado de um dado pedido (especificado pelo utilizador)\n");
			printf("stop – terminar a execução do sistema SOVACCINES\n");
		}
		else {
			printf("\"%s\" nao é uma operação permitida!\n", command);
		}

		printf("\n");
	}
}

/* Se o limite de operações ainda não tiver sido atingido, cria uma nova
* operação identificada pelo valor atual de op_counter, escrevendo a mesma
* no buffer de memória partilhada entre main e clientes e efetuando a 
* necessária sincronização antes e depois de escrever. Imprime o id da
* operação e incrementa o contador de operações op_counter.
*/
void create_request(int *op_counter, struct communication_buffers *buffers, struct main_data *data, struct semaphores *sems) {
	if(*op_counter < data->max_ops) {
		struct operation* op = create_dynamic_memory(sizeof(struct operation));
		op->id = *op_counter;

		produce_begin(sems->main_cli);
		write_rnd_access_buffer(buffers->main_cli, data->buffers_size, op);
		produce_end(sems->main_cli);

		printf("OP->ID = %d\n", op->id);

		*op_counter = *op_counter + 1;  

		destroy_dynamic_memory(op);
	}
}

/* Função que lê um id de operação do utilizador e verifica se a mesma
* é valida e se já foi respondida por um servidor. Em caso afirmativo,
* imprime informação da mesma, nomeadamente o seu estado, e os ids do 
* cliente, proxy e servidor que a processaram. O acesso à estrutura 
* data->results deve ser sincronizado com as funções e semáforos
* respetivos.
*/
void read_answer(struct main_data *data, struct semaphores *sems) {
	int flag;

	int opId;
	printf("Indique o id da operação a ler: ");
	scanf("%d", &opId);

	struct operation* op = create_dynamic_memory(sizeof(struct operation));
	op->id = opId;

	for(int i = 0; i < data->max_ops; i++) {
		struct operation* opResults = create_dynamic_memory(sizeof(struct operation));

		semaphore_mutex_lock(sems->results_mutex);
		*opResults = data->results[i];
		semaphore_mutex_unlock(sems->results_mutex);

		if((op->id == opResults->id) && (opResults->status == 'S')) {
			printf("OP->ID = %d\n", opResults->id);
			printf("OP->STATUS = %c\n", opResults->status);
			printf("OP->CLIENT = %d\n", opResults->client);
			printf("OP->PROXY = %d\n", opResults->proxy);
			printf("OP->SERVER = %d\n", opResults->server);

			flag = 1;
		}

		destroy_dynamic_memory(opResults);
	}

	destroy_dynamic_memory(op);

	if(!flag) {
		printf("A operação indicado não é válida ou ainda não está disponível\n");
	}
}

/* Função que termina a execução do programa sovaccines. Deve começar por 
* afetar a flag data->terminate com o valor 1. De seguida, e por esta
* ordem, deve acordar processos adormecidos, esperar que terminem a sua 
* execução, escrever as estatisticas finais do programa, e por fim libertar
* os semáforos e zonas de memória partilhada e dinâmica previamente 
*reservadas. Para tal, pode usar as outras funções auxiliares do main.h.
*/
void stop_execution(struct main_data *data, struct communication_buffers *buffers, struct semaphores *sems) {
	*(data->terminate) = 1;
	wakeup_processes(data, sems);
	wait_processes(data);
	write_statistics(data);
	destroy_semaphores(sems);
	destroy_shared_memory_buffers(data, buffers);
	destroy_dynamic_memory_buffers(data);
	exit(0);
}

/* Função que acorda todos os processos adormecidos em semáforos, para que
* estes percebam que foi dada ordem de terminação do programa. Para tal,
* pode ser usada a função produce_end sobre todos os conjuntos de semáforos
* onde possam estar processos adormecidos e um número de vezes igual ao 
* máximo de processos que possam lá estar.
*/
void wakeup_processes(struct main_data *data, struct semaphores *sems) {
	for(int i = 0; i < data->max_ops; i++) {
		produce_end(sems->main_cli);
		produce_end(sems->cli_prx);
		produce_end(sems->prx_srv);
		produce_end(sems->srv_cli);
	}
}

/* Função que espera que todos os processos previamente iniciados terminem,
* incluindo clientes, proxies e servidores. Para tal, pode usar a função 
* wait_process do process.h.
*/
void wait_processes(struct main_data *data) {
	for(int i = 0; i < data->n_clients; i++) {
		data->client_stats[i] = wait_process(data->client_pids[i]);
	}

	for(int i = 0; i < data->n_proxies; i++) {
		data->proxy_stats[i] = wait_process(data->proxy_pids[i]);
	}

	for(int i = 0; i < data->n_servers; i++) {
		data->server_stats[i] = wait_process(data->server_pids[i]);
	}
}

/* Função que imprime as estatisticas finais do sovaccines, nomeadamente quantas
* operações foram processadas por cada cliente, proxy e servidor.
*/
void write_statistics(struct main_data *data) {
	for(int i = 0; i < data->n_clients; i++) {
		printf("O cliente %d processou %d operações.\n", i, data->client_stats[i]);
	}

	for(int i = 0; i < data->n_proxies; i++) {
		printf("O proxy %d processou %d operações.\n", i, data->proxy_stats[i]);
	}

	for(int i = 0; i < data->n_servers; i++) {
		printf("O server %d processou %d operações.\n", i, data->server_stats[i]);
	}
}

/* Função que liberta todos os buffers de memória dinâmica previamente
* reservados na estrutura data.
*/
void destroy_dynamic_memory_buffers(struct main_data *data) {
	destroy_dynamic_memory(data->client_pids);
	destroy_dynamic_memory(data->proxy_pids);
	destroy_dynamic_memory(data->server_pids);

	destroy_dynamic_memory(data->client_stats);
	destroy_dynamic_memory(data->proxy_stats);
	destroy_dynamic_memory(data->server_stats);
}

/* Função que liberta todos os buffers de memória partilhada previamente
* reservados nas estruturas data e buffers.
*/
void destroy_shared_memory_buffers(struct main_data *data, struct communication_buffers *buffers) {
	destroy_shared_memory(STR_SHM_MAIN_CLI_PTR, buffers->main_cli->ptr, data->buffers_size * sizeof(int));
	destroy_shared_memory(STR_SHM_MAIN_CLI_BUFFER, buffers->main_cli->ops, data->buffers_size * sizeof(struct operation));

	destroy_shared_memory(STR_SHM_CLI_PRX_PTR, buffers->cli_prx->ptr, 2 * sizeof(int));
	destroy_shared_memory(STR_SHM_CLI_PRX_BUFFER, buffers->cli_prx->ops, data->buffers_size * sizeof(struct operation));

	destroy_shared_memory(STR_SHM_PRX_SRV_PTR, buffers->prx_srv->ptr, data->buffers_size * sizeof(int));
	destroy_shared_memory(STR_SHM_PRX_SRV_BUFFER, buffers->prx_srv->ops, data->buffers_size * sizeof(struct operation));

	destroy_shared_memory(STR_SHM_SRV_CLI_PTR, buffers->srv_cli->ptr, 2 * sizeof(int));
	destroy_shared_memory(STR_SHM_SRV_CLI_BUFFER, buffers->srv_cli->ops, data->buffers_size * sizeof(struct operation));

	destroy_shared_memory(STR_SHM_RESULTS, data->results, data->max_ops * sizeof(struct operation));

	destroy_shared_memory(STR_SHM_TERMINATE, data->terminate, sizeof(int));
}

/* Função que liberta todos os semáforos da estrutura semaphores.
*/
void destroy_semaphores(struct semaphores *sems) {
	semaphore_destroy(STR_SEM_MAIN_CLI_FULL, sems->main_cli->full);
	semaphore_destroy(STR_SEM_MAIN_CLI_EMPTY, sems->main_cli->empty);
	semaphore_destroy(STR_SEM_MAIN_CLI_MUTEX, sems->main_cli->mutex);

	semaphore_destroy(STR_SEM_CLI_PRX_FULL, sems->cli_prx->full);
	semaphore_destroy(STR_SEM_CLI_PRX_EMPTY, sems->cli_prx->empty);
	semaphore_destroy(STR_SEM_CLI_PRX_MUTEX, sems->cli_prx->mutex);

	semaphore_destroy(STR_SEM_PRX_SRV_FULL, sems->prx_srv->full);
	semaphore_destroy(STR_SEM_PRX_SRV_EMPTY, sems->prx_srv->empty);
	semaphore_destroy(STR_SEM_PRX_SRV_MUTEX, sems->prx_srv->mutex);

	semaphore_destroy(STR_SEM_SRV_CLI_FULL, sems->srv_cli->full);
	semaphore_destroy(STR_SEM_SRV_CLI_EMPTY, sems->srv_cli->empty);
	semaphore_destroy(STR_SEM_SRV_CLI_MUTEX, sems->srv_cli->mutex);

	semaphore_destroy(STR_SEM_RESULTS_MUTEX, sems->results_mutex);
}