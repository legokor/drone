# Drone firmware

## Building

```bash
cmake .
make
arm-none-eabi-objcopy -O binary FC_firmware.elf FC_firmware.bin
st-flash write FC_firmware.bin 0x8000000
```

## Uploading

```bash
arm-none-eabi-objcopy -O binary FC_firmware.elf FC_firmware.bin
st-flash write FC_firmware.bin 0x8000000
```

## Debugging

Start the server (after flashing):
```bash
st-util
```

```bash
arm-none-eabi-gdb FC_firmware.elf -q -ex 'tar ext :4242'
```

### VSCode

`launch.json`:
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
