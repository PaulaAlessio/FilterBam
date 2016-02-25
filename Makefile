# project name 
TARGET	= FilterBam

# compiler
CC	= g++

# compiling flags
CFLAGS   = -Wall -O3 -march=native -std=gnu++11  -Iinclude/ \
	   -lstdc++ -Wl,--no-as-needed -ldl 
LFLAGS   = -Wall  -lstdc++ -Wl,--no-as-needed -ldl 

LINKER   = g++ -o

# change these to set the proper directories where each files should be
SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.cc)
INCLUDES := $(wildcard $(INCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)
rm       = rm -f



$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cc
	@echo $(SOURCES)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
 
