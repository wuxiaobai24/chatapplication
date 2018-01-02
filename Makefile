Make:
	gcc -o client client.c config.c message.c -g
	gcc -o chatserver chatserver.c userlist.c userfile.c config.c message.c -g -pthread

cleanfifo:
	rm ../wukunhan2015170*
