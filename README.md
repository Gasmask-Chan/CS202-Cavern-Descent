<div align="center">
  <h1>🦇 Cavern Descent 🦇</h1>
  <p><i>A Spelunky-inspired 2D Roguelike Platformer</i></p>
  
  <p>
    <img src="https://img.shields.io/badge/C++-17-blue.svg?style=for-the-badge&logo=c%2B%2B" alt="C++17" />
    <img src="https://img.shields.io/badge/Raylib-5.x-red.svg?style=for-the-badge" alt="Raylib" />
    <img src="https://img.shields.io/badge/Platform-Windows-lightgrey.svg?style=for-the-badge&logo=windows" alt="Windows" />
  </p>
</div>

<br/>

## 📖 About The Game

**Cavern Descent** is a fast-paced, procedurally generated 2D platformer. Delve deep into shifting underground caverns, navigate deadly traps, fight terrifying creatures, and collect as much treasure as you can before the Nemesis Ghost catches you! 

Every run is entirely unique thanks to our robust procedural generation engine, making exploration both dangerous and highly rewarding.

---

## ✨ Key Features

### 🗺️ Procedural World Generation
- **Graph-Based Generation:** A 4x4 macro-grid generates a unique "Golden Path" to the exit every run.
- **Three Unique Biomes:** Descend through the Caves, Lush Jungles, and the Ancient Temple.
- **Environmental Modifiers:** Beware of dynamic floor modifiers like *Dark Floors*, *Flooded Floors*, and *Cursed Floors*.

### ⚔️ Dynamic Gameplay & Systems
- **Fluid Custom Physics:** Built-from-scratch AABB physics engine ensuring crisp, tight platforming.
- **Destructible Terrain:** Use bombs to blow up walls, drain lakes, and forge new paths.
- **Cellular Automata Liquids:** Realistic water and lava simulation that flows, cascades, and reacts to explosions.
- **Dynamic Lighting & Shadowcasting:** A recursive 8-octant shadowcasting system that brings dark caves to life with immersive torchlight and shadows.
- **The Nemesis Ghost:** Take too long? A terrifying wall-phasing ghost will hunt you down.

### 🎒 Roguelike Elements
- **Multiple Classes:** Play as the agile Ninja, the resilient Tank, or the balanced Explorer.
- **Economy & Upgrades:** Collect gold, build combo multipliers, and buy supplies from hidden Shopkeepers.
- **Fog of War Minimap:** Keep track of your exploration with a real-time updating HUD minimap.

### 🛠️ In-Game Level Editor
Unleash your creativity with a fully-featured, native in-game level editor. Design your own treacherous rooms, serialize them to `.lvl` files, and play them instantly!

---

## 🎮 Controls

| Action | Keybinding |
| :--- | :--- |
| **Move** | `W`, `A`, `S`, `D`|
| **Jump** | `Space` |
| **Climb** | `W` / `S` |
| **Attack (Whip)** | `J` |
| **Throw Bomb** | `K` |
| **Throw Rope** | `L` |
| **Interact (Shop)** | `Y` |
| **Open Door/Chest** | `W` + `Y` |
| **Pause** | `Esc` |

---

## 🏗️ Technical Architecture

Cavern Descent was engineered with clean, scalable, and modular C++ code, leveraging classic Gang of Four Design Patterns:
- **Entity Component/Hierarchy:** Deep, memory-safe entity management using `std::unique_ptr` and factory generation.
- **State Machine AI:** Enemies transition seamlessly between Idle, Chase, and Return states.
- **Strategy Pattern:** Player movement and physics behavior dynamically swap based on the chosen character class.
- **EventBus (Observer):** A decoupled event system handles everything from cascading terrain destruction to scoring and audio triggers.

---

## ⚙️ Build Instructions (Windows)

The game compiles effortlessly using MinGW-w64 (GCC). Raylib is pre-configured in the project.

1. **Clone the repository** (or download the source).
2. **Open a terminal** in the project root directory.
3. **Compile the game** using:
   ```bash
   mingw32-make clean
   mingw32-make
   ```
4. **Run the game:**
   ```bash
   ./game.exe
   ```

---

## 👥 Credits
- **Development:** Built from the ground up for the CS202 Game Engine Architecture course.
- **Assets:** Visual and audio assets are inspired by the legendary *Spelunky* series (for educational, non-commercial purposes only).
