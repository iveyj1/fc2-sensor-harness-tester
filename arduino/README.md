# Arduino Nano command-line build/flash environment

This directory contains a self-contained command-line Arduino development setup for an
Arduino Nano connected on `/dev/ttyUSB0`.

The active app is `VoltageWindowD2D3`: it reads A0 and A1 continuously. If A0 is within +/-5% of 1.67 V and A1 is within +/-5% of 1.81 V, it turns D2 on and D3 off; otherwise it turns D2 off and D3 on.

## What was set up

- Installed `arduino-cli` locally into this project:
  - `./bin/arduino-cli`
- Created an Arduino CLI config file:
  - `arduino-cli.yaml`
- Installed the Arduino AVR board core used by the Nano:
  - `arduino:avr`
- Created the active sketch:
  - `VoltageWindowD2D3/VoltageWindowD2D3.ino`
- Kept the original example sketch:
  - `Blinky10Hz/Blinky10Hz.ino`
- Created a `Makefile` with common development commands.
- Built and uploaded the sketch to the connected Nano.

The connected board uploaded successfully using the newer/official Nano bootloader:

```sh
arduino:avr:nano:cpu=atmega328
```

## Project layout

```text
.
├── VoltageWindowD2D3/
│   └── VoltageWindowD2D3.ino # active Arduino sketch
├── Blinky10Hz/
│   └── Blinky10Hz.ino        # original blink example
├── Makefile                # build/upload helper commands
├── README.md               # this document
├── arduino-cli.yaml        # local Arduino CLI config
├── bin/
│   └── arduino-cli         # locally installed Arduino CLI
└── build/                  # generated build artifacts
```

## Requirements

The current setup already has the required CLI and core installed locally. If setting
up again from scratch, you need:

- Linux user with access to the serial port
- Arduino Nano connected over USB
- Network access for first-time Arduino core installation

The serial device currently used is:

```sh
/dev/ttyUSB0
```

The user must be in the `dialout` group to upload without `sudo`.

## Common commands

List detected boards/ports:

```sh
make list
```

Build the sketch:

```sh
make compile
```

Build and upload/flash:

```sh
make flash
```

Install or update the Arduino AVR core:

```sh
make deps
```

Remove generated build output:

```sh
make clean
```

## Developing the app

Edit the sketch here:

```text
VoltageWindowD2D3/VoltageWindowD2D3.ino
```

Then build:

```sh
make compile
```

If the build succeeds, flash it:

```sh
make flash
```

The default Makefile settings are:

```make
PORT=/dev/ttyUSB0
FQBN=arduino:avr:nano:cpu=atmega328
SKETCH=VoltageWindowD2D3
```

You can override them on the command line. Examples:

```sh
make flash PORT=/dev/ttyACM0
make compile SKETCH=MyOtherSketch
make flash FQBN=arduino:avr:nano:cpu=atmega328old
```

## Nano bootloader note

Arduino Nanos may use one of two common bootloaders:

- Newer/official bootloader:

  ```sh
  arduino:avr:nano:cpu=atmega328
  ```

- Older clone bootloader:

  ```sh
  arduino:avr:nano:cpu=atmega328old
  ```

This connected Nano was successfully flashed with the newer/official bootloader.
If upload errors show `programmer is not responding` or `not in sync`, try the old
bootloader target:

```sh
make flash FQBN=arduino:avr:nano:cpu=atmega328old
```

## Recreating the setup manually

These are the commands used, summarized for reference:

```sh
# Install arduino-cli locally into ./bin
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh -s -- 1.3.1

# Create local config
./bin/arduino-cli config init --overwrite --dest-dir .

# Install board package index and AVR core
./bin/arduino-cli core update-index --config-file arduino-cli.yaml
./bin/arduino-cli core install arduino:avr --config-file arduino-cli.yaml

# Build
make compile

# Upload
make flash
```

## Voltage thresholds

The active app assumes the Arduino default analog reference, nominally 5.0 V.

- A0 pass window: 1.67 V +/-5%
- A1 pass window: 1.81 V +/-5%
- Passing: D2 HIGH, D3 LOW
- Failing: D2 LOW, D3 HIGH
