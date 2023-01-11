all: clean client server

client:
	gcc src/client/*.c src/common/*.c -o client -I./includes

server:
	gcc src/server/*.c src/common/*.c -o server -I./includes

clean:
	rm -f client server
