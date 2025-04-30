#!/bin/python

from dataclasses import dataclass, field
from enum import EnumType

@dataclass
class Field:
    offset: int
    length: int

    name: str
    desc: str

    newlines: int = 1

@dataclass
class Register:
    address: int

    name: str
    desc: str

    fields: list[Field] = field(default_factory=list)

    newlines: int = 1

type RegisterType = list[Field] | EnumType | None

def gen_defs(mod: str, regs: list[Register], ifndef=True):
    if ifndef:
        print(f'#ifndef {mod.upper()}_REGS_H')
        print(f'#define {mod.upper()}_REGS_H\n')

    for reg in regs:
        print(f'// [[REGISTER]] {reg.desc}')
        print(f'#define _{mod}_REG_{reg.name} {hex(reg.address)}')
        print('\n' * reg.newlines, end='')

        for field in reg.fields:
            print(f'// [[FIELD]] {field.desc}')
            print(f'#define _{mod}_REG__{reg.name}__{field.name}__OFFSET {hex(field.offset)}')
            print(f'#define _{mod}_REG__{reg.name}__{field.name}__LENGTH {field.length}')
            print('\n' * field.newlines, end='')

    if ifndef:
        print(f'#endif // {mod.upper()}_REGS_H')
