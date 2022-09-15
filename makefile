CC = gcc
CFLAGS = -Wall -ansi -pedantic
OBJ = simulator.o scheduler.o
EXEC = simulator

$(EXEC) : $(OBJ)
	$(CC) -pthread $(OBJ) -o $(EXEC)

simulator.o : simulator.c scheduler.h
	$(CC) -c simulator.c $(CFLAGS)

scheduler.o: scheduler.c scheduler.h
	$(CC) -c scheduler.c $(CFLAGS)

clean :
	rm -f $(EXEC) $(OBJ)