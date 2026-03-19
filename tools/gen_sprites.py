"""
Brick Blaster - Sprite Generator
License: MIT License (c) 2026 ISSALIG
"""
import os
from PIL import Image

def px2byte(p0, p1):
    b = 0
    if p0 & 1: b |= 0x80
    if p1 & 1: b |= 0x40
    if p0 & 4: b |= 0x20
    if p1 & 4: b |= 0x10
    if p0 & 2: b |= 0x08
    if p1 & 2: b |= 0x04
    if p0 & 8: b |= 0x02
    if p1 & 8: b |= 0x01
    return b

# Palette Color for levels (Mode 0: 16 colors)
chars = {
    '.': 0,   # 0:  HW_BLACK (Backgrounds/Transparent)
    'B': 1,   # 1:  HW_BRIGHT_BLUE (Paddle border / neon grids)
    'W': 2,   # 2:  HW_BRIGHT_WHITE (Ball / bright highlights)
    'R': 3,   # 3:  HW_BRIGHT_RED (Brick 0, paddle tips, circuits)
    'Y': 4,   # 4:  HW_BRIGHT_YELLOW (Brick 1, lasers)
    'G': 5,   # 5:  HW_BRIGHT_GREEN (Brick 2)
    'C': 6,   # 6:  HW_BRIGHT_CYAN (Brick 3)
    'M': 7,   # 7:  HW_BRIGHT_MAGENTA (Brick 4)
    'r': 8,   # 8:  HW_RED (Brick 5, darker red)
    'g': 9,   # 9:  HW_GREEN (Brick 6, darker green)
    'c': 10,  # 10: HW_CYAN (Brick 7, darker cyan)
    'w': 11,  # 11: HW_WHITE (Silver Bricks, Walls, Paddle Body)
    'b': 12,  # 12: HW_BLUE (Available Blue for patterns/sprites)
    'o': 13,  # 13: HW_ORANGE (Available Orange for lasers/etc)
    'm': 14,  # 14: HW_MAUVE (Mauve - new decorative color)
    'y': 15,  # 15: HW_YELLOW (Gold Bricks)
}

# 12 pixels x 6 lines
paddle = [
    ".RwwwwwwwwR.",
    "RRWWWWWWWWRR",
    "RRWwwwwwwWRR",
    "RRWwwwwwwWRR",
    "RRWwwwwwwWRR",
    ".RwwwwwwwwR."
]

# 6 pixels x 6 lines
paddle_tiny = [
    ".RwwR.",
    "RRWWRR",
    "RRwwRR",
    "RRwwRR",
    "RRwwRR",
    ".RwwR."
]

# 20 pixels x 6 lines
paddle_wide = [
    ".RwwwwwwwwwwwwwwwwR.",
    "RRWWWWWWWWWWWWWWWWRR",
    "RRWWwwwwwwwwwwwwWWRR",
    "RRWWwwwwwwwwwwwwWWRR",
    "RRWWwwwwwwwwwwwwWWRR",
    ".RwwwwwwwwwwwwwwwwR."
]

def str2bytes(art):
    out = []
    for line in art:
        if len(line) % 2 != 0:
            line += "."
        for i in range(0, len(line), 2):
            c0 = chars.get(line[i], 0)
            c1 = chars.get(line[i+1], 0)
            out.append(px2byte(c0, c1))
    return out

ball = [
    "wWw",
    "WWW",
    "wWw"
]

fireball = [
    "rRr",
    "RYR",
    "rRr"
]

out_c = """#ifndef SPRITES_H
#define SPRITES_H

#include <types.h>

"""
out_c += f"#define PADDLE_WIDTH_BYTES {(len(paddle[0]) + 1) // 2}\n"
out_c += f"#define PADDLE_HEIGHT      {len(paddle)}\n"
out_c += f"#define PADDLE_WIDTH_WIDE  {(len(paddle_wide[0]) + 1) // 2}\n"
out_c += f"#define PADDLE_WIDTH_TINY  {(len(paddle_tiny[0]) + 1) // 2}\n"
out_c += f"#define BALL_SIZE_BYTES    {(len(ball[0]) + 1) // 2}\n"
out_c += f"#define BALL_HEIGHT        {len(ball)}\n\n"

