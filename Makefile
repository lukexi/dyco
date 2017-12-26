all: main.app main2.app main3.app main4.app main5.app

main.app: main.c
	clang -o $@ $^

main2.app: main2.c
	clang -o $@ $^

main3.app: main3.c
	clang -o $@ $^

main4.app: main4.c
	clang -o $@ $^

main5.app: main5.c
	clang -o $@ $^
