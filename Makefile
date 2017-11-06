Make:
	gcc -o client client.c -g
	gcc -o chatserver chatserver.c -g

cleanfifo:
	rm ../data/client_fifo/*
	rm ../data/server_fifo/*
