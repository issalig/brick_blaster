#!/usr/bin/env python3
import sys
import os

def format_cpc_text(input_file, output_file):
    """
    Format a text file for Amstrad CPC:
    - Normalizes line endings to CRLF (\r\n)
    - Appends the standard EOF character (\x1A / Control-Z)
    - Converts unknown characters to ASCII placeholders
    """
    try:
        with open(input_file, 'rb') as f:
            raw_data = f.read()
        
        # Decode input
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
            
        # Write output
        with open(output_file, 'wb') as f:
            f.write(final_data)
        
        print(f"Successfully formatted {input_file} -> {output_file}")
        print(f"Final size: {len(final_data)} bytes")

    except Exception as e:
        print(f"Error processing file: {e}")
        sys.exit(1)

def main():
    if len(sys.argv) != 3:
        print("Usage: cpc_text_format.py <input_file> <output_file>")
        print("Description: Normalizes line endings to CRLF and appends EOF (&1A) for Amstrad CPC.")
        sys.exit(1)
        
    input_f = sys.argv[1]
    output_f = sys.argv[2]
    
    if not os.path.exists(input_f):
        print(f"Error: Input file '{input_f}' not found.")
        sys.exit(1)
        
    format_cpc_text(input_f, output_f)

if __name__ == "__main__":
    main()