out_c += "const u8 paddle_sprite[] = {\n    "
b = str2bytes(paddle)
out_c += ", ".join(f"0x{x:02X}" for x in b)

out_c += "\n};\n\nconst u8 paddle_wide_sprite[] = {\n    "
b = str2bytes(paddle_wide)
out_c += ", ".join(f"0x{x:02X}" for x in b)

out_c += "\n};\n\nconst u8 paddle_tiny_sprite[] = {\n    "
b = str2bytes(paddle_tiny)
out_c += ", ".join(f"0x{x:02X}" for x in b)

out_c += "\n};\n\nconst u8 fireball_sprite[] = {\n    "
b = str2bytes(fireball)
out_c += ", ".join(f"0x{x:02X}" for x in b)

out_c += "\n};\n\nconst u8 ball_sprite[] = {\n    "
b = str2bytes(ball)
out_c += ", ".join(f"0x{x:02X}" for x in b)
out_c += "\n};\n\n"

# Alien Moped (Legacy - Now moved to src/assets/alien_moped.h via tools/sprite2h.py)
# out_c += f"#define ALIEN_MOPED_WIDTH_BYTES {(len(alien_moped_sprite_data[0]) + 1) // 2}\n"
# out_c += f"#define ALIEN_MOPED_HEIGHT      {len(alien_moped_sprite_data)}\n\n"
# out_c += "const u8 alien_moped_sprite[] = {\n    "
# b = str2bytes(alien_moped_sprite_data)
# out_c += ", ".join(f"0x{x:02X}" for x in b)
# out_c += "\n};\n\n"

# 4 bytes wide = 8 pixels. 8 lines high.
bg_pattern_1 = [
    "BBBBBBBB",
    "BBBBBBBb",
    "BBBBBBbb",
    "BBBBBbbb",
    "BBBBbbbb",
    "BBBbbbbb",
    "BBbbbbbb",
    "Bbbbbbbb"
]

bg_pattern_0 = [
    "bbb...Bb",
    "bb..BBBb",
    "b.BBBBBB",
    "BBBBbbbb",
    "bbbbbbbb",
    "..bbbbbb",
    "BB..bbbB",
    "BBBB..bB"
]

bg_pattern_4 = [ # Diamonds
    "...BB...",
    "..BBBB..",
    ".BBBBBB.",
    "BBBBBBBB",
    "BBBBBBBB",
    ".BBBBBB.",
    "..BBBB..",
    "...BB..."
]

bg_pattern_5 = [ # Faceted cube
    "BBBBbbbb",
    "BBBbbbbb",
    "BBbbbbbb",
    "Bbbbbbbb",
    "bbbbBBBB",
    "bbbbbBBB",
    "bbbbbbBB",
    "bbbbbbbB"
]

# 14 pixels x 6 lines (matching BRICK_WIDTH_BYTES 7 and BRICK_HEIGHT 6)
def make_brick(color_main, color_light, color_dark):
    return [
        color_light * 13 + ".",
        color_light + color_main * 12 + color_dark,
        color_light + color_main * 12 + color_dark,
        color_light + color_main * 12 + color_dark,
        color_light + color_main * 12 + color_dark,
        "." + color_dark * 13
    ]

# Generate sprites for all brick types
# Colors: R, Y, G, C, M, r, g, c, w (Silver), y (Gold)
brick_sprites_defs = [
    make_brick("R", "W", "r"), # Type 0: Red
    make_brick("Y", "W", "y"), # Type 1: Yellow
    make_brick("G", "W", "g"), # Type 2: Green
    make_brick("C", "W", "c"), # Type 3: Cyan
    make_brick("M", "W", "r"), # Type 4: Magenta (using dark red as shadow)
    make_brick("r", "R", "."), # Type 5: Dark Red
    make_brick("g", "G", "."), # Type 6: Dark Green
    make_brick("c", "C", "."), # Type 7: Dark Cyan
    make_brick("w", "W", "b"), # Silver: light grey body, white highlight, blue shadow
    make_brick("y", "W", "r"), # Gold: gold body, white highlight, dark red shadow
]

# Wall sprites: 4x8 pixels (2x8 bytes)
wall_v2 = [
    "cBWB",
    "cBWB",
    "cBWB",
    "cBWB",
    "cBWB",
    "cBWB",
    "cBWB",
    "cBWB"
]

