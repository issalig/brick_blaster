/**
 * Brick Blaster (Arkanoid Clone)
 * ----------------------------------------------------
 * @author  ISSALIG
 * 
 * Target:  Amstrad CPC 464/6128 (Mode 0)
 * Library: CPCtelera 1.7+
 * 
 * License: MIT License (c) 2026 ISSALIG
 * ----------------------------------------------------
 */

#include <cpctelera.h>
#include "music/music.h"    // Got from Arkos Tracker examples, thanks ULTRASYD.
#include "lang.h"           // Localization
#include "assets/sprites.h" // Awesome sprites
#include "assets/boss.h"    // Final Boss sprite



// Screen dimensions in Mode 0 (pixels and bytes)
#define SCREEN_WIDTH_BYTES  80
#define SCREEN_HEIGHT       200

// Colors (Logical palette index 0-15 mapped to HW colors later)
#define COLOR_BG            0
#define COLOR_PADDLE        1
#define COLOR_BALL          2
// Brick colors: palette slots 3-10 (8 brick colors)
#define COLOR_BRICK_BASE    3
#define COLOR_BRICK_SILVER  11
#define COLOR_BRICK_GOLD    15
#define COLOR_WALL          11

// Entity dimensions
#define WALL_LEFT_BYTES     2
#define WALL_RIGHT_BYTES    78
#define WALL_TOP            16

// Precomputed fixed-point wall boundaries (avoids runtime * FP_SCALE on every frame)
#define WALL_LEFT_FP    (WALL_LEFT_BYTES  * FP_SCALE)
#define WALL_RIGHT_FP   (WALL_RIGHT_BYTES * FP_SCALE)
#define WALL_TOP_FP     (WALL_TOP * FP_SCALE)

#define PADDLE_START_Y      180
#define PADDLE_SPEED        2

#define BALLL_SPEED_Y       1   // Integer part of ball vertical speed

#define BRICK_TOP_OFFSET    20

// Fixed point 10.6 physics
#define FP_SCALE_SHIFT  6                      // log2(FP_SCALE): change here to adjust precision
#define FP_SCALE        (1 << FP_SCALE_SHIFT)  // 64: integer units per pixel
#define FP_INT(x)       ((x) >> FP_SCALE_SHIFT) // Convert fixed-point value to integer (pixel coords)
#define FP_VEL(d,s)     (((d) * (s)) >> FP_SCALE_SHIFT) // Compute velocity component: (dir * speed) / FP_SCALE

// Initial ball speed scalar (in FP_SCALE, so 128 = 2.0 pixels/frame)
#define INITIAL_BALL_SPEED  150

#define BRICK_ROWS          8
#define BRICK_COLS          8
#define NUM_LEVELS          10
#define BRICK_WIDTH_BYTES   7   // 14 pixels
#define BRICK_HEIGHT        6   // Full sprite height
#define BRICK_START_X       8   // Left margin
#define BRICK_START_Y       28  // Top margin
#define BRICK_GAP_X         1   // 2 pixels gap
#define BRICK_GAP_Y         2   // Gap between rows

// Power-up drop capsule dimensions
#define DROP_WIDTH          3   // 6 pixels wide in Mode 0 (3 bytes)
#define DROP_HEIGHT         11

// Laser beam dimensions (LASER_SPEED must equal LASER_HEIGHT for clean trail erasing)
#define MAX_LASER_PAIRS     5   // Up to 5 shots (pair of two beams) in flight simultaneously
#define LASER_WIDTH         1   // bytes (2 pixels)
#define LASER_HEIGHT        3   // scan lines
#define LASER_SPEED         3   // px/frame upward (= LASER_HEIGHT → no gap, no overlap)

// Enemies
#define MAX_ENEMIES         3
#define ENEMY_WIDTH         4   // 8 pixels
#define ENEMY_HEIGHT        8   // 8 scan lines
#define ENEMY_SPEED_Y       1   // y movement per frame
#define COLOR_LASER         PLT_BRIGHT_YELLOW  // HW_BRIGHT_YELLOW in palette slot 14

// --- Brick byte encoding ---
// Bits 0-1: Type
// Bits 2-4: Color (0-7 -> 8 palette slots)
// Bits 5-7: Power-up (0=none, 1=laser, 2=slow, 3=glue, 4=life, ...)
// Bits 0-1: Type
// Bits 2-3: Color (0-3 -> 4 palette slots)
// Bits 4-7: Power-up (0-15 types supported)
#define BRICK_TYPE(b)     ((u8)((b) & 0x03))
#define BRICK_COLOR(b)    ((u8)(((b) >> 2) & 0x03))
#define BRICK_POWER(b)    ((u8)((b) >> 4))
#define BRICK(type, color, power) ((((u16)(power)) << 4) | (((u16)(color)) << 2) | ((u16)(type)))

// Type values
#define BTYPE_EMPTY       0   // Empty space in level map
#define BTYPE_NORMAL      1   // 1-hit brick
#define BTYPE_HARD        2   // 2-hit brick (silver)
#define BTYPE_GOLD        3   // Eternal/Indestructible brick (gold)

// Power values and their corresponding capsule color:
#define BPOWER_NONE       0
#define BPOWER_LASER      1   // L - Red
#define BPOWER_SLOW       2   // S - Yellow
#define BPOWER_GLUE       3   // C - Green (Catch)
#define BPOWER_LIFE       4   // P - Blue (Player)
#define BPOWER_WARP       5   // B - Dark Cyan (Break)
#define BPOWER_EXPAND     6   // E - Bright Cyan (Enlarge)
#define BPOWER_MULTI      7   // M - Magenta (Multi)
#define BPOWER_ICE        8   // I - Light Blue (Trap: Freeze Paddle)
#define BPOWER_MAGNET     9   // U - Mauve (Trap: Repel Ball)
#define BPOWER_AUTOPILOT  10  // A - Greenish/Cyan (Help: Follow Ball)
#define BPOWER_DRUNK      11  // D - Mauve (Trap: Inverted Controls)
#define BPOWER_FAST       12  // V - Orange (Trap: Fast Ball)
#define BPOWER_TINY       13  // T - Dark Red (Trap: Tiny Paddle)
#define BPOWER_GRAVITY    14  // G - Dark Green (Trap: Gravity)
#define BPOWER_FIREBALL   15  // F - Red (Benefit: Pierce Bricks)

// Extra balls for multiball power-up
#define MAX_EXTRA_BALLS   2

// Expanded paddle
// Wide paddle width in bytes is now pulled from sprites.h

// Runtime physical state stored in bricks[][] (use values > 3 to not alias BTYPE)
#define BSTATE_EMPTY      0   // Dead/destroyed
#define BSTATE_NORMAL     1   // Live, 1HP remaining
#define BSTATE_HARD       2   // Live, 2HP remaining
#define BSTATE_GOLD       3   // Live, indestructible
#define BSTATE_NEEDS_ERASE 8  // Destroyed this frame, waiting to be erased

typedef struct {
    u8 x, y;      // x in bytes (0-79), y in lines (0-199)
    u8 old_x, old_y;
    u8 width, height;
    u8 old_width;
} paddle_t;

typedef struct {
    u8 x, y;
    u8 old_x, old_y;
    
    // Fixed point internal positions
    i16 fx, fy;   
    
    // Direction and speed
    i16 dir_x, dir_y; // Normalized direction vector (scaled by FP_SCALE)
    i16 speed;        // Scalar speed (scaled by FP_SCALE)
    i16 dx, dy;       // Velocities in fixed point (dx = (dir_x * speed) >> 6)
    
    u8 width, height;
    u8 active;    // 0 = attached to paddle, 1 = moving freely
} ball_t;

typedef struct {
    u8 x, y;
    u8 old_y;
    u8 active;
    u8 power_type;
    u8 timer; // Frames until next move (throttles fall speed)
} drop_t;

// Each shot fires a LEFT beam and a RIGHT beam that share the same Y position.
// Treating them as one unit halves all loop iterations vs. individually tracked beams.
typedef struct { u8 x_left; u8 x_right; u8 y; u8 old_y; u8 active; u8 needs_bg_repaint; } laser_pair_t;

typedef struct {
    u8 active;
    u8 x, y;
    u8 old_x, old_y;
    u8 color;
    u8 type;
    u8 frame;
    i8 dx;
} enemy_t;

typedef struct {
    u8 active;
    i16 fx, fy;    // Fixed point position
    i16 dx, dy;    // Fixed point velocity
    u8 x, y;       // Screen coords
    u8 old_x, old_y;
    u8 width, height;
    u8 health;     // Hits remaining
    u8 state;      // 0=sleeping, 1=moving/attacking, 2=exploding
    u8 timer;
    u8 hit_timer;
} boss_t;

typedef struct {
    u8  lives;
    u16 score;
    u8  current_level;
    u8  bricks[BRICK_ROWS][BRICK_COLS];
    u8  active_bricks;
    u8  score_str[7];
    u8  level_cleared; // 1 if this player just finished a level
} player_state_t;

paddle_t paddle;
ball_t ball;
drop_t drop;
laser_pair_t lasers[MAX_LASER_PAIRS];
ball_t extra_balls[MAX_EXTRA_BALLS];
enemy_t enemies[MAX_ENEMIES];
boss_t boss;
// --- Game State and Flags ---
u8 brick_map[BRICK_ROWS][BRICK_COLS];
u8 bricks[BRICK_ROWS][BRICK_COLS];
u16 enemy_spawn_timer;
u8 current_level;
u8 lives;
u8 active_bricks;
u16 score;
u8 score_str[7] = "000000";
u8 hud_dirty;     // 1 = HUD needs redraw this frame (score/lives/level changed)
u8 paused;        // 1 = game is paused
u8 music_on;      // 1 = music+sfx playing, 0 = muted (M key toggle)

// --- 2-Player Mode Globals ---
player_state_t players[2];
u8 num_players = 1;     // 1 or 2
u8 current_player = 0;  // 0 or 1
u8 level_cleared = 0;   // Global flag to trigger level transition

// --- Input and Edge Detection ---
u8 key_m_held;    // Edge detection for M key
u8 key_esc_held;  // Edge detection for ESC key (Toggle pause)

// --- Power-up States ---
u8 glue_active;   // 1 = next paddle hit sticks the ball
u16 glue_timer;   // Countdown frames until auto-launch (used when ball is stuck)
u8 expand_active; // 1 = paddle is currently expanded
u8 slow_active;   // 1 = ball speed was reset/slowed
u8 freeze_active; // 1 = paddle is frozen (ice trap)
u16 freeze_timer;  // frames until ice melts
u8 magnet_active; // 1 = paddle repels ball (magnet trap)
u8 autopilot_active; // 1 = paddle follows ball automatically
u16 autopilot_timer; // duration of autopilot
u8 laser_active;       // Power-up flag
u8 laser_fire_timer;   // Rate-limiter (~0.3s between bursts)
u8 drunk_active;     // 1 = controls inverted (D trap)
u8 gravity_active;   // 1 = ball falls faster (G trap)
u8 fireball_active;  // 1 = ball pierces bricks (F benefit)
u8 victory_walk;     // 1 = boss dead, just walk to exit
u16 victory_palette_timer;
u8 victory_palette_offset;
u8 ghost_timer;      // internal timer for ghost
u8 ghost_period = 8; // period for ghost blinking
u8 fast_active;      // 1 = ball is extra fast (V trap)
u8 door_open;     // 1 = exit door is open
// Door Energy Field Animation State
#define DOOR_ANIM_SPEED 4
u8 door_anim_frame;
u8 door_anim_timer;
u8 launch_timer;  // Counts down from 250 (5s at 50Hz); ball auto-launches at 0
u8 game_won;      // 1 = player defeated the boss

// --- Redefinable Controls ---
u16 g_key_left  = Key_CursorLeft;
u16 g_key_right = Key_CursorRight;
u16 g_key_fire  = (u16)Key_Space;
u16 g_key_pause = (u16)Key_Esc;
u16 g_key_music = (u16)Key_M;
u8  g_use_joystick = 0; // 0 = Keyboard only, 1 = Keyboard + Joystick

// Pre-computed background rows for blazing fast rendering without stack allocation overhead
u8 bg_row_cache[8][80];

void initBackgroundCache() {
    u8 r, c;
    if (current_level == NUM_LEVELS - 1) {
        // Boss level: all black
        for (r = 0; r < 8; r++) {
            for (c = 0; c < 80; c++) {
                bg_row_cache[r][c] = 0;
            }
        }
    } else {
        u8 p_idx = (current_level % NUM_BG_PATTERNS); // Use modulo to stay within bounds [0..3]
        const u8* pattern = bg_patterns[p_idx];
        for (r = 0; r < 8; r++) {
            const u8* pat_row = pattern + (r << 2); 
            for (c = 0; c < 80; c++) {
                bg_row_cache[r][c] = pat_row[c & 3];
            }
        }
    }
}

// Background drawing helper function.
// Redraws a portion of the screen perfectly aligned to the current level's tiled pattern.
void drawBackgroundRect(u8 x, u8 y, u8 w, u8 h) {
    u8 r;
    u8* mem;
    if (w == 0 || h == 0 || w > 80) return;

    // Safety guard: prevent drawing beyond the screen bottom (wrap-around to HUD)
    if (y >= 200) return;
    if (y + h > 200) {
        h = 200 - y;
    }

    for (r = 0; r < h; r++) {
        mem = cpct_getScreenPtr((void*)0xC000, x, y + r);
        // Copy the precalculated pattern row for this specific scanline (y+r)
        cpct_memcpy(mem, &bg_row_cache[(y + r) & 7][x], w);
    }
}

// Pre-computed brick pixel positions (filled once by initBrickPositions from defines)
u8 brick_x[BRICK_COLS];
u8 brick_y[BRICK_ROWS];

// Fast Lookup Tables for instant coordinate-to-brick-index conversion
// O(1) complexity replaces O(COLS) and O(ROWS) searches
u8 g_x_to_col[80];
u8 g_y_to_row[200];

// Direct erase tracking: set when a brick is destroyed, read and cleared in drawGame()
#define MAX_ERASE 12
u8 erase_r[MAX_ERASE];
u8 erase_c[MAX_ERASE];
u8 erase_count;

#define MAX_FLASH 10
u8 flash_count;
u8 flash_r[MAX_FLASH];
u8 flash_c[MAX_FLASH];
u8 flash_timer[MAX_FLASH];

u16 rng_seed; // Galois LFSR seed (must be non-zero)

// --- Difficulty and Spawning ---
u8 enemy_spawn_threshold;


// =============================================================================
// Amstrad CPC Color Table
// A palette in mode Mode 0 will be 16 colors from this table.
// =============================================================================
// FW  HW    Colour Name        R%  G%  B%   Hex         RGB
// --  ----  -----------------  --  --  ---  -------     -----------
// 0   54h   Black              0   0   0    #000000     0,   0,   0
// 1   44h*  Blue               0   0   50   #000080     0,   0, 128
// 2   55h   Bright Blue        0   0   100  #0000FF     0,   0, 255
// 3   5Ch   Red                50  0   0    #800000   128,   0,   0
// 4   58h   Magenta            50  0   50   #800080   128,   0, 128
// 5   5Dh   Mauve              50  0   100  #8000FF   128,   0, 255
// 6   4Ch   Bright Red         100 0   0    #FF0000   255,   0,   0
// 7   45h*  Purple             100 0   50   #FF0080   255,   0, 128
// 8   4Dh   Bright Magenta     100 0   100  #FF00FF   255,   0, 255
// 9   56h   Green              0   50  0    #008000     0, 128,   0
// 10  46h   Cyan               0   50  50   #008080     0, 128, 128
// 11  57h   Sky Blue           0   50  100  #0080FF     0, 128, 255
// 12  5Ee   Yellow             50  50  0    #808000   128, 128,   0
// 13  40h*  White              50  50  50   #808080   128, 128, 128
// 14  5Fh   Pastel Blue        50  50  100  #8080FF   128, 128, 255
// 15  4Eh   Orange             100 50  0    #FF8000   255, 128,   0
// 16  47h   Pink               100 50  50   #FF8080   255, 128, 128
// 17  4Fh   Pastel Magenta     100 50  100  #FF80FF   255, 128, 255
// 18  52h   Bright Green       0   100 0    #00FF00     0, 255,   0
// 19  42h*  Sea Green          0   100 50   #00FF80     0, 255, 128
// 20  53h   Bright Cyan        0   100 100  #00FFFF     0, 255, 255
// 21  5Ah   Lime               50  100 0    #80FF00   128, 255,   0
// 22  59h   Pastel Green       50  100 50   #80FF80   128, 255, 128
// 23  5Bh   Pastel Cyan        50  100 100  #80FFFF   128, 255, 255
// 24  4Ah   Bright Yellow      100 100 0    #FFFF00   255, 255,   0
// 25  43h*  Pastel Yellow      100 100 50   #FFFF80   255, 255, 128
// 26  4Bh   Bright White       100 100 100  #FFFFFF   255, 255, 255
// (* alternative hw numbers: 44h/50h, 45h/48h, 40h/41h, 42h/51h, 43h/49h)

