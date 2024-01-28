CC=gcc
CFLAGS=-Wall -Wextra

all:main

stack.o: stack.h
main.o: stack.h
lexer.o: lexer.h token.h
token.o: token.h
parser.o: parser.h ast.h stack.h lexer.h
ast.o: ast.h token.h
main: main.o stack.o lexer.o token.o parser.o ast.o

	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o
	rm main