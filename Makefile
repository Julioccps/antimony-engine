# Compiler: gcc is default, can be overridden (e.g., make CC=clang)
CC ?= gcc

# Directories
SRCDIR := src
INCDIR := include
BUILDDIR := build
SANDBOXDIR := sandbox

# Source files
SRC_ENG := $(wildcard $(SRCDIR)/*.c)
SRC_BOX := $(wildcard $(SANDBOXDIR)/*.c)

# Object files
OBJ_ENG := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRC_ENG))
OBJ_BOX := $(patsubst $(SANDBOXDIR)/%.c, $(BUILDDIR)/%.o, $(SRC_BOX))

# Targets
LIB_OUT := $(BUILDDIR)/libsb.so
BIN_BOX := $(BUILDDIR)/sandbox

# Flags
CFLAGS := -Wall -Wextra -O2 -g -I$(INCDIR) -fPIC
LDFLAGS_LIB := -shared
LDFLAGS_BIN := -L$(BUILDDIR) -lsb -Wl,-rpath='$$ORIGIN'
LDLIBS := -lvulkan -lglfw -lm

# Phony targets
.PHONY: all clean run dirs

# Default target
all: dirs $(LIB_OUT) $(BIN_BOX)

# Link the shared library
$(LIB_OUT): $(OBJ_ENG)
	@echo "Linking shared library..."
	$(CC) $(LDFLAGS_LIB) -o $@ $^ $(LDLIBS)

# Link the sandbox executable
$(BIN_BOX): $(OBJ_BOX) $(LIB_OUT)
	@echo "Linking sandbox executable..."
	$(CC) -o $@ $^ $(LDFLAGS_BIN) $(LDLIBS)

# Compile source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SANDBOXDIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
dirs:
	@mkdir -p $(BUILDDIR)

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILDDIR)

# Run the sandbox
run: all
	@echo "Running sandbox..."
	@./$(BIN_BOX)