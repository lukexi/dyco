APP_SOURCES = $(wildcard *-app.c)
APPS = $(APP_SOURCES:%-app.c=%.app)

all: $(APPS)

OBJECTS=process.o compile.o dynamic.o ringbuffer.o pa_ringbuffer.o utils.o

%.o: %.c
	clang -o $@ $^ -c

%.app: %-app.c $(OBJECTS)
	clang -o $@ $^

GL_OBJECTS=gl.o shader.o quad.o framebuffer.o texture.o
GL_FLAGS=`pkg-config --libs SDL2 GLEW` -framework OpenGL
AUDIO_FLAGS=`pkg-config --libs jack`

audio-dyn.app: audio-dyn-app.c $(OBJECTS) $(GL_OBJECTS) audio-jack.o
	clang -o $@ $^ $(GL_FLAGS) $(AUDIO_FLAGS)

audio-fm.app: audio-fm-app.c $(OBJECTS) $(GL_OBJECTS) audio-jack.o
	clang -o $@ $^ $(GL_FLAGS) $(AUDIO_FLAGS)

audio.app: audio-app.c $(OBJECTS) $(GL_OBJECTS) audio-jack.o
	clang -o $@ $^ $(GL_FLAGS) $(AUDIO_FLAGS)

audio2.app: audio2-app.c $(OBJECTS) audio-jack.o
	clang -o $@ $^ $(AUDIO_FLAGS)

cv.app: cv-app.c $(OBJECTS) $(GL_OBJECTS)
	clang -o $@ $^ $(GL_FLAGS)

renderpipeline.app: renderpipeline-app.c $(OBJECTS) $(GL_OBJECTS)
	clang -o $@ $^ $(GL_FLAGS)

reaction.app: reaction-app.c $(OBJECTS) $(GL_OBJECTS)
	clang -o $@ $^ $(GL_FLAGS)

globalload.app: globalload-app.c $(OBJECTS)
	clang -o $@ $^ -undefined dynamic_lookup

clean:
	rm -f *.app *.so *.dSYM *.o
