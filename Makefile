CC=gcc
HD= -Iinclude
SC=-c $<
OBJ=-o $@
THREAD=-pthread

application:bin/wukunhan2015170297Server bin/client
	
bin/wukunhan2015170297Server:obj/chatserver.o obj/userlist.o obj/config.o obj/message.o obj/userfile.o
	$(CC) $(OBJ) $^ $(THREAD)

bin/client:obj/client.o obj/config.o obj/message.o
	$(CC) $(OBJ) $^

obj/chatserver.o: code/chatserver.c
	$(CC) $(HD) $(SC) $(OBJ)

obj/client.o: code/client.c
	$(CC) $(HD) $(SC) $(OBJ)

obj/message.o: code/message.c
	$(CC) $(HD) $(SC) $(OBJ)

obj/userlist.o: code/userlist.c
	$(CC) $(HD) $(SC) $(OBJ)

obj/userfile.o: code/userfile.c
	$(CC) $(HD) $(SC) $(OBJ)

obj/config.o: code/config.c
	$(CC) $(HD) $(SC) $(OBJ)

Make:
	gcc -o client client.c config.c message.c -g
	gcc -o chatserver chatserver.c userlist.c userfile.c config.c message.c -g -pthread

cleanfifo:
	rm ../wukunhan2015170*
	rm ../data/*t
cleanuser:
	rm ../data/*c
	rm ../data/userfile

clean:
	rm ./bin/*
	rm ./obj/*
