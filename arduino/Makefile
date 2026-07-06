ARDUINO_CLI ?= ./bin/arduino-cli
CONFIG      ?= arduino-cli.yaml
SKETCH      ?= SensorHarnessTester
PORT        ?= /dev/ttyUSB0
# This connected Nano uploaded successfully with the newer/official bootloader.
# Use cpu=atmega328old for many Nano clones with the old bootloader.
FQBN        ?= arduino:avr:nano:cpu=atmega328
BUILD_DIR   ?= build/$(subst :,_,$(FQBN))

.PHONY: all deps list compile upload flash clean

all: compile

deps:
	$(ARDUINO_CLI) core update-index --config-file $(CONFIG)
	$(ARDUINO_CLI) core install arduino:avr --config-file $(CONFIG)

list:
	$(ARDUINO_CLI) board list --config-file $(CONFIG)

compile:
	$(ARDUINO_CLI) compile --config-file $(CONFIG) --fqbn $(FQBN) --build-path $(BUILD_DIR) $(SKETCH)

upload flash: compile
	$(ARDUINO_CLI) upload --config-file $(CONFIG) -p $(PORT) --fqbn $(FQBN) --input-dir $(BUILD_DIR) $(SKETCH)

clean:
	rm -rf build
