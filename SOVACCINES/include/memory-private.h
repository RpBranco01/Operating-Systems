/* Grupo SO-007
Rodrigo Branco - FC54457
Vasco Lopes - FC54410 */

#ifndef MEMORY_PRIVATE_H_GUARD
#define MEMORY_PRIVATE_H_GUARD

#include "memory.h"


//estrutura que representa um buffer circular, completar pelos alunos
struct circular_buffer { 	
    int *ptr;   // ptr[0]->in   ptr[1]->out
    struct operation* ops;
};

//estrutura que representa um buffer de acesso aleatório, completar pelos alunos
struct rnd_access_buffer { 		
    int* ptr;
    struct operation* ops;
};

#endif
