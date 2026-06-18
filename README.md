Expect for folder structure
```bash
cavern-descent/
├── assets/
└── src/
    ├── core/
    │   ├── Game.h / Game.cpp
    │   ├── GameManager.h / GameManager.cpp
    │   └── GameState.h
    │
    ├── player/
    │   ├── Player.h / Player.cpp
    │   └── MovementStrategy.h
    │
    ├── physics/
    │   └── PhysicsSystem.h / PhysicsSystem.cpp
    │
    ├── lighting/
    │   └── LightingSystem.h / LightingSystem.cpp
    │
    ├── level/
    │   ├── TileMap.h / TileMap.cpp
    │   └── LevelGenerator.h / LevelGenerator.cpp
    │
    ├── entities/
    │   ├── Entity.h / Entity.cpp
    │   ├── Enemy.h / Enemy.cpp
    │   ├── Item.h / Item.cpp
    │   └── Trap.h / Trap.cpp
    │
    └── liquid/
        └── LiquidSimulator.h / LiquidSimulator.cpp
    │
    ...
```