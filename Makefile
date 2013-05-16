UNAME := $(shell uname)
sources := $(wildcard *.c)
objects := $(sources:%.c=%.o)

CFLAGS = -Wall
ifeq ($(UNAME), Darwin)
CFLAGS += -m32
endif
CFLAGS += -I. -I/usr/X11/include -I/usr/local/include
CFLAGS += -g -fno-omit-frame-pointer -I.

LDFLAGS = -L/usr/local/lib -L/usr/X11/lib
ifeq ($(UNAME), Linux)
LDFLAGS += -lm -lglut -lGLU -lGL
endif
ifeq ($(UNAME), Darwin)
LDFLAGS += -framework OpenGL -framework GLUT
endif

all: objview

objview: $(objects)
	gcc $(CFLAGS) -o objview $(objects) $(LDFLAGS)

%.o : %.c $(wildcard *.h)
	gcc $(CFLAGS) -c $<

clean:
	rm -f objview
	rm -f *.o*
