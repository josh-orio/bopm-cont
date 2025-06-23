BUILD_DIR := build
EXECUTABLE := $(BUILD_DIR)/bopm

.PHONY: all build clean run

all: build run

build:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && $(MAKE) -j

debug: build
	@lldb $(EXECUTABLE)

clean:
	@rm -rf $(BUILD_DIR)

run:
	@$(EXECUTABLE)
