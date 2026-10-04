CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = test

SRCS = $(SRC_DIR)/ConsoleView.cpp \
       $(SRC_DIR)/Card.cpp \
       $(SRC_DIR)/Account.cpp \
       $(SRC_DIR)/UserController.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

TEST_C_SRC = $(TEST_DIR)/test_member_c.cpp
TEST_C_BIN = $(BUILD_DIR)/test_member_c

TEST_A_SRC = $(TEST_DIR)/test_member_a.cpp
TEST_A_BIN = $(BUILD_DIR)/test_member_a

all: $(OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: test_c test_a

test_c: $(OBJS) $(TEST_C_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_C_SRC) -o $(TEST_C_BIN)
	./$(TEST_C_BIN)

test_a: $(OBJS) $(TEST_A_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_A_SRC) -o $(TEST_A_BIN)
	./$(TEST_A_BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test test_c test_a clean
