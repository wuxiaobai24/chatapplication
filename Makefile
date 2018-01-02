Make:
	gcc -o client client.c config.c message.c -g
	gcc -o chatserver chatserver.c userlist.c userfile.c config.c message.c -g -pthread

cleanfifo:
	rm ../wukunhan2015170*
	rm ../data/*t
cleanuser:
	rm ../data/*c
	rm ../data/userfile
