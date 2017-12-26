all: main.app main2.app main3.app

main.app: main.c
	clang -o $@ $^

main2.app: main2.c
	clang -o $@ $^

main3.app: main3.c
	clang -o $@ $^