wall_v = [
    "BWBc",
    "BWBc",
    "BWBc",
    "BWBc",
    "BWBc",
    "BWBc",
    "BWBc",
    "BWBc"
]

wall_h = [
    "BBBB",
    "BBBB",
    "WWWW",
    "WWWW",
    "BBBB",    
    "BBBB",
    "cccc",
    "cccc"
]



wall_corner_l = [
    "...B",
    "..BB",
    ".BWW",
    "BWWW",
    "BWBB",
    "BWBB",
    "BWBc",
    "BWBc" 
]

wall_corner_r = [
    "B...",
    "BB..", 
    "WWB.", 
    "WWWB", 
    "BBWB",    
    "BBWB", 
    "cBWB", 
    "cBWB" 
]

# 8x8 pixels (4 bytes x 8 lines) Comedic enemies
enemy_alien = [
    ".G....G."
    "..G..G..",
    ".GGGGGG.",
    "GG.GG.GG",
    "GGGGGGGG",
    "G.GGGG.G",    
    "G.G..G.G",
    "..G..G.."
]

enemy_atom = [
    ".rRRRr..",
    "rRRRRRr.",
    "rrRRRrg.",
    ".rrgGGGg",
    ".bgGGGGG",
    "bBBBGGGg",
    "BBBBBbgg",
    "bBBBbbb."
]

# enemy_ufo = [
#     "...ww...",
#     "..wccw..",
#     ".bccccb.",
#     ".bccccb.",
#     "bMMMMMMb",
#     "bMMMMMMb",    
#     "mmmmmmmm",
#     "mmmmmmmm"
# ]

# enemy_boss_fire_large = [
#     ".cCCCCc.",
#     "cCWWWWCc",
#     "cCWWWWCc",
#     "cCWWWWCc",
#     "cCCWWCCc",
#     "cCCWWCCc",
#     "ccCWWCcc",
#     ".ccCCCc.",

#     "..cCCc..",
#     "..cCCc..",
#     "..cCCc..",
#     "..cCCc..",
#     "..ccCc..",
#     "...cc...",
#     "...cc...",
#     "...c...."
# ]

enemy_fire_blue = [
    "...cc...",    
    "..cCCc..",
    "..cCCc..",
    ".ccCCCc.",
    "cCCWWCCc",
    "cCWWWWCc",
    "cCWWWWCc",
    ".cCCCCc."
]

enemy_fire = [
    "...rr...",    
    "..rRRr..",
    "..rRRr..",
    ".rrRRRr.",
    "rRRYYRRr",
    "rRYWWYRr",
    "rRYYYYRr",
    ".rRRRRr."
]


# Door Animation - Energy field / magnetic waves
# 8 pixels (4 chars = 2 bytes) wide, 16 pixels high.
# Cyans (C, c) and White (W) for electric effect.
door_anim_frames = [
    [ # Frame 0: Zigzag starting from top-left
        "W...", ".w..", "..c.", "...C", 
        "..c.", ".w..", "W...", ".w..", 
        "..c.", "...C", "..c.", ".w..", 
        "W...", ".w..", "..c.", "...C"
    ],
    [ # Frame 1: Shifted down 1 line
        "...C", "W...", ".w..", "..c.", 
        "...C", "..c.", ".w..", "W...", 
        ".w..", "..c.", "...C", "..c.", 
        ".w..", "W...", ".w..", "..c."
    ],
    [ # Frame 2: Shifted down 2 lines
        "..c.", "...C", "W...", ".w..", 
        "..c.", "...C", "..c.", ".w..", 
        "W...", ".w..", "..c.", "...C", 
        "..c.", ".w..", "W...", ".w..",
    ],
    [ # Frame 3: Shifted down 3 lines
        ".w..", "..c.", "...C", "W...", 
        ".w..", "..c.", "...C", "..c.", 
        ".w..", "W...", ".w..", "..c.", 
        "...C", "..c.", ".w..", "W..."
    ]
]

