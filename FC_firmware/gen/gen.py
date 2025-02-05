#!/bin/python

from jinja2 import Environment, FileSystemLoader
from dataclasses import dataclass
from pathlib import Path
import os

def ask(prompt: str) -> bool:
    return input(f'{prompt} ').lower() in ('y', 'yes')

@dataclass
class Module:
    name: str
    struct: bool
    init: bool

    def dir_path(self, dir: str | Path) -> Path:
        return Path(dir).joinpath(self.name)

    def header_path(self, dir: Path) -> Path:
        return dir.joinpath(f'{self.name}.h')

    def impl_path(self, dir: Path) -> Path:
        return dir.joinpath('impl').joinpath(f'{self.name}.c')

dirs = {
    'Drivers': [ #    struct init
        Module('gps',  True, True),
        Module('mag',  True, True),
        Module('acc',  True, True),
        Module('imu',  True, True),
        Module('bar',  True, True),
        Module('lora', True, True),
        Module('esc',  True, True),
        Module('rc',   True, True),
        Module('sd',   True, True),
        Module('uart', True, True),
    ],

    'Src': [ #          struct init
        Module('ctrl',  False, True),
        Module('dsp',   False, True),
        Module('act',   False, True),
        Module('llc',   False, True),
        Module('tel',   False, True),
        Module('guide', False, True),
        Module('sys',   False, True),
        Module('log',   False, True),
    ]
}

def main():
    env = Environment(loader = FileSystemLoader(os.path.dirname(__file__)))
    
    templ_h = env.get_template('mod.h.jinja')
    templ_c = env.get_template('mod.c.jinja')
    
    for d in dirs:
        for mod in dirs[d]:
            dir = mod.dir_path(d)

            dir.joinpath('impl').mkdir(parents=True, exist_ok=True)

            params = {
                'name': mod.name,
                'Name': mod.name.capitalize(),
                'NAME': mod.name.upper(),
                'struct': mod.struct,
                'init': mod.init
            }

            h = templ_h.render(params)
            i = templ_c.render(params)

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
