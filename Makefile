# Definir el compilador
CXX = g++

# Definir las banderas del compilador
#CXXFLAGS = -Wall -std=c++17
CXXFLAGS = -g


# Definir los directorios de los includes y las librerías
#INCLUDES = -I C:/msys64/mingw64/include/allegro5
LIBS = -L C:/msys64/mingw64/bin 
#-lallegro-5.2 -lallegro_main-5.2 -lallegro_primitives-5.2 -lallegro_image-5.2 -lallegro_font-5.2 -lallegro_ttf-5.2 -lallegro_audio-5.2 -lallegro_acodec-5.2

# Definir los archivos fuente y el ejecutable
SRCS = main.cpp lexico.cpp sintaxis.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = compilador.exe

# Regla para compilar el ejecutable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Regla para compilar los archivos objeto
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Regla para limpiar los archivos compilados
clean:
	rm -f $(OBJS) $(EXEC)



