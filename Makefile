CC=gcc
CFLAGS=-Wall -Wextra -Werror -g

all:clean bee lines

debug_run: clean main lines 
	gdb --args ./main test.b

stack.o: stack.h
main.o: stack.h
lexer.o: lexer.h token.h error.h type.h
token.o: token.h type.h
parser.o: parser.h ast.h stack.h lexer.h type.h
parser_tok.o: parser.h ast.h stack.h lexer.h
ast.o: ast.h token.h common.h stack.h
common.o: common.h
error.o: error.h
generator.o: generator.h ast.h parser_tok.h
type.o: type.h
fundef_table.o:	fundef_table.h ast.h type.h

bee: main.o stack.o lexer.o token.o parser.o ast.o error.o common.o parser_tok.o generator.o type.o fundef_table.o


	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o
	rm -f main

lines:
	wc -l $$( find -name '*.[hcb]') | tail -n 1

run:
	./bee test.b