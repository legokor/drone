#!/bin/python

from dataclasses import dataclass
from pathlib import Path

def ask(prompt: str) -> bool:
    return input(f'{prompt} ').lower() in ('y', 'yes')

@dataclass
class Module:
    name: str
    struct: bool = True
    init: bool = True

    def dir_path(self, dir: str | Path) -> Path:
        return Path(dir).joinpath(self.name)

    def header_path(self, dir: Path) -> Path:
        return dir.joinpath(f'{self.name}.h')

    def impl_path(self, dir: Path) -> Path:
        return dir.joinpath('impl').joinpath(f'{self.name}.c')

    def gen_header(self) -> str:
        mod = self.name
        Mod = mod.capitalize()
        MOD = mod.upper()

        struct_def = f'''
typedef struct {mod}_{Mod} {{
}} {mod}_{Mod};
'''

        init_args = f'{mod}_{Mod}* {mod}' if self.struct else 'void'
        init_fn = f'void {mod}_Init({init_args});' if self.init else ''

        return f'''
#ifndef {MOD}_H 
#define {MOD}_H
{struct_def if self.struct else ''}
{init_fn}

#endif // {MOD}_H
'''[1:] # remove begin newline

    def gen_impl(self) -> str:
        mod = self.name
        Mod = mod.capitalize()

        init_args = f'{mod}_{Mod}* {mod}' if self.struct else 'void'

        init_fn = f'''
void {mod}_Init({init_args}) {{
    log_Debug("Initalizing {mod}...");

}}
''' if self.init else ''

        return f'''
#include "{mod}.h"
#include "log.h"
{init_fn}'''[1:]

dirs = {
    'Drivers': [
        Module('gps'),
        Module('mag'),
        Module('acc'),
        Module('imu'),
        Module('bar'),
        Module('lora'),
        Module('esc'),
        Module('rc'),
    ],

    'Src': [
        Module('ctrl'),
        Module('dsp'),
        Module('act'),
        Module('llc'),
        Module('tel'),
        Module('guide'),
        Module('sys', struct=False),
        Module('log', struct=False, init=False),
    ]
}

def main():
    for d in dirs:
        for mod in dirs[d]:
            dir = mod.dir_path(d)

            if not dir.is_dir():
                dir.joinpath('impl').mkdir(parents=True)

            h = mod.gen_header()
            i = mod.gen_impl()

            hp = mod.header_path(dir)
            ip = mod.impl_path(dir)

            if hp.is_file() and ip.is_file():
                if hp.read_text() == h and ip.read_text() == i:
                    print(f'skipped {mod.name}')
                    continue
                else:
                    if not ask(f'Overwrite {mod.name}?'):
                        continue

            with open(hp, 'w') as f: f.write(h)
            with open(ip, 'w') as f: f.write(i)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
