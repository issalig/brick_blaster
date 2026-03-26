# Brick Blaster - Technical Architecture

This document provides a deep dive into the internal architecture, data structures, and logic of **Brick Blaster** for the Amstrad CPC.

This game has been developed using AI assistance. The AI was able to generate the entire game code, including graphics, sound, and music, as well as the web-based emulator portal and the documentation for the game. But think of it as an iterative and collaborative process between me and the AI.

For this reason, some statements may not be entirely accurate or correct. If you find any inaccuracies, please feel free to contact me.

## 1. Technical Overview

-   **Platform**: Amstrad CPC 464/6128 (64KB minimum).
-   **Video Mode**: Mode 0 (160x200, 16 colors).
-   **Language**: C (SDCC 3.6.8) with inline Z80 Assembly for hot paths.
-   **Framework**: CPCtelera 1.7.

## 2. Core Data Structures

The game is designed around several key structures defined in `src/main.c`:

### Physics & Entities
-   **`ball_t`**: Uses 10.6 Fixed Point math for smooth sub-pixel movement.
    -   `x`, `y`: Current screen coordinates (byte-aligned X, scanline Y).
    -   `old_x`, `old_y`: Previous coordinates used for Delta Erasing.
    -   `fpos_x`, `fpos_y`: Internal 16-bit fixed-point positions.
    -   `dir_x`, `dir_y`: Direction vector components (normalized to `FP_SCALE`).
    -   `speed`: Scalar speed value.
    -   `speed_x`, `speed_y`: Precomputed velocity components (`speed_x = (dir_x * speed) >> 6`).
    -   `active`: Flag for multiball handling.
-   **`paddle_t`**: Manages position and variable widths (Normal, Wide, Tiny).
    -   `x`, `y`: Current screen coordinates.
    -   `old_x`, `old_y`: Previous coordinates for Delta Erasure.
    -   `width`, `height`: Current dimensions.
    -   `old_width`: Used to detect size changes for full background restoration.
-   **`boss_t`**: A state-machine based structure for the final encounter, using fixed-point physics similar to the ball.
    -   `fpos_x`, `fpos_y`, `speed_x`, `speed_y`: 10.6 Fixed-point physics state.
    -   `x`, `y`, `old_x`, `old_y`: Screen and previous coordinates.
    -   `state`: 0=Sleeping, 1=Attacking, 2=Exploding.
    -   `health`: Hit point tracking.
-   **`demo_mode`**: A global flag that enables autonomous gameplay.
    -   **Autopilot AI**: Automatically steers the paddle towards the ball's X-coordinate.
    -   **Auto-Fire**: Automatically fires lasers when the power-up is active.
    -   **Auto-Launch**: Launches the ball after a 2-second delay if it's held.
    -   **Auto-Exit**: Detects when the exit door is open and steers the paddle toward it.
-   **`enemy_t`**: Represents flying hazards that spawn periodically.
    -   `x`, `y`, `old_x`, `old_y`: Screen and previous coordinates for rendering.
    -   `type`: Determines the sprite and movement pattern.
    -   `speed_x`: Horizontal drift speed.
    -   **Interaction**: Enemies serve as moving obstacles and scoring targets. When hit by a ball, they reflect it; when they touch the paddle, they are destroyed and award bonus points (**500 HUD pts**) without costing a life.
-   **`drop_t`**: Represents power-up capsules that fall from destroyed bricks.
    -   `x`, `y`: Current screen coordinates.
    -   `old_y`: Previous Y coordinate (for Delta Erasing).
    -   `active`: Flag (1 if falling, 0 if inactive).
    -   `power_type`: Character code identifying the benefit (e.g., 'L', 'E', 'B').
    -   `timer`: Used to throttle the vertical fall speed.
-   **`laser_pair_t`**: Manages the dual beams fired when the Laser power-up is active.
    -   `x_left`, `x_right`: Fixed horizontal offsets for each beam.
    -   `y`, `old_y`: Current and previous vertical positions.
    -   `active`: Flag for visibility and collision processing.
