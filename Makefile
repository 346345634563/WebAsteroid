# Compiler
CC := emcc
CFLAGS := -Wall -Wextra -O3 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=2  -s SAFE_HEAP=1 -s TOTAL_STACK=16MB -s USE_WEBGL2=1 -Iinclude -gsource-map

# Directories
SRC_DIR := source
BUILD_DIR := docs
INC_DIR := include

# Source Files
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Executable Name
EXEC := $(BUILD_DIR)/game_page/asteroid.html



# Deploy command
DEPLOY := emrun --no_browser --port 8000 $(BUILD_DIR)
DEPLOY_TEST := emrun --no_browser --port 8000 $(BUILD_DIR)/game_page/
CLEAN := rm -rf build/game_page/*

.PHONY: all compile deploy clean

all: clean compile deploy 

compile: $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXEC) --preload-file misc
	mv $(BUILD_DIR)/game_page/asteroid.data $(BUILD_DIR)/ 


deploy: 
	$(DEPLOY)

deployTest:
	$(DEPLOY_TEST)

clean:
	$(CLEAN)
