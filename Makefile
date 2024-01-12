CC = clang
GLSLC = glslc

LIBS = glfw3 vulkan

CFLAGS += -g3                                       \
          `pkg-config --libs --cflags $(LIBS)`

MAIN_OBJ = src/main.o   \
           src/lexer.o  \
           src/shared.o \

GRAPHICS_OBJ = src/graphics.o \
               src/shared.o   \

bin/main: $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

bin/graphics: $(GRAPHICS_OBJ) bin/vert.spv bin/frag.spv
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(GRAPHICS_OBJ)

bin/vert.spv: src/shaders/shader.vert
	mkdir -p bin
	$(GLSLC) -o $@ $^

bin/frag.spv: src/shaders/shader.frag
	mkdir -p bin
	$(GLSLC) -o $@ $^

.PHONY: clean
clean:
	rm -rf bin
	rm -f $(MAIN_OBJ) $(GRAPHICS_OBJ)