-   **`powerup_state_t`**: Encapsulates all active power-up flags and their associated timers.
    -   `glue_active`, `glue_timer`: Sticky paddle state.
    -   `expand_active`, `tiny_active`: Paddle size modifiers.
    -   `laser_active`, `laser_fire_timer`: Combat state.
    -   `slow_active`, `fast_active`, `fireball_active`: Ball physics modifiers.
    -   `freeze_active`, `freeze_timer`: Ice trap state.
    -   `drunk_active`: Inverted controls state.
    -   `magnet_active`: Ball repulsion trap.
    -   `autopilot_active`, `autopilot_timer`: AI help state.
-   **`player_state_t`**: Encapsulates the complete context for a player in 2-player mode.
    -   `lives`, `score`, `current_level`: Standard progression trackers.
    -   `bricks[10][10]`, `active_bricks`: The unique brick layout state for this player.
    -   `level_cleared`: Flag used to manage transitions between players or levels.

### Brick System & Level Encoding
The core of the level design is a compact 2D grid (`bricks[10][10]`). To minimize memory usage while maximizing features, each cell is a single byte packed with metadata:

-   **Bits 0-1 (Type)**: `0=Empty`, `1=Normal` (1 HP), `2=Hard` (2 HP), `3=Gold` (Invincible).
-   **Bits 2-3 (Color)**: Indexes one of 4 predefined color pairs for the brick's gradient.
-   **Bits 4-7 (Power-up)**: Defines which capsule (if any) will drop upon destruction.

#### Level Lifecycle:
1.  **Static Data**: Levels are stored as a 3D constant array `level_data[NUM_LEVELS][10][10]` in ROM.
2.  **Runtime Loading**: When a level starts, this data is copied into the mutable RAM array `bricks[10][10]`.
3.  **Sprite Selection**: The function `getBrickSpriteIndex(r, c)` centralizes the logic for picking the correct sprite based on brick type and color.
4.  **Destruction Workflow**: When the ball hits a brick:
    -   If HP > 1: Type is decremented but the brick remains.
    -   If HP == 1: The brick is marked with a special flag `BSTATE_NEEDS_ERASE`.
    -   **Deferred Erasure**: The main loop detects the erase flag, triggers the power-up drop if applicable, and restores the background pattern to "erase" the brick visually without a full screen redraw.
4.  **Completion Condition**: The game tracks a counter `active_bricks`. Only `BTYPE_NORMAL` and `BTYPE_HARD` contribute to this count. When `active_bricks` reaches 0, the current level is marked as cleared (`level_cleared = 1`), and the transition logic is triggered.

The game features three selectable difficulty levels (Easy, Normal, Hard) that control the ball's speed expansion rate and the probability of power-up drops in `assignPowerups()`:

| Difficulty | Speed Incr. | Power-up % |
| :--- | :--- | :--- |
| **Easy (0)** | +8 units | 50% |
| **Normal (1)** | +15 | 30% |
| **Hard (2)** | +20 | 15% |

- **Formula**: `INITIAL_BALL_SPEED + (current_level / 2) * increment`.
- **Key Logic**: The **Left/Right** keys cycle through options in the main menu. The **Space/Fire** key cycles through difficulty levels only when that specific menu option is selected. To prevent animation stutter, it uses optimized **partial redrawing**, refreshing only the difficulty text line during menu interaction.

## 4. Scoring System

The game uses a `u16` internal counter with a "Zero Hack" (virtual 10x multiplier) for display. The following points are awarded (values shown as they appear on the HUD):

| Event | HUD Points | Internal Increments |
| :--- | :--- | :--- |
| **Normal Brick** | 50 pts | `+5` |
| **Hard Brick (Hit)** | 20 pts | `+2` |
| **Hard Brick (Destroyed)** | 50 pts | `+5` |
| **Enemy (Hit by Ball)** | 500 pts | `+50` |
| **Enemy (Hit by Paddle)** | 500 pts | `+50` |
| **Power-up Caught** | 0 pts* | *Effect only* |
| **Boss (Per Hit)** | 100 pts | `+10` |
| **Boss (Defeated)** | 10,000 pts | `+1000` |

