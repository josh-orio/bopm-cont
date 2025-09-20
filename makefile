BUILD_DIR := build
EXECUTABLE := $(BUILD_DIR)/bopm

.PHONY: all 

all: test

test:
	@clear
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -D CMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && $(MAKE) -j
	@$(EXECUTABLE)

recompile:
	@clear
	@rm -rf .cache build
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -D CMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && $(MAKE) -j

clean:
	@rm -rf $(BUILD_DIR) .cache

debug:
	@clear
	@rm -rf .cache build
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && $(MAKE) -j
	@lldb  $(EXECUTABLE)

run:
	@$(EXECUTABLE)
