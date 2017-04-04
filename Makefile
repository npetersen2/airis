CC=g++

TARGET_EXEC ?= airis

BUILD_DIR ?= build
SRC_DIR ?= src

INCLUDES=-Iinclude
LIBS=-ltbb -lsqlite3

SRCS := $(shell find $(SRC_DIR) -name *.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CPPFLAGS ?= -MMD -MP -O0 -std=c++11

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) $(INCLUDES) $(LIBS) -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -o $@


.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
	rm $(TARGET_EXEC)

-include $(DEPS)

MKDIR_P ?= @mkdir -p