*\*Note: Some power-ups like multi-ball or laser indirectly lead to much higher scores through rapid brick destruction.*

## 5. Physics & Dynamics

### Fixed Point Arithmetic
To avoid slow 16-bit multiplications and divisions on the Z80, the game uses a **10.6 Fixed Point** system (`FP_SCALE = 64`). 
-   Movements are calculated as `pos += velocity`.
-   Rendering coordinates are derived using `FP_INT(pos)` (right-shift by 6).
-   Velocities are precomputed using `FP_VEL` macros to avoid runtime overhead.

### Ball Movement Logic (`move_ball`)
The `move_ball` function handles the core displacement and basic environmental collisions:
1.  **Integration**: Fixed-point positions are updated: `fpos_x += speed_x` and `fpos_y += speed_y`.
2.  **Special Effects**: If the **Magnet Trap** is active, the ball's `fpos_x` is nudged away from the paddle's center when in close proximity.
3.  **Coordinate Conversion**: Internal positions are converted to screen coordinates via `FP_INT` (right-shift by 6).
4.  **Boundary Bouncing**:
    -   If a wall or the ceiling is hit (checked against `WALL_XXX_FP` thresholds), the corresponding direction component (`dir_x` or `dir_y`) is negated.
    -   The velocity components `speed_x` and `speed_y` are then recomputed using `FP_VEL` to ensure the trajectory is consistent with the current speed.
5.  **Clamping**: Final screen coordinates are clamped to the play area boundaries to prevent visual artifacts or HUD overwrites.

### Collision Engine
1.  **Wall Bouncing**: Hardcoded boundaries based on the level frame.
2.  **Paddle Reflection**: The ball's horizontal direction is influenced by *where* it hits the paddle (segmented bounce angles).
3.  **Brick Collision**:
    -   A simplified AABB check against the `bricks` grid.
    -   `BTYPE_GOLD` bricks reflect but never break.
    -   `BTYPE_HARD` requires two hits.
    -   `fireball_active` flag skips velocity reflection for piercing effects.

## 4. Rendering Strategy

The Amstrad CPC lacks hardware scrolling and a standard double buffer in this project. To prevent flickering, we use a **Delta Erase / Selective Redraw** strategy:

1.  **Wait VSYNC**: Synchronize with the monitor's vertical refresh.
2.  **Delta Erase**: Only the "old" area of a moving sprite (paddle, ball, enemies) is restored using the background tiled pattern.
3.  **Background Cache**: The level's tiled pattern is cached in `bg_row_cache` (8x80 bytes) during level initialization for blazing-fast redraws.
4.  **Direct Drawing**: Sprites are drawn directly to the Video RAM (`0xC000`).

## 5. Localization System

The game supports English, Spanish, French, Greek, and Valencian via a specialized header-and-source strategy:
-   **`lang.h`**: Defines a `string_id_t` enum shared across the logic.
-   **`lang_xx.c`**: Contains the actual constant strings for each language.
-   **`lang_strings`**: A global pointer array indexed by `current_lang` used by the `GET_STR(id)` macro for O(1) string retrieval.

## 6. Custom Sprite Font System

Brick Blaster bypasses the Amstrad CPC's standard firmware font to gain full control over styling, coloring, and scaling.

-   **Sprite-Based Glyph Engine**: Every character (A-Z, 0-9, and symbols) is stored as a monochrome 8x8 sprite (3x8 bytes in Mode 0).
-   **Dynamic Coloring**: The `draw` functions take a palette index and use `cpct_px2byteM0` to create a bitmask. This allows rendering the exact same font data in any of the 16 available colors.
-   **Multi-Scale Rendering**:
    -   **Normal**: 1:1 mapping.
    -   **Large**: 2x horizontal scaling.
    -   **X-Large**: 2x horizontal and 2x vertical scaling (used for the Intro Title).
-   **Localization Hooks**: The `get_sprite_index` function includes manual UTF-8 decoding to support special characters across languages (e.g., Spanish `¡`, `ñ`, and the Greek alphabet via Greeklish). Portuguese is supported using unaccented capital letters to maintain font compatibility.