# 6x11 pixels (3 bytes x 11 lines).
# 6 pixels wide in Mode 0 = 12 physical units.
# 11 lines high = 11 physical units. Almost perfectly square pill.
def create_capsule(letter_lines, color, letter_color='W'):
    # Capsule width: 6 pixels = 3 bytes
    # Letter width: 4 pixels = 2 bytes 
    rows = [
        "." + color*4 + ".",                     # Row 0: Cap
        color*6,                                 # Row 1: Border
    ]
    for line in letter_lines:                    # Rows 2-8: letter (4 chars = 2 bytes)
        # We must replace carefully to not overwrite letter_color with color
        filled_line = "".join(letter_color if c == 'W' else color for c in line)
        rows.append(color + filled_line + color)
    rows.append(color*6)                         # Row 9: Border
    rows.append("." + color*4 + ".")             # Row 10: Cap
    return rows

# Power-up Colors & Letters Mapping in Arkanoid:
# L - Laser (Red 'R')
# S - Slow (Yellow 'Y') 
# C - Catch/Glue (Green 'G')
# P - Player/Life (Blue 'b' / HW_BLUE)
# B - Break/Warp (Dark Cyan 'c' / HW_CYAN)
# E - Enlarge (Bright Cyan 'C' / HW_BRIGHT_CYAN)
# M - Disrupt/Multi (Magenta 'M')
capsule_sprites_defs = [
    create_capsule(["W...", "W...", "W...", "W...", "W...", "W...", "WWWW"], "R"), # L (Laser = Red)
    create_capsule([".WW.", "W..W", "W...", ".WW.", "...W", "W..W", ".WW."], "Y", "."), # S (Slow = Yellow)
    create_capsule([".WW.", "W..W", "W...", "W...", "W...", "W..W", ".WW."], "G", "."), # C (Catch = Green)
    create_capsule(["WWW.", "W..W", "W..W", "WWW.", "W...", "W...", "W..."], "B"), # P (Player/Life = Bright Blue)
    create_capsule(["WWW.", "W..W", "W..W", "WWW.", "W..W", "W..W", "WWW."], "c", "."), # B (Break = Dark Cyan)
    create_capsule(["WWWW", "W...", "W...", "WWW.", "W...", "W...", "WWWW"], "C", "."), # E (Enlarge = Bright Cyan)
    create_capsule(["W..W", "WWWW", "W..W", "W..W", "W..W", "W..W", "W..W"], "M"), # M (Multi = Magenta)
    create_capsule([".WWW", "..W.", "..W.", "..W.", "..W.", "..W.", ".WWW"], "B", "."), # I (Ice = Light Blue)
    create_capsule(["W..W", "W..W", "W..W", "W..W", "W..W", "W..W", ".WW."], "m"),  # U (Magnet = Mauve/Purple)
    create_capsule([".WW.", "W..W", "WWWW", "W..W", "W..W", "W..W", "W..W"], "G", "."), # A (Autopilot = Greenish/Cyan)
    create_capsule(["WWW.", "W..W", "W..W", "W..W", "W..W", "W..W", "WWW."], "m", "."), # D (Drunk = Mauve)
    create_capsule(["W..W", "W..W", "W..W", "W..W", "W..W", ".W.W", "..W."], "o", "."), # V (Fast = Orange)
    create_capsule(["WWWW", ".W..", ".W..", ".W..", ".W..", ".W..", ".W.."], "r"), # T (Tiny = Dark Red)
    create_capsule([".WW.", "W...", "W.WW", "W..W", "W..W", "W..W", ".WW."], "g", "."), # G (Gravity = Dark Green)
    create_capsule(["WWWW", "W...", "WWW.", "W...", "W...", "W...", "W..."], "r", "."), # F (Fireball = Red)
]

bg_patterns = [bg_pattern_0, bg_pattern_1, bg_pattern_4, bg_pattern_5]

out_c += f"#define PATTERN_WIDTH_BYTES {(len(bg_pattern_0[0]) + 1) // 2}\n"
out_c += f"#define PATTERN_HEIGHT      {len(bg_pattern_0)}\n"
out_c += f"#define NUM_BG_PATTERNS {len(bg_patterns)}\n\n"

out_c += "const u8 bg_patterns[NUM_BG_PATTERNS][PATTERN_HEIGHT * PATTERN_WIDTH_BYTES] = {\n"
for idx, pat in enumerate(bg_patterns):
    out_c += "    // Pattern " + str(idx) + "\n    {"
    b = str2bytes(pat)
    out_c += ", ".join(f"0x{x:02X}" for x in b)
    out_c += "}" + ("," if idx < len(bg_patterns)-1 else "") + "\n"
