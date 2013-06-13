UNAME := $(shell uname)

CFLAGS := -Wall -g -I/usr/local/include

LDFLAGS := -L/usr/local/lib

ifeq ($(UNAME), Linux)
LDFLAGS += -lglut -lGLU -lGL -lm
endif

ifeq ($(UNAME), Darwin)
LDFLAGS += -framework GLUT -framework OpenGL
endif

LDFLAGS += -llua
all: objview

headers := $(wildcard *.h)
sources := $(wildcard *.c)
objects := $(sources:%.c=%.o)

$(objects): $(headers)

objview: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f objview *.o