## 7. Memory Map

| Address Range | Description |
| :--- | :--- |
| `&0500 - &1ABF` | Music Song Data (Arkos Tracker) |
| `&1C00 - &94D1` | Game Code, Compiled Sprites, and Font Data |
| `&94D2 - &BFFF` | Global Variables & Background Row Cache |
| `&C000 - &FFFF` | Video RAM (Mode 0) |

## 8. Arcade Tricks & Optimizations

Beyond the core structures, several "dirty" but effective arcade tricks are used to enhance the user experience and performance:

### The "Zero Hack" (Virtual 32-bit Scoring)
To avoid the overhead of 32-bit math on an 8-bit CPU, the score is internally tracked as a `u16` (max 65,535). However, a constant '0' is appended to the score string during HUD rendering. This makes a 1,000-point brick feel like a 10,000-point achievement, mimicking high-stakes arcade scoring systems with zero CPU cost.

### Python-Powered Asset Pipeline
Rather than using heavy editors, the game uses custom Python scripts to convert ASCII art and modern PNGs directly into the bit-packed, interleaved Mode 0 format.
-   **`img2scr.py`**: Handles screen-sized graphics like the Intro Loading screens and background patterns.
-   **`gen_sprites.py`**: Generates all game entities (Ball, Paddle, Boss, Enemies, Power-ups) and the custom font system directly into the `src/assets/sprites.h` and `src/assets/boss.h` headers.
This automated pipeline allows for rapid iteration of graphics (like the animated Fireball or localized loading screens).

### O(1) Language Switching
The localization system avoids string searching or heavy branching. Instead, it uses a global array of pointers (`lang_strings`). Switching the entire game's language is as simple as updating a single `current_lang` index, making `GET_STR` operations nearly instantaneous.

### Web Portal Structure
The web portal's directory structure is organized as follows:
-   `index.html`: Main entry point with the arcade/neon theme.
-   `emulator.html`: Isolated container for the RVM Player.
-   `style.css`: Visual styling and animations (starfield, 8-bit buttons).
-   `assets/`: Core UI assets (e.g., `i18n.js`).
-   `assets/disks/`: Dynamic game data generated by the build system (localized `.dsk`, `.sna`, and RVM-compatible `.js` versions).

### Web Mobile Controls (Virtual Gamepad)
To make the game playable on mobile devices, the web portal includes a virtual gamepad layer.
- **Event Bridging**: Circular on-screen buttons capture `touchstart` events and translate them into a sequence of `keydown`/`keyup` events.
- **Iframe Messaging**: Since the emulator runs in an isolated iframe, events are dispatched directly to the iframe's `contentWindow`, ensuring the emulation engine reacts as if a physical keyboard was used.
- **Split Layout**: Controls are split (Movement on left, Fire on right) for an ergonomic mobile experience.

### Web CORS Bypass (Base64 Disks)
The web emulator (RVM) bypasses modern browser security (CORS) which normally blocks loading local files via `file://`. We achieve this by converting the binary `.dsk` images into Base64-encoded strings inside `.js` files. This makes the game truly "portable" and playable from a local folder without a web server.

### Optimized Custom Random Number Generator (RNG)
The game uses a custom **Galois Linear-Feedback Shift Register (LFSR)** for generating random numbers (`rand8()`) instead of the standard CPCtelera or C library random functions.

-   **Implementation**: A 16-bit shift register (`rng_seed`) with maximal-length XOR taps (`0xB400`).
-   **Efficiency**: The Z80 lacks a native multiplication or division instruction. A typical LCG (Linear Congruential Generator) would require several expensive 16x16 multiplications. In contrast, the Galois LFSR uses only single-bit shifts and conditional XORs, making `rand8()` extremely fast (approx. 300-400 cycles per 8-bit number).
-   **Statistical Properties**: While not cryptographically secure, the LFSR provides a uniform distribution over its period (65,535 non-zero values), which is more than sufficient for game logic like enemy movement patterns, power-up probabilities, and starfield randomization.
-   **Entropy**: The `rng_seed` is naturally randomized by user interaction (sampling the frame counter during the intro/menu screens) to ensure each play-through feels unique.

