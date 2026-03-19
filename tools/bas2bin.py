import sys
import struct

def tokenize(text):
    # Minimal tokens for the keywords used in DISC.BAS
    # Source: Locomotive BASIC 1.1 reference
    tokens = {
        "MODE": b"\x94",
        "BORDER": b"\x83",
        "DATA": b"\x84",
        "FOR": b"\x89",
        "TO": b"\x9d",
        "READ": b"\xab",
        "INK": b"\x90",
        "NEXT": b"\x95",
        "LOAD": b"\x91",
        "RUN": b"\xaf",
        "REM": b"\x9f",
        ":": b"\x3a"
    }
    
    result = b""
    i = 0
    while i < len(text):
        if text[i] == '"': # Skip strings
            end = text.find('"', i + 1)
            if end == -1: end = len(text)
            result += text[i:end+1].encode('ascii')
            i = end + 1
            continue
            
        # Try to match keywords (check longest keywords first)
        found_token = False
        remaining = text[i:].upper()
        # Sort keywords by length descending
        for kw in sorted(tokens.keys(), key=len, reverse=True):
            if remaining.startswith(kw):
                result += tokens[kw]
                i += len(kw)
                found_token = True
                break
        
        if not found_token:
            result += text[i].encode('ascii')
            i += 1
            
    return result

def main():
    if len(sys.argv) < 3:
        print("Usage: python bas2bin.py input.bas output.bin")
        sys.exit(1)
        
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    with open(input_file, 'r') as f:
        lines = f.readlines()
        
    binary_content = b""
    base_addr = 0x0170
    current_addr = base_addr
    
    for line in lines:
        line = line.strip()
        if not line: continue
        
        # Expecting line in format: "10 CODE"
        parts = line.split(' ', 1)
        if len(parts) < 2: continue
        
        try:
            line_num = int(parts[0])
            content = parts[1]
        except ValueError:
            continue
            
        tokenized_line = tokenize(content)
        # Line format: [2-byte next line pointer] [2-byte line num] [content] [0x00]
        
        line_data = struct.pack("<H", line_num) + tokenized_line + b"\x00"
        next_addr = current_addr + 2 + len(line_data)
        
        binary_content += struct.pack("<H", next_addr) + line_data
        current_addr = next_addr
        
    binary_content += b"\x00\x00" # End of program
    
    with open(output_file, 'wb') as f:
        f.write(binary_content)

if __name__ == "__main__":
    main()
