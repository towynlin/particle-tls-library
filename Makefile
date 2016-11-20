TARGET_LIB ?= particletls.a
TEST_RUNNER ?= runner

BUILD_DIR ?= ./build
SRC_DIR ?= ./src
TEST_DIR ?= ./test

SRCS := $(shell find $(SRC_DIR) -name *.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

TEST_SRCS := $(shell find $(TEST_DIR) -name *.cpp)
TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o)
TEST_DEPS := $(TEST_OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
ARFLAGS ?= rvs


# The first rule is the default target. Just build the library.
$(BUILD_DIR)/$(TARGET_LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TEST_DIR)/$(TEST_RUNNER): $(TEST_OBJS) $(OBJS)
	$(CXX) $(TEST_OBJS) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(TEST_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean test

clean:
	$(RM) -r $(BUILD_DIR)

test: $(BUILD_DIR)/$(TEST_DIR)/$(TEST_RUNNER)
	$(BUILD_DIR)/$(TEST_DIR)/$(TEST_RUNNER)

-include $(DEPS)

MKDIR_P ?= mkdir -p

