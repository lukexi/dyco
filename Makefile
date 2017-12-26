all: main.app \
	main2.app \
	main3.app \
	main4.app \
	main5.app \
	main6.app

OBJECTS=process.o compile.o

%.o: %.c
	clang -o $@ $^ -c

%.app: %.c $(OBJECTS)
	clang -o $@ $^
