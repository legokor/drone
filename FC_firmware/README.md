# Drone firmware

## Building

```sh
cmake .
make
```

### Dependencies

- `cmake`
- `make`
- `arm-none-eabi-gcc`
- `arm-none-eabi-binutils`

On Linux / [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) (recommended) install dependencies with system package manager. On Windows (not recommended), [`MSYS2`](https://www.msys2.org/) is probably the best option.

## Uploading

[stlink](https://github.com/stlink-org/stlink/) is recommended for flashing / debugging.

```bash
st-flash write FC_firmware.bin 0x8000000
```

## Debugging

### gdb

Start the server (after flashing):
```sh
st-util
```
Connect to the server from a separate shell:

```sh
arm-none-eabi-gdb FC_firmware.elf -q -ex 'tar ext :4242'
```

### VSCode

Install the `cortex-debug` extension, then you can use the following [`launch.json`](.vscode/launch.json):

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceFolder}/FC_firmware.elf",
            "name": "Debug with ST-Util",
            "request": "launch",
            "type": "cortex-debug",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "none",
            "servertype": "stutil"
        },
    ]
}
```
