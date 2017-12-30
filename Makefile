all: test-compile.app test-dynfunc.app test-audio.app test-renderpipeline.app

OBJECTS=process.o compile.o dynamic.o ringbuffer.o pa_ringbuffer.o

%.o: %.c
	clang -o $@ $^ -c

%.app: %.c $(OBJECTS)
	clang -o $@ $^

GL_OBJECTS=gl.o shader.o quad.o framebuffer.o
GL_FLAGS=`pkg-config --libs SDL2 GLEW` -framework OpenGL
AUDIO_FLAGS=`pkg-config --libs jack`

test-audio.app: test-audio.c $(OBJECTS) $(GL_OBJECTS)
	clang -o $@ $^ $(GL_FLAGS) $(AUDIO_FLAGS)

test-renderpipeline.app: test-renderpipeline.c $(OBJECTS) $(GL_OBJECTS)
	clang -o $@ $^ $(GL_FLAGS)
