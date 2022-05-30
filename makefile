
main : 
	make server
	make client

server : 
	gcc -o server/server server/server.c server/funcServ.c server/threadServ.c -lpthread

client :
	gcc -o client/client client/client.c client/funcCli.c client/threadCli.c -lpthread