// =========================================================================
// GAME PALETTE MAPPING (Mode 0: 16 colors)
// This is perfectly synced with the 'chars' mapping in Python scripts.
// =========================================================================
//  0: '.' -> HW_BLACK          (Backgrounds and transparent pixel empty space)    #000000
//  1: 'B' -> HW_BRIGHT_BLUE    (Paddle border / neon grid patterns)               #0000FF
//  2: 'W' -> HW_BRIGHT_WHITE   (Ball, bright highlights, powerups)                #FFFFFF
//  3: 'R' -> HW_BRIGHT_RED     (Brick type 0, paddle tips, circuit patterns)      #FF0000
//  4: 'Y' -> HW_BRIGHT_YELLOW  (Brick type 1)                                     #FFFF00
//  5: 'G' -> HW_BRIGHT_GREEN   (Brick type 2)                                     #00FF00
//  6: 'C' -> HW_BRIGHT_CYAN    (Brick type 3, cyan patterns)                      #00FFFF
//  7: 'M' -> HW_BRIGHT_MAGENTA (Brick type 4)                                     #FF00FF
//  8: 'r' -> HW_RED            (Brick type 5, darker reds)                        #800000
//  9: 'g' -> HW_GREEN          (Brick type 6, darker greens)                      #008000
// 10: 'c' -> HW_CYAN           (Brick type 7, darker cyans)                       #008080
// 11: 'w' -> HW_WHITE          (Silver brick, walls, paddle body)                 #FFFFFF
// 12: 'b' -> HW_BLUE           (Available extra Blue color)                       #0000FF
// 13: 'o' -> HW_ORANGE         (Orange - new decorative color)                    #FF8000
// 14: 'm' -> HW_MAUVE          (Mauve - new decorative color)                     #8000FF
// 15: 'y' -> HW_YELLOW         (Gold Blocks indestructible / very hard hits)      #808000
// =========================================================================

#define PLT_BLACK          0
#define PLT_BRIGHT_BLUE    1
#define PLT_BRIGHT_WHITE   2
#define PLT_BRIGHT_RED     3
#define PLT_BRIGHT_YELLOW  4
#define PLT_BRIGHT_GREEN   5
#define PLT_BRIGHT_CYAN    6
#define PLT_BRIGHT_MAGENTA 7
#define PLT_RED            8
#define PLT_GREEN          9
#define PLT_CYAN           10
#define PLT_WHITE          11
#define PLT_BLUE           12
#define PLT_ORANGE         13
#define PLT_MAUVE          14
#define PLT_YELLOW         15

const u8 default_palette[16] = {
    HW_BLACK,           // 0: '.' 
    HW_BRIGHT_BLUE,     // 1: 'B'
    HW_BRIGHT_WHITE,    // 2: 'W'
    HW_BRIGHT_RED,      // 3: 'R'
    HW_BRIGHT_YELLOW,   // 4: 'Y'
    HW_BRIGHT_GREEN,    // 5: 'G'
    HW_BRIGHT_CYAN,     // 6: 'C'
    HW_BRIGHT_MAGENTA,  // 7: 'M'
    HW_RED,             // 8: 'r'
    HW_GREEN,           // 9: 'g'
    HW_CYAN,            // 10: 'c'
    HW_WHITE,           // 11: 'w'
    HW_BLUE,            // 12: 'b'
    HW_ORANGE,          // 13: 'o'
    HW_MAUVE,           // 14: 'm'
    HW_YELLOW,          // 15: 'y'
};



u8 hw_palette[16];
const i8 enemy_dx_lut[16] = { 1, 1, 1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1 };

// Pending capsule erase (set when drop deactivates so drawGame can clean up)
u8 drop_erase_pending;
u8 drop_erase_x;
u8 drop_erase_y;

// -----------  Level Data  -----------
// Brick Color Mapping 'c' for N(c), H(c), etc:
// 0: Red        4: Magenta
// 1: Yellow     5: Dark Red
// 2: Green      6: Dark Green
// 3: Cyan       7: Dark Cyan
//

// Each brick = 1 byte: BRICK(type, color, power)
#define _  BRICK(BTYPE_EMPTY, 0, 0)  // Shortcut for empty cell
#define N(c) BRICK(BTYPE_NORMAL, c, 0)  // Normal brick, color c
#define H(c) BRICK(BTYPE_HARD, c, 0)    // Hard brick, color c
#define NP(c,p) BRICK(BTYPE_NORMAL, c, p) // Normal brick with power-up
#define G(c) BRICK(BTYPE_GOLD, c, 0)    // Gold brick, color c

const u8 level_data[NUM_LEVELS][BRICK_ROWS][BRICK_COLS] = {
    { // Level 1: "The Wall" - Simple first level
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    N(0), N(1), N(2), N(3), N(0), N(0), _    },
        { _,    N(1), N(2), N(3), N(0), N(0), N(1), _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    N(1), N(2), N(3), N(1), N(2), N(3), _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
    },
    { // Level 2: "The Vault" - Hard shell with hidden Laser and Expand
        { H(3), H(3), H(3), H(3), H(3), H(3), H(3), H(3) },
        { H(3), N(0), N(1), N(2), N(3), N(0), N(0), H(3) },
        { H(3), N(1), NP(2, BPOWER_LASER), N(3), N(0), N(0), N(1), H(3) },
        { H(3), N(2), _,    _,    _,    _,    N(2), H(3) },
        { H(3), N(3), _,    _,    _,    _,    N(3), H(3) },
        { H(3), N(0), N(0), NP(1, BPOWER_EXPAND), N(2), N(3), N(0), H(3) },
        { H(3), N(0), N(1), N(2), N(3), N(0), N(0), H(3) },
        { H(3), H(3), H(3), H(3), H(3), H(3), H(3), H(3) },
    },
    { // Level 3: "Space Invader" - Tribute to the classic arcade icon
        { _,    _,    N(2), N(2), N(2), N(2), _,    _    },
        { _,    N(2), N(2), N(2), N(2), N(2), N(2), _    },
        { N(2), N(2), N(2), N(2), N(2), N(2), N(2), N(2) },
        { N(2), N(2), _,    N(2), N(2), _,    N(2), N(2) },
        { N(2), N(2), N(2), N(2), N(2), N(2), N(2), N(2) },
        { _,    _,    N(2), NP(1, BPOWER_MULTI), NP(1, BPOWER_MULTI), N(2), _,    _   },
        { _,    N(2), _,    NP(2, BPOWER_ICE), NP(2, BPOWER_MAGNET), _,    N(2), _    },
        { N(2), _,    _,    _,    _,    _,    _,    N(2) },
    },
    { // Level 4: "The Gauntlet" - Precision movement required
        { H(3), _,    _,    _,    _,    _,    _,    H(3) },
        { _,    H(3), _,    _,    _,    _,    H(3), _    },
        { _,    _,    H(3), NP(0, BPOWER_LIFE), NP(0, BPOWER_SLOW), H(3), _,    _    },
        { _,    _,    N(1), N(1), N(1), N(1), _,    _    },
        { _,    _,    N(2), N(2), N(2), N(2), _,    _    },
        { _,    H(3), _,    _,    _,    _,    H(3), _    },
        { H(3), _,    NP(0, BPOWER_WARP), _,    _,    NP(0, BPOWER_WARP), _,    H(3) },
        { H(3), H(3), H(3), H(3), H(3), H(3), H(3), H(3) },
    },
    { // Level 5: "The Diamond"
        { _,    _,    _,    H(3), H(3), _,    _,    _    },
        { _,    _,    N(0), H(3), H(3), N(0), _,    _    },
        { _,    N(1), N(1), H(3), H(3), N(1), N(1), _    },
        { N(2), N(2), N(2), G(3), G(3), NP(2, BPOWER_DRUNK), N(2), N(2) },
        { N(3), N(3), N(3), G(3), G(3), N(3), N(3), N(3) },
        { _,    N(0), N(0), H(3), H(3), N(0), N(0), _    },
        { _,    _,    N(1), H(3), H(3), N(1), _,    _    },
        { _,    _,    _,    H(3), H(3), _,    _,    _    },
    },
    { // Level 6: "The Triangle"
        { N(0), _,    _,    _,    _,    _,    _,    _    },
        { N(1), N(1), _,    _,    _,    _,    _,    _    },
        { N(2), N(2), N(2), _,    _,    _,    _,    _    },
        { N(3), N(3), N(3), NP(3, BPOWER_MULTI), _, _, _, _ },
        { N(0), N(0), N(0), N(0), N(0), _,    _,    _    },
        { N(1), N(1), N(1), N(1), N(1), NP(1, BPOWER_LASER), _, _ },
        { N(2), NP(2, BPOWER_ICE), NP(2, BPOWER_MAGNET), N(2), N(2), N(2), N(2), _    },
        { H(3), H(3), H(3), H(3), H(3), H(3), H(3), H(3) },
    },
    { // Level 7: "The Checkerboard" - A classic geometric challenge
        { N(1), _,    N(2), _,    N(3), _,    NP(0, BPOWER_ICE), _    },
        { _,    NP(1, BPOWER_MAGNET), _,    N(2), _,    N(3), _,    N(0) },
        { N(2), _,    NP(3, BPOWER_GLUE), _,    N(0), _,    N(1), _    },
        { _,    N(2), _,    N(3), _,    N(0), _,    N(1) },
        { N(3), _,    N(0), _,    N(1), _,    NP(2, BPOWER_MULTI), _    },
        { _,    N(3), _,    N(0), _,    N(1), _,    N(2) },
        { N(0), _,    N(1), _,    N(2), _,    NP(3, BPOWER_ICE), _    },
        { _,    NP(0, BPOWER_MAGNET), _,    N(1), _,    N(2), _,    N(3) },
    },
    { // Level 8: "The Beer Mug" - Refreshing mid-game snack
        { _,    H(0),  H(0),  H(0),  H(0),  _,    _,    _    }, // Foam (Hard/White)
        { _,    H(0),  H(0),  H(0),  H(0),  _,    _,    _    }, // Foam (Hard/White)
        { _,    N(1),  N(1),  N(1),  N(1),  H(0), H(0), _    }, // Beer + Handle top
        { _,    NP(1, BPOWER_DRUNK),  N(1),  N(1),  N(1),  _,    H(0), _    }, // Beer + Handle middle
        { _,    NP(1, BPOWER_ICE), N(1), N(1), N(1), H(0), H(0), _    }, // Beer + Handle bottom
        { _,    N(1),  N(1),  N(1),  N(1),  _,    _,    _    }, // Beer
        { _,    N(1),  N(1),  N(1),  N(1),  _,    _,    _    }, // Beer
        { _,    H(0),  H(0),  H(0),  H(0),  _,    _,    _    }, // Mug base
    },
    { // Level 9: "Pacman" - Waka waka
        { _,    _,    N(1), N(1), N(1), N(1), _,    _    },
        { _,    N(1), N(1), N(1), N(1), N(1), N(1), _    },
        { N(1), N(1), N(1), N(1), N(1), N(1), N(1), _    },
        { N(1), N(1), N(1), N(1), N(1), N(1), _,    _    },
        { NP(1, BPOWER_TINY), N(1), N(1), NP(1, BPOWER_ICE), _,    _,    _,    _    },
        { N(1), N(1), N(1), N(1), N(1), N(1), _,    _    },
        { _,    N(1), N(1), N(1), N(1), N(1), N(1), _    },
        { _,    _,    N(1), NP(1, BPOWER_DRUNK), N(1), N(1), _,    _    },
    },
    { // Level 10: "The Boss Room"
        // Top 3 rows empty for boss movement
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
        { _,    _,    _,    _,    _,    _,    _,    _    },
    },
};

// Undefine shortcut macros to avoid polluting the namespace
#undef _
#undef N
#undef H
#undef NP

// Initial ball direction variables (can be modified dynamically)
i16 initial_dir_x; // DO NOT INITIALIZE HERE (e.g. = 55) because of --no-std-crt0
i16 initial_dir_y; // Initialize them inside initGame() or other functions instead

void drawBackground(); // Forward declaration
void initBrickPositions(); // Forward declaration
void flashBrick(u8 r, u8 c); // Forward declaration
void uint16_to_str(u16 value, u8* buffer); // Forward declaration
void drawHUD(); // Forward declaration
void nextLevel(); // Forward declaration
void drawDoor(); // Forward declaration
void initPaddle();
void initBall();
void initEnemies();
void initBoss();
void updateBoss();
void checkBossCollision(ball_t* b);
void updateExtraBalls(); // Forward declaration
void updateBallVelocity(); // Forward declaration
void openDoor(); // Forward declaration
void updateVictoryPalette();
void resetPowerups(); // Forward declaration
void spawnMultiBalls(); // Forward declaration
void closeDoor(); // Forward declaration
void initGame();          // Forward declaration
void savePlayerState(u8 p);
void loadPlayerState(u8 p);
void switchPlayer();
// Redundant forward declaration removed: showModeSelect
void showPlayerStart(u8 p);
void initEnemies(); // Forward declaration
void updateEnemies(); // Forward declaration
void drawEnemies(); // Forward declaration
u8 showVictory(); // Forward declaration
u8 showControls(); // Forward declaration
u8 showCapsuleDocs(); // Forward declaration
void drawIntroContent(); // Forward declaration
u16 getNormalBallSpeed(); // Forward declaration
u8 getSpriteIndex(const u8** text_ptr); // Forward declaration
u8 getCenteredX(const u8* text, u8 bytes_per_char); // Forward declaration
u8 isAnyBallActive(); // Forward declaration
u8 areExtrasActive(); // Forward declaration

// Shared ball physics helpers (operate on a ball_t* to avoid duplication)
void moveBall(ball_t *b);
u8   bouncePaddle(ball_t *b);  // Returns 1 if paddle was hit
u8   collideBricks(ball_t *b); // Returns 1 if level cleared (nextLevel called)
u8   handleBallPhysics(ball_t *b); // Unified physics for all balls

// Compute brick pixel positions once from #defines, so the hot path uses array lookups
void initBrickPositions() {
    u8 i, r, c;
    for (i = 0; i < BRICK_COLS; i++)
        brick_x[i] = BRICK_START_X + i * (BRICK_WIDTH_BYTES + BRICK_GAP_X);
    for (i = 0; i < BRICK_ROWS; i++)
        brick_y[i] = BRICK_START_Y + i * (BRICK_HEIGHT + BRICK_GAP_Y);
    
    // Reset LUTs to "no brick" value
    for (i = 0; i < 80; i++)  g_x_to_col[i] = 0xFF;
    for (i = 0; i < 200; i++) g_y_to_row[i] = 0xFF;

    // Populate X to Column LUT
    for (c = 0; c < BRICK_COLS; c++) {
        for (i = 0; i < BRICK_WIDTH_BYTES; i++) {
            u8 x = brick_x[c] + i;
            if (x < 80) g_x_to_col[x] = c;
        }
    }

    // Populate Y to Row LUT
    for (r = 0; r < BRICK_ROWS; r++) {
        for (i = 0; i < BRICK_HEIGHT; i++) {
            u8 y = brick_y[r] + i;
            if (y < 200) g_y_to_row[y] = r;
        }
    }

    erase_count = 0;
}

// Galois LFSR: fast Z80 RNG using only shifts and XOR (no multiplication)
u8 rand8() {
    u8 i, lsb;
    for (i = 0; i < 8; i++) {
        lsb = rng_seed & 1;
        rng_seed >>= 1;
        if (lsb) rng_seed ^= 0xB400; // Maximal-length taps for 16-bit LFSR
    }
    return (u8)(rng_seed & 0xFF);
}

// Fan two extra balls from the highest currently active ball.
// Called when the player catches the MULTI power-up capsule.
void spawnMultiBalls() {
    u8 eb, j;
    i16 src_dy;
    ball_t *source = &ball;

    // Find the highest (lowest Y value) active ball to use as spawn origin
    if (!ball.active) source = 0;
    for (j = 0; j < MAX_EXTRA_BALLS; j++) {
        if (extra_balls[j].active) {
            if (source == 0 || extra_balls[j].y < source->y)
                source = &extra_balls[j];
        }
    }

    // If the main ball is inactive, reactivate it at the source position
    if (!ball.active) {
        ball.active = 1;
        if (source) {
            ball.x = source->x;  ball.y = source->y;
            ball.fx = source->fx; ball.fy = source->fy;
            ball.speed = source->speed;
            ball.dir_x = source->dir_x;
            ball.dir_y = source->dir_y;
        } else {
            // No ball at all — launch from paddle center
            ball.x = paddle.x + (paddle.width / 2) - (ball.width / 2);
            ball.y = paddle.y - ball.height;
            ball.fx = (i16)ball.x * FP_SCALE;
            ball.fy = (i16)ball.y * FP_SCALE;
            ball.dir_x = initial_dir_x;
            ball.dir_y = initial_dir_y;
        }
        updateBallVelocity();
    }

    // Fan two extra balls symmetrically left and right of the main ball direction
    src_dy = (ball.dir_y < 0) ? ball.dir_y : -ball.dir_y;
    for (eb = 0; eb < MAX_EXTRA_BALLS; eb++) {
        if (!extra_balls[eb].active) {
            extra_balls[eb].old_x = ball.x;
            extra_balls[eb].old_y = ball.y;
        }
        extra_balls[eb].x      = ball.x;     extra_balls[eb].y      = ball.y;
        extra_balls[eb].fx     = ball.fx;    extra_balls[eb].fy     = ball.fy;
        extra_balls[eb].width  = ball.width; extra_balls[eb].height = ball.height;
        extra_balls[eb].speed  = ball.speed; extra_balls[eb].active = 1;
        extra_balls[eb].dir_x  = ball.dir_x + ((eb == 0) ? -18 : 18);
        extra_balls[eb].dir_y  = src_dy;
        extra_balls[eb].dx = FP_VEL(extra_balls[eb].dir_x, extra_balls[eb].speed);
        extra_balls[eb].dy = FP_VEL(extra_balls[eb].dir_y, extra_balls[eb].speed);
    }
}

