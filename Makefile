all: file-app.app dynfunc-app.app audio-app.app renderpipeline-app.app globalload-app.app

OBJECTS=process.o compile.o dynamic.o ringbuffer.o pa_ringbuffer.o utils.o

%.o: %.c
	clang -o $@ $^ -c

%.app: %.c $(OBJECTS)
	clang -o $@ $^

GL_OBJECTS=gl.o shader.o quad.o framebuffer.o
GL_FLAGS=`pkg-config --libs SDL2 GLEW` -framework OpenGL
AUDIO_FLAGS=`pkg-config --libs jack`

audio-app.app: audio-app.c $(OBJECTS) $(GL_OBJECTS) audio-jack.o
	clang -o $@ $^ $(GL_FLAGS) $(AUDIO_FLAGS)

renderpipeline-app.app: renderpipeline-app.c $(OBJECTS) $(GL_OBJECTS)
	clang -o $@ $^ $(GL_FLAGS)

globalload-app.app: globalload-app.c $(OBJECTS)
	clang -o $@ $^ -undefined dynamic_lookup
