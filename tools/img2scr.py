#!/usr/bin/env python3
import sys
import argparse
import math
from PIL import Image

# Amstrad CPC full hardware palette (27 colors)
CPC_PALETTE = [
    (  0,   0,   0), (  0,   0, 128), (  0,   0, 255),
    (128,   0,   0), (128,   0, 128), (128,   0, 255),
    (255,   0,   0), (255,   0, 128), (255,   0, 255),
    (  0, 128,   0), (  0, 128, 128), (  0, 128, 255),
    (128, 128,   0), (128, 128, 128), (128, 128, 255),
    (255, 128,   0), (255, 128, 128), (255, 128, 255),
    (  0, 255,   0), (  0, 255, 128), (  0, 255, 255),
    (128, 255,   0), (128, 255, 128), (128, 255, 255),
    (255, 255,   0), (255, 255, 128), (255, 255, 255),
]

def get_closest_palette_index(pixel, palette):
    best_dist = float('inf')
    best_idx = 0
    for i, p_color in enumerate(palette):
        dist = sum((pixel[j] - p_color[j])**2 for j in range(3))
        if dist < best_dist:
            best_dist = dist
            best_idx = i
    return best_idx

def encode_mode0(p1, p2):
    """
    Encode two pixels into one CPC Mode 0 byte.
    Mode 0 pixel bits (4 bits per pixel):
    P1: b0 b1 b2 b3
    P2: b0 b1 b2 b3
    Byte: P1_b0 P2_b0 P1_b2 P2_b2 P1_b1 P2_b1 P1_b3 P2_b3
    """
    byte = 0
    # Bit 0
    if p1 & 1: byte |= 0x80
    if p2 & 1: byte |= 0x40
    # Bit 1
    if p1 & 2: byte |= 0x08
    if p2 & 2: byte |= 0x04
    # Bit 2
    if p1 & 4: byte |= 0x20
    if p2 & 4: byte |= 0x10
    # Bit 3
    if p1 & 8: byte |= 0x02
    if p2 & 8: byte |= 0x01
    return byte

def main():
    parser = argparse.ArgumentParser(description='Convert image to raw CPC 16KB SCR.')
    parser.add_argument('filename', help='Input PNG/JPG')
    parser.add_argument('output', help='Output SCR file')
    parser.add_argument('--basic', help='Optionally update a BASIC file (e.g. DISC.BAS) with the new palette DATA')
    args = parser.parse_args()

    img = Image.open(args.filename).convert('RGB')
    # Resize to Mode 0 dimensions (160x200)
    img = img.resize((160, 200), Image.LANCZOS)
    
    # Quantize to 16 colors using CPC palette
    # First, find the 16 best colors from the image that exist in the CPC palette
    img_data = list(img.getdata())
    unique_colors = {}
    for p in img_data:
        cpc_idx = get_closest_palette_index(p, CPC_PALETTE)
        cpc_color = CPC_PALETTE[cpc_idx]
        unique_colors[cpc_color] = unique_colors.get(cpc_color, 0) + 1
    
    # Sort by frequency and take top 16
    sorted_colors = sorted(unique_colors.items(), key=lambda x: x[1], reverse=True)
    local_palette = [color for color, count in sorted_colors[:16]]
    # Ensure black is there
    if (0, 0, 0) not in local_palette:
        local_palette[-1] = (0, 0, 0)
    
    print(f"Selected palette: {local_palette}")
    
    # Map pixels to our 16-color local palette
    pixels_idx = []
    palette_indices = []
    for color in local_palette:
        palette_indices.append(str(CPC_PALETTE.index(color)))

    for p in img_data:
        pixels_idx.append(get_closest_palette_index(p, local_palette))
    
    # Create the 16KB interleaved buffer
    scr_buffer = bytearray(16384)
    
    for y in range(200):
        # CPC Offset: 80 * (y >> 3) + 2048 * (y & 7)
        base_addr = 80 * (y // 8) + 2048 * (y % 8)
        for x_byte in range(80):
            p1 = pixels_idx[y * 160 + x_byte * 2]
            p2 = pixels_idx[y * 160 + x_byte * 2 + 1]
            scr_buffer[base_addr + x_byte] = encode_mode0(p1, p2)
            
    with open(args.output, 'wb') as f:
        f.write(scr_buffer)
    
    # Update BASIC file if requested
    if args.basic:
        try:
            # Open in binary mode to preserve \r\n and \x1a
            with open(args.basic, 'rb') as f:
                content = f.read()
            
            # Use binary string for the DATA line (using \r\n which is safer for CPC BASIC)
            data_line = ("50 DATA " + ", ".join(palette_indices)).encode('ascii') + b"\r\n"
            
            # Split lines keeping separators, find and replace
            lines = content.splitlines(keepends=True)
            new_lines = []
            updated = False
            for line in lines:
                # Remove \x1a for comparison if at the end
                clean_line = line.strip(b'\x1a').strip()
                if clean_line.startswith(b"50 DATA"):
                    new_lines.append(data_line)
                    updated = True
                else:
                    new_lines.append(line)
            
            if not updated:
                print(f"Warning: Could not find line 50 DATA in {args.basic}")
            else:
                # Re-join and ensure exactly one \x1a at the very end
                final_content = b"".join(new_lines).rstrip(b'\x1a').rstrip() + b"\r\n\x1a"
                with open(args.basic, 'wb') as f:
                    f.write(final_content)
                print(f"Updated palette in {args.basic} (Binary Mode)")
        except Exception as e:
            print(f"Error updating BASIC file: {e}")

    # Output the palette to console as BASIC INK commands
    print("\nBASIC Palette Setup:")
    for i, idx in enumerate(palette_indices):
        print(f"INK {i}, {idx}")

if __name__ == "__main__":
    main()
