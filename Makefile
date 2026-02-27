OBJS = achat-xor.c achat-debug.c achat-opus.c achat-termios.c achat.c

all:
	@gcc -std=c99 $(OBJS) -o achat -lopus -lasound -lpthread
	@strip -s achat

clean:
	@rm -rf achat *.o
