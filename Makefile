OBJS=server.c tpool.c
OBJS_CLIENT=client.c

all: $(OBJS) $(OBJS_CLIENT)
	rm -f server
	rm -f client
	gcc -g -pthread $(OBJS) -lpthread -o server
	gcc -g -pthread $(OBJS_CLIENT) -lpthread -o client
	rm -f *.o

clean:
	rm -f *.op
	rm -f server
	rm -f client