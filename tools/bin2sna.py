import sys
import struct

def create_sna(bin_path, sna_path, load_addr, pc=None, sp=None, version=2):
    if pc is None:
        pc = load_addr
    if sp is None:
        # Use 0xF000 as a safer, slightly higher stack for boot snapshots
        sp = 0xF000
        
    # Standard 256 byte SNA Header
    header = bytearray(256)
    
    # Snapshot ID
    header[0:8] = b'MV - SNA'
    
    # Version
    header[0x10] = version
    
    # Registers (SP at 0x21, PC at 0x23)
    header[0x21] = sp & 0xFF
    header[0x22] = (sp >> 8) & 0xFF
    header[0x23] = pc & 0xFF
    header[0x24] = (pc >> 8) & 0xFF
    
    header[0x25] = 1 # IM 1
    
    # Z80 state (Interrupts OFF on boot for stability)
    header[0x1B] = 0 # IFF0
    header[0x1C] = 0 # IFF1
    
    # GA Selected Pen (0x2E)
    header[0x2E] = 0
    
    # Palette (Offset 0x2F to 0x3F) - 17 bytes (16 pens + border)
    palette = [0x04, 0x0A, 0x13, 0x0C, 0x0B, 0x14, 0x15, 0x0D, 0x06, 0x1E, 0x1F, 0x07, 0x12, 0x19, 0x04, 0x04]
    header[0x2F:0x3F] = bytes(palette)
    header[0x3F] = 0x04 # Border (Black)
    
    if version == 1:
        # Match working WinApe V1 example
        header[0x40] = 0x9C # GA Multi-config
        header[0x41] = 0x00 # RAM configuration (V1 uses bits 7-6 as zero for Bank 0)
    else:
        # GA Multi-configuration (0x40)
        header[0x40] = 0x8C # 0x80 | 0x0C (Mode 0 + lower/upper ROM disabled)
        # RAM configuration (0x41)
        header[0x41] = 0xC0 # Standard RAM config for V2
    
    # CRTC Selected Register (0x42)
    header[0x42] = 0
    
    # CRTC Registers (Offset 0x43 to 0x54) - 18 bytes
    crtc = [
        0x3F, 0x28, 0x2E, 0x8E, 0x26, 0x00, 0x19, 0x1E, 
        0x00, 0x07, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00,
        0x00, 0x00
    ]
    if version == 1:
        # Match working WinApe V1 example for CRTC light pen
        crtc[16] = 0x3F # R16
        crtc[17] = 0x28 # R17
    
    header[0x43:0x55] = bytes(crtc)
    
    # ROM selection (0x55)
    header[0x55] = 0
    
    # PPI/PSG (0x56 onwards - leave as 0 for basic compatibility)
    header[0x59] = 0x82 # Typical PPI control byte (Mode 0, Ports A/B/C input/output)
    
    # Memory Size (Offset 0x6B)
    header[0x6B] = 64 # 64KB
    
    if version >= 2:
        # CPC Type (V2/V3 specific) at 0x6D
        header[0x6D] = 2 # CPC 6128
        
        # Creator string
        header[0xE0:0xF0] = b'bin2sna 2026    '
    
    # RAM (64KB)
    ram = bytearray(64 * 1024)
    
    with open(bin_path, 'rb') as f:
        data = f.read()
        
    # Inject binary into RAM
    end_addr = load_addr + len(data)
    if end_addr > 0x10000:
        print(f"Error: Binary too large to fit in 64K RAM (ends at {hex(end_addr)})")
        sys.exit(1)
        
    ram[load_addr:end_addr] = data
    
    # Write SNA file
    with open(sna_path, 'wb') as f:
        f.write(header)
        f.write(ram)
    
    print(f"Successfully created {sna_path} (V{version}, Load: {hex(load_addr)}, PC: {hex(pc)}, SP: {hex(sp)})")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python3 bin2sna.py <input.bin> <output.sna> <load_addr> [pc] [sp] [version]")
        sys.exit(1)
        
    bin_file = sys.argv[1]
    sna_file = sys.argv[2]
    load_addr = int(sys.argv[3], 16)
    
    pc_addr = None
    if len(sys.argv) > 4:
        pc_addr = int(sys.argv[4], 16)
        
    sp_addr = None
    if len(sys.argv) > 5:
        sp_addr = int(sys.argv[5], 16)
        
    sna_version = 2
    if len(sys.argv) > 6:
        sna_version = int(sys.argv[6])
        
    create_sna(bin_file, sna_file, load_addr, pc_addr, sp_addr, sna_version)