// Apply effect when player catches a power-up capsule.
// Cancels conflicting power-ups and activates the new one (permanent until next level).
void applyPowerup(u8 ptype) {
    u8 i;
    // SFX: power-up caught — instrument 3 on multiple channels (A+C)
    cpct_akp_SFXPlay(3, 15, 72, 0, 40, AY_CHANNEL_ALL);

    // Cancel conflicting power-ups and consolidate balls when a new one is picked.
    if (ptype != BPOWER_LIFE && ptype != BPOWER_MULTI) {
        // Find the highest active ball (smallest Y) to keep it as the main ball
        u8 best_y = 255;
        i8 best_idx = -2; // -2: none, -1: main, 0..N: extra_balls
        
        // --- Explicitly erase ALL balls immediately to prevent ghost trails ---
        if (ball.active && ball.old_x != 0xFF) {
            if (ball.old_y <= SCREEN_HEIGHT - ball.height && ball.old_x < 80 && ball.old_y >= WALL_TOP) {
                drawBackgroundRect(ball.old_x, ball.old_y, ball.width, ball.height);
            }
            ball.old_x = 0xFF; // Prevent drawGame from double-erasing
        }
        for (i = 0; i < MAX_EXTRA_BALLS; i++) {
            if (extra_balls[i].active && extra_balls[i].old_x != 0xFF) {
                if (extra_balls[i].old_y <= SCREEN_HEIGHT - extra_balls[i].height && extra_balls[i].old_x < 80 && extra_balls[i].old_y >= WALL_TOP) {
                    drawBackgroundRect(extra_balls[i].old_x, extra_balls[i].old_y, extra_balls[i].width, extra_balls[i].height);
                }
                extra_balls[i].old_x = 0xFF;
            }
        }
        // ----------------------------------------------------------------------
        
        if (ball.active) {
            best_y = ball.y;
            best_idx = -1;
        }
        
        for (i = 0; i < MAX_EXTRA_BALLS; i++) {
            if (extra_balls[i].active && extra_balls[i].y < best_y) {
                best_y = extra_balls[i].y;
                best_idx = (i8)i;
            }
        }

        // If an extra ball was highest, promote it to main ball
        if (best_idx >= 0) {
            cpct_memcpy(&ball, &extra_balls[best_idx], sizeof(ball_t));
        }
        
        // Deactivate extra balls
        for (i = 0; i < MAX_EXTRA_BALLS; i++) extra_balls[i].active = 0;
        
        // Ensure main ball is active if we had any ball in play
        if (best_idx != -2) ball.active = 1;

        glue_active = 0;
        laser_active = 0;
        laser_fire_timer = 0;
        freeze_active = 0;
        magnet_active = 0;
        // Expand is cancelled by anything except Expand itself
        if (ptype != BPOWER_EXPAND) {
            paddle.width = PADDLE_WIDTH_BYTES;
            expand_active = 0;
        }
        if (slow_active) {
            ball.speed = getNormalBallSpeed();
            updateBallVelocity();
        }
        slow_active = 0;
        autopilot_active = 0;
        autopilot_timer = 0;
        drunk_active = 0;
        gravity_active = 0;
        fireball_active = 0;
        fast_active = 0;
    }

    // Activate new power-up
    if (ptype == BPOWER_AUTOPILOT) {
        autopilot_active = 1;
        autopilot_timer = 300; // ~6 seconds at 50Hz
    } else if (ptype == BPOWER_DRUNK) {
        drunk_active = 1;
    } else if (ptype == BPOWER_FAST) {
        fast_active = 1;
        ball.speed += (ball.speed >> 1); // +50% speed
        updateBallVelocity();
    } else if (ptype == BPOWER_TINY) {
        paddle.width = PADDLE_WIDTH_TINY; 
    } else if (ptype == BPOWER_GRAVITY) {
        gravity_active = 1;
    } else if (ptype == BPOWER_FIREBALL) {
        fireball_active = 1;
    } else if (ptype == BPOWER_LASER) {
        laser_active = 1;
        // Clean up any still-flying lasers from the old laser pickup
        for (i = 0; i < MAX_LASER_PAIRS; i++) { if (lasers[i].active) { lasers[i].active = 0; lasers[i].needs_bg_repaint = 1; } }
    } else if (ptype == BPOWER_GLUE) {
        glue_active = 1;
    } else if (ptype == BPOWER_LIFE) {
        if (lives < 9) { lives++; hud_dirty = 1; }
    } else if (ptype == BPOWER_SLOW) {
        if (!slow_active) {
            slow_active = 1;
            ball.speed >>= 1; // Halve speed
            updateBallVelocity();
        }
    } else if (ptype == BPOWER_WARP) {
        openDoor(); // Open the exit door on the right wall
    } else if (ptype == BPOWER_MULTI) {
        spawnMultiBalls(); // Fan extra balls from the highest active ball
    } else if (ptype == BPOWER_EXPAND) {
        // Widen the paddle (only if not already wide, to avoid going past the right wall)
        if (paddle.width != PADDLE_WIDTH_WIDE) {
            if (paddle.x + PADDLE_WIDTH_WIDE > WALL_RIGHT_BYTES) {
                paddle.x = WALL_RIGHT_BYTES - PADDLE_WIDTH_WIDE;
            }
            paddle.width = PADDLE_WIDTH_WIDE;
        }
        expand_active = 1;
    } else if (ptype == BPOWER_ICE) {
        freeze_active = 1;
        freeze_timer = 150; // 3 seconds at 50Hz
    } else if (ptype == BPOWER_MAGNET) {
        magnet_active = 1;
    } else if (ptype == BPOWER_AUTOPILOT) {
        // Handled above at line 743
    }

    hud_dirty = 1;
}

// Fire a laser pair (left + right beams) from the paddle edges, using a free slot.
void fireLaser() {
    u8 i;
    for (i = 0; i < MAX_LASER_PAIRS; i++) {
        if (!lasers[i].active) {
            lasers[i].x_left  = paddle.x;
            lasers[i].x_right = paddle.x + paddle.width - LASER_WIDTH;
            lasers[i].y       = paddle.y - LASER_HEIGHT;
            lasers[i].old_y   = lasers[i].y;
            lasers[i].active  = 1;
            lasers[i].needs_bg_repaint = 0;
            break;
        }
    }
}

// Apply the effect of a laser beam hitting brick at (r, c).
// Returns 1 if all bricks cleared (nextLevel already called), 0 otherwise.
u8 laserHitBrick(u8 r, u8 c) {
    if (bricks[r][c] == BSTATE_HARD) {
        bricks[r][c] = BSTATE_NORMAL;
        score += 2;
        flashBrick(r, c);
    } else if (bricks[r][c] == BSTATE_GOLD) {
        // Gold brick: indestructible — just flash, laser is consumed
        flashBrick(r, c);
    } else {
        // Normal brick: destroy and check for level clear
        bricks[r][c] = BSTATE_NEEDS_ERASE;
        if (erase_count < MAX_ERASE) {
            erase_r[erase_count] = r;
            erase_c[erase_count] = c;
            erase_count++;
        }
        score += 5;
        active_bricks--;
        cpct_akp_SFXPlay(8, 15, 77, 0, 0, AY_CHANNEL_ALL); // F5 // SFX: brick destroyed by laser
    }
    hud_dirty = 1;
    if (active_bricks == 0) {
        nextLevel();
        return 1;
    }
    return 0;
}

// Move laser pairs upward and check brick/enemy collisions.
// Each pair has a shared Y and two X positions (left and right beam).
// Collision uses precomputed brick columns to avoid a full cols×rows scan.
void updateLasers() {
    u8 i, r, e;
    for (i = 0; i < MAX_LASER_PAIRS; i++) {
        if (!lasers[i].active) continue;
        
        lasers[i].old_y = lasers[i].y;
        lasers[i].needs_bg_repaint = 1;
        
        // Hit top wall
        if (lasers[i].y <= WALL_TOP + LASER_SPEED) {
            lasers[i].y = WALL_TOP;
            lasers[i].active = 0;
            continue;
        }
        lasers[i].y -= LASER_SPEED;

        // Enemy collision (check both beams against each enemy)
        for (e = 0; e < MAX_ENEMIES; e++) {
            if (!enemies[e].active || !lasers[i].active) continue;
            if (lasers[i].y <= enemies[e].y + ENEMY_HEIGHT && lasers[i].y >= enemies[e].y) {
                if ((lasers[i].x_left  + LASER_WIDTH >= enemies[e].x && lasers[i].x_left  <= enemies[e].x + ENEMY_WIDTH) ||
                    (lasers[i].x_right + LASER_WIDTH >= enemies[e].x && lasers[i].x_right <= enemies[e].x + ENEMY_WIDTH)) {
                    enemies[e].active = 0;
                    lasers[i].active = 0;
                    score += 50;
                    hud_dirty = 1;
                    cpct_akp_SFXPlay(13, 15, 48, 0, 0, AY_CHANNEL_ALL);
                }
            }
        }
        if (!lasers[i].active) continue;

        // --- LUT-based brick collision ---
        // Instantly find the row and column of each beam
        r = g_y_to_row[lasers[i].y];
        if (r != 0xFF) {
            u8 col_l = g_x_to_col[lasers[i].x_left];
            u8 col_r = g_x_to_col[lasers[i].x_right];
            
            // Check left beam
            if (col_l != 0xFF && bricks[r][col_l] != BSTATE_EMPTY && bricks[r][col_l] != BSTATE_NEEDS_ERASE) {
                lasers[i].active = 0;
                if (laserHitBrick(r, col_l)) return;
            }
            // Check right beam
            if (lasers[i].active && col_r != 0xFF && bricks[r][col_r] != BSTATE_EMPTY && bricks[r][col_r] != BSTATE_NEEDS_ERASE) {
                lasers[i].active = 0;
                if (laserHitBrick(r, col_r)) return;
            }
        }
    }
}

// Move the falling capsule down one step and check paddle collision
void updateDrop() {
    if (!drop.active) return;
    // Throttle: only move 1px every 3 frames
    if (drop.timer > 0) { drop.timer--; return; }
    drop.timer = 1; // Move 1px every 2 frames (~25px/sec)
    drop.y++;
    // Caught by paddle
    if (drop.y + DROP_HEIGHT >= paddle.y &&
        drop.y <= paddle.y + paddle.height &&
        drop.x + DROP_WIDTH >= paddle.x &&
        drop.x <= paddle.x + paddle.width) {
        applyPowerup(drop.power_type);
        drop.active = 0;
        drop_erase_pending = 1;
        drop_erase_x = drop.x;
        drop_erase_y = drop.old_y; // old_y = last position DRAWN on screen (y is new, not drawn yet)
        return;
    }
    // Fell off the bottom
    if (drop.y >= SCREEN_HEIGHT) {
        drop.active = 0;
        drop_erase_pending = 0; // Already off screen, no erase needed
    }
}

// Custom lightweight unsigned integer to string (no stdio.h sprintf)
void uint16_to_str(u16 value, u8* buffer) {
    u8 i;
    // Extract 5 digits (0-65535 max) from right to left
    for (i = 5; i > 0; i--) {
        buffer[i - 1] = '0' + (value % 10);
        value /= 10;
    }
    buffer[5] = '\0'; // Null terminator
}

// Helper to get font sprite index from current string pointer (handles UTF-8)
u8 getSpriteIndex(const u8** text_ptr) {
    u8 c = **text_ptr;
    u8 sprite_index = 36; // Default: Space
    if (c >= 'A' && c <= 'Z') sprite_index = c - 'A';
    else if (c >= 'a' && c <= 'z') sprite_index = c - 'a';
    else if (c >= '0' && c <= '9') sprite_index = 26 + (c - '0');
    else if (c == ' ') sprite_index = 36;
    else if (c == '/') sprite_index = 37;
    else if (c == ':') sprite_index = 38;
    else if (c == '.') sprite_index = 39;
    else if (c == '!') sprite_index = 40;
    else if (c == ',') sprite_index = 41;
    else if (c == 0xC2 && (*text_ptr)[1] == 0xA1) { sprite_index = 42; (*text_ptr)++; } // ¡
    else if (c == 0xC3) {
        if ((*text_ptr)[1] == 0x91 || (*text_ptr)[1] == 0xB1) { sprite_index = 43; (*text_ptr)++; } // Ñ/ñ
    }
    return sprite_index;
}

// Helper to calculate X coordinate for centering text
u8 getCenteredX(const u8* text, u8 bytes_per_char) {
    u8 chars = 0;
    const u8* ptr = text;
    while (*ptr != '\0') {
        getSpriteIndex(&ptr); // This moves ptr correctly even for multi-byte
        chars++;
        ptr++;
    }
    return (chars * bytes_per_char >= 80) ? 0 : (80 - (chars * bytes_per_char)) / 2;
}

// Draw a string using our custom arcade sprite font
void drawCustomText(const u8* text, u8 x_bytes, u8 y_lines, u8 color) {
    const u8* ptr = text;
    u8 mask = cpct_px2byteM0(color, color);
    u8 sprite_index, row, col;
    u8* screen_pt;
    const u8* spr_data;

    while(*ptr != '\0') {
        sprite_index = getSpriteIndex(&ptr);
        spr_data = font_sprites[sprite_index];
        for(row = 0; row < 6; row++) {
            if (y_lines + row >= 200) break;
            screen_pt = cpct_getScreenPtr((void*)0xC000, x_bytes, y_lines + row);
            for(col = 0; col < 2; col++) {
                screen_pt[col] = spr_data[row * 2 + col] & mask;
            }
        }
        
        ptr++;
        x_bytes += 2; // Each letter is 2 bytes wide (4 Mode 0 pixels)
    }
}

// Draw a string using our custom arcade sprite font (6x8)
void drawCustomTextLarge(const u8* text, u8 x_bytes, u8 y_lines, u8 color) {
    const u8* ptr = text;
    u8 mask = cpct_px2byteM0(color, color);
    u8 sprite_index, row, col;
    u8* screen_pt;
    const u8* spr_data;

    while(*ptr != '\0') {
        sprite_index = getSpriteIndex(&ptr);
        spr_data = font_large_sprites[sprite_index];
        for(row = 0; row < 8; row++) {
            if (y_lines + row >= 200) break;
            screen_pt = cpct_getScreenPtr((void*)0xC000, x_bytes, y_lines + row);
            for(col = 0; col < 3; col++) {
                screen_pt[col] = spr_data[row * 3 + col] & mask;
            }
        }
        
        ptr++;
        x_bytes += 3; // Each large letter is 3 bytes wide (6 Mode 0 pixels)
    }
}

// ── Centered text helpers ───────────────────────────────────────────────────
// Screen is 80 bytes wide. Small font: 2 bytes/char; Large font: 3 bytes/char.
// Formula: x = (80 - len * bytes_per_char) / 2
// If the string is wider than the screen, x clamps to 0.

void drawCustomTextCentered(const u8* text, u8 y_lines, u8 color) {
    drawCustomText(text, getCenteredX(text, 2), y_lines, color);
}

void drawCustomTextLargeCentered(const u8* text, u8 y_lines, u8 color) {
    drawCustomTextLarge(text, getCenteredX(text, 3), y_lines, color);
}

#ifdef MOVE_TITLE_SPRITE

// Off-screen buffer for the Extra Large splash title (78 bytes wide * 17 pixels high = 1326 bytes)
u8 title_sprite[17 * 78];

// Render a string using our custom arcade sprite font scaled 2x into the title_sprite memory buffer
// Uses logical OR to allow compositing multiple layers (like drop shadows)
void renderTitleSprite(const u8* text, u8 color, u8 x_off_bytes, u8 y_off_lines) {
    const u8* ptr = text;
    u8 x_bytes = 0;
    u8 mask = cpct_px2byteM0(color, color);
    u8 sprite_index, row, col, sub_y;
    const u8* spr_data;

    while(*ptr != '\0') {
        sprite_index = getSpriteIndex(&ptr);
        spr_data = font_large_sprites[sprite_index];
        
        // Manual 2x scaling draw (8 rows * 3 bytes) -> (16 rows * 6 bytes)
        for(row = 0; row < 8; row++) {
            // Draw each row twice to scale vertically
            for(sub_y = 0; sub_y < 2; sub_y++) {
                // Calculate the flat index into the 78-byte-wide sprite array
                u16 y_idx = row * 2 + sub_y + y_off_lines;
                u16 buffer_offset;
                
                if (y_idx >= 17) continue; // Out of bounds safety
                
                buffer_offset = y_idx * 78 + x_bytes + x_off_bytes;
                for(col = 0; col < 3; col++) {
                    u8 val = spr_data[row * 3 + col];
                    u8 p0_dup = ((val & 0xAA) | ((val & 0xAA) >> 1)) & mask;
                    u8 p1_dup = (((val & 0x55) << 1) | (val & 0x55)) & mask;
                    
                    if (buffer_offset < 17 * 78) title_sprite[buffer_offset++] |= p0_dup;
                    if (buffer_offset < 17 * 78) title_sprite[buffer_offset++] |= p1_dup;
                }
            }
        }
        
        ptr++;
        x_bytes += 6; // scaled from 3 to 6 bytes wide
    }
}

