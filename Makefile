CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC_DIR = src
INC_DIR = include
OBJ_DIR = build
TEST_DIR = test

SRCS = $(SRC_DIR)/Admin.cpp \
       $(SRC_DIR)/Card.cpp \
       $(SRC_DIR)/Account.cpp \
       $(SRC_DIR)/Transaction.cpp \
       $(SRC_DIR)/FileService.cpp \
       $(SRC_DIR)/ConsoleView.cpp \
       $(SRC_DIR)/AtmController.cpp

MAIN_SRC = $(SRC_DIR)/main.cpp
TEST_SRC = $(TEST_DIR)/test_atm.cpp

TARGET = atm_project.exe
TEST_TARGET = test_atm.exe

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
MAIN_OBJ = $(OBJ_DIR)/main.o

# Detect OS for clean command
ifeq ($(OS),Windows_NT)
    MKDIR = if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
    RM = del /Q /F
    RMCLEAN = del /Q /F $(OBJ_DIR)\*.o $(TARGET) $(TEST_TARGET) 2>NUL || exit 0
else
    MKDIR = mkdir -p $(OBJ_DIR)
    RM = rm -rf
    RMCLEAN = rm -rf $(OBJ_DIR) $(TARGET) $(TEST_TARGET)
endif

all: $(TARGET)

$(OBJ_DIR):
	@$(MKDIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(MAIN_OBJ): $(MAIN_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(OBJS) $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	@$(RMCLEAN)

.PHONY: all run test clean
