UNAME = $(shell uname)
objects =	main.o \
			model-iqm.o\
			shader.o \
			image.o \
			vector.o

ifeq ($(UNAME), Linux)
objects += strlcpy.o
endif

sources =	main.c \
			model-iqm.c\
			image.c \
			shader.c \
			strlcpy.c \
			vector.c

ifeq ($(UNAME), Linux)
sources += strlcpy.c
endif

CFLAGS =
ifeq ($(UNAME), Darwin)
CFLAGS += -m32 -Wall
endif
CFLAGS += -I. -I/usr/X11/include -I/usr/local/include
CFLAGS += -ggdb -fno-omit-frame-pointer -I.

LDFLAGS = -L/usr/local/lib -L/usr/X11/lib
ifeq ($(UNAME), Linux) 
LDFLAGS += -ljpeg -lm -lglut -lGLEW -lGLU -lGL -lc
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
