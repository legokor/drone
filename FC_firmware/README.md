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

Gets the RC controller's channels from SBUS.

### \[G\] Guidance

Input:
 - RC: angles (RC hover mode)
 - waypoint: next point(s) / curve
Output: desired angles.

Multiple implementations for RC, GPS waypoints, etc.

### \[LLC\] Low-level Control

Input: reference signal for drone angles & thrust.
Output: thrust vector.

### \[ACT\] Physical Output

Input: arm signal & thrust vector.
Output: ESC signals.

### \[T\] Telemetry

### \[S\] Temporary Storage

### \[DSP\] Digital Signal Processing

Calculates the estimated state.

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
cmake . -B build
cmake --build build
```

### Dependencies

- `cmake`
- `make`
- `arm-none-eabi-gcc`
- `arm-none-eabi-binutils`

### Uploading

[stlink](https://github.com/stlink-org/stlink/) is recommended for flashing / debugging.

```bash
st-flash --flash=512k --opt write build/FC_firmware.bin 0x8000000

# restart (needed to start after flashing)
st-flash reset
```

### Debugging

#### gdb

Start the server (after flashing):

```sh
st-util
```

Connect to the server from a separate shell:

```sh
arm-none-eabi-gdb build/FC_firmware.elf -q -ex 'tar ext :4242'
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
            "executable": "${workspaceFolder}/build/FC_firmware.elf",
            "servertype": "stutil",
            "cwd": "${workspaceFolder}",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "none",
        },
    ]
}
```

## Building on Windows

> Note: You can imitate linux with [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) and proceed like in the [linux section](#building-on-linux). You can also use [MSYS2](https://www.msys2.org/) to achieve the same thing.

You need [CMake](https://cmake.org/), [Ninja](https://ninja-build.org/) and the [Arm Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) for building.

Using the following commands in CMD / PowerShell ([Windows Terminal](https://github.com/microsoft/terminal/) is recommended) is the easiest to install them:
> Note: `winget` is installed by default on Windows 11 and newer versions of Windows 10. If not installed use this PowerShell command: `Add-AppxPackage -RegisterByFamilyName -MainPackage Microsoft.DesktopAppInstaller_8wekyb3d8bbwe` acccording to [Microsoft documentation](https://learn.microsoft.com/en-us/windows/package-manager/winget/#install-winget).
```ps
# recommended
winget install -e --id Git.Git
winget install -e --id Microsoft.WindowsTerminal

winget install -e --id Arm.GnuArmEmbeddedToolchain
winget install -e --id Ninja-build.Ninja
winget install -e --id Kitware.CMake
```

<!-- TODO: document that you can import CMake projects using CubeIDE -->
After installation you need to generate build files for CubeIDE. Run this in the `FC_firmware` folder:

> Note: CMake reports this generator as deprecated.

```ps
cmake . -G "Eclipse CDT4 - Ninja"
```

<!-- TODO: document importing + launch config -->
You can then import the project into CubeIDE. You can create a launch config to make building / debugging easier.
