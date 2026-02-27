all:
	@gcc -std=c99 -Wall -pedantic  achat.c -o achat -lopus -lasound -lpthread
	@strip -s achat

clean:
	@rm -rf achat *.o
