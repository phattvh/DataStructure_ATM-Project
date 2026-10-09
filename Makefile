CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = test

SRCS = $(SRC_DIR)/ConsoleView.cpp \
       $(SRC_DIR)/Card.cpp \
       $(SRC_DIR)/Account.cpp \
       $(SRC_DIR)/UserController.cpp \
       $(SRC_DIR)/Admin.cpp \
       $(SRC_DIR)/Transaction.cpp \
       $(SRC_DIR)/FileService.cpp \
       $(SRC_DIR)/AtmController.cpp \
       $(SRC_DIR)/AdminController.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

APP_SRC = $(SRC_DIR)/main.cpp
APP_BIN = $(BUILD_DIR)/atm_app

TEST_C_SRC = $(TEST_DIR)/test_member_c.cpp
TEST_C_BIN = $(BUILD_DIR)/test_member_c

TEST_A_SRC = $(TEST_DIR)/test_member_a.cpp
TEST_A_BIN = $(BUILD_DIR)/test_member_a

TEST_AC_SRC = $(TEST_DIR)/test_phase_1_AC.cpp
TEST_AC_BIN = $(BUILD_DIR)/test_phase_1_AC

TEST_RUNNER_SRC = $(TEST_DIR)/test.cpp
TEST_RUNNER_BIN = $(BUILD_DIR)/test_runner

TEST_PHASE2_SRC = $(TEST_DIR)/test_phase_2_b.cpp
TEST_PHASE2_BIN = $(BUILD_DIR)/test_phase_2_b

TEST_PHASE2_A_SRC = $(TEST_DIR)/test_phase_2_a.cpp
TEST_PHASE2_A_BIN = $(BUILD_DIR)/test_phase_2_a

TEST_PHASE2_C_SRC = $(TEST_DIR)/test_phase_2_c.cpp
TEST_PHASE2_C_BIN = $(BUILD_DIR)/test_phase_2_c

TEST_PHASE3_A_SRC = $(TEST_DIR)/test_phase_3_a.cpp
TEST_PHASE3_A_BIN = $(BUILD_DIR)/test_phase_3_a

TEST_MEM_SRC = $(TEST_DIR)/test_memory_leak.cpp
TEST_MEM_BIN = $(BUILD_DIR)/test_memory_leak

all: $(OBJS) app

app: $(OBJS) $(APP_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(APP_SRC) -o $(APP_BIN)

#=======================================================================

#=======================================================================

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: test_c test_a test_ac test_runner test_phase_2 test_phase_2_a test_phase_2_c test_phase_3_a test_mem

test_c: $(OBJS) $(TEST_C_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_C_SRC) -o $(TEST_C_BIN)
	./$(TEST_C_BIN)

test_a: $(OBJS) $(TEST_A_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_A_SRC) -o $(TEST_A_BIN)
	./$(TEST_A_BIN)

test_ac: $(OBJS) $(TEST_AC_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_AC_SRC) -o $(TEST_AC_BIN)
	./$(TEST_AC_BIN)

test_runner: $(OBJS) $(TEST_RUNNER_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_RUNNER_SRC) -o $(TEST_RUNNER_BIN)
	./$(TEST_RUNNER_BIN)

test_phase_2: $(OBJS) $(TEST_PHASE2_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_PHASE2_SRC) -o $(TEST_PHASE2_BIN)
	./$(TEST_PHASE2_BIN)

test_phase_2_a: $(OBJS) $(TEST_PHASE2_A_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_PHASE2_A_SRC) -o $(TEST_PHASE2_A_BIN)
	./$(TEST_PHASE2_A_BIN)

test_phase_2_c: $(OBJS) $(TEST_PHASE2_C_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_PHASE2_C_SRC) -o $(TEST_PHASE2_C_BIN)
	./$(TEST_PHASE2_C_BIN)

test_phase_3_a: $(OBJS) $(TEST_PHASE3_A_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_PHASE3_A_SRC) -o $(TEST_PHASE3_A_BIN)
	./$(TEST_PHASE3_A_BIN)

test_mem: $(OBJS) $(TEST_MEM_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_MEM_SRC) -o $(TEST_MEM_BIN)
	./$(TEST_MEM_BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all app test test_c test_a test_ac test_runner test_phase_2 test_phase_2_a test_phase_2_c test_phase_3_a test_mem clean
