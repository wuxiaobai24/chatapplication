Make:
#	gcc -o client client.c -g
	gcc -o chatserver chatserver.c userlist.c userfile.c config.c -g -pthread

cleanfifo:
	rm ../data/client_fifo/*
	rm ../data/server_fifo/*
