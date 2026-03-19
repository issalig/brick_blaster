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
-   **`ball_t`**: Uses 10.6 Fixed Point math for smooth movement.
    -   `fx`, `fy`: Sub-pixel position.
    -   `dx`, `dy`: Velocity components derived from `dir` and `speed`.
    -   `active`: Flag for multiball handling.
-   **`paddle_t`**: Manages position and variable widths (Normal, Wide, Tiny).
-   **`boss_t`**: A state-machine based structure for the final encounter.
    -   `state`: 0=Sleeping, 1=Attacking, 2=Exploding.
    -   `health`: Hit point tracking.
-   **`enemy_t`**: Represents flying hazards that spawn periodically.
    -   `type`: Determines the sprite and movement pattern.
    -   `dx`: Horizontal drift direction.
    -   **Interaction**: Enemies serve as moving obstacles and scoring targets. When hit by a ball, they reflect it; when they touch the paddle, they are destroyed and award bonus points (**500 HUD pts**) without costing a life.

### Brick System & Level Encoding
The core of the level design is a compact 2D grid (`bricks[8][8]`). To minimize memory usage while maximizing features, each cell is a single byte packed with metadata:

-   **Bits 0-1 (Type)**: `0=Empty`, `1=Normal` (1 HP), `2=Hard` (2 HP), `3=Gold` (Invincible).
-   **Bits 2-3 (Color)**: Indexes one of 4 predefined color pairs for the brick's gradient.
-   **Bits 4-7 (Power-up)**: Defines which capsule (if any) will drop upon destruction.

#### Level Lifecycle:
1.  **Static Data**: Levels are stored as a 3D constant array `level_data[NUM_LEVELS][8][8]` in ROM.
2.  **Runtime Loading**: When a level starts, this data is copied into the mutable RAM array `bricks[8][8]`.
3.  **Destruction Workflow**: When the ball hits a brick:
    -   If HP > 1: Type is decremented but the brick remains.
    -   If HP == 1: The brick is marked with a special flag `BSTATE_NEEDS_ERASE`.
    -   **Deferred Erasure**: The main loop detects the erase flag, triggers the power-up drop if applicable, and restores the background pattern to "erase" the brick visually without a full screen redraw.
4.  **Completion Condition**: The game tracks a counter `active_bricks`. Only `BTYPE_NORMAL` and `BTYPE_HARD` contribute to this count. When `active_bricks` reaches 0, the current level is marked as cleared (`level_cleared = 1`), and the transition logic is triggered.

## 3. Difficulty & Progression

The game's difficulty scales primarily through **Ball Speed** increments every two levels:

| User-Facing Levels | Internal Index | Speed (Fixed Point) | Pixels/Frame |
| :--- | :--- | :--- | :--- |
| **1 - 2** | 0 - 1 | 150 | 2.3 px |
| **3 - 4** | 2 - 3 | 165 | 2.5 px |
| **5 - 6** | 4 - 5 | 180 | 2.8 px |
| **7 - 8** | 6 - 7 | 195 | 3.0 px |
| **9 - 10** | 8 - 9 | 210 | 3.2 px |

- **Formula**: `INITIAL_BALL_SPEED + (current_level / 2) * 15`.
- **Cap**: The speed is capped at 300 units to ensure gameplay remains manageable even in custom high-level scenarios.

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

The game supports English, Spanish, French, and Greek via a specialized header-and-source strategy:
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
-   **Localization Hooks**: The `getSpriteIndex` function includes manual UTF-8 decoding to support special characters across languages (e.g., Spanish `¡`, `ñ`, and the Greek alphabet).

## 7. Memory Map (Verified)

| Address Range | Description |
| :--- | :--- |
| `&0500 - &1BFF` | Music Song Data (Arkos Tracker) |
| `&1C00 - &954C` | Game Code, Compiled Sprites, and Font Data |
| `&954D - &BFFF` | Global Variables & Background Row Cache |
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

### Web CORS Bypass (Base64 Disks)
The web emulator (RVM) bypasses modern browser security (CORS) which normally blocks loading local files via `file://`. We achieve this by converting the binary `.dsk` images into Base64-encoded strings inside `.js` files. This makes the game truly "portable" and playable from a local folder without a web server.

### Pseudo-Parallax Starfield
Used in the Intro, Menu, and Game Over screens, the starfield provides a sense of depth and motion:
-   **Triple-Layer Depth**: 40 stars are split into three layers with different speeds (1, 2, and 3 pixels per frame) and colors (Blue, Cyan, White) to simulate parallax.
-   **Non-Destructive Drawing**: To prevent stars from "eating" through the menu text, the `updateStarfield` logic checks if a destination pixel is black (`0x00`) before drawing.
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
-   **PSG Direct Access**: For specific effects (like the "Mute" toggle), the game writes directly to the **AY-3-8912 (PSG)** registers, bypassing the player for immediate audio control.
-   **`DISC.BAS` Loader**: A small BASIC script used as the primary entry point on the disk image. It's responsible for the following startup sequence:
    1.  **Environment Setup**: Sets `MODE 0` and `BORDER 0`.
    2.  **Palette Initialization**: Reads and applies a 16-color palette (provided by `img2scr.py`) using `INK` commands to match the loading screen's colors.
    3.  **Visual Feedback**: Loads `loading.scr` directly into Video RAM (`&C000`) to display the title/loading screen.
    4.  **Execution**: Launches the main game binary (`RUN"!brickbla.bin"`) to start the compiled engine.

---
*Brick Blaster Architecture - Documented for the future.* 🕹️🧠