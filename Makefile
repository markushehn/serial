# Project
TARGET = serial_test

# Toolchain definitions
CXX = g++
CC = gcc

# C flags
CFLAGS += -Wall
CFLAGS += -O3
CFLAGS += -MMD -MP
CFLAGS += -std=c11

# CXX flags
CXXFLAGS += -Wall
CXXFLAGS += -O3
CXXFLAGS += -MMD -MP

# Linker flags
LFLAGS += -Wall
LFLAGS += -O3

SRCS += $(shell find ./src -name *.cpp -or -name *.c)
OBJS += $(SRCS:%=%.o)
DEPS += $(OBJS:.o=.d)

INCLUDE = -I"./inc"
SYMBOLS = -DTEST




.PHONY: all
all: $(TARGET)

%.c.o: %.c
	$(CC) -c $(CFLAGS) $(SYMBOLS) $(INCLUDE) $< -o $@

%.cpp.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(SYMBOLS) $(INCLUDE) $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $^ $(LFLAGS) -o $@

-include $(DEPS)


.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(TARGET)







