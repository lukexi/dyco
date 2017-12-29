all: test-compile.app test-dynfunc.app test-audio.app

OBJECTS=process.o compile.o dynamic.o ringbuffer.o pa_ringbuffer.o

%.o: %.c
	clang -o $@ $^ -c

%.app: %.c $(OBJECTS)
	clang -o $@ $^

test-audio.app: test-audio.c $(OBJECTS) shader.o quad.o gl.o
	clang -o $@ $^ `pkg-config --libs --cflags jack SDL2 GLEW` -framework OpenGL
