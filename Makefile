VULKAN_INC = $$HOME/VulkanSDK/1.3.268.1/macOS/include
VULKAN_LNK = $$HOME/VulkanSDK/1.3.268.1/macOS/lib

CC = clang
GLSLC = glslc

CFLAGS += -g3                                       \
          `pkg-config --libs --cflags glfw3 vulkan` \
          -I $(VULKAN_INC)                          \
          -L $(VULKAN_LNK)                          \
          -lvulkan

bin/main: src/main.c bin/vert.spv bin/frag.spv
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.c -o bin/main

bin/vert.spv: src/shaders/shader.vert
	mkdir -p bin
	$(GLSLC) -o $@ $^

bin/frag.spv: src/shaders/shader.frag
	mkdir -p bin
	$(GLSLC) -o $@ $^

.PHONY: clean
clean:
	rm -rf bin