out_c += "};\n\n"

# Output bricks as an array of pointers or a flat array
out_c += "#define NUM_BRICK_SPRITES 10\n"
out_c += "const u8 brick_sprites[NUM_BRICK_SPRITES][6 * 7] = {\n"
for idx, brk in enumerate(brick_sprites_defs):
    out_c += "    {"
    b = str2bytes(brk)
    out_c += ", ".join(f"0x{x:02X}" for x in b)
    out_c += "}" + ("," if idx < len(brick_sprites_defs)-1 else "") + "\n"
out_c += "};\n\n"

# Wall sprites
out_c += "const u8 wall_v_sprite[16] = {" + ", ".join(f"0x{x:02X}" for x in str2bytes(wall_v)) + "};\n"
out_c += "const u8 wall_v2_sprite[16] = {" + ", ".join(f"0x{x:02X}" for x in str2bytes(wall_v2)) + "};\n"
out_c += "const u8 wall_h_sprite[16] = {" + ", ".join(f"0x{x:02X}" for x in str2bytes(wall_h)) + "};\n"
out_c += "const u8 wall_cl_sprite[16] = {" + ", ".join(f"0x{x:02X}" for x in str2bytes(wall_corner_l)) + "};\n"
out_c += "const u8 wall_cr_sprite[16] = {" + ", ".join(f"0x{x:02X}" for x in str2bytes(wall_corner_r)) + "};\n\n"




# Enemy sprites
out_c += "#define NUM_ENEMY_SPRITES 3\n"
out_c += "const u8 enemy_sprites[NUM_ENEMY_SPRITES][4 * 8] = {\n"
for idx, enm in enumerate([enemy_alien, enemy_atom, enemy_fire]):
    out_c += "    {"
    b = str2bytes(enm)
    out_c += ", ".join(f"0x{x:02X}" for x in b)
    out_c += "}" + ("," if idx < 2 else "") + "\n"
out_c += "};\n\n"

# Power-up sprites (6x11 pixels -> 3 bytes x 11 lines)
out_c += f"#define NUM_POWERUP_SPRITES {len(capsule_sprites_defs)}\n"
out_c += "const u8 powerup_sprites[NUM_POWERUP_SPRITES][3 * 11] = {\n"
for idx, pwr in enumerate(capsule_sprites_defs):
    out_c += "    {"
    b = str2bytes(pwr)
    out_c += ", ".join(f"0x{x:02X}" for x in b)
    out_c += "}" + ("," if idx < len(capsule_sprites_defs) - 1 else "") + "\n"
out_c += "};\n\n"

# Door Animation Sprites
out_c += f"#define NUM_DOOR_FRAMES {len(door_anim_frames)}\n"
out_c += "const u8 door_anim_sprites[NUM_DOOR_FRAMES][2 * 16] = {\n"
for idx, anim in enumerate(door_anim_frames):
    out_c += "    {"
    b = str2bytes(anim)
    out_c += ", ".join(f"0x{x:02X}" for x in b)
    out_c += "}" + ("," if idx < len(door_anim_frames) - 1 else "") + "\n"
out_c += "};\n\n"

