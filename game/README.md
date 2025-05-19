# Temple of Secrets - Text Adventure Game

## Overview
*Temple of Secrets* is a text-based adventure game written in C. Explore an ancient temple, solve puzzles, and collect items to unlock the final treasure chamber. Manage your inventory, combine items to create useful tools, and interact with mysterious objects as your progress is logged throughout your journey!

## Gameplay

### Rooms & Exploration
- **Entrance Hall:** Your starting point with a golden door to the north.
- **Jungle Room:** Home to a mysterious jaguar statue and hidden key parts.
- **Engine Room:** Contains complex machinery and a secret rucksack.
- **Cyber Room:** A high-tech area with a locked door.
- **Gold Room:** The final treasure chamber (unlocked with the Golden Key).

### Inventory System
- **Limited Capacity:** Start with 1 slot, expandable up to 10 slots by obtaining the Rucksack.
- **Item Combination:** Merge items (e.g., Rusty Cog + Anti-Rust Solution → Clean Cog) to create new tools.
- **Item Usage:** Use items on objects (e.g., use Clean Cog on Machine to get Key Part 3).

### Puzzles & Challenges
- **Jaguar Riddle:** Answer the jaguar’s riddle to access a chest containing the first key part.
- **Key Assembly:** Gather three key parts and combine them to form the Golden Key.
- **Glass Pane Challenge:** Use a cog to smash the glass pane and retrieve a crowbar.
- **Crate Puzzle:** Use the crowbar in the Entrance Hall to open a crate and obtain Key Part 2.

### Commands
| Command                   | Action                            | Example                        |
| ------------------------- | --------------------------------- | ------------------------------ |
| `north` / `n`             | Move north                        | `north`                        |
| `take [item]`             | Pick up an item                   | `take rusty cog`               |
| `drop [item]`             | Drop an item from inventory       | `drop note`                    |
| `examine [item]`          | View item details                 | `examine keycard`              |
| `interact [object]`       | Interact with an object           | `interact jaguar`              |
| `use [item] [target]`     | Use an item on something          | `use crowbar crate`            |
| `combine [item1] [item2]`   | Merge two items                  | `combine key part 1 key part 2`|
| `push [object]`           | Push a movable object             | `push crate`                   |
| `look`                    | View details about the room       | `look`                       |
| `inventory` / `i`         | Check your inventory              | `inventory`                   |
| `help`                    | Show list of available commands   | `help`                        |
| `quit`                    | Exit the game                     | `quit`                        |

## Build & Run

### Dependencies
This game uses standard C libraries:
- `<stdlib.h>`
- `<string.h>`
- `<stdio.h>`
- `<stdbool.h>`
- `<ctype.h>`
- `<time.h>`

### Compilation & Execution
To compile the game using GCC, run:
```sh
gcc full_game.c -o temple_of_secrets
```
Then, execute the game with:
```sh
./temple_of_secrets
```
Game logs will be saved in `game_log.txt`.

## Key Functions
- `DoCommand()`: Processes player input.
- `MergeItems()`: Handles item combinations.
- `DoInteract()`: Manages object interactions.
- `WriteToLog()`: Records player actions.

## Walkthrough (Spoilers!)
1. Find the Rucksack: In the Engine Room, push the crate to reveal the rucksack.
2. Solve the Jaguar Riddle to the jaguar statue to obtain Key Part 1.
3. Use the Crowbar on the Crate: In the Entrance Hall, use the crowbar on the crate to get Key Part 2.
4. Clean the Rusty Cog: Combine the Rusty Cog with the Anti-Rust Solution to clean it.
5. Insert the Clean Cog into the Machine: In the Engine Room, use the clean cog on the machine to receive Key Part 3.
6. Combine all Key Parts: Merge the three key parts to form the Golden Key.
7. Unlock the Gold Room: Use the Golden Key to access the final treasure chamber and win the game!

## License
This game is free to use, modify, and distribute. If you enjoy the game, consider giving credit to the original creators (SALOME SHIOSHVILI, ALEJANDRO ZAPATA PINTO, LORENZO DE BARROS COELHO HOFFMANN, MOHAMED ABDELHADI MOUMENI).

## AI Disclaimer
Very limited parts of this project, such as handling multi-word names (e.g., "rusty cog", "metal door"), were modified with the assistance of AI tools. The AI helped generate a checker loop that validates the order of words in these names, ensuring that commands are parsed correctly. These contributions were used solely to fix specific issues in our existing code rather than to create the project from scratch. Every effort was made to ensure accuracy and quality.

Enjoy the adventure! 🗝️✨
