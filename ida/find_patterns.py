import traceback
from typing import Dict, List

import ida_bytes
import ida_idaapi
import ida_kernwin
import ida_nalt
import idc

ida_idaapi.require('patterns')

cached_patterns: Dict[str, List[int]] = dict()

def bin_search(bin_str: str) -> List[int]:
    if not isinstance(bin_str, str):
        raise ValueError('bin_str must be a string')

    if bin_str in cached_patterns:
        return cached_patterns[bin_str]

    bin_list = bin_str.split()
    image = bytearray()
    mask = bytearray()

    # Create the mask and convert '?' to 'CC'.
    for i in range(len(bin_list)):
        byte = bin_list[i]
        if byte== '?':
            image.append(int('CC', 16))
            mask.append(0)
        else:
            image.append(int(byte, 16))
            mask.append(1)

    image = bytes(image)
    mask = bytes(mask)

    start = ida_nalt.get_imagebase()
    end = ida_idaapi.BADADDR

    addrs: List[int] = []

    ea = ida_bytes.bin_search(start, end, image, mask, 0, ida_bytes.BIN_SEARCH_FORWARD)
    while ea != ida_idaapi.BADADDR:
        addrs.append(ea)
        ea = ida_bytes.bin_search(ea + len(image), end, image, mask, 0, ida_bytes.BIN_SEARCH_FORWARD)

    cached_patterns[bin_str] = addrs
    return cached_patterns[bin_str]

def find_pattern(pattern: str, expected: int = 1, index: int = 0) -> int:
    if not isinstance(expected, int):
        raise ValueError('expected must be an integer')

    if not isinstance(index, int):
        raise ValueError('index must be an integer')

    addrs = bin_search(pattern)
    if len(addrs) != expected:
        print(f'Found {len(addrs)} match(es) but {expected} match(es) were expected for pattern "{pattern}"')
        return ida_idaapi.BADADDR

    return addrs[index]

def find_function(pattern: str, expected: int = 1, index: int = 0, offset: int = 0) -> int:
    addr = find_pattern(pattern, expected, index)
    if addr == ida_idaapi.BADADDR:
        return addr

    if offset != 0:
        disp = ida_bytes.get_dword(addr + offset)

        if disp >= 1 << 31:
            disp = 0xFFFFFFFF - disp
            disp = disp + 1
            disp = -disp

        # Real address is: pattern_addr + offset + displacement + size_of_displacement.
        return addr + offset + disp + 4
    
    return addr

def find_ptr(pattern: str, expected: int = 1, index: int = 0, offset: int = 0) -> int:
    addr = find_pattern(pattern, expected, index)
    if addr == ida_idaapi.BADADDR:
        return addr

    disp = ida_bytes.get_dword(addr + offset)

    # Real address is: pattern_addr + offset + displacement + size_of_displacement.
    return addr + offset + disp + 4

try:
    groups = patterns.get_groups()
    total = sum(map(lambda g: len(g.pointers) + len(g.functions), groups))

    groups.sort(key=lambda g: g.name.lower())

    addr = find_ptr(pattern='4C 8D 05 ? ? ? ? 48 89 ? ? ? 00 00', expected=9, index=2, offset=3)
    version = idc.get_strlit_contents(addr)

    print(f'Finding {total} item(s)...')
    with open('Addresses.h', 'w') as file:
        file.write('#pragma once\n')
        file.write('\n')
        file.write('/*\n')
        file.write(' * This file is generated. DO NOT modify it!\n')
        file.write(' *\n')
        file.write(' * Add new patterns in "patterns.py" file located in "project_root/scripts" and run "find_patterns.py".\n')
        file.write(' * The new file should be located in "idb_path/Addresses.h".\n')
        file.write(' */\n')
        file.write('#include <cstdint>\n')
        file.write('\n')
        file.write(f'// Addresses for Cyberpunk 2077, version {version.decode()}.\n')
        file.write('namespace CyberEngineTweaks::Addresses\n')
        file.write('{\n')
        file.write(f'constexpr uintptr_t ImageBase = 0x{ida_nalt.get_imagebase():X};\n')
        file.write('\n')

        for group in groups:
            if group.name:
                file.write(f'#pragma region {group.name}\n')

            for ptr in group.pointers:
                addr = find_ptr(pattern=ptr.pattern, expected=ptr.expected, index=ptr.index, offset=ptr.offset)
                if addr == ida_idaapi.BADADDR:
                    file.write(f'#error Could not find pattern "{ptr.pattern}", expected: {ptr.expected}, index: {ptr.index}, offset: {ptr.offset}\n')
                    continue

                if not group.name and not ptr.name:
                    ptr.name = f'ptr_{addr:X}'

                file.write('constexpr uintptr_t ')
                if group.name:
                    file.write(f'{group.name}')

                    if ptr.name:
                        file.write('_')

                ptr.name = ptr.name.replace('::', '_')

                file.write(f'{ptr.name} = 0x{addr:X} - ImageBase; ')
                file.write(f'// {ptr.pattern}, expected: {ptr.expected}, index: {ptr.index}, offset: {ptr.offset}\n')

            for func in group.functions:
                addr = find_function(pattern=func.pattern, expected=func.expected, index=func.index, offset=func.offset)
                if addr == ida_idaapi.BADADDR:
                    file.write(f'#error Could not find pattern "{func.pattern}", expected: {func.expected}, index: {func.index}\n')
                    continue

                file.write('constexpr uintptr_t ')

                if group.name:
                    file.write(f'{group.name}_')

                if not func.name:
                    func.name = f'sub_{addr:X}'

                func.name = func.name.replace('::', '_')

                file.write(f'{func.name} = 0x{addr:X} - ImageBase; ')
                file.write(f'// {func.pattern}, expected: {func.expected}, index: {func.index}\n')

            if group.name:
                file.write(f'#pragma endregion\n')

            if group != groups[-1]:
                file.write('\n')

        file.write('} // namespace CyberEngineTweaks::Addresses\n')

        print('Done!')
        ida_kernwin.beep()
except:
    traceback.print_exc()