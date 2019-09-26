OBJS=server.c
OBJS_CLIENT=client.c

all: $(OBJS) $(OBJS_CLIENT)
	rm -f server
	rm -f client
	gcc -pthread $(OBJS) -o server
	gcc -pthread $(OBJS_CLIENT) -o client
	rm -f *.o

clean:
	rm -f *.op
	rm -f server
	rm -f client