# Custom Arcade Font (4x6 pixels = 2 bytes x 6 lines)
# Character set: A-Z, 0-9, Space, Slash (38 characters)
# To avoid letters glued together, max width is 3 pixels, 4th column is always '.'
raw_font = [
    # A
    [".w..", "w.w.", "www.", "w.w.", "w.w.", "...."],
    # B
    ["ww..", "w.w.", "ww..", "w.w.", "ww..", "...."],
    # C
    [".ww.", "w...", "w...", "w...", ".ww.", "...."],
    # D
    ["ww..", "w.w.", "w.w.", "w.w.", "ww..", "...."],
    # E
    ["www.", "w...", "ww..", "w...", "www.", "...."],
    # F
    ["www.", "w...", "ww..", "w...", "w...", "...."],
    # G
    [".ww.", "w...", "w.w.", "w.w.", ".ww.", "...."],
    # H
    ["w.w.", "w.w.", "www.", "w.w.", "w.w.", "...."],
    # I
    ["www.", ".w..", ".w..", ".w..", "www.", "...."],
    # J
    ["..w.", "..w.", "..w.", "w.w.", ".w..", "...."],
    # K
    ["w.w.", "ww..", "w...", "ww..", "w.w.", "...."],
    # L
    ["w...", "w...", "w...", "w...", "www.", "...."],
    # M
    ["w.w.", "www.", "www.", "w.w.", "w.w.", "...."],
    # N
    ["ww..", "w.w.", "w.w.", "w.w.", "w.w.", "...."],
    # O
    [".w..", "w.w.", "w.w.", "w.w.", ".w..", "...."],
    # P
    ["ww..", "w.w.", "ww..", "w...", "w...", "...."],
    # Q
    [".w..", "w.w.", "w.w.", "w.w.", ".ww.", "...."],
    # R
    ["ww..", "w.w.", "ww..", "w.w.", "w.w.", "...."],
    # S
    [".ww.", "w...", ".w..", "..w.", "ww..", "...."],
    # T
    ["www.", ".w..", ".w..", ".w..", ".w..", "...."],
    # U
    ["w.w.", "w.w.", "w.w.", "w.w.", ".w..", "...."],
    # V
    ["w.w.", "w.w.", "w.w.", ".w..", ".w..", "...."],
    # W
    ["w.w.", "w.w.", "w.w.", "www.", "w.w.", "...."],
    # X
    ["w.w.", "w.w.", ".w..", "w.w.", "w.w.", "...."],
    # Y
    ["w.w.", "w.w.", ".w..", ".w..", ".w..", "...."],
    # Z
    ["www.", "..w.", ".w..", "w...", "www.", "...."],
    
    # 0 (idx 26)
    [".w..", "w.w.", "w.w.", "w.w.", ".w..", "...."],
    # 1
    [".w..", "ww..", ".w..", ".w..", "www.", "...."],
    # 2
    ["ww..", "..w.", ".w..", "w...", "www.", "...."],
    # 3
    ["ww..", "..w.", ".w..", "..w.", "ww..", "...."],
    # 4
    ["w.w.", "w.w.", "www.", "..w.", "..w.", "...."],
    # 5
    ["www.", "w...", "ww..", "..w.", "ww..", "...."],
    # 6
    [".ww.", "w...", "ww..", "w.w.", ".w..", "...."],
    # 7
    ["www.", "..w.", ".w..", "w...", "w...", "...."],
    # 8
    [".w..", "w.w.", ".w..", "w.w.", ".w..", "...."],
    # 9
    [".w..", "w.w.", ".ww.", "..w.", ".w..", "...."],
    
    # Space (idx 36)
    ["....", "....", "....", "....", "....", "...."],
    
    # Slash / (idx 37)
    ["..w.", "..w.", ".w..", ".w..", "w...", "...."],

    # Colon : (idx 38)
    ["....", ".w..", "....", ".w..", "....", "...."],
    
    # Dot . (idx 39)
    ["....", "....", "....", "....", ".w..", "...."],
    
    # Exclamation ! (idx 40)
    [".w..", ".w..", ".w..", "....", ".w..", "...."],
    
    # Comma , (idx 41)
    ["....", "....", "....", "....", ".w..", "w..."],

    # Inverted Exclamation ¡ (idx 42)
    [".w..", "....", ".w..", ".w..", ".w..", "...."],
    
    # Ñ (idx 43)
    ["www.", "....", "ww..", "w.w.", "w.w.", "...."]
]

