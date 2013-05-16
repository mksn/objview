UNAME := $(shell uname)

CFLAGS := -Wall -g

ifeq ($(UNAME), Linux)
LDFLAGS := -lglut -lGLU -lGL -lm
endif

ifeq ($(UNAME), Darwin)
LDFLAGS := -framework GLUT -framework OpenGL
endif

all: objview

headers := $(wildcard *.h)
sources := $(wildcard *.c)
objects := $(sources:%.c=%.o)

$(objects): $(headers)

objview: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f objview *.o
