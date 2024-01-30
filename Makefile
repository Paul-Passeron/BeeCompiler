CC=gcc
CFLAGS=-Wall -Wextra -g

all:clean main

stack.o: stack.h
main.o: stack.h
lexer.o: lexer.h token.h error.h
token.o: token.h
parser.o: parser.h ast.h stack.h lexer.h
ast.o: ast.h token.h common.h stack.h
common.o: common.h
error.o: error.h
main: main.o stack.o lexer.o token.o parser.o ast.o error.o common.o

	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o
	rm -f main