// Draw the cached Extra Large title to the screen using fast memory copy
void drawTitleSprite(u8 x_bytes, u8 y_lines) {
    u8 row;
    u8* screen_pt;
    for(row = 0; row < 17; row++) {
        // Out of bounds safety
        if (y_lines + row >= 200) continue;
        screen_pt = cpct_getScreenPtr((void*)0xC000, x_bytes, y_lines + row);
        cpct_memcpy(screen_pt, &title_sprite[row * 78], 78);
    }
}

#else // !MOVE_TITLE_SPRITE

// Draw a string using our custom arcade sprite font scaled 2x directly to screen
// Each glyph: 8 rows * 3 bytes -> 16 lines * 6 bytes (2x horizontal + 2x vertical)
void drawCustomTextXLarge(const u8* text, u8 x_bytes, u8 y_lines, u8 color) {
    const u8* ptr = text;
    u8 mask = cpct_px2byteM0(color, color);
    u8 sprite_index, row, col, sub_y;
    const u8* spr_data;

    while(*ptr != '\0') {
        sprite_index = getSpriteIndex(&ptr);
        spr_data = font_large_sprites[sprite_index];

        // 2x vertical: draw each source row twice
        for(row = 0; row < 8; row++) {
            for(sub_y = 0; sub_y < 2; sub_y++) {
                u8 y = y_lines + row * 2 + sub_y;
                u8* screen_pt;
                if (y >= 200) continue; // bounds safety
                screen_pt = cpct_getScreenPtr((void*)0xC000, x_bytes, y);
                // 2x horizontal: each source byte expands to 2 bytes
                for(col = 0; col < 3; col++) {
                    u8 val = spr_data[row * 3 + col];
                    u8 p0_dup = ((val & 0xAA) | ((val & 0xAA) >> 1)) & mask;
                    u8 p1_dup = (((val & 0x55) << 1) | (val & 0x55)) & mask;
                    screen_pt[col * 2]     |= p0_dup;
                    screen_pt[col * 2 + 1] |= p1_dup;
                }
            }
        }

        ptr++;
        x_bytes += 6; // 3 source bytes -> 6 screen bytes per glyph
    }
}

// Screen is 80 bytes wide. XLarge font: 6 bytes/char.
// Center formula: x = (80 - len * 6) / 2
void drawCustomTextXLargeCentered(const u8* text, u8 y_lines, u8 color) {
    drawCustomTextXLarge(text, getCenteredX(text, 6), y_lines, color);
}

#endif // MOVE_TITLE_SPRITE

// Randomly assign power-ups to normal bricks at level start
void assignPowerups() {
    u8 r, c, rnd;
    rng_seed ^= (u16)(current_level + 1) * 0x1234;
    if (rng_seed == 0) rng_seed = 0xACE1;
    for (r = 0; r < BRICK_ROWS; r++) {
        for (c = 0; c < BRICK_COLS; c++) {
            if (BRICK_TYPE(brick_map[r][c]) == BTYPE_NORMAL) {
                // ~25% chance of having a power-up
                rnd = rand8();
                //if ((rnd & 0x03) == 0) { // 0x03 = 00000011 last 2 bits from 8 are 0, so 1/4 chance
                if(1){
                    // Pick a random power-up type: 1 to NUM_POWERUP_SPRITES
                    // 0 is BPOWER_NONE
                    u8 pwr = 1 + ((rnd >> 2) % NUM_POWERUP_SPRITES);
                    brick_map[r][c] = BRICK(BTYPE_NORMAL, BRICK_COLOR(brick_map[r][c]), pwr);
                }
                //brick_map[r][c] = BRICK(BTYPE_NORMAL, BRICK_COLOR(brick_map[r][c]), BPOWER_ICE);
            }
        }
    }
}

// Loads the current level into brick_map[] and bricks[] runtime state
void loadLevel() {
    u8 r, c, t, b;
    active_bricks = 0;
    for (r = 0; r < BRICK_ROWS; r++) {
        for (c = 0; c < BRICK_COLS; c++) {
            b = level_data[current_level % NUM_LEVELS][r][c];
            brick_map[r][c] = b;
            t = BRICK_TYPE(b);
            if (t == BTYPE_NORMAL) bricks[r][c] = BSTATE_NORMAL;
            else if (t == BTYPE_HARD) bricks[r][c] = BSTATE_HARD;
            else if (t == BTYPE_GOLD) bricks[r][c] = BSTATE_GOLD;
            else bricks[r][c] = BSTATE_EMPTY;

            if (bricks[r][c] == BSTATE_NORMAL || bricks[r][c] == BSTATE_HARD) {
                active_bricks++;
            }
        }
    }
    enemy_spawn_threshold = ((u16)active_bricks * 40) / 100;

    // We are now using bg_patterns from sprites.h for backgrounds.
    // They are natively drawn using drawBackgroundRect.

    drop.active = 0;       // Clear any pending drop
    drop_erase_pending = 0; // Don't erase old position on new screen
    erase_count = 0;        // Clear any pending erases from previous level
    door_open = 0;          // Ensure door starts closed
    resetPowerups();
    assignPowerups(); // Randomly tag some normal bricks with powerups
}

// --- 2-Player State Management ---

void savePlayerState(u8 p) {
    if (p >= 2) return;
    players[p].lives = lives;
    players[p].score = score;
    players[p].current_level = current_level;
    players[p].active_bricks = active_bricks;
    cpct_memcpy(players[p].score_str, score_str, 6);
    cpct_memcpy(players[p].bricks, bricks, sizeof(bricks));
}

void loadPlayerState(u8 p) {
    if (p >= 2) return;
    lives = players[p].lives;
    score = players[p].score;
    current_level = players[p].current_level;
    active_bricks = players[p].active_bricks;
    cpct_memcpy(score_str, players[p].score_str, 6);
    cpct_memcpy(bricks, players[p].bricks, sizeof(bricks));
    
    // Refresh background and objects for the player's level
    initBackgroundCache();
    initPaddle();
    initBall();
    initEnemies();
    if (current_level == NUM_LEVELS - 1) initBoss();
}

void switchPlayer() {
    if (num_players < 2) return;
    
    savePlayerState(current_player);
    current_player = 1 - current_player;
    loadPlayerState(current_player);
    
    // Check if new current player is already dead
    if (lives == 0) {
        // Try to switch back if the other player is alive
        current_player = 1 - current_player;
        loadPlayerState(current_player);
        if (lives == 0) {
            // Both are dead - this should be handled by the game over check elsewhere
            return;
        }
    }
}

// Reset all active power-up states and free laser/extra-ball slots.
// Call this on level start, life lost, or new game.
// NOTE: does NOT touch the drop capsule — callers with a falling capsule handle that separately.
void resetPowerups() {
    u8 i;
    // --- Power-up Reset ---
    glue_active = 0;
    glue_timer = 0;
    expand_active = 0;
    slow_active = 0;
    laser_active = 0;
    laser_fire_timer = 0;
    drunk_active = 0;
    gravity_active = 0;
    fireball_active = 0;
    ghost_timer = 0;
    fast_active = 0;
    freeze_active = 0;
    freeze_timer = 0;
    magnet_active = 0;
    autopilot_active = 0;
    autopilot_timer = 0;
    
    initPaddle(); // Reset paddle width and position
    initBall();   // Ball starts attached to paddle
    initEnemies();
    
    if (current_level == NUM_LEVELS - 1) {
        initBoss();
        door_open = 0; // Forced closed until boss dies
    } else {
        boss.active = 0;
    }

    launch_timer = 250; // 5 seconds at 50Hz before auto-launch
    paddle.width = PADDLE_WIDTH_BYTES; // Restore normal paddle width
    for (i = 0; i < MAX_LASER_PAIRS; i++) {
        lasers[i].active = 0;
        lasers[i].needs_bg_repaint = 0;
        lasers[i].old_y = lasers[i].y;
    }
    for (i = 0; i < MAX_EXTRA_BALLS; i++) {
        extra_balls[i].active = 0;
        extra_balls[i].old_x = 0xFF;
    }
}

u16 getNormalBallSpeed() {
    u16 speed = INITIAL_BALL_SPEED + (current_level / 2) * 15;
    if (speed > INITIAL_BALL_SPEED + 150) speed = INITIAL_BALL_SPEED + 150;
    return speed;
}

// Helper to update dx, dy based on current speed and direction
void updateBallVelocity() {
    // Both dir and speed are signed 16-bit. 55 * 150 = 8250, fitting well inside i16 (max 32767).
    // Using >> 6 is much faster on Z80 than 32-bit division and prevents __mullong compiler bugs.
    ball.dx = FP_VEL(ball.dir_x, ball.speed);
    ball.dy = FP_VEL(ball.dir_y, ball.speed);
}

void initGame() {
    // Basic hardware/state init
    initPaddle();
    paddle.old_x = paddle.x;
    paddle.old_y = paddle.y;
    paddle.old_width = paddle.width;

    ball.width = BALL_SIZE_BYTES;
    ball.height = BALL_HEIGHT;
    ball.active = 0; 
    ball.x = paddle.x + (paddle.width / 2);
    ball.y = paddle.y - ball.height;
    
    ball.old_x = ball.x;
    ball.old_y = ball.y;
    ball.fx = (i16)ball.x * FP_SCALE;
    ball.fy = (i16)ball.y * FP_SCALE;
    
    ball.speed = getNormalBallSpeed();
    ball.dir_x = 0;
    ball.dir_y = 0;
    
    initial_dir_x = 16;
    initial_dir_y = -55;

    // Reset scores and power-up states
    hud_dirty = 1;
    flash_count = 0;
    door_open = 0;
    resetPowerups();

    // 2-Player initial state allocation
    if (num_players == 2) {
        u8 p;
        for (p = 0; p < 2; p++) {
            players[p].lives = 3;
            players[p].score = 0;
            players[p].current_level = current_level; 
            cpct_memcpy(players[p].score_str, "00000", 6);
            
            // Generate initial brick layout for each player
            current_player = p; 
            loadLevel();
            savePlayerState(p);
        }
        current_player = 0;
        loadPlayerState(0);
    } else {
        lives = 3;
        score = 0;
        cpct_memcpy(score_str, "00000", 6);
        loadLevel();
    }
}


// Draw the currently active power-up name in the top-left HUD area.
// Area is always cleared to black first (it sits above the play field).

void drawHUD() {
    u8 temp_str[6];
    u8 val = current_level + 1;
    
    // Refresh the score string exactly once per redraw (avoids repeated O(N) divisions)
    uint16_to_str(score, score_str);
    // Append a trailing '0' to make scores look 10x higher (Arcade style)
    score_str[5] = '0';
    score_str[6] = '\0';
    
    // Draw score (top-center of screen, inside the wall area)
    // 6 chars * 3 bytes = 18 bytes. Center = (80-18)/2 = 31
    drawCustomTextLarge((const u8*)score_str, 31, 0, PLT_BRIGHT_WHITE);

    // Draw player indicator in 2-Player mode
    if (num_players == 2) {
        temp_str[0] = 'P';
        temp_str[1] = '1' + current_player;
        temp_str[2] = '\0';
        drawCustomTextLarge(temp_str, 12, 0, PLT_BRIGHT_YELLOW);
    }

    // Draw level number (top-right of playfield)
    drawCustomText((const u8*)"L:", 60, 0, PLT_BRIGHT_YELLOW);
    uint16_to_str(val, temp_str);
    
    // Skip leading zeros for level display
    {
        u8* ptr = temp_str;
        while (*ptr == '0' && *(ptr+1) != '\0') ptr++;
        drawCustomText(ptr, 64, 0, PLT_BRIGHT_WHITE);
    }

    // Draw lives as mini-paddles in the bottom-left corner.
    // Shows (lives - 1) to match classic Arkanoid convention (the paddle in play is not counted).
    {
        u8 l;
        u8 display_lives = (lives > 1) ? (lives - 1) : 0;
        u8 max_lives_to_draw = (display_lives < 6) ? display_lives : 6; // Cap at 6 to stay in bounds
        // Erase the full possible area first (6 lives * 7 bytes = 42 bytes wide, 6 lines high)
        drawBackgroundRect(WALL_LEFT_BYTES + 1, 194, 45, 6);
        // Draw each life as a actual paddle sprite
        for (l = 0; l < max_lives_to_draw; l++) {
            cpct_drawSprite(paddle_sprite, cpct_getScreenPtr((void*)0xC000, WALL_LEFT_BYTES + 1 + (l * 7), 195), PADDLE_WIDTH_BYTES, PADDLE_HEIGHT);
        }
    }


    // Draw "PAUSE" overlay in the center when paused.
    if (paused) {
        drawCustomTextCentered(GET_STR(STR_PAUSE), 96, PLT_BRIGHT_RED);
    }
}

void nextLevel() {
    victory_walk = 0;
    current_level++;
    
    // If we passed the last level (Boss Level 7), we won!
    if (current_level >= NUM_LEVELS) {
        // Restore palette just in case we were in victory walk
        u8 i;
        for (i = 0; i < 16; i++) {
            cpct_setPALColour(i, default_palette[i]);
        }
        game_won = 1;
        return;
    }
    level_cleared = 1; // Mark level as cleared for this player

    // Re-initialize paddle positioning
    paddle.x = (SCREEN_WIDTH_BYTES - PADDLE_WIDTH_BYTES) / 2;
    paddle.y = PADDLE_START_Y;
    paddle.old_x = paddle.x;
    paddle.old_y = paddle.y;
    paddle.old_width = paddle.width;
    
    // Re-attach ball to paddle
    ball.active = 0;
    ball.x = paddle.x + (paddle.width / 2) - (ball.width / 2);
    ball.y = paddle.y - ball.height;
    
    ball.fx = (i16)ball.x * FP_SCALE;
    ball.fy = (i16)ball.y * FP_SCALE;
    
    // Increase difficulty based on level
    ball.speed = getNormalBallSpeed();
    
    ball.old_x = ball.x;
    ball.old_y = ball.y;

    // Advance safely to next level and loop
    loadLevel();
    
    // Reset all power-ups on level change
    resetPowerups();

    initEnemies();

    // Redraw entire board and HUD
    drawBackground();
    drawHUD();
}

void openDoor() {
    door_open = 1;
    door_anim_frame = 0;
    door_anim_timer = 0;
    // Draw first frame immediately
    cpct_drawSprite(door_anim_sprites[0], cpct_getScreenPtr((void*)0xC000, WALL_RIGHT_BYTES, 172), 2, 16);
}

void closeDoor() {
    if (!door_open) return;
    door_open = 0;
    // Redraw the wall section over the hole (height 16 = two 8-pixel tiles)
    cpct_drawSprite(wall_v2_sprite, cpct_getScreenPtr((void*)0xC000, 78, 172), 2, 8);
    cpct_drawSprite(wall_v2_sprite, cpct_getScreenPtr((void*)0xC000, 78, 180), 2, 8);
}

void updateVictoryPalette() {
    if (!victory_walk) return;
    
    victory_palette_timer++;
    if ((victory_palette_timer & 0x07) == 0) { // Every 8 frames
        u8 i;
        victory_palette_offset = (victory_palette_offset + 1) & 0x0F;
        for (i = 0; i < 16; i++) {
            cpct_setPALColour(i, default_palette[(i + victory_palette_offset) & 0x0F]);
        }
    }
}

// Called every frame to animate the exit energy field
void updateDoorAnim() {
    if (door_open) {
        door_anim_timer++;
        if (door_anim_timer >= DOOR_ANIM_SPEED) {
            door_anim_timer = 0;
            door_anim_frame++;
            if (door_anim_frame >= NUM_DOOR_FRAMES) {
                door_anim_frame = 0;
            }
            // Draw the next frame of the energy field over the hole
            cpct_drawSprite(door_anim_sprites[door_anim_frame], 
                            cpct_getScreenPtr((void*)0xC000, WALL_RIGHT_BYTES, 172), 
                            2, 16);
        }
    }
}

void updatePaddle() {
    u8 speed = PADDLE_SPEED;
    if (freeze_active) {
        speed = 0;
        if (freeze_timer > 0) {
            freeze_timer--;
            if (freeze_timer == 0) freeze_active = 0;
        }
    }

    if (autopilot_active) {
        u8 target_x;
        // Center the paddle under the ball
        // ball.width = 2, so center is ball.x + 1
        // target_x = (ball.x + 1) - (paddle.width / 2)
        target_x = ball.x + 1;
        if (target_x >= (paddle.width >> 1)) {
            target_x -= (paddle.width >> 1);
        } else {
            target_x = 0;
        }

        if (paddle.x < target_x) {
            paddle.x += speed;
            if (paddle.x > target_x) paddle.x = target_x;
        } else if (paddle.x > target_x) {
            if (paddle.x > speed) paddle.x -= speed;
            else paddle.x = 0;
            if (paddle.x < target_x) paddle.x = target_x;
        }

        // Clamp to walls
        if (paddle.x < WALL_LEFT_BYTES) paddle.x = WALL_LEFT_BYTES;
        if (!door_open && paddle.x + paddle.width > WALL_RIGHT_BYTES) paddle.x = WALL_RIGHT_BYTES - paddle.width;

        // Decrement timer
        if (autopilot_timer > 0) {
            autopilot_timer--;
            if (autopilot_timer == 0) autopilot_active = 0;
        }
    } else {
        // Check redefinable keys or joystick
        u8 left_p = cpct_isKeyPressed(g_key_left) || (g_use_joystick && cpct_isKeyPressed(Joy0_Left));
        u8 right_p = cpct_isKeyPressed(g_key_right) || (g_use_joystick && cpct_isKeyPressed(Joy0_Right));
        if (drunk_active) { u8 tmp = left_p; left_p = right_p; right_p = tmp; }

        if (left_p) {
            if (paddle.x >= WALL_LEFT_BYTES + speed) {
                paddle.x -= speed;
            } else {
                paddle.x = WALL_LEFT_BYTES; 
            }
        }
        
        if (right_p) {
            if (!door_open && paddle.x + paddle.width + speed > WALL_RIGHT_BYTES) {
                paddle.x = WALL_RIGHT_BYTES - paddle.width; 
            } else {
                paddle.x += speed;
                if (door_open && paddle.x >= WALL_RIGHT_BYTES) {
                    nextLevel();
                    return;
                }
            }
        }
    }
}

// Returns 1 if any ball (main or extra) is currently active.
u8 isAnyBallActive() {
    if (ball.active) return 1;
    return areExtrasActive();
}

// Returns 1 if any extra ball is currently active.
u8 areExtrasActive() {
    u8 i;
    for (i = 0; i < MAX_EXTRA_BALLS; i++) {
        if (extra_balls[i].active) return 1;
    }
    return 0;
}

// ──────────────── Shared ball physics helpers ────────────────

// Move ball in fixed-point space, bounce off walls, clamp to play area.
// Updates b->x, b->y from fixed-point position.
void moveBall(ball_t *b) {
    i16 next_x, next_y;

    b->fx += b->dx;
    b->fy += b->dy;
    if (gravity_active) {
        b->fy += 120; // Much stronger gravity (approx 2 pixels per frame pull)
    }

    // Reverse Magnet Trap: repel ball from paddle when close
    if (magnet_active && b->y > paddle.y - 40 && b->y < paddle.y + 10) {
        // If ball is roughly above/near the paddle
        if (b->x + b->width >= paddle.x && b->x <= paddle.x + paddle.width) {
            // Repel: nudge the fixed-point X away from the paddle's center
            u8 paddle_center = paddle.x + (paddle.width >> 1);
            if (b->x < paddle_center) {
                b->fx -= 48; // Stronger nudge left (~0.75 pixels)
            } else {
                b->fx += 48; // Stronger nudge right
            }
        }
    }

    next_x = FP_INT(b->fx);
    next_y = FP_INT(b->fy);

    // Bounce left/right walls
    if (b->fx <= WALL_LEFT_FP || b->fx + (b->width * FP_SCALE) >= WALL_RIGHT_FP) {
        b->dir_x = -b->dir_x;
        b->dx = FP_VEL(b->dir_x, b->speed);
        b->dy = FP_VEL(b->dir_y, b->speed);
        if (b->fx <= WALL_LEFT_FP) b->fx = WALL_LEFT_FP;
        if (b->fx + (b->width * FP_SCALE) >= WALL_RIGHT_FP) b->fx = (WALL_RIGHT_BYTES - b->width) * FP_SCALE;
        next_x = FP_INT(b->fx);
        cpct_akp_SFXPlay(8, 15, 77, 0, 0, AY_CHANNEL_ALL); // F5 - same as normal brick
    }

    // Bounce top wall
    if (b->fy <= WALL_TOP_FP) {
        b->dir_y = -b->dir_y;
        b->dx = FP_VEL(b->dir_x, b->speed);
        b->dy = FP_VEL(b->dir_y, b->speed);
        b->fy = WALL_TOP_FP;
        next_y = WALL_TOP;
        cpct_akp_SFXPlay(8, 15, 77, 0, 0, AY_CHANNEL_ALL); // F5 - same as normal brick
    }

    // Clamp visual bounds
    if (next_y < WALL_TOP) next_y = WALL_TOP;
    if (next_x < WALL_LEFT_BYTES) next_x = WALL_LEFT_BYTES;
    if (next_x > WALL_RIGHT_BYTES - b->width) next_x = WALL_RIGHT_BYTES - b->width;

    b->x = next_x;
    b->y = next_y;

    // Bounce off active enemies
    for (u8 i=0; i < MAX_ENEMIES; i++) {
        // Skip enemies that have reached the bottom or are inactive
        if (enemies[i].active && enemies[i].y < 200) {
            if (b->x + b->width >= enemies[i].x && b->x <= enemies[i].x + ENEMY_WIDTH &&
                b->y + b->height >= enemies[i].y && b->y <= enemies[i].y + ENEMY_HEIGHT) {
                
                enemies[i].active = 0;
                b->dir_y = -b->dir_y; // bounce ball
                b->dx = FP_VEL(b->dir_x, b->speed);
                b->dy = FP_VEL(b->dir_y, b->speed);
                
                score += 50;
                hud_dirty = 1;
                cpct_akp_SFXPlay(13, 15, 48, 0, 0, AY_CHANNEL_ALL); // SFX: enemy killed by ball
            }
        }
    }
}

// Bounce ball off paddle using 5-zone angle system. Returns 1 if paddle was hit.
// Does NOT handle glue — caller must check glue_active after this returns 1.
u8 bouncePaddle(ball_t *b) {
    i8 paddle_center, ball_center, diff;

    if (b->dy <= 0) return 0;
    if (b->y + b->height < paddle.y) return 0;
    if (b->y > paddle.y + paddle.height) return 0;
    if (b->x + b->width < paddle.x) return 0;
    if (b->x > paddle.x + paddle.width) return 0;

    b->y = paddle.y - b->height;
    b->fy = (i16)b->y * FP_SCALE;

    paddle_center = paddle.x + (paddle.width / 2);
    ball_center = b->x + (b->width / 2);
    diff = ball_center - paddle_center;

    if (diff < -1)       { b->dir_x = -20; b->dir_y = -27; }
    else if (diff == -1) { b->dir_x = -18; b->dir_y = -44; }
    else if (diff == 0)  { b->dir_x = (b->dir_x > 0) ? 16 : -16; b->dir_y = -55; }
    else if (diff == 1)  { b->dir_x = 18;  b->dir_y = -44; }
    else                 { b->dir_x = 20;  b->dir_y = -27; }

    b->dx = FP_VEL(b->dir_x, b->speed);
    b->dy = FP_VEL(b->dir_y, b->speed);
    return 1;
}

// Apply the effect of a ball hitting a brick at (r, c).
// Handles state change, score, drop spawn, erase queue, and SFX.
// Returns 1 if all bricks are cleared (nextLevel already called), 0 otherwise.
u8 hitBrick(u8 r, u8 c) {
    if (bricks[r][c] == BSTATE_HARD) {
        bricks[r][c] = BSTATE_NORMAL;
        score += 2;
        flashBrick(r, c);
        cpct_akp_SFXPlay(8, 15, 77, 0, 0, AY_CHANNEL_ALL); // F5
    } else if (bricks[r][c] == BSTATE_GOLD) {
        // Gold brick: indestructible, just flash
        flashBrick(r, c);
        cpct_akp_SFXPlay(8, 15, 79, 0, 0, AY_CHANNEL_ALL); // G5
    } else {
        // Normal brick: destroy and possibly spawn a drop
        if (BRICK_POWER(brick_map[r][c]) != 0 && !drop.active) {
            drop.x = brick_x[c] + (BRICK_WIDTH_BYTES / 2) - (DROP_WIDTH / 2);
            drop.y = brick_y[r] + BRICK_HEIGHT;
            drop.old_y = drop.y;
            drop.power_type = BRICK_POWER(brick_map[r][c]);
            drop.active = 1;
        }
        bricks[r][c] = BSTATE_NEEDS_ERASE;
        if (erase_count < MAX_ERASE) {
            erase_r[erase_count] = r;
            erase_c[erase_count] = c;
            erase_count++;
        }
        score += 5;
        active_bricks--;
        cpct_akp_SFXPlay(8, 15, 77, 0, 0, AY_CHANNEL_ALL); // F5
    }
    hud_dirty = 1;

    if (active_bricks == 0) {
        nextLevel();
        return 1;
    }
    return 0;
}

// Check brick collision for one ball. Returns 1 if level was cleared, 0 otherwise.
u8 collideBricks(ball_t *b) {
    u8 r, c;
    u8 r_down, c_right;

    // Optimization & Safety: Bricks are only in the top area.
    // This prevents "ghost bounces" at the bottom due to LUT OOB access.
    if (b->y > 195) return 0;
    
    // Direct lookup for the top-left corner of the ball
    r = g_y_to_row[b->y];
    c = g_x_to_col[b->x];

    // Check top-left corner
    if (r != 0xFF && c != 0xFF) {
        if (bricks[r][c] != BSTATE_EMPTY && bricks[r][c] != BSTATE_NEEDS_ERASE) {
            goto hit;
        }
    }

    // Check bottom-right corner (ball can be up to 2x8 pixels, so 2 bytes wide)
    {
        u8 y_bottom = b->y + b->height - 1;
        u8 x_right = b->x + b->width - 1;
        
        // Safety check for bottom-right OOB
        if (y_bottom >= 200 || x_right >= 80) return 0;

        r_down = g_y_to_row[y_bottom];
        c_right = g_x_to_col[x_right];
    }

    if (r_down != 0xFF && c_right != 0xFF) {
        if (bricks[r_down][c_right] != BSTATE_EMPTY && bricks[r_down][c_right] != BSTATE_NEEDS_ERASE) {
            r = r_down; c = c_right;
            goto hit;
        }
    }

    // Check top-right and bottom-left for thoroughness (if different)
    if (r != 0xFF && c_right != 0xFF && (r != r_down || c != c_right)) {
         if (bricks[r][c_right] != BSTATE_EMPTY && bricks[r][c_right] != BSTATE_NEEDS_ERASE) {
            c = c_right;
            goto hit;
        }
    }
    if (r_down != 0xFF && c != 0xFF && (r != r_down || c != c_right)) {
         if (bricks[r_down][c] != BSTATE_EMPTY && bricks[r_down][c] != BSTATE_NEEDS_ERASE) {
            r = r_down;
            goto hit;
        }
    }

    return 0;

hit:
    {
        u8 by = brick_y[r];
        u8 bx = brick_x[c];
        u8 was_outside_x = (b->old_x + b->width <= bx) || (b->old_x >= bx + BRICK_WIDTH_BYTES);
        u8 was_outside_y = (b->old_y + b->height <= by) || (b->old_y >= by + BRICK_HEIGHT);

        if (was_outside_x && !was_outside_y) { 
            if (!fireball_active || bricks[r][c] == BSTATE_GOLD) b->dir_x = -b->dir_x; 
        }
        else { 
            if (!fireball_active || bricks[r][c] == BSTATE_GOLD) b->dir_y = -b->dir_y; 
        }
        b->dx = FP_VEL(b->dir_x, b->speed);
        b->dy = FP_VEL(b->dir_y, b->speed);

        return hitBrick(r, c);
    }
}

// ──────────────────────────────────────────────────────────────
void flashBrick(u8 r, u8 c) {
    if (flash_count < MAX_FLASH) {
        flash_r[flash_count] = r;
        flash_c[flash_count] = c;
        flash_timer[flash_count] = 4; // 4 frames duration = 80ms
        flash_count++;
    }
}

// ──────────────────────────────────────────────────────────────
void muteAY() {
    __asm
    ld b, #0xF4
    ld a, #8
    out (c), a
    ld bc, #0xF6C0
    out (c), c
    ld bc, #0xF600
    out (c), c
    ld b, #0xF4
    xor a
    out (c), a
    ld bc, #0xF680
    out (c), c
    ld bc, #0xF600
    out (c), c
    
    ld b, #0xF4
    ld a, #9
    out (c), a
    ld bc, #0xF6C0
    out (c), c
    ld bc, #0xF600
    out (c), c
    ld b, #0xF4
    xor a
    out (c), a
    ld bc, #0xF680
    out (c), c
    ld bc, #0xF600
    out (c), c

    ld b, #0xF4
    ld a, #10
    out (c), a
    ld bc, #0xF6C0
    out (c), c
    ld bc, #0xF600
    out (c), c
    ld b, #0xF4
    xor a
    out (c), a
    ld bc, #0xF680
    out (c), c
    ld bc, #0xF600
    out (c), c
    __endasm;
}

void loseLife() {
    u8 j;
    if (victory_walk) return;
    // SFX: life lost 
    cpct_akp_SFXPlay(13, 24, 36, 0, 0, AY_CHANNEL_ALL);
        
    lives--;

    // Deactivate and IMMEDIATELY erase any falling capsule
    if (drop.active) {
        if (drop.y < SCREEN_HEIGHT) {
            drawBackgroundRect(drop.x, drop.y, DROP_WIDTH, DROP_HEIGHT);
        }
        drop.active = 0;
        drop_erase_pending = 0;
    }

    // Deactivate and IMMEDIATELY erase any in-flight lasers
    for (j = 0; j < MAX_LASER_PAIRS; j++) { 
        if (lasers[j].active) { 
            if (lasers[j].y > WALL_TOP && lasers[j].y < SCREEN_HEIGHT) {
                drawBackgroundRect(lasers[j].x_left,  lasers[j].y, LASER_WIDTH, LASER_HEIGHT);
                drawBackgroundRect(lasers[j].x_right, lasers[j].y, LASER_WIDTH, LASER_HEIGHT);
            }
            lasers[j].active = 0; 
        } 
        lasers[j].needs_bg_repaint = 0;
        lasers[j].old_y = 0xFF;
    }

    // Deactivate and IMMEDIATELY erase all enemies
    for (j = 0; j < MAX_ENEMIES; j++) {
        if (enemies[j].active) {
            drawBackgroundRect(enemies[j].x, enemies[j].y, ENEMY_WIDTH, ENEMY_HEIGHT);
            enemies[j].active = 0;
        }
        enemies[j].old_x = 0xFF; // Sentinel to prevent drawEnemies from erasing again
    }

    drawHUD();
    hud_dirty = 0;

    if (num_players == 2 && lives == 0) {
        // Current player is out of lives, check if the other one has any left
        switchPlayer();
        if (lives > 0) {
            showPlayerStart(current_player);
            return;
        }
        // Both dead? Fall through to standard game over handling
    }

    if (lives > 0) {
        // Restore palette in case it was flickering (e.g. boss hit)
        for (j = 0; j < 16; j++) {
            cpct_setPALColour(j, default_palette[j]);
        }
        
        if (num_players == 2) {
             switchPlayer();
             showPlayerStart(current_player);
        } else {
            ball.active = 0;
            resetPowerups();
            closeDoor();
        }
    }
}

// ──────────────────────────────────────────────────────────────

// Handle laser input: fire on Space, rate-limited.
void updateLaserInput() {
    if (!laser_active) return;
    if (laser_fire_timer > 0) laser_fire_timer--;
    if ((cpct_isKeyPressed(g_key_fire) || (g_use_joystick && cpct_isKeyPressed(Joy0_Fire1))) && laser_fire_timer == 0) {
        fireLaser();
        laser_fire_timer = 15;
    }
}

// Update main ball while it is attached to the paddle (waiting for launch).
void updateBallOnPaddle() {
    ball.old_x = ball.x;
    ball.old_y = ball.y;
    ball.x = paddle.x + (paddle.width / 2) - (ball.width / 2);
    ball.y = paddle.y - ball.height;
    ball.fx = (i16)ball.x * FP_SCALE;
    ball.fy = (i16)ball.y * FP_SCALE;

    // Auto-launch countdown: fires the ball automatically after ~5 seconds
    if (launch_timer > 0) launch_timer--;

    if (victory_walk) return;

    if (glue_active) {
        // Glue mode: wait for glue_timer (set on paddle bounce), then auto-release
        if (glue_timer > 0) glue_timer--;
        if (glue_timer == 0 || cpct_isKeyPressed(g_key_fire) || (g_use_joystick && cpct_isKeyPressed(Joy0_Fire1))) {
            glue_timer = 0;
            launch_timer = 250; // Reset for next ball
            ball.active = 1;
            ball.dir_x = initial_dir_x;
            ball.dir_y = initial_dir_y;
            updateBallVelocity();
        }
    } else {
        // Normal mode: Fire OR auto-launch countdown reaching 0
        if (cpct_isKeyPressed(g_key_fire) || (g_use_joystick && cpct_isKeyPressed(Joy0_Fire1)) || launch_timer == 0) {
            launch_timer = 250; // Reset for next ball
            ball.active = 1;
            ball.dir_x = initial_dir_x;
            ball.dir_y = initial_dir_y;
            updateBallVelocity();
        }
    }
}

// Update main ball while it is in play (moving, bouncing, colliding).
// Returns 1 if the level was cleared (nextLevel was called), 0 otherwise.
// Unified physics handler for any ball (main or extra).
// Handles movement, bounces, collisions, and deactivation/life-loss logic.
// Returns 1 if the level was cleared (bricks exhausted), 0 otherwise.
u8 handleBallPhysics(ball_t *b) {
    moveBall(b);

    // Paddle bounce + glue/sfx (only for main ball)
    if (bouncePaddle(b)) {
        if (b == &ball) {
            cpct_akp_SFXPlay(8, 15, 84, 0, 0, AY_CHANNEL_ALL); //C6
            if (glue_active) {
                b->active = 0;
                b->dir_x = 0; b->dir_y = 0;
                b->dx = 0; b->dy = 0;
                glue_timer = 200;
            }
        }
    }

    // Fall through bottom
    if (b->y >= SCREEN_HEIGHT) {
        b->active = 0;
        if (!isAnyBallActive()) {
            loseLife();
        }
        return 0;
    }

    // Boss collision
    if (boss.active) {
        checkBossCollision(b);
    }

    // Brick collision
    return collideBricks(b);
}

u8 updateBallInPlay() {
    return handleBallPhysics(&ball);
}

void updateBall() {
    updateLaserInput();

    if (!ball.active) {
        if (!areExtrasActive()) updateBallOnPaddle();
    } else {
        if (updateBallInPlay()) return;
    }
}


// Draw and erase laser pairs every frame.
// Each pair draws/erases two separate beams (x_left and x_right) at the same Y.
void drawLasers() {
    u8 i;
    u8* pMem;
    u8 laser_byte = cpct_px2byteM0(COLOR_LASER, COLOR_LASER);
    for (i = 0; i < MAX_LASER_PAIRS; i++) {
        if (lasers[i].old_y == lasers[i].y && !lasers[i].needs_bg_repaint) continue;
        if (lasers[i].needs_bg_repaint && lasers[i].old_y > WALL_TOP && lasers[i].old_y + LASER_HEIGHT <= SCREEN_HEIGHT) {
            drawBackgroundRect(lasers[i].x_left,  lasers[i].old_y, LASER_WIDTH, LASER_HEIGHT);
            drawBackgroundRect(lasers[i].x_right, lasers[i].old_y, LASER_WIDTH, LASER_HEIGHT);
            lasers[i].needs_bg_repaint = 0;
        }
        lasers[i].old_y = lasers[i].y;
        if (lasers[i].active && lasers[i].y > WALL_TOP && lasers[i].y + LASER_HEIGHT <= SCREEN_HEIGHT) {
            pMem = cpct_getScreenPtr((void*)0xC000, lasers[i].x_left,  lasers[i].y);
            cpct_drawSolidBox(pMem, laser_byte, LASER_WIDTH, LASER_HEIGHT);
            pMem = cpct_getScreenPtr((void*)0xC000, lasers[i].x_right, lasers[i].y);
            cpct_drawSolidBox(pMem, laser_byte, LASER_WIDTH, LASER_HEIGHT);
        }
    }
}

// Draw/erase the falling power-up capsule.
// Draws ONLY the sprite without erase (used for early-frame pass to protect from CRT tearing).
void drawDropSprite() {
    u8* pVideoMemory;
    if (!drop.active) return;
    if (drop.y < SCREEN_HEIGHT - DROP_HEIGHT && drop.power_type > 0 && drop.power_type <= NUM_POWERUP_SPRITES) {
        pVideoMemory = cpct_getScreenPtr((void*)0xC000, drop.x, drop.y);
        cpct_drawSprite(powerup_sprites[drop.power_type - 1], pVideoMemory, DROP_WIDTH, DROP_HEIGHT);
    }
}

// ---------------------------------------------------------------------------
// TECHNIQUE: DELTA ERASE
// ---------------------------------------------------------------------------
// The CPC draws the screen with an electron beam scanning top to bottom at
// 50Hz. The Z80 writes to the same VRAM while the CRT is actively scanning.
//
// Problem (with a large 4x8px sprite):
//   When moving the capsule 1px downward, the entire old area (8 rows) was
//   erased before redrawing at the new position. This opened an 8-row blank
//   window. If the CRT scanned that region during the gap, the player saw a
//   background flash → visible flicker.
//
// Solution (delta erase):
//   Since the capsule only moves 1px down, only the TOP row of the old area
//   is newly uncovered. We erase just that 1-pixel strip (4 bytes) and
//   immediately redraw the sprite at the new position.
//   The blank window shrinks from 8 rows to 1 row → imperceptible.
//
// General rule: for a sprite that moves slowly in one direction, only erase
//   the strip that is ACTUALLY uncovered (delta = dy or dx).
// ---------------------------------------------------------------------------
void drawDrop() {
    u8* pVideoMemory;
    if (drop_erase_pending) {
        if (drop_erase_y < SCREEN_HEIGHT) {
            drawBackgroundRect(drop_erase_x, drop_erase_y, DROP_WIDTH, DROP_HEIGHT);
        }
        drop_erase_pending = 0;
    }

    if (!drop.active) return;

    if (drop.old_y != drop.y && drop.old_y < SCREEN_HEIGHT) {
        // DELTA ERASE: The capsule falls 1px per move.
        // Only erase the TOP strip that becomes uncovered (usually 1px),
        // NOT the entire previous sprite area (DROP_HEIGHT rows).
        // This minimises the time the CRT can see blank pixels.
        u8 dy = drop.y - drop.old_y;   // pixels moved (normally 1)
        if (dy > 0 && dy < DROP_HEIGHT) {
            // Erase only the newly uncovered top strip (usually 1px)
            drawBackgroundRect(drop.x, drop.old_y, DROP_WIDTH, dy);
        } else {
            // Fallback: large jump or teleport → full erase
            drawBackgroundRect(drop.x, drop.old_y, DROP_WIDTH, DROP_HEIGHT);
        }
    }
    // Redraw IMMEDIATELY after erasing to minimise the blank window.
    // The shorter the gap between erase and redraw, the less likely the CRT
    // will scan that area while it is blank.
    if (drop.y < SCREEN_HEIGHT - DROP_HEIGHT) {
        if (drop.power_type > 0 && drop.power_type <= NUM_POWERUP_SPRITES) {
            pVideoMemory = cpct_getScreenPtr((void*)0xC000, drop.x, drop.y);
            cpct_drawSprite(powerup_sprites[drop.power_type - 1], pVideoMemory, DROP_WIDTH, DROP_HEIGHT);
        }
    }

    // Repaint any bricks uncovered by the erased top strip.
    // Done AFTER drawing the sprite so the CRT sees the correct capsule
    // position before we repaint the bricks below it.
    if (drop.old_y != drop.y && drop.old_y < SCREEN_HEIGHT) {
        u8 r, c;
        u8 dy = drop.y - drop.old_y;
        if (dy > 0 && dy < DROP_HEIGHT) {
            for (r = 0; r < BRICK_ROWS; r++) {
                if (brick_y[r] + BRICK_HEIGHT <= drop.old_y) continue;
                if (brick_y[r] >= drop.old_y + dy) break;
                for (c = 0; c < BRICK_COLS; c++) {
                    if (brick_x[c] + BRICK_WIDTH_BYTES <= drop.x) continue;
                    if (brick_x[c] >= drop.x + DROP_WIDTH) break;
                    if (bricks[r][c] != BSTATE_EMPTY && bricks[r][c] != BSTATE_NEEDS_ERASE) {
                        u8 btype = BRICK_TYPE(brick_map[r][c]);
                        u8 s_idx;
                        if (btype == BTYPE_HARD) s_idx = 8;
                        else if (btype == BTYPE_GOLD) s_idx = 9;
                        else s_idx = BRICK_COLOR(brick_map[r][c]);
                        pVideoMemory = cpct_getScreenPtr((void*)0xC000, brick_x[c], brick_y[r]);
                        cpct_drawSprite(brick_sprites[s_idx], pVideoMemory, BRICK_WIDTH_BYTES, BRICK_HEIGHT);
                    }
                }
            }
        }
    }

    // Sync old coordinates to match what's on screen after drawing.
    drop.old_y = drop.y;
}

// ──────────────────────────────────────────────────────────────
// FINAL BOSS LOGIC
// ──────────────────────────────────────────────────────────────

void initBoss() {
    boss.active = 1;
    boss.width = BOSS_NEW_WIDTH_BYTES;   // 40 bytes = 80 pixels
    boss.height = BOSS_NEW_HEIGHT;      // 55 lines
    boss.health = 15;//20; // For testing
    boss.state = 1;
    boss.timer = 0;
    boss.x = (SCREEN_WIDTH_BYTES - boss.width) / 2;
    boss.y = 20;
    boss.old_x = 255; // Force initial draw
    boss.old_y = 255;
    boss.fx = (i16)boss.x << FP_SCALE_SHIFT;
    boss.fy = (i16)boss.y << FP_SCALE_SHIFT;
    boss.dx = 0; // Stationary for now as requested
    boss.dy = 0;
    
    // Clear the top of the level layout
    drawBackgroundRect(WALL_LEFT_BYTES, WALL_TOP, WALL_RIGHT_BYTES - WALL_LEFT_BYTES, boss.height + 10);
}

void updateBoss() {
    if (!boss.active) return;
    
    if (boss.hit_timer > 0) {
        boss.hit_timer--;
        if (boss.hit_timer > 0) {
            cpct_setPALColour(1, HW_BLACK);
        } else {
            cpct_setPALColour(1, default_palette[1]);
        }
    }
    if (boss.state == 1) { // Stationary
        // Boss doesn't move right now.
        // Occasional downward attack?
        boss.timer++;
        if (boss.timer > 150) {  // every 150 frames = 3 seconds approx
            u8 i;
            boss.timer = 0;
            // Spawn an enemy drop from the boss's center
            for (i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) {
                    enemies[i].active = 1;
                    enemies[i].x = boss.x + (boss.width / 2) - (ENEMY_WIDTH / 2);
                    enemies[i].y = boss.y + boss.height;
                    enemies[i].old_x = enemies[i].x;
                    enemies[i].old_y = enemies[i].y;
                    enemies[i].color = PLT_BRIGHT_YELLOW; // Specific color for boss drops?
                    enemies[i].type = rand8() % NUM_ENEMY_SPRITES;
                    enemies[i].dx = (rand8() & 1) ? 1 : -1;
                    enemies[i].frame = 0;
                    break;
                }
            }
        }
    } else if (boss.state == 2) { // Exploding
        boss.timer++;
        
        // Palette flicker effect: Toggle colors <-> Black
        if (boss.timer % 4 < 2) {
            cpct_setPALColour(PLT_BLUE, HW_BLACK);
            cpct_setPALColour(PLT_GREEN, HW_BLACK);
        } else {
            cpct_setPALColour(PLT_BLUE, HW_BLUE);
            cpct_setPALColour(PLT_GREEN, HW_GREEN);
        }

        if (boss.timer % 8 == 0) {
            // Flash color or trigger mini explosions
            cpct_akp_SFXPlay(2, 15, 30 + (rng_seed % 20), 0, 0, AY_CHANNEL_ALL);
        }
        if (boss.timer > 100) {
            // Restore actual palette before finishing
            cpct_setPALColour(PLT_BLUE, HW_BLUE);
            cpct_setPALColour(PLT_GREEN, HW_GREEN);
            
            boss.active = 0; // Dead
            boss.state = 0;
            
            // Victory Walk begins
            victory_walk = 1;
            ball.active = 0;
            {
                u8 i;
                for (i = 0; i < MAX_EXTRA_BALLS; i++) extra_balls[i].active = 0;
            }
            openDoor();
            
            drawBackgroundRect(boss.old_x, boss.old_y, boss.width, boss.height); // Erase
            cpct_akp_SFXPlay(4, 15, 60, 0, 0, AY_CHANNEL_ALL); // Victory sound
        }
    }
}

void checkBossCollision(ball_t* b) {
    if (!boss.active || !b->active) return;
    
    if (b->x < boss.x + boss.width && b->x + b->width > boss.x &&
        b->y < boss.y + boss.height && b->y + b->height > boss.y) {
        
        u8 was_outside_x = (b->old_x + b->width <= boss.x) || (b->old_x >= boss.x + boss.width);
        u8 was_outside_y = (b->old_y + b->height <= boss.y) || (b->old_y >= boss.y + boss.height);

        if (was_outside_x && !was_outside_y) { 
            b->dir_x = -b->dir_x; 
        } else { 
            b->dir_y = -b->dir_y; 
        }
        
        b->dx = FP_VEL(b->dir_x, b->speed);
        b->dy = FP_VEL(b->dir_y, b->speed);
        
        // Damage boss
        if (boss.health > 0) boss.health--;
        
        if (boss.health == 0) {
            boss.state = 2; // Explode
            boss.timer = 0;
            boss.hit_timer = 0; // Stop flashing
            cpct_setPALColour(1, default_palette[1]); // Ensure restored
            score += 1000;
            hud_dirty = 1;
        } else {
            score += 10;
            boss.hit_timer = 10; // 0.2s flash
            hud_dirty = 1;
            cpct_akp_SFXPlay(2, 15, 50, 0, 0, AY_CHANNEL_ALL); // Hit boss
        }
    }
}

void drawBoss() {
    if (!boss.active) return;
    
    // State 1: normal, only redraw if moved (or initial draw where old_x == 255)
    if (boss.state == 1) {
        if (boss.x != boss.old_x || boss.y != boss.old_y) {
            // Erase old only if it's a valid old position
            if (boss.old_x != 255) {
                drawBackgroundRect(boss.old_x, boss.old_y, boss.width, boss.height);
            }
            boss.old_x = boss.x;
            boss.old_y = boss.y;
            
            cpct_drawSprite(boss_new_sprite, cpct_getScreenPtr((void*)0xC000, boss.x, boss.y), boss.width, boss.height);
        }
    } 
    // State 2: Exploding, flashing rect
    else if (boss.state == 2) { 
        // Only draw on the exact frame the flash state changes to save CPU
        if (boss.timer % 4 == 0) {
            if ((boss.timer / 4) % 2 == 0) {
                 cpct_drawSprite(boss_new_sprite, cpct_getScreenPtr((void*)0xC000, boss.x, boss.y), boss.width, boss.height);
            } else {
                 cpct_drawSolidBox(cpct_getScreenPtr((void*)0xC000, boss.x, boss.y), cpct_px2byteM0(HW_WHITE, HW_WHITE), boss.width, boss.height);
            }
        }
    }
}

// Draw/erase extra balls (multiball).
void drawExtraBalls() {
    u8 eb;
    u8* pMem;
    for (eb = 0; eb < MAX_EXTRA_BALLS; eb++) {
        ball_t *b = &extra_balls[eb];
        if (b->active || b->old_x != 0xFF) {
            // 1. Erase old position if it exists
            if (b->old_x != 0xFF) {
                drawBackgroundRect(b->old_x, b->old_y, b->width, b->height);
            }
            // 2. Draw and sync if still active
            if (b->active) {
                if (b->y <= SCREEN_HEIGHT - b->height && b->x < 80 && b->y >= WALL_TOP) {
                    if (1) { // Always draw Fireball (no blinking)
                        pMem = cpct_getScreenPtr((void*)0xC000, b->x, b->y);
                        cpct_drawSprite(fireball_active ? fireball_sprite : ball_sprite, pMem, b->width, b->height);
                    }
                }
                b->old_x = b->x;
                b->old_y = b->y;
            } else {
                // Was active last frame (or had an old position), but now it's not.
                // It's already erased, so we just mark it as clean.
                b->old_x = 0xFF;
            }
        }
    }
}

// Draw flashing brick overlays and tick their timers.
void drawFlashBricks() {
    u8 f = 0;
    u8* pMem;
    // Draw flash overlay
    for (f = 0; f < flash_count; f++) {
        u8 r = flash_r[f]; u8 c = flash_c[f];
        if (bricks[r][c] != BSTATE_EMPTY && bricks[r][c] != BSTATE_NEEDS_ERASE) {
            u8 fcol = (bricks[r][c] == BSTATE_GOLD) ? COLOR_LASER : COLOR_BALL;
            pMem = cpct_getScreenPtr((void*)0xC000, brick_x[c], brick_y[r]);
            cpct_drawSolidBox(pMem, cpct_px2byteM0(fcol, fcol), BRICK_WIDTH_BYTES, BRICK_HEIGHT);
        }
    }
    // Tick timers and dequeue expired entries
    f = 0;
    while (f < flash_count) {
        flash_timer[f]--;
        if (flash_timer[f] == 0) {
            u8 r = flash_r[f]; u8 c = flash_c[f];
            if (bricks[r][c] != BSTATE_EMPTY && bricks[r][c] != BSTATE_NEEDS_ERASE) {
                u8 btype = BRICK_TYPE(brick_map[r][c]);
                u8 s_idx;
                if (btype == BTYPE_HARD) s_idx = 8;
                else if (btype == BTYPE_GOLD) s_idx = 9;
                else s_idx = BRICK_COLOR(brick_map[r][c]);

                pMem = cpct_getScreenPtr((void*)0xC000, brick_x[c], brick_y[r]);
                cpct_drawSprite(brick_sprites[s_idx], pMem, BRICK_WIDTH_BYTES, BRICK_HEIGHT);
            }
            flash_count--;
            flash_r[f]     = flash_r[flash_count];
            flash_c[f]     = flash_c[flash_count];
            flash_timer[f] = flash_timer[flash_count];
        } else {
            f++;
        }
    }
}

