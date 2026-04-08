#!/usr/bin/env python3
import os
import struct
import argparse

def calculate_checksum(header_67bytes):
    """Calculate the 16-bit checksum for the first 67 bytes of the header."""
    return sum(header_67bytes) & 0xFFFF

def create_header(filename, extension, file_type, load_addr, exec_addr, data_len, user=0):
    """Create a 128-byte AMSDOS header matching Zigazou's addamsdosheader.c logic."""
    header = bytearray(128)
    
    # 0-15: Filename Structure
    header[0] = user & 0x0F
    name = filename.upper()[:8].ljust(8)
    header[1:9] = name.encode('ascii', errors='replace')
    ext = extension.upper()[:3].ljust(3)
    header[9:12] = ext.encode('ascii', errors='replace')
    
    # 16-18: Block/Type info
    header[16] = 0 # Block number
    header[17] = 0 # Last block
    header[18] = file_type # 0: Basic, 2: Binary
    
    # 19-20: Data length (16-bit)
    header[19:21] = struct.pack('<H', data_len)
    
    # 21-22: Load Address (16-bit)
    header[21:23] = struct.pack('<H', load_addr)
    
    # 23: First block flag (&FF for the first/only block)
    header[23] = 0 # Per user request and CPCTech docs: 0 for disk
    
    # 24-25: Logical length (16-bit)
    header[24:26] = struct.pack('<H', data_len)
    
    # 26-27: Entry address (Execution, 16-bit)
    header[26:28] = struct.pack('<H', exec_addr)
    
    # 64-66: File length (24-bit little-endian)
    header[64] = data_len & 0xFF
    header[65] = (data_len >> 8) & 0xFF
    header[66] = (data_len >> 16) & 0xFF
    
    # 67-68: Checksum (Sum of bytes 0-66)
    checksum = calculate_checksum(header[:67])
    header[67:69] = struct.pack('<H', checksum)
    
    return header

def add_header(input_file, output_file, file_type, load_addr, exec_addr, user=0, amsdos_name=None):
    with open(input_file, 'rb') as f:
        raw_data = f.read()
    
    # --- PRE-PROCESS CONTENT FOR BASIC (TYPE 0) ---
    if file_type == 0:
        try:
            text = raw_data.decode('utf-8')
        except UnicodeDecodeError:
            text = raw_data.decode('latin-1')

        # 1. Normalize Line Endings to CRLF (\r\n)
        # We first clean up any existing variations to avoid doubling up
        text = text.replace('\r\n', '\n').replace('\r', '\n').replace('\n', '\r\n')
        
        # 2. Ensure it ends with CRLF before EOF to prevent losing the last line
        if not text.endswith('\r\n'):
            text += '\r\n'
            
        # 3. Convert to bytes (ASCII)
        final_data = bytearray(text.encode('ascii', errors='replace'))
        
        # 4. Ensure it ends with EOF (&1A / Control-Z)
        if len(final_data) == 0 or final_data[-1] != 0x1A:
            final_data.append(0x1A)
            
    else:
        final_data = raw_data
    
    # --- FILENAME LOGIC ---
    if amsdos_name:
        if '.' in amsdos_name:
            filename, extension = amsdos_name.rsplit('.', 1)
        else:
            filename, extension = amsdos_name, ""
    else:
        basename = os.path.basename(input_file)
        if '.' in basename:
            filename, extension = basename.rsplit('.', 1)
        else:
            filename, extension = basename, ""
            
    header = create_header(filename, extension, file_type, load_addr, exec_addr, len(final_data), user)
    
    with open(output_file, 'wb') as f:
        f.write(header)
        f.write(final_data)
    print(f"Added AMSDOS header to {output_file} ({len(final_data)} bytes, type {file_type})")

def strip_header(input_file, output_file):
    with open(input_file, 'rb') as f:
        header = f.read(128)
        data = f.read()
    
    if len(header) < 128:
        print("Error: File is smaller than 128 bytes, cannot strip header.")
        return

    with open(output_file, 'wb') as f:
        f.write(data)
    print(f"Stripped 128-byte header from {input_file}. Data saved to {output_file}")

def main():
    parser = argparse.ArgumentParser(description="Add or remove AMSDOS headers (with CRLF normalization).")
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")

    add_parser = subparsers.add_parser("add", help="Add an AMSDOS header to a file")
    add_parser.add_argument("input", help="Raw input file")
    add_parser.add_argument("output", help="Output file with header")
    add_parser.add_argument("--type", type=int, default=2, help="File type (0: BASIC, 2: Binary)")
    add_parser.add_argument("--load", type=lambda x: int(x, 0), default=0x4000, help="Load address (target)")
    add_parser.add_argument("--exec", type=lambda x: int(x, 0), default=0x4000, help="Execution address")
    add_parser.add_argument("--user", type=int, default=0, help="User number (0-15)")
    add_parser.add_argument("--name", help="Override AMSDOS filename (8.3 format)")

    strip_parser = subparsers.add_parser("strip", help="Remove the AMSDOS header from a file")
    strip_parser.add_argument("input", help="File with header")
    strip_parser.add_argument("output", help="Raw output file")

    args = parser.parse_args()

    if args.command == "add":
        add_header(args.input, args.output, args.type, args.load, args.exec, args.user, args.name)
    elif args.command == "strip":
        strip_header(args.input, args.output)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