### Pseudo-Parallax Starfield
Used in the Intro, Menu, and Game Over screens, the starfield provides a sense of depth and motion:
-   **`star_t` Data Structure**:
    -   `x`, `y`: Screen coordinates.
    -   `speed`: Determines which depth layer the star belongs to (1, 2, or 3).
    -   `color`: The Mode 0 byte color representing the star.
    -   `is_drawn`: Tracking flag to ensure stars are only erased if they were actually drawn by this routine.
-   **Triple-Layer Depth**: 40 stars are split into three layers with different speeds and colors (Blue, Cyan, White) to simulate parallax.
-   **Non-Destructive Drawing**: To prevent stars from "eating" through the menu text, the `update_starfield` logic checks if a destination pixel is black (`0x00`) before drawing.
-   **LFSR Randomization**: When a star reaches the bottom, its X coordinate is reset using the global `rand8()` RNG to ensure a non-repeating pattern.

## 9. Size Optimizations (Compilation Flags)

To fit the extensive logic, multi-language support, and assets into the CPC's limited 64KB, the build system uses aggressive SDCC optimizations:

-   **`--opt-code-size`**: Directs the compiler to prioritize binary footprint over execution speed.
-   **`--max-allocs-per-node 20000`**: Increases the search depth of the register allocator, allowing for more compact code sequences at the cost of compilation time.
-   **`--peep-return`**: Activates advanced peephole optimizations for function return sequences.
-   **`--no-std-crt0`**: Replaces the standard C runtime with a minimal, custom loader to eliminate unused overhead.

### Avoiding Standard Library Bloat
To keep the binary under the 64KB limit, the game avoids heavy standard C functions:
-   **No `printf`/`sprintf`**: These functions are notoriously large on 8-bit systems. Instead, we use a custom, specialized `uint16_to_str` function that only handles what we need for the HUD.
-   **Optimized Memory Ops**: Most memory operations use **CPCtelera's `cpct_memcpy`**, which is written in hand-optimized Z80 assembly, instead of the generic `memcpy`.
-   **Direct VRAM Access**: Rather than using high-level drawing abstractions, we write directly to the screen memory address (`0xC000`) whenever possible to save CPU cycles and code space.

## 10. Sound & Interrupt System

The game's audio is powered by the **Arkos Tracker (AKP)** player, integrated with a specialized timing strategy to ensure high-fidelity playback:

-   **ISR-Driven Playback**: The music and SFX rendering (`cpct_akp_musicPlay`) is tied to the CPC's **Interrupt Service Routine (ISR)** at 50Hz.
-   **Execution Stability**: By using interrupts, the music tempo is decoupled from the game logic. Even during heavy CPU moments (like the Boss encounter or massive brick explosions), the music never slows down or "drags", maintaining a perfect 50 FPS rhythm.
-   **Instrument-Based SFX**: Due to CPCtelera's sound API limitations, which do not allow loading or playing multiple songs (or a separate SFX bank) simultaneously, we use **instruments from the main song** to generate all SFX (via `cpct_akp_SFXPlay`). By re-triggering these instruments at different frequencies, we create complex sounds for collisions and power-ups without needing additional audio buffers.
-   **`DISC.BAS` Loader**: A small BASIC script used as the primary entry point on the disk image. It's responsible for the following startup sequence:
    1.  **Environment Setup**: Sets `MODE 0` and `BORDER 0`.
    2.  **Palette Initialization**: Reads and applies a 16-color palette (provided by `img2scr.py`) using `INK` commands to match the loading screen's colors.
    3.  **Visual Feedback**: Loads `loading.scr` directly into Video RAM (`&C000`) to display the title/loading screen.
    4.  **Execution**: Launches the main game binary (`RUN"!brickb.bin"`) to start the compiled engine.

## 11. Technical Refinements & Fixes

