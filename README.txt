Este projeto foi desenvolvido pelos alunos:
	* Rodrigo Pereira Branco - FC54457
	* Vasco de Oliveira Lopes - FC54410
do grupo SO-007.

Ao executar o programa usando o valgrind para detetar perdas de memória numa 
máquina virtual com a imagem disponibilizada pela FCUL, o programa termina com 
13 erros em 13 contextos. No entanto, usando uma máquina virtual com a imagem 
oficial Ubuntu 20.04.2 LTS, o programa termina sem erros. Nós não conseguimos 
perceber o porquê de isto acontecer.

No HEAP SUMMARY do valgrind, independentemente da imagem usada, o campo 
"in use at exit" tem sempre 320 bytes em 11 blocos.