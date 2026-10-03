CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = test

SRCS = $(SRC_DIR)/ConsoleView.cpp \
       $(SRC_DIR)/Card.cpp \
       $(SRC_DIR)/Account.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

TEST_SRC = $(TEST_DIR)/test_member_c.cpp
TEST_BIN = $(BUILD_DIR)/test_member_c

all: $(OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(OBJS) $(TEST_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_SRC) -o $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