void drawGame() {
    cpct_waitVSYNC();
    
    if (victory_walk) {
        updateVictoryPalette();
        // Persistent victory message
        drawCustomTextCentered(GET_STR(STR_GO_TO_THE_DOOR), 96, PLT_BRIGHT_RED);
    }
    
    // NOTE: The drop capsule is drawn TWICE per frame:
    //   1) drawDropSprite() here at the start: the CRT sees the capsule at its
    //      current position before any erase operation can damage it.
    //   2) drawDrop() at the end: handles delta erase of the old position and
    //      leaves the correct state for the next frame.
    // This strategy compensates for the lack of double buffering on the CPC.
    drawDropSprite();
    
    // Erase old paddle position if it moved OR if width changed.
    if (paddle.old_x != paddle.x || paddle.old_y != paddle.y || paddle.old_width != paddle.width) {
        if (paddle.old_y <= SCREEN_HEIGHT - paddle.height && paddle.old_x < 80) {
            drawBackgroundRect(paddle.old_x, paddle.old_y, paddle.old_width, paddle.height);
        }
    }

    // Draw paddle.
    if (paddle.y <= SCREEN_HEIGHT - paddle.height && paddle.x < 80) {
        u8* pMem = cpct_getScreenPtr((void*)0xC000, paddle.x, paddle.y);
        if (paddle.width == PADDLE_WIDTH_WIDE) {
            cpct_drawSprite(paddle_wide_sprite, pMem, paddle.width, paddle.height);
        } else if (paddle.width == PADDLE_WIDTH_TINY) {
            cpct_drawSprite(paddle_tiny_sprite, pMem, paddle.width, paddle.height);
        } else {
            cpct_drawSprite(paddle_sprite, pMem, paddle.width, paddle.height);
        }
    }
    
    // Sync old coordinates and width to match what's on screen after drawing.
    paddle.old_x = paddle.x;
    paddle.old_y = paddle.y;
    paddle.old_width = paddle.width;
    
    // Erase old ball position if it moved.
    if (ball.old_x != ball.x || ball.old_y != ball.y) {
        if (ball.old_y <= SCREEN_HEIGHT - ball.height && ball.old_x < 80 && ball.old_y >= WALL_TOP) {
            drawBackgroundRect(ball.old_x, ball.old_y, ball.width, ball.height);
        }
    }

    // Draw ball.
    if (ball.y <= SCREEN_HEIGHT - ball.height && ball.x < 80 && ball.y >= WALL_TOP) {
        if (1) {
            u8* pMem = cpct_getScreenPtr((void*)0xC000, ball.x, ball.y);
            cpct_drawSprite(fireball_active ? fireball_sprite : ball_sprite, pMem, ball.width, ball.height);
        }
    }
    if (fireball_active) ghost_timer++;
    
    // Sync old coordinates to match what's on screen after drawing.
    ball.old_x = ball.x;
    ball.old_y = ball.y;


    // Erase destroyed bricks (direct index lookup, no full 64-cell scan per frame).
    for (u8 e = 0; e < erase_count; e++) {
        drawBackgroundRect(brick_x[erase_c[e]], brick_y[erase_r[e]], BRICK_WIDTH_BYTES, BRICK_HEIGHT);
        bricks[erase_r[e]][erase_c[e]] = BSTATE_EMPTY;
    }
    erase_count = 0;

    drawLasers();
    drawExtraBalls();
    drawEnemies();
    if (boss.active || boss.state == 2) drawBoss();
    
    // --- Gameplay Logic ---only if something changed (score, lives, level, or active power-up).
    if (hud_dirty) { drawHUD(); hud_dirty = 0; }

    drawFlashBricks();

    // Call door energy field animation if open
    updateDoorAnim();

    // Draw drop LAST so no other erase/repaint can overwrite it this frame.
    drawDrop();
}

void drawBackground() {
    u8* pVideoMemory;

    cpct_memset((void*)0xC000, 0x00, 0x4000);
    
    // Compute the global pattern array once per level load
    initBackgroundCache();

    // Draw textured background ONLY in the play area (Between the walls)
    // WALL_LEFT_BYTES is 2, WALL_RIGHT_BYTES is 78. Width = 76.
    // WALL_TOP is 16. Height = SCREEN_HEIGHT - 16 = 184.
    drawBackgroundRect(WALL_LEFT_BYTES, WALL_TOP, WALL_RIGHT_BYTES - WALL_LEFT_BYTES, SCREEN_HEIGHT - WALL_TOP);

    // Draw top corners
    cpct_drawSprite(wall_cl_sprite, cpct_getScreenPtr((void*)0xC000, 0, 8), 2, 8);
    cpct_drawSprite(wall_cr_sprite, cpct_getScreenPtr((void*)0xC000, 78, 8), 2, 8);

    // Draw horizontal wall (Y=8, height=8) from X=2 to X=77
    for (u8 wx = 2; wx < 78; wx += 2) {
        cpct_drawSprite(wall_h_sprite, cpct_getScreenPtr((void*)0xC000, wx, 8), 2, 8);
    }

    // Draw vertical walls (X=0 and X=78, height=8 per tile) from Y=16 downwards
    for (u16 wy = 16; wy < SCREEN_HEIGHT; wy += 8) {
        cpct_drawSprite(wall_v_sprite, cpct_getScreenPtr((void*)0xC000, 0, (u8)wy), 2, 8);
        cpct_drawSprite(wall_v2_sprite, cpct_getScreenPtr((void*)0xC000, 78, (u8)wy), 2, 8);
    }

    // Initial draw of all bricks
    for (u8 r = 0; r < BRICK_ROWS; r++) {
        for (u8 c = 0; c < BRICK_COLS; c++) {
            if (bricks[r][c] != BSTATE_EMPTY && bricks[r][c] != BSTATE_NEEDS_ERASE) {
                u8 bx = BRICK_START_X + c * (BRICK_WIDTH_BYTES + BRICK_GAP_X);
                u8 by = BRICK_START_Y + r * (BRICK_HEIGHT + BRICK_GAP_Y);
                u8 btype = BRICK_TYPE(brick_map[r][c]);
                u8 s_idx;
                if (btype == BTYPE_HARD) s_idx = 8;
                else if (btype == BTYPE_GOLD) s_idx = 9;
                else s_idx = BRICK_COLOR(brick_map[r][c]);

                pVideoMemory = cpct_getScreenPtr((void*)0xC000, bx, by);
                cpct_drawSprite(brick_sprites[s_idx], pVideoMemory, BRICK_WIDTH_BYTES, BRICK_HEIGHT);
            }
        }
    }
}

// Move and collide extra balls spawned by multiball power-up.
// Extra balls never stick to paddle (no glue), and losing them doesn't cost a life.
void updateExtraBalls() {
    u8 eb;
    for (eb = 0; eb < MAX_EXTRA_BALLS; eb++) {
        ball_t *b = &extra_balls[eb];
        if (!b->active) continue;

        if (handleBallPhysics(b)) return; // Level cleared
    }
}

// Global Starfield for intros and game over screens
typedef struct {
    u8 x; 
    u8 y;
    u8 speed; 
    u8 color;
    u8 is_drawn;
} TStar;
TStar stars[40];

void initStarfield() {
    u8 s, r;
    for (s = 0; s < 40; s++) {
        stars[s].x = rand8() % 80;
        stars[s].y = rand8() % 200;
        stars[s].is_drawn = 0;
        r = rand8() % 100;
        if (r < 25) { 
            stars[s].speed = 3; 
            stars[s].color = 0x0C; // White (PEN 2) for closest stars
        } else if (r < 60) { 
            stars[s].speed = 2; 
            stars[s].color = 0x0F; // Cyan (PEN 10) for mid stars
        } else { 
            stars[s].speed = 1; 
            stars[s].color = 0xC0; // Blue (PEN 1) for furthest stars
        }
    }
}

void updateStarfield() {
    u8 s;
    u8* p;
    // Erase old stars ONLY if they were actually drawn by us
    for (s = 0; s < 40; s++) {
        if (stars[s].is_drawn) {
            p = cpct_getScreenPtr((void*)0xC000, stars[s].x, stars[s].y);
            *p = 0x00; 
            stars[s].is_drawn = 0;
        }
    }
    
    // Move and draw stars
    for (s = 0; s < 40; s++) {
        stars[s].y += stars[s].speed;
        
        // Wrap around (200 is screen bottom)
        if (stars[s].y >= 200) {
            stars[s].y = 0;
            stars[s].x = rand8() % 80;
        }
        
        // Draw new star ONLY if destination is empty (black)
        p = cpct_getScreenPtr((void*)0xC000, stars[s].x, stars[s].y);
        if (*p == 0x00) {
            *p = stars[s].color;
            stars[s].is_drawn = 1;
        }
    }
}

u8 showVictory() {
    cpct_memset((void*)0xC000, 0x00, 0x4000);
    initStarfield();

    // Draw "CONGRATS" in Cyan (Large)
    drawCustomTextLargeCentered(GET_STR(STR_CONGRATS), 40, PLT_ORANGE); 

    // Draw funny comic messages in White (Small)
    drawCustomTextCentered(GET_STR(STR_RECOVERED_MOTO), 70, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_NOW_GO_BREAD), 90, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_NOW_GO_BREAD_2), 105, PLT_BRIGHT_WHITE); 
    drawCustomTextCentered(GET_STR(STR_EAT_SANDWICH), 125, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_SQUID_BOCATA), 140, PLT_BRIGHT_WHITE);
    
    // Silence audio
    if (music_on) cpct_akp_stop();

    while(1) {
        cpct_waitVSYNC();
        updateStarfield();
        cpct_scanKeyboard_f();
        if (cpct_isKeyPressed(Key_Space)) return 0;
        if (cpct_isKeyPressed(Key_Esc)) return 1;
    }
}

u8 showGameOver() {
    cpct_memset((void*)0xC000, 0x00, 0x4000);
    initStarfield();

    // Draw "GAME OVER" in red (Large)
    drawCustomTextLargeCentered(GET_STR(STR_GAME_OVER), 60, PLT_ORANGE);

    // Draw funny subtext (Small)
    drawCustomTextCentered(GET_STR(STR_NO_MOTO_NO_EAT), 90, PLT_BRIGHT_YELLOW);
    
    // Silence audio because the draw loop is stopped
    if (music_on) cpct_akp_stop();

    while(1) {
        cpct_waitVSYNC();
        updateStarfield();
        cpct_scanKeyboard_f();
        if (cpct_isKeyPressed(Key_Space)) return 0;
        if (cpct_isKeyPressed(Key_Esc)) return 1;
    }
}

void initPaddle() {
    paddle.x = (SCREEN_WIDTH_BYTES - PADDLE_WIDTH_BYTES) / 2;
    paddle.y = PADDLE_START_Y;
    paddle.width = PADDLE_WIDTH_BYTES;
    paddle.height = PADDLE_HEIGHT;
}

void initBall() {
    ball.width = BALL_SIZE_BYTES;
    ball.height = BALL_HEIGHT;
    ball.active = 0; // Attached to paddle initially
    ball.x = paddle.x + (paddle.width / 2);
    ball.y = paddle.y - ball.height;
    
    ball.fx = (i16)ball.x * FP_SCALE;
    ball.fy = (i16)ball.y * FP_SCALE;
    
    ball.speed = getNormalBallSpeed(); // Start speed
    ball.dir_x = 0;
    ball.dir_y = 0;
    
    ball.old_x = ball.x;
    ball.old_y = ball.y;
}

// Helper to check if an enemy at (x, y) overlaps any non-empty brick.
// Returns 1 if hit, 0 otherwise.
u8 checkEnemyBrickCollision(u8 x, u8 y) {
    if (y + ENEMY_HEIGHT <= BRICK_START_Y || y >= BRICK_START_Y + BRICK_ROWS * (BRICK_HEIGHT + BRICK_GAP_Y)) {
        return 0;
    }
    for (u8 r = 0; r < BRICK_ROWS; r++) {
        u8 by = brick_y[r];
        if (y + ENEMY_HEIGHT <= by) break; 
        if (y >= by + BRICK_HEIGHT) continue;
        
        for (u8 c = 0; c < BRICK_COLS; c++) {
            u8 bx = brick_x[c];
            if (x + ENEMY_WIDTH <= bx) break; 
            if (x >= bx + BRICK_WIDTH_BYTES) continue;
            
            if (bricks[r][c] != BSTATE_EMPTY && bricks[r][c] != BSTATE_NEEDS_ERASE) {
                return 1;
            }
        }
    }
    return 0;
}

void initEnemies() {
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemies[i].old_x = 0xFF; // dummy to mark clean
    }
    enemy_spawn_timer = 200;
}

void updateEnemies() {
    u8 i;
    
    // Spawn logic: Regular levels only. Boss handles its own spawning.
    if (!boss.active && active_bricks <= enemy_spawn_threshold && active_bricks > 0 && !door_open) {
        if (enemy_spawn_timer > 0) {
            enemy_spawn_timer--;
        } else {
            for (i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) {
                    enemies[i].active = 1;
                    enemies[i].x = (rand8() & 1) ? 20 : 56;
                    enemies[i].y = WALL_TOP; 
                    enemies[i].old_x = enemies[i].x;
                    enemies[i].old_y = enemies[i].y;
                    enemies[i].color = COLOR_BRICK_BASE + (rand8() % 7);
                    enemies[i].type = rand8() % NUM_ENEMY_SPRITES;
                    enemies[i].dx = (rand8() & 1) ? 1 : -1;
                    enemies[i].frame = 0;
                    enemy_spawn_timer = 250 + (rand8() & 127);
                    break;
                }
            }
        }
    }

    // Move logic
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemy_t *e = &enemies[i];
        if (!e->active) continue;
        
        e->old_x = e->x;
        e->old_y = e->y;
        e->frame++;
        
        // 1. Vertical movement
        if ((e->frame & 3) == 0) {
            e->y += ENEMY_SPEED_Y;
            // Check vertical collision
            if (checkEnemyBrickCollision(e->x, e->y)) {
                e->y = e->old_y; // Blocked vertically, stay on top or bottom
            }
        }
        
        // 2. Horizontal movement
        if ((e->frame & 7) == 0) {
            e->x += e->dx;
            // Check wall collision
            if (e->x < WALL_LEFT_BYTES) {
                e->x = WALL_LEFT_BYTES;
                e->dx = 1;
            } else if (e->x + ENEMY_WIDTH > WALL_RIGHT_BYTES) {
                e->x = WALL_RIGHT_BYTES - ENEMY_WIDTH;
                e->dx = -1;
            } else {
                // Check brick horizontal collision
                if (checkEnemyBrickCollision(e->x, e->y)) {
                    e->x = e->old_x;
                    e->dx = -e->dx; // Bounce horizontally
                }
            }
        }

        // Periodic random-ish direction change (every ~2.5 seconds) based on frame counter
        if ((e->frame & 0x7F) == (i << 5)) {
            e->dx = -e->dx;
        }

        if (enemies[i].y >= SCREEN_HEIGHT) {
            enemies[i].active = 0;
            continue;
        }

        // Collide with paddle
        if (enemies[i].y + ENEMY_HEIGHT >= paddle.y && enemies[i].y <= paddle.y + paddle.height) {
            if (enemies[i].x + ENEMY_WIDTH >= paddle.x && enemies[i].x <= paddle.x + paddle.width) {
                score += 50;
                hud_dirty = 1;
                enemies[i].active = 0;
            }
        }
    }
}

void drawEnemies() {
    u8 i;
    u8* pVideoMemory;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemy_t *e = &enemies[i];
        if (!e->active) {
            if (e->old_x != 0xFF) {
                drawBackgroundRect(e->old_x, e->old_y, ENEMY_WIDTH, ENEMY_HEIGHT);
                e->old_x = 0xFF;
                e->old_y = 0xFF;
            }
            continue;
        }
        
        // 1. Erase only if moved
        if (e->old_x != e->x || e->old_y != e->y) {
            if (e->old_x != 0xFF) {
                drawBackgroundRect(e->old_x, e->old_y, ENEMY_WIDTH, ENEMY_HEIGHT);
            }
        }

        // 2. Draw current state
        if (e->type < NUM_ENEMY_SPRITES) {
            pVideoMemory = cpct_getScreenPtr((void*)0xC000, e->x, e->y);
            cpct_drawSprite(enemy_sprites[e->type], pVideoMemory, ENEMY_WIDTH, ENEMY_HEIGHT);
        }

        // 3. Sync old coordinates to match what's on screen
        e->old_x = e->x;
        e->old_y = e->y;
    }
}
  
// =============================================================================
// HARDWARE INTERRUPT (300 Hz)
// The Amstrad CPC triggers an interrupt 300 times per second.
// Arkos Tracker music needs to be called 50 times per second (50Hz).
// So we use a counter to call it every 6 interrupts (300 / 6 = 50).
// =============================================================================
void music_isr(void) {
    static u8 tick = 0;
    tick++;
    if (tick == 6) {
        tick = 0;
        if (music_on && !paused) {
            cpct_akp_musicPlay();
        }
    }
}

u8 showStory() {
    cpct_memset((void*)0xC000, 0x00, 0x4000);
    initStarfield();

    // --- CENTERING INSTRUCTIONS ---
    // The screen is 80 bytes wide.
    // For drawCustomText: each character is 2 bytes wide.
    // Formula: x = (80 - (number_of_chars * 2)) / 2
    // For drawCustomTextLarge: each character is 3 bytes wide.
    // Formula: x = (80 - (number_of_chars * 3)) / 2
    // ------------------------------

    // Story text — centered automatically with helpers
    drawCustomTextCentered(GET_STR(STR_STORY_1), 25, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_STORY_2), 40, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_STORY_3), 55, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_STORY_4), 80, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_STORY_5), 95, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_STORY_6), 120, PLT_BRIGHT_WHITE);
    drawCustomTextCentered(GET_STR(STR_STORY_7), 135, PLT_BRIGHT_WHITE);
    drawCustomTextLargeCentered(GET_STR(STR_STORY_WIN_1), 160, PLT_ORANGE);
    drawCustomTextLargeCentered(GET_STR(STR_STORY_WIN_2), 175, PLT_ORANGE);

    while (1) {
        cpct_waitVSYNC();
        
        // --- Starfield Update ---
        updateStarfield();
        
        cpct_scanKeyboard_f();
        if (cpct_isKeyPressed(Key_Space)) return 0;
        if (cpct_isKeyPressed(Key_Esc)) return 1;
    }
}

