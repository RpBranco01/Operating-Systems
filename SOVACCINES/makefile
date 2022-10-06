OBJ_dir = obj
OBJETOS = client.o main.o memory.o process.o proxy.o server.o synchronization.o
client.o = memory.h main.h
main.o = memory.h synchronization.h
memory.o = memory-private.h
process.o = memory.h main.h
proxy.o = memory.h main.h
server.o = memory.h main.h
synchronization.o = 
CC = gcc

vpath %.o $(OBJ_dir)

sovaccines: $(OBJETOS)
		$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS)) -o bin/sovaccines -lrt -pthread -g

%.o: src/%.c $($@)
		$(CC) -I include -o $(OBJ_dir)/$@ -c $<