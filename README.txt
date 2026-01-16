# GTA – Project README

## Authors
- **Or-Ram Atar** | 325004851  
- **Romi Sinizkey** | *(add ID here)*

---

## Project Overview
This project is a 2D game inspired by the **Grand Theft Auto (GTA)** concept. The player can move freely in an open world, drive vehicles, use multiple weapon types, fight NPCs (and earn money), complete missions, and interact with a dynamic police/wanted system.

The game includes a wide variety of in-game entities: player characters, pedestrians, police units, vehicles, weapons, pickups, missions, and an optimized collision system.

---

## Design & Architecture
The game is built using an **object-oriented architecture**, where each major game component is represented by a dedicated class. The design emphasizes modularity and separation of responsibilities: game loop management, entity logic, AI, resources, collisions, missions, and UI are handled by dedicated modules/managers.

### Core Entities (High-Level)
- **GameManager**  
  The central orchestrator of the game. Responsible for the main game loop, event processing, updating all game entities, and rendering. Holds references to managers (e.g., `CarManager`, `PoliceManager`) and the player.

- **GameObject (abstract)**  
  Base class for all game objects. Defines a common interface such as `update`, `draw`, and collision handling.

- **MovingObject (abstract)**  
  Inherits from `GameObject`. Base class for objects that can move (characters and vehicles).

- **Character**  
  Inherits from `MovingObject`. Represents characters such as the player, pedestrians, and police. Manages health, damage, and movement.

- **Player**  
  Inherits from `Character`. Handles user input, inventory, and interactions with the world.

- **Vehicle**  
  Inherits from `MovingObject`. Manages vehicle movement, player driving controls, AI driving behavior, and damage/destruction.

### Managers
- **PoliceManager**  
  Controls all police entities (cops, police cars, helicopters, tanks). Responsible for spawning, updating, and responding to the player’s actions (wanted level escalation).

- **CarManager**  
  Controls civilian traffic: spawning, road movement logic, and interaction with the environment.

- **PedestrianManager**  
  Spawns and updates pedestrians and their random movement behavior.

### Missions
- **Mission (abstract base class)**  
  Base for missions. Multiple mission types inherit from it (e.g., `KillMission`, `CarMission`, `PackageMission`, `SurviveMission`) and define their own completion logic.

### Pickups / Presents
- **Present (abstract)**  
  Base class for pickups/rewards with an effect on the player (health, money, weapons, ammo).

- **WeaponPresent (abstract)**  
  Base class for weapon pickups (e.g., `Pistol`, `Rifle`, `Minigun`, `Bazooka`, `Knife`, `Grenade`).

### Spatial Optimization
- **QuadTree**  
  Used to optimize collision checks by spatial partitioning—querying only nearby entities instead of brute-force checking all pairs.

---

## Main Data Structures
### QuadTree
A spatial partitioning structure used to optimize collision detection. The world is split into regions, and collision checks are performed only among nearby objects. This significantly improves performance compared to checking every pair of entities.

### Object Pools (e.g., `BulletPool`, `VehiclePool`)
Object pools reduce frequent dynamic allocations/deallocations. Instead of creating a new object each time, the system reuses inactive objects from the pool and returns them when no longer needed—improving runtime performance.

---

## Notable Algorithms
### A* Pathfinding
Used for route planning:
- Police officers and police vehicles use A* to chase the player.
- Civilian cars use path planning logic on road segments.

### SAT (Separating Axis Theorem)
Used for polygon-polygon collision detection. SAT tests whether a separating axis exists between two convex shapes; if none exists, they collide.

---

## Known Bugs
No known bugs.

---

## Other Notes
No additional notes.

---

## Files We Implemented / Modified (Selected)
> The project contains many classes. Below is a structured overview of the key modules.

### `include/AmmoRifle.h`, `src/AmmoRifle.cpp`
**AmmoRifle** (inherits `Present`)  
- Responsibility: Represents a rifle ammo pickup.  
- Key method:
  - `applyEffect(Player& player)`: Adds rifle ammo to the player (currently commented in code).  
- Relationships: Inherits `Present`, affects `Player`.

### `include/MovingObject.h`
**MovingObject** (abstract, inherits `GameObject`)  
- Responsibility: Defines the interface for objects that can move.  
- Key methods:
  - `move(...)`
  - `getSpeed() const`  
- Inherited by: `Character`, `Vehicle`.

### `include/PackageMission.h`, `src/PackageMission.cpp`
**PackageMission** (inherits `Mission`)  
- Responsibility: Deliver-a-package mission to a target location.  
- Key methods:
  - `update(float dt, Player& player)`
  - `isCompleted() const`

### `include/Vehicle.h`, `src/Vehicle.cpp`
**Vehicle** (inherits `MovingObject`)  
- Responsibility: Vehicle motion for AI and player control, and vehicle destruction/damage.  
- Key methods:
  - `update(float dt, ...)`
  - `accelerate(float dt)`, `brake(float dt)`, `steerLeft(float dt)`, `steerRight(float dt)`
  - `startTurn(...)` (AI turning)

### `include/VehiclePool.h`, `src/VehiclePool.cpp`
**VehiclePool**  
- Responsibility: Reuse `Vehicle` objects efficiently.  
- Key methods:
  - `getVehicle()`
  - `returnVehicle(Vehicle* vehicle)`
  - `getAllVehicles()`

### `include/WeaponPresent.h`, `src/WeaponPresent.cpp`
**WeaponPresent** (abstract, inherits `Present`)  
- Responsibility: Base class for weapon pickups.  
- Key method:
  - `applyEffect(Player& player)`: Adds the weapon to the player’s inventory.  
- Inherited by: `Pistol`, `Rifle`, `Minigun`, `Bazooka`, `Knife`, `Grenade`.

---

## Additional Modules (Overview)

### UI / Menus
- **Menu** (`include/Menu.h`, `src/Menu.cpp`) – Main menu UI and selection logic.
- **PauseMenu** (`include/PauseMenu.h`, `src/PauseMenu.cpp`) – Pause menu (resume/new game/map/stats/volume).
- **Settings** (`include/Settings.h`, `src/Settings.cpp`) – Settings UI (volume/brightness).
- **HUD** (`include/HUD.h`, `src/HUD.cpp`) – Displays player status (health/armor/money/weapon/ammo/wanted level).
- **Help** (`include/Help.h`, `src/Help.cpp`) – Help screens / instructions menu.
- **InventoryUI** (`include/InventoryUI.h`, `src/InventoryUI.cpp`) – Inventory UI drawing and input handling.

### Pathfinding
- **Pathfinder** (`include/Pathfinder.h`, `src/Pathfinder.cpp`) – A* pathfinding between two points.
- **PathfindingGrid** (`include/PathfindingGrid.h`, `src/PathfindingGrid.cpp`) – World-to-grid conversion and walkability checks.
- **PathPlanner** (`include/PathPlanner.h`) – Placeholder (currently empty).

### Collision / Spatial
- **QuadTree** (`include/QuadTree.h`) – Spatial partitioning for efficient collision queries.
- **CollisionUtils** (`include/CollisionUtils.h`, `src/CollisionUtils.cpp`) – Collision helper methods (point/polygon, circle/polygon, polygon/polygon).

### Managers
- **PoliceManager** (`include/PoliceManager.h`, `src/PoliceManager.cpp`) – Spawns/updates police units based on wanted level.
- **CarManager** (`include/CarManager.h`, `src/CarManager.cpp`) – Manages civilian vehicles spawning and road logic.
- **PedestrianManager** (`include/PedestrianManager.h`, `src/PedestrianManager.cpp`) – Spawns and updates pedestrians.

### Object Pools
- **BulletPool** (`include/BulletPool.h`, `src/BulletPool.cpp`) – Reuses bullets efficiently.
- **VehiclePool** (`include/VehiclePool.h`, `src/VehiclePool.cpp`) – Reuses vehicles efficiently.

### Resources / Audio
- **ResourceManager** (`include/ResourceManager.h`, `src/ResourceManager.cpp`) – Singleton for textures/fonts/sounds.
- **ResourceInitializer** (`include/ResourceInitializer.h`, `src/ResourceInitializer.cpp`) – Loads initial and game resources.
- **SoundManager** (`include/SoundManager.h`, `src/SoundManager.cpp`) – Singleton for sound playback and volume.

### Misc
- **GameFactory** (`include/GameFactory.h`, `src/GameFactory.cpp`) – Creates player, managers, presents, stores.
- **Store** (`include/Store.h`, `src/Store.cpp`) – In-game store for buying items.
- **Constants** (`include/Constants.h`) – Global constants (map size, speeds, damages, etc.).
- **PatrolZone** (`include/PatrolZone.h`) – Patrol zone data used by police logic.
- **RoadSegment** (`include/RoadSegment.h`) – Road representation used by car AI.

---

## Build / Run
> Fill this section according to your actual build system (CMake/Makefile/SFML setup).

Example:
```bash
cmake -S . -B build
cmake --build build
./build/GTA