void drawIntroContent() {
    // Draw 2x scaled title with multiple colors
    drawCustomTextXLarge((const u8*)"BR", 3, 40, PLT_CYAN);
    drawCustomTextXLarge((const u8*)"IC", 15, 40, PLT_GREEN);
    drawCustomTextXLarge((const u8*)"K", 27, 40, PLT_BRIGHT_GREEN);
    // (Space at 33)
    drawCustomTextXLarge((const u8*)"B", 39, 40, PLT_ORANGE);
    drawCustomTextXLarge((const u8*)"LA", 45, 40, PLT_BRIGHT_RED);
    drawCustomTextXLarge((const u8*)"ST", 57, 40, PLT_BRIGHT_MAGENTA);
    drawCustomTextXLarge((const u8*)"ER", 69, 40, PLT_MAUVE);

    // Draw player selection options (aligned to column)
    drawCustomText(GET_STR(STR_1_PLAYER),   26, 90, PLT_BRIGHT_WHITE);
    drawCustomText(GET_STR(STR_2_PLAYERS),  26, 105, PLT_BRIGHT_WHITE);
    drawCustomText(GET_STR(STR_PRESS_H_HELP), 26, 120, PLT_BRIGHT_WHITE);

    // Draw credits
    drawCustomTextCentered(GET_STR(STR_CREDITS_CODE), 155, PLT_BRIGHT_YELLOW);
    drawCustomTextCentered(GET_STR(STR_CREDITS_MUSIC), 168, PLT_BRIGHT_YELLOW);
    drawCustomTextCentered(GET_STR(STR_CREDITS_POWERED), 181, PLT_BRIGHT_YELLOW);
    drawCustomTextCentered((const u8*)"2026", 194, PLT_BRIGHT_YELLOW);
}

void showIntro() {
    u8 i;
    u8 frame = 0;
    u8 pal[16];
    u8 tx = 1;
    u8 ty = 0;
    u8 old_ty = 0;
    u8 target_ty = 40;
    
    // Copy the default hardware palette
    for(i=0; i<16; i++) pal[i] = hw_palette[i];

    // Clear the screen to black
    cpct_memset((void*)0xC000, 0x00, 0x4000);
    
    // Initialize standard parallax starfield
    initStarfield();
    
    // Draw intro screen content
    drawIntroContent();

    while (1) {
        cpct_waitVSYNC();
        frame++;
        updateStarfield();

        cpct_scanKeyboard_f();
        if (cpct_isKeyPressed(Key_1)) {
            num_players = 1;
            break;
        }
        if (cpct_isKeyPressed(Key_2)) {
            num_players = 2;
            break;
        }
        if (cpct_isKeyPressed(Key_H)) {
            showControls();
            
            // Redraw intro screen after returning from help
            cpct_memset((void*)0xC000, 0x00, 0x4000);
            initStarfield();
            drawIntroContent();
        }
    }

    // Restore the regular game palette before continuing
    cpct_setPalette(hw_palette, 16);
}

// Controls screen: key controls. Shown once after the intro.
// Mode 0: each char = 4 bytes. Screen = 80 bytes = 20 chars wide.
// Center formula: x = (80 - chars * 4) / 2
u8 showControls() {
    cpct_memset((void*)0xC000, 0x00, 0x4000);
    initStarfield();

    drawCustomTextLargeCentered(GET_STR(STR_CONTROLS), 10, PLT_ORANGE);

    // Key listings (left-aligned from x=18, value from x=42)
    // Row format:  [KEY]  [ACTION]
    drawCustomText(GET_STR(STR_KEY_LEFT),  18, 35, PLT_BRIGHT_WHITE);
    drawCustomText(GET_STR(STR_MOVE_LEFT), 42, 35, PLT_BRIGHT_YELLOW);

    drawCustomText(GET_STR(STR_KEY_RIGHT), 18, 55, PLT_BRIGHT_WHITE);
    drawCustomText(GET_STR(STR_MOVE_RIGHT), 42, 55, PLT_BRIGHT_YELLOW);

    drawCustomText(GET_STR(STR_KEY_SPACE),   18, 75, PLT_BRIGHT_WHITE);
    drawCustomText(GET_STR(STR_FIRE),      42, 75, PLT_BRIGHT_YELLOW);

    drawCustomText(GET_STR(STR_KEY_ESC),     18, 95, PLT_BRIGHT_WHITE);
    drawCustomText(GET_STR(STR_PAUSE),    42, 95, PLT_BRIGHT_YELLOW);

    drawCustomText(GET_STR(STR_KEY_M),       18, 115, PLT_BRIGHT_WHITE);
    drawCustomText(GET_STR(STR_MUSIC),    42, 115, PLT_BRIGHT_YELLOW);



    while (1) {
        cpct_waitVSYNC();
        updateStarfield();
        cpct_scanKeyboard_f();
        if (cpct_isKeyPressed(Key_Space)) {
            if (showCapsuleDocs()) return 1;
            break;
        }
        if (cpct_isKeyPressed(Key_Esc)) return 1;
    }
    return 0;
}

// Humorous capsule documentation screen.
u8 showCapsuleDocs() {
    u8 page;
    for (page = 0; page < 2; page++) {
        cpct_memset((void*)0xC000, 0x00, 0x4000);
        initStarfield();

        drawCustomTextLargeCentered(GET_STR(STR_CAPSULE_GUIDE), 5, PLT_ORANGE);
        drawCustomTextCentered(GET_STR(STR_CLUELESS), 17, PLT_ORANGE);
        
        if (page == 0) {
            // Page 1: L, S, C, P, B, E, M, I
            cpct_drawSprite(powerup_sprites[0],  cpct_getScreenPtr((void*)0xC000, 8, 35), 3, 11);
            drawCustomText(GET_STR(STR_L_DESC), 16, 37, PLT_BRIGHT_RED);
            cpct_drawSprite(powerup_sprites[1],  cpct_getScreenPtr((void*)0xC000, 8, 52), 3, 11);
            drawCustomText(GET_STR(STR_S_DESC), 16, 54, PLT_BRIGHT_YELLOW);
            cpct_drawSprite(powerup_sprites[2],  cpct_getScreenPtr((void*)0xC000, 8, 69), 3, 11);
            drawCustomText(GET_STR(STR_C_DESC), 16, 71, PLT_BRIGHT_GREEN);
            cpct_drawSprite(powerup_sprites[3],  cpct_getScreenPtr((void*)0xC000, 8, 86), 3, 11);
            drawCustomText(GET_STR(STR_P_DESC), 16, 88, PLT_BRIGHT_WHITE);
            cpct_drawSprite(powerup_sprites[4],  cpct_getScreenPtr((void*)0xC000, 8, 103), 3, 11);
            drawCustomText(GET_STR(STR_B_DESC), 16, 105, PLT_CYAN);
            cpct_drawSprite(powerup_sprites[5],  cpct_getScreenPtr((void*)0xC000, 8, 120), 3, 11);
            drawCustomText(GET_STR(STR_E_DESC), 16, 122, PLT_BRIGHT_CYAN);
            cpct_drawSprite(powerup_sprites[6],  cpct_getScreenPtr((void*)0xC000, 8, 137), 3, 11);
            drawCustomText(GET_STR(STR_M_DESC), 16, 139, PLT_BRIGHT_MAGENTA);
            cpct_drawSprite(powerup_sprites[7],  cpct_getScreenPtr((void*)0xC000, 8, 154), 3, 11);
            drawCustomText(GET_STR(STR_I_DESC), 16, 156, PLT_BRIGHT_CYAN);
        } else {
            // Page 2: U, A, D, V, T, G, X
            cpct_drawSprite(powerup_sprites[8],  cpct_getScreenPtr((void*)0xC000, 8, 35), 3, 11);
            drawCustomText(GET_STR(STR_U_DESC), 16, 37, PLT_MAUVE);
            cpct_drawSprite(powerup_sprites[9],  cpct_getScreenPtr((void*)0xC000, 8, 52), 3, 11);
            drawCustomText(GET_STR(STR_A_DESC), 16, 54, PLT_BRIGHT_GREEN);
            cpct_drawSprite(powerup_sprites[10], cpct_getScreenPtr((void*)0xC000, 8, 69), 3, 11);
            drawCustomText(GET_STR(STR_D_DESC), 16, 71, PLT_MAUVE);
            cpct_drawSprite(powerup_sprites[11], cpct_getScreenPtr((void*)0xC000, 8, 86), 3, 11);
            drawCustomText(GET_STR(STR_V_DESC), 16, 88, PLT_ORANGE);
            cpct_drawSprite(powerup_sprites[12], cpct_getScreenPtr((void*)0xC000, 8, 103), 3, 11);
            drawCustomText(GET_STR(STR_T_DESC), 16, 105, PLT_BRIGHT_RED);
            cpct_drawSprite(powerup_sprites[13], cpct_getScreenPtr((void*)0xC000, 8, 120), 3, 11);
            drawCustomText(GET_STR(STR_G_DESC), 16, 122, PLT_GREEN);
            cpct_drawSprite(powerup_sprites[14], cpct_getScreenPtr((void*)0xC000, 8, 137), 3, 11);
            drawCustomText(GET_STR(STR_F_DESC), 16, 139, PLT_BRIGHT_RED);
        }



        while (1) {
            cpct_waitVSYNC();
            updateStarfield();
            cpct_scanKeyboard_f();
            if (cpct_isKeyPressed(Key_Space)) break;
            if (cpct_isKeyPressed(Key_Esc)) return 1;
        }
        // Wait for release
        while (cpct_isKeyPressed(Key_Space)) {
            cpct_waitVSYNC();
            cpct_scanKeyboard_f();
        }
    }
    return 0;
}

// Redundant function showModeSelect removed

void showPlayerStart(u8 p) {
    u8 str[32]; // Increased buffer for localized strings
    u8 i;
    cpct_memset((void*)0xC000, 0x00, 0x4000);
    initStarfield();
    
    // Safety copy with null termination
    for(i=0; i<28; ++i) { // Smaller limit to leave space for " X"
        str[i] = GET_STR(STR_PLAYER_START)[i];
        if (str[i] == '\0') break;
    }
    // Append space and player number
    str[i] = ' ';
    str[i+1] = '1' + p;
    str[i+2] = '\0';
    
    drawCustomTextLargeCentered(str, 80, PLT_BRIGHT_YELLOW);


    while (1) {
        cpct_waitVSYNC();
        updateStarfield();
        cpct_scanKeyboard_f();
        if (cpct_isKeyPressed(Key_Space) || (g_use_joystick && cpct_isKeyPressed(Joy0_Fire1))) break;
    }
    
    // Clear screen before starting
    cpct_memset((void*)0xC000, 0x00, 0x4000);
    drawBackground();
    drawHUD();
}

u8 showLevelSelect() {
    u8 selection = 0;
    u8 joyLR_pressed = 0; // to prevent fast auto-repeat
    u8 str_buf[10];
    u8 dirty = 1;

    // Fill screen with background color
    cpct_memset((void*)0xC000, 0x00, 0x4000);
    
    initStarfield();

    drawCustomTextLargeCentered(GET_STR(STR_LEVEL), 60, PLT_ORANGE);
    
    while(1) {
        cpct_waitVSYNC();
        
        updateStarfield();
        
        if (dirty) {
            if (selection + 1 < 10) {
                str_buf[0] = '0' + (selection + 1);
                str_buf[1] = '\0';
            } else {
                str_buf[0] = '0' + ((selection + 1) / 10);
                str_buf[1] = '0' + ((selection + 1) % 10);
                str_buf[2] = '\0';
            }
            
            // Draw over the old selection (erase with black first)
            cpct_drawSolidBox(cpct_getScreenPtr((void*)0xC000, 35, 100), 0x00, 10, 16);
            drawCustomTextLargeCentered((const u8*)str_buf, 100, PLT_BRIGHT_YELLOW);
            dirty = 0;
        }

        cpct_scanKeyboard_f();
        
        if (cpct_isKeyPressed(Key_Esc)) return 1;

        if (cpct_isKeyPressed(g_key_left) || (g_use_joystick && cpct_isKeyPressed(Joy0_Left))) {
            if (!joyLR_pressed) {
                if (selection > 0) selection--;
                else selection = NUM_LEVELS - 1;
                joyLR_pressed = 1;
                dirty = 1;
            }
        } 
        else if (cpct_isKeyPressed(g_key_right) || (g_use_joystick && cpct_isKeyPressed(Joy0_Right))) {
            if (!joyLR_pressed) {
                if (selection < NUM_LEVELS - 1) selection++;
                else selection = 0;
                joyLR_pressed = 1;
                dirty = 1;
            }
        } 
        else {
            joyLR_pressed = 0;
        }

        if (cpct_isKeyPressed(g_key_fire) || (g_use_joystick && cpct_isKeyPressed(Joy0_Fire1))) {
            current_level = selection;
            
            // Wait for release
            while (cpct_isKeyPressed(g_key_fire) || (g_use_joystick && cpct_isKeyPressed(Joy0_Fire1))) {
                cpct_waitVSYNC();
                cpct_scanKeyboard_f();
            }
            return 0;
        }

    }
}

// ──────────────────────────────────────────────────────────────
// Check and handle Pause (ESC) and Music (M) toggles with edge detection
void handleGameToggles() {
    u8 esc_now = cpct_isKeyPressed(g_key_pause);
    u8 m_now   = cpct_isKeyPressed(g_key_music);

    // ESC: toggle pause (edge detection)
    if (esc_now && !key_esc_held) {
        paused ^= 1;
        hud_dirty = 1; // Force HUD to redraw/clear "PAUSE" text
        if (paused) {
            muteAY();            // Kill any ringing note immediately
        } else {
            // Erase the PAUSE text area: 20 bytes wide (5 chars x 4 bytes in Mode 0), 8 lines
            drawBackgroundRect(30, 96, 20, 8);
        }
    }
    key_esc_held = esc_now;

    // M: toggle music+sfx (edge detection)
    if (m_now && !key_m_held) {
        music_on ^= 1;
        if (!music_on) {
            cpct_akp_stop();
        } else {
            cpct_akp_musicInit(music_song);
        }
    }
    key_m_held = m_now;
}

void main(void) {
    u8 i;
    // VERY IMPORTANT for CPCtelera with --no-std-crt0:
    // Global initialized variables (e.g. u8 x = 5) are NOT automatically initialized!
    // We must copy our const default palette into the RAM hw_palette manually.
    for (i = 0; i < 16; i++) {
        hw_palette[i] = default_palette[i];
    }

    // Disable firmware to take full control
    cpct_disableFirmware();

    // Initialize RNG used by stars, bricks, and ball bounces
    rng_seed = 0xACE1;

    // Set screen mode 0 (16 colors)
    cpct_setVideoMode(0);

    // Set hardware palette
    cpct_setPalette(hw_palette, 16);
    
    // Disable Amstrad border
    cpct_setBorder(HW_BLACK);

    // Initialize Arkos Tracker music & SFX (uses same .aks instruments for both)
    // NOTE: 'music_song' array name is auto-generated by cpct_aks2c. You defined it in music_conversion.mk.
    // If you add your own song in the future, change this name to your new array.
    cpct_akp_musicInit(music_song);
    cpct_akp_SFXInit(music_song);
    music_on = 1; // disabled because we are still testing
    key_m_held = 0;
    
    // Default keys init
    g_key_left  = Key_CursorLeft;
    g_key_right = Key_CursorRight;
    g_key_fire  = (u16)Key_Space;
    g_key_pause = (u16)Key_Esc;
    g_key_music = (u16)Key_M;
    g_use_joystick = 0;

    // Register the custom ISR so the music plays automatically in the background
    cpct_setInterruptHandler(music_isr);

    // Main game loop (returns here after Game Over or Victory)
    while (1) {
        // Show the animated intro
        showIntro();

        // Show the game story
        if (showStory()) continue;

        // Then jump into the Level Selection menu
        if (showLevelSelect()) continue;

        // Pre-compute brick pixel position lookup tables (once, from #defines)
        initBrickPositions();

        // Initialize game state for a new session
        lives = 3;
        game_won = 0;
        initGame();
        
        drawBackground();
        drawHUD();

        // Inner play loop: runs until all lives are gone or game is won
        while (lives > 0 && !game_won) {
            // --- Input and Toggles ---
            cpct_scanKeyboard_f();
            handleGameToggles();

            // --- Update (skipped when paused) ---
            if (!paused) {
                updatePaddle();
                updateBall();
                updateExtraBalls();
                updateDrop();
                updateLasers();
                updateEnemies();
                if (boss.active || boss.state == 2) updateBoss();
            }

            // --- Draw (always called: maintains VSYNC, music tick, pause text) ---
            drawGame();
        }

        if (game_won) {
            showVictory();
        } else {
            // Lives reached 0, game over
            showGameOver();
        }
    }
}