# Custom Arcade Font LARGE (6x8 pixels = 3 bytes x 8 lines)
# Max width 5 pixels, 6th column (byte boundary) is '.'
raw_font_large = [
    # A
    ["..w...", ".w.w..", ".w.w..", "wwww..", "w..w..", "w..w..", "w..w..", "......"],
    # B
    ["www...", "w..w..", "www...", "w..w..", "w..w..", "www...", "......", "......"],
    # C
    [".www..", "w.....", "w.....", "w.....", "w.....", ".www..", "......", "......"],
    # D
    ["ww....", "w.w...", "w..w..", "w..w..", "w.w...", "ww....", "......", "......"],
    # E
    ["wwww..", "w.....", "www...", "w.....", "w.....", "wwww..", "......", "......"],
    # F
    ["wwww..", "w.....", "www...", "w.....", "w.....", "w.....", "......", "......"],
    # G
    [".www..", "w.....", "w.ww..", "w..w..", "w..w..", ".www..", "......", "......"],
    # H
    ["w..w..", "w..w..", "wwww..", "w..w..", "w..w..", "w..w..", "......", "......"],
    # I
    ["www...", ".w....", ".w....", ".w....", ".w....", "www...", "......", "......"],
    # J
    ["...w..", "...w..", "...w..", "w..w..", "w..w..", ".ww...", "......", "......"],
    # K
    ["w..w..", "w.w...", "ww....", "w.w...", "w..w..", "w..w..", "......", "......"],
    # L
    ["w.....", "w.....", "w.....", "w.....", "w.....", "wwww..", "......", "......"],
    # M
    ["w...w.", "ww.ww.", "w.w.w.", "w...w.", "w...w.", "w...w.", "......", "......"],
    # N
    ["w..w..", "ww.w..", "ww.w..", "w.ww..", "w.ww..", "w..w..", "......", "......"],
    # O
    [".ww...", "w..w..", "w..w..", "w..w..", "w..w..", ".ww...", "......", "......"],
    # P
    ["www...", "w..w..", "www...", "w.....", "w.....", "w.....", "......", "......"],
    # Q
    [".ww...", "w..w..", "w..w..", "w.ww..", "w..w..", ".ww...", "......", "......"],
    # R
    ["www...", "w..w..", "www...", "w.w...", "w..w..", "w..w..", "......", "......"],
    # S
    [".www..", "w.....", ".ww...", "...w..", "w..w..", ".ww...", "......", "......"],
    # T
    ["wwwww.", "..w...", "..w...", "..w...", "..w...", "..w...", "......", "......"],
    # U
    ["w..w..", "w..w..", "w..w..", "w..w..", "w..w..", ".ww...", "......", "......"],
    # V
    ["w..w..", "w..w..", "w..w..", ".w.w..", ".w.w..", "..w...", "......", "......"],
    # W
    ["w...w.", "w...w.", "w...w.", "w.w.w.", "w.w.w.", ".w.w..", "......", "......"],
    # X
    ["w..w..", ".ww...", "..w...", ".ww...", "w..w..", "w..w..", "......", "......"],
    # Y
    ["w..w..", "w..w..", ".ww...", "..w...", "..w...", "..w...", "......", "......"],
    # Z
    ["wwww..", "...w..", "..w...", ".w....", "w.....", "wwww..", "......", "......"],
    
    # 0 (idx 26)
    [".ww...", "w..w..", "w.ww..", "ww.w..", "w..w..", ".ww...", "......", "......"],
    # 1
    ["..w...", ".ww...", "..w...", "..w...", "..w...", ".www..", "......", "......"],
    # 2
    [".ww...", "w..w..", "...w..", "..w...", ".w....", "wwww..", "......", "......"],
    # 3
    ["www...", "...w..", ".ww...", "...w..", "w..w..", ".ww...", "......", "......"],
    # 4
    ["w..w..", "w..w..", "wwww..", "...w..", "...w..", "...w..", "......", "......"],
    # 5
    ["wwww..", "w.....", "www...", "...w..", "w..w..", ".ww...", "......", "......"],
    # 6
    [".ww...", "w.....", "www...", "w..w..", "w..w..", ".ww...", "......", "......"],
    # 7
    ["wwww..", "...w..", "..w...", "..w...", ".w....", ".w....", "......", "......"],
    # 8
    [".ww...", "w..w..", ".ww...", "w..w..", "w..w..", ".ww...", "......", "......"],
    # 9
    [".ww...", "w..w..", "w..w..", ".www..", "...w..", ".ww...", "......", "......"],
    
    # Space (idx 36)
    ["......", "......", "......", "......", "......", "......", "......", "......"],
    
    # Slash / (idx 37)
    ["....w.", "...w..", "...w..", "..w...", "..w...", ".w....", "w.....", "......"],

    # Colon : (idx 38)
    ["......", "..ww..", "..ww..", "......", "..ww..", "..ww..", "......", "......"],
    
    # Dot . (idx 40)
    ["......", "......", "......", "......", "......", "..ww..", "..ww..", "......"],
    
    # Exclamation ! (idx 41)
    ["..ww..", "..ww..", "..ww..", "..ww..", "......", "..ww..", "..ww..", "......"],
    
    # Comma , (idx 42)
    ["......", "......", "......", "......", "......", "..ww..", "..ww..", ".w...."],

    # Inverted Exclamation ¡ (idx 43)
    ["..ww..", "..ww..", "......", "..ww..", "..ww..", "..ww..", "..ww..", "......"],
    # Ñ (idx 46)
    ["..www.", "......", "w..w..", "ww.w..", "w.ww..", "w..w..", "w..w..", "......"]
]

