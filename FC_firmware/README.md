# FC firmware

## Software architecture

![modules](modules.png "Modules")

The code is modular and divided into modules.

### \[SYS\] System Core

Initializes the other modules and runs the main loop.


### \[C\] High Level Control

Switches between control modes.
Modes:
 - Idle
 - RC remote control
 - Soft landing (normal / abort)
 - Waypoint

### \[RC\] Remote Control

Uses SBUS to get .


### \[G\] Guidance

### \[T\] Telemetry

### \[LLC\] Low-level Control

### \[ACT\] Physical Output

### \[S\] Temporary Storage

### \[DSP\] Digital Signal Processing

### \[P GPS\] GPS IC

#### \[GPS\] GPS driver

### \[P MAG\] Magnetometer IC

#### \[MAG\] Magnetometer driver

### \[P IMU\] IMU IC / driver

#### \[ACC\] Accelerometer data source

#### \[GYR\] Gyroscope data source

### \[P BAR\] Barometer IC

#### \[BAR\] Barometer driver

## Building on linux

```sh
cmake .
make
```

### Dependencies

- `cmake`
- `make`
- `arm-none-eabi-gcc`
- `arm-none-eabi-binutils`

### Uploading

[stlink](https://github.com/stlink-org/stlink/) is recommended for flashing / debugging.

```bash
st-flash write FC_firmware.bin 0x8000000
```

### Debugging

#### gdb

Start the server (after flashing):

```sh
st-util
```

Connect to the server from a separate shell:

```sh
arm-none-eabi-gdb FC_firmware.elf -q -ex 'tar ext :4242'
```

#### VSCode

Install the `cortex-debug` extension, then you can use the following [`launch.json`](.vscode/launch.json):

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug firmware with ST-Util",
            "request": "launch",
            "type": "cortex-debug",
            "executable": "${workspaceFolder}/FC_firmware.elf",
            "servertype": "stutil",
            "cwd": "${workspaceFolder}",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "none",
        },
    ]
}
```

## Building on Windows

> Note: Keeping this section up to date will be done on a best-effort basis, as Linux is the recommended platform for this project.
<!-- comment to separate block quotes -->
> Note: You can imitate linux with [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) and proceed like in the [linux section](#building-on-linux). You can also use [MSYS2](https://www.msys2.org/) to achieve the same thing.

Use [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) as an IDE. You need [CMake](https://cmake.org/), [Ninja](https://ninja-build.org/) and the [Arm Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) for building.

Use the following commands in CMD / PowerShell ([Windows Terminal](https://github.com/microsoft/terminal/) is recommended):

```ps
winget install -e --id Arm.GnuArmEmbeddedToolchain
winget install -e --id Ninja-build.Ninja
winget install -e --id Kitware.CMake

# recommended
winget install -e --id Git.Git
winget install -e --id Microsoft.WindowsTerminal
```

After installation you need to generate build files for CubeIDE. Run this in the `FC_firmware` folder:

> Note: CMake reports this generator as deprecated.

```ps
cmake . -G "Eclipse CDT4 - Ninja"
```

<!-- TODO: document importing + launch config -->
You can then import the project into CubeIDE. You can create a launch config to make building / debugging easier.
