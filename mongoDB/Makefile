all:	client server
client: client.cpp
		g++ client.cpp -g -o client -libverbs -lmlx4 -pthread
		g++ client.cpp -g -DDEBUG -o client_d -libverbs -lmlx4 -pthread
server: server.cpp
		g++ server.cpp -g -o server -libverbs -lmlx4 -pthread
		g++ server.cpp -g -DDEBUG -o server_d -libverbs -lmlx4 -pthread