# We generate two colored sets: White ('W') and Red ('R'), 
# plus we just map 'w' in our raw_font to the target color.
def generate_font_c_array(name, font_def, color_char):
    res = f"#define NUM_{name.upper()}_CHARS {len(font_def)}\n"
    res += f"const u8 {name}[{len(font_def)}][{len(font_def[0][0])//2} * {len(font_def[0])}] = {{\n"
    for idx, fdef in enumerate(font_def):
        res += "    {"
        colored_def = [line.replace('w', color_char) for line in fdef]
        b = str2bytes(colored_def)
        res += ", ".join(f"0x{x:02X}" for x in b)
        res += "}" + ("," if idx < len(font_def)-1 else "") + "\n"
    res += "};\n\n"
    return res

# Generate a single base font using 'y' (Color 15, all bits 1) for easy bitmask recoloring in C
out_c += generate_font_c_array("font_sprites", raw_font, "y")
out_c += generate_font_c_array("font_large_sprites", raw_font_large, "y")

out_c += "\n#endif\n"

with open("src/assets/sprites.h", "w") as f:
    f.write(out_c)

print("Generated src/assets/sprites.h")

# =============================================================================
# Amstrad CPC Color Table
# FW  HW    Colour Name        R%  G%  B%   Hex       RGB
# --  ----  -----------------  --  --  ---  -------   -----------
# 0   54h   Black              0   0   0    #000000     0,   0,   0
# 1   44h*  Blue               0   0   50   #000080     0,   0, 128
# 2   55h   Bright Blue        0   0   100  #0000FF     0,   0, 255
# 3   5Ch   Red                50  0   0    #800000   128,   0,   0
# 4   58h   Magenta            50  0   50   #800080   128,   0, 128
# 5   5Dh   Mauve              50  0   100  #8000FF   128,   0, 255
# 6   4Ch   Bright Red         100 0   0    #FF0000   255,   0,   0
# 7   45h*  Purple             100 0   50   #FF0080   255,   0, 128
# 8   4Dh   Bright Magenta     100 0   100  #FF00FF   255,   0, 255
# 9   56h   Green              0   50  0    #008000     0, 128,   0
# 10  46h   Cyan               0   50  50   #008080     0, 128, 128
# 11  57h   Sky Blue           0   50  100  #0080FF     0, 128, 255
# 12  5Eh   Yellow             50  50  0    #808000   128, 128,   0
# 13  40h*  White              50  50  50   #808080   128, 128, 128
# 14  5Fh   Pastel Blue        50  50  100  #8080FF   128, 128, 255
# 15  4Eh   Orange             100 50  0    #FF8000   255, 128,   0
# 16  47h   Pink               100 50  50   #FF8080   255, 128, 128
# 17  4Fh   Pastel Magenta     100 50  100  #FF80FF   255, 128, 255
# 18  52h   Bright Green       0   100 0    #00FF00     0, 255,   0
# 19  42h*  Sea Green          0   100 50   #00FF80     0, 255, 128
# 20  53h   Bright Cyan        0   100 100  #00FFFF     0, 255, 255
# 21  5Ah   Lime               50  100 0    #80FF00   128, 255,   0
# 22  59h   Pastel Green       50  100 50   #80FF80   128, 255, 128
# 23  5Bh   Pastel Cyan        50  100 100  #80FFFF   128, 255, 255
# 24  4Ah   Bright Yellow      100 100 0    #FFFF00   255, 255,   0
# 25  43h*  Pastel Yellow      100 100 50   #FFFF80   255, 255, 128
# 26  4Bh   Bright White       100 100 100  #FFFFFF   255, 255, 255
# (* alternative hw numbers: 44h/50h, 45h/48h, 40h/41h, 42h/51h, 43h/49h)
