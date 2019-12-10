all: client server gserver gclient

client: client_ch.c
	gcc -pthread client_ch.c -o client_ch

server: server_ch.c
	gcc -pthread server_ch.c -o server_ch

gserver: game_server.c
	gcc -pthread game_server.c -o game_server

gclient: game_client.c
	gcc -pthread game_client.c -o game_client

clean:
	rm -rf client_ch server_ch game_server game_client
