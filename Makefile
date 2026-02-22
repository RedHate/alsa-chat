all:
	gcc -std=c99 -Wall -pedantic alsa-chat.c -o achat -lasound -lpthread  
	strip -s achat

clean:
	rm -rf achat
