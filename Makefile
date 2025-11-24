# Makefile for PlatformIO Project
#
# Targets:
#   all (default) : Build the project
#   clean         : Clean build artifacts
#   upload        : Upload firmware to the device
#   monitor       : Start serial monitor
#   run           : Upload and then monitor
#   erase         : Erase flash memory
#   package       : Build and copy firmware to release/ folder
#   menuconfig    : Run menuconfig (if applicable)
#

# Default environment (can be overridden: make ENV=other_env)
ENV ?= esp32dev

# PlatformIO command
PIO = pio

# Output directory for package
RELEASE_DIR = release

.PHONY: all clean upload monitor run erase package help

all:
	$(PIO) run -e $(ENV)

clean:
	$(PIO) run -e $(ENV) -t clean
	rm -rf $(RELEASE_DIR)

upload:
	$(PIO) run -e $(ENV) -t upload

monitor:
	# baud rate 115200
	$(PIO) device monitor -e $(ENV) -b 115200

run: upload monitor

erase:
	$(PIO) run -e $(ENV) -t erase

menuconfig:
	$(PIO) run -e $(ENV) -t menuconfig

package: all
	@mkdir -p $(RELEASE_DIR)
	@echo "Packaging firmware..."
	@cp .pio/build/$(ENV)/firmware.bin $(RELEASE_DIR)/firmware_$(ENV).bin
	@echo "Firmware saved to $(RELEASE_DIR)/firmware_$(ENV).bin"

help:
	@echo "Makefile for PlatformIO Project"
	@echo ""
	@echo "Usage:"
	@echo "  make           - Build the project (default)"
	@echo "  make clean     - Clean build artifacts"
	@echo "  make upload    - Upload firmware to device"
	@echo "  make monitor   - Start serial monitor"
	@echo "  make run       - Upload and then monitor"
	@echo "  make erase     - Erase flash memory"
	@echo "  make package   - Build and copy firmware to 'release' folder"
	@echo "  make menuconfig- Run menuconfig"
	@echo ""
	@echo "Variables:"
	@echo "  ENV            - PlatformIO environment (default: $(ENV))"
