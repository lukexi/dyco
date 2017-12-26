all: test-compile.app

OBJECTS=process.o compile.o dynamic.o

%.o: %.c
	clang -o $@ $^ -c

%.app: %.c $(OBJECTS)
	clang -o $@ $^
