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
# Default environment (can be overridden: make ENV=esp8266)
ENV ?= esp8266

# PlatformIO command
PIO = pio

# Output directory for package
RELEASE_DIR = release

.PHONY: all clean upload monitor run erase package help esp32 esp8266 upload-esp32 upload-esp8266

all:
	$(PIO) run -e $(ENV)

esp32:
	$(PIO) run -e esp32dev

esp8266:
	$(PIO) run -e esp8266

clean:
	$(PIO) run -e $(ENV) -t clean
	rm -rf $(RELEASE_DIR)

upload:
	$(PIO) run -e $(ENV) -t upload

upload-esp32:
	$(PIO) run -e esp32dev -t upload

upload-esp8266:
	$(PIO) run -e esp8266 -t upload

monitor:
	# baud rate 115200
	$(PIO) device monitor -e $(ENV) -b 115200

run: upload monitor

erase:
	$(PIO) run -e $(ENV) -t erase

erase-esp32:
	$(PIO) run -e esp32dev -t erase

erase-esp8266:
	$(PIO) run -e esp8266 -t erase

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
	@echo "  make           - Build the project (default: $(ENV))"
	@echo "  make esp32     - Build for ESP32"
	@echo "  make esp8266   - Build for ESP8266"
	@echo "  make clean     - Clean build artifacts"
	@echo "  make upload    - Upload firmware to device (default: $(ENV))"
	@echo "  make upload-esp32   - Upload to ESP32"
	@echo "  make upload-esp8266 - Upload to ESP8266"
	@echo "  make monitor   - Start serial monitor"
	@echo "  make run       - Upload and then monitor"
	@echo "  make erase     - Erase flash memory (default: $(ENV))"
	@echo "  make erase-esp32    - Erase ESP32 flash"
	@echo "  make erase-esp8266  - Erase ESP8266 flash"
	@echo "  make package   - Build and copy firmware to 'release' folder"
	@echo "  make menuconfig- Run menuconfig"
	@echo ""
	@echo "Variables:"
	@echo "  ENV            - PlatformIO environment (default: $(ENV))"