### Enemy Screen Wrap Protection
To prevent visual glitches where enemies appeared in the HUD area, a safety clipping mechanism was implemented. Enemies are now deactivated if their vertical coordinate exceeds `192` (8 lines before the physical screen bottom). Additionally, the drawing routine performs a bounds check `y + height <= 200` before writing to VRAM.

### Optimized Menu Redrawing
Menu interactions (like switching difficulty) use **Delta Redrawing**. Instead of clearing and repainting the entire menu, only the specific line containing the changed value is updated. This prevents the pseudo-parallax starfield and Arkos Tracker music from stuttering due to high CPU load during full-screen redraws.

### Automated Exit Transition
In Demo and Autopilot modes, the paddle AI is enhanced with a **Door Detection** state. Once `door_open` is true, the paddle ignores the ball and steers directly towards the exit threshold (`WALL_RIGHT_BYTES`), facilitating a hands-free transition to the next level. Exiting demo mode via key press returns the player directly to the start menu, bypassing the "Game Over" screen for a smoother experience.

## 12. Main Game Loop

The game's execution flow is managed by a two-tiered loop structure in `main.c` that separates high-level menu navigation from low-level frame updates.

### Outer Loop (Game Lifecycle)
The primary `while(1)` in `main()` manages the transition between major game states:
1.  **Screens**: Intro (Animated) -> Story -> Level Selection.
2.  **Initialization**: Resets scores, lives, and triggers `init_game()`.
3.  **Session Loop**: Runs the gameplay until a termination condition is met.
4.  **Conclusion**: Displays either `Victory` or `Game Over` and returns to the Intro.

### Inner Loop (Frame-by-Frame Gameplay)
The core gameplay runs within an efficient `while(lives > 0 && !game_won)` loop:
1.  **Input Processing**: `cpct_scanKeyboard_f()` and `handle_game_toggles()` for music/pause.
2.  **State Updates**: (Only if not paused)
    -   `update_paddle()`: Physics and boundaries.
    -   `update_ball()`: Fixed-point motion and collisions.
    -   `update_extra_balls()`, `update_drop()`, `update_lasers()`, `update_enemies()`: Entity management.
    -   `update_boss()`: Boss state machine (if active).
3.  **Rendering**: `draw_game()` synchronizes with VSYNC and performs the Delta Redraw strategy.

## 13. Snapshot Generation (.SNA)

Beyond standard disk images, the build system generates **WinApe-compatible SNA snapshots** for instant game loading and debugging. This process is handled by a custom delivery pipeline:

### The `bin2sna.py` Tool
A specialized Python script converts the project's binary output into an Amstrad CPC snapshot, supporting both **SNA v1** (default) and **SNA v2**:
-   **Version Support**: V1 is used by default for maximum compatibility with older emulators and tools. V2 can be targeted for features like creator strings.
-   **Header Standard**: Strictly follows SNA offsets for Z80 registers (SP at `&21`, PC at `&23`) and hardware state.
-   **Hardware Handover**: Configures the Gate Array (Mode 0, ROMs disabled) and CRTC registers (R0-R17). For V1 snapshots, specific register patterns (like R16/R17) are matched against proven working examples (WinApe) for stable boot.
-   **Interrupt Safety**: The snapshot starts with **Interrupts Disabled** (`IFF0=0`) to ensure absolute control over the CPU state before enabling the ISR.

### Automated Delivery Pipeline
The `Makefile` automates the complex mapping required for different localized builds:
-   **Dynamically Detected Entry Point**: The build system parses `obj/brickb.map` for each language to find the exact address of `_main`, passing it as the PC to the snapshot tool.
-   **Memory Alignment **: Unlike standard builds that load at `&1C00`, snapshots inject the binary starting at **`&0500`**. This ensures that the **Arkos Tracker music data** (placed at the beginning of the binary) is correctly mapped.
-   **Build Rules**: Standard snapshots (V1) are generated via `make sna LANG=XX`, while V2 versions are available via `make sna_v2 LANG=XX`. Batch generation is supported via `all_sna` and `all_sna_v2`.

---
*Brick Blaster Architecture - Documented for the future.* 🕹️🧠