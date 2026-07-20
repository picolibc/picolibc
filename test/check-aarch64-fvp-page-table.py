#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright (c) 2026, Arm Limited.
#

import argparse
import shutil
import struct
import subprocess
import sys
from pathlib import Path

MMU_BLOCK_NS = 1 << 5
MMU_BLOCK_NSE = 1 << 11

EXPECTED = [
    0x0000000000000000,
    0x0000000040000000,
    0x0000000080000F01,
    0x00000000C0000F01,
    0x0000000100000000,
    0x0000000140000000,
    0x0000000180000000,
    0x00000001C0000000,
]


def error(message):
    print(message, file=sys.stderr)
    return 1


def find_tool(tool, compiler):
    candidates = []

    if compiler:
        compiler_path = shutil.which(compiler)
        if compiler_path:
            candidates.append(Path(compiler_path).with_name(tool))

    path_tool = shutil.which(tool)
    if path_tool:
        candidates.append(Path(path_tool))

    for candidate in candidates:
        if candidate.exists():
            return str(candidate)

    raise FileNotFoundError(tool)


def run(command):
    return subprocess.run(
        command,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    ).stdout


def find_symbol(nm, elf, symbol):
    output = run([nm, "--defined-only", "--print-size", elf])
    for line in output.splitlines():
        fields = line.split()
        if len(fields) >= 4 and fields[3] == symbol:
            return int(fields[0], 16), int(fields[1], 16)
    raise ValueError(f"symbol {symbol} not found")


def read_bytes(objdump, elf, address, size):
    stop_address = address + size
    output = run(
        [
            objdump,
            "-s",
            f"--start-address=0x{address:x}",
            f"--stop-address=0x{stop_address:x}",
            elf,
        ]
    )

    data = bytearray()
    next_address = address
    for line in output.splitlines():
        fields = line.split()
        if len(fields) < 2:
            continue
        try:
            row_address = int(fields[0], 16)
        except ValueError:
            continue

        row_data = bytearray()
        for field in fields[1:]:
            if len(field) % 2 != 0:
                break
            try:
                row_data.extend(bytes.fromhex(field))
            except ValueError:
                break

        row_end = row_address + len(row_data)
        if row_end <= next_address:
            continue
        if row_address > next_address:
            break

        copy_start = next_address - row_address
        copy_end = min(row_end, stop_address) - row_address
        data.extend(row_data[copy_start:copy_end])
        next_address += copy_end - copy_start
        if next_address >= stop_address:
            return bytes(data)

    raise ValueError(f"could not read {size} bytes at 0x{address:x}")


def parse_args(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--cc", help="compiler used to build the ELF")
    parser.add_argument("elf")
    return parser.parse_args(argv)


def main(argv):
    args = parse_args(argv[1:])

    try:
        nm = find_tool("llvm-nm", args.cc)
        objdump = find_tool("llvm-objdump", args.cc)
        page_table_addr, page_table_size = find_symbol(nm, args.elf, "__identity_page_table")
        if page_table_size < 8 * 8:
            return error(f"__identity_page_table is too small: {page_table_size} bytes")
        entries = list(struct.unpack("<8Q", read_bytes(objdump, args.elf, page_table_addr, 8 * 8)))
    except (FileNotFoundError, subprocess.CalledProcessError, ValueError) as err:
        return error(str(err))

    if entries != EXPECTED:
        for index, entry in enumerate(entries):
            print(f"entry {index}: 0x{entry:016x}", file=sys.stderr)
        return error("unexpected AArch64 FVP identity page table")

    for index in (2, 3):
        if not entries[index] & MMU_BLOCK_NSE:
            return error(f"entry {index} does not set NSE")
        if entries[index] & MMU_BLOCK_NS:
            return error(f"entry {index} unexpectedly sets NS")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
