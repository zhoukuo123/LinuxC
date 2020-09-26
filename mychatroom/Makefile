client:src/client.c src/md5.c src/menu.c src/wrap.c
	gcc src/client.c src/md5.c src/menu.c src/wrap.c -Iinclude -lpthread -lmysqlclient -o client

server:src/server.c src/md5.c src/menu.c src/wrap.c
	gcc src/server.c src/md5.c src/menu.c src/wrap.c -Iinclude -lpthread -lmysqlclient -o server