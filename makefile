PROGRAM=mirrorstage

# Target principal, el que se llama cuando no explicitas ninguno
all: $(PROGRAM)

# Definición de variables

SOURCES = $(shell find . -name "*.cpp")

OBJS = $(SOURCES:.cpp=.o)

MUST_CFLAGS = -I. `pkg-config lua5.1 gl sdl --cflags`
CFLAGS = -O2 -g -Wall

LDFLAGS=
LIBS=`pkg-config lua5.1 gl sdl --libs` -lboost_serialization-mt -lboost_system-mt -lboost_filesystem-mt

# Compila el programa principal
$(PROGRAM): $(OBJS)
	g++ $(LDFLAGS) $+ -o $@ $(LIBS)

# esto compila cada uno de los .o a partir de cada .cpp
# untarget es del estilo quesecompila: dependencia1 dependencia2 dependencian
# $@ es quesecompila $< es dependencia1 $+ es dependencia1 dependencia2 dependencian
%.o: %.cpp
	g++ -o $@ -c $+ $(CFLAGS) $(MUST_CFLAGS)

# Compila individualmente cada uno de los ficheros .o que derivan de ficheros .c
%.o: %.c
	gcc -o $@ -c $+ $(CFLAGS) $(MUST_CFLAGS)

# Limpia todos los resultados de una compilación
clean:
	rm -f $(OBJS)
	rm -f $(PROGRAM)
	rm -f *.o *.a *~
