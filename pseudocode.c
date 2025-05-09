#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/*******************************************
 * ADVENTURE GAME - GROUP PROJECT OVERVIEW
 *******************************************/

/* GAME STATE VARIABLES */
char player_location[20] = "Spawn Room";
char inventory[10][30];  // Fix: Changed array size from [1] to [30]
int inventory_count = 0;
bool has_rucksack = false;
char cog_state[10] = "rusty";  // States: "rusty", "clean", "used"
bool riddle_solved = false;
bool glass_broken = false;
FILE* log_file = NULL;

/* FUNCTION PROTOTYPES */
// 1. Navigation functions
void describe_location();
void move(const char* direction);

// 2. Inventory and item functions
bool add_to_inventory(const char* item);
bool remove_from_inventory(const char* item);
bool has_item(const char* item);
bool pickup_item(const char* item);
void drop_item(const char* item);
void examine_item(const char* item);

// 3. Puzzle-solving functions
void solve_riddle(const char* answer);
void use_item(const char* item, const char* target);

// 4. Helper functions
void to_lower(char* str);

/*******************************************
 * MAIN FUNCTION - GAME LOOP
 *******************************************/
int main() {
    char input[100];
    bool game_over = false;
    
    printf("Welcome to the Adventure Game!\n");
    log_action("Game started", "Welcome message displayed");
    describe_location();
    
    while (!game_over) {
        printf("> ");
        fgets(input, 100, stdin);
        input[strcspn(input, "\n")] = 0;  // Remove newline
        to_lower(input);
        
        log_action("Player input", input);
        
        // COMMAND PARSING
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            log_action("Game ended", "Player quit");
            game_over = true;
        }
        
        // NAVIGATION: north, south, east, west
        else if (strcmp(input, "north") == 0 || strcmp(input, "south") == 0 || 
                 strcmp(input, "east") == 0 || strcmp(input, "west") == 0) {
            move(input);
        }
        
        // ITEM INTERACTION: pick up [item]
        else if (strncmp(input, "pick up ", 8) == 0 || strncmp(input, "get ", 4) == 0) {
            char* item_name = strncmp(input, "pick up ", 8) == 0 ? input + 8 : input + 4;
            if (!pickup_item(item_name)) {
                game_over = true;  // Game over if picking up cog without rucksack
            }
        }
        
        // ITEM INTERACTION: drop [item]
        else if (strncmp(input, "drop ", 5) == 0) {
            drop_item(input + 5);
        }
        
        // ITEM INTERACTION: examine [item]
        else if (strncmp(input, "examine ", 8) == 0 || strncmp(input, "look at ", 8) == 0) {
            examine_item(input + 8);
        }
        
        // PUZZLE SOLVING: use [item] on [target]
        else if (strncmp(input, "use ", 4) == 0) {
            char item[30], target[30];
            char* token = strtok(input + 4, " on ");
            
            if (token != NULL) {
                strcpy(item, token);
                token = strtok(NULL, "");
                if (token != NULL) {
                    strcpy(target, token);
                    use_item(item, target);
                } else {
                    use_item(item, "read");  // Default action
                }
            }
        }
        
        // PUZZLE SOLVING: answer [riddle]
        else if (strncmp(input, "answer ", 7) == 0 && strcmp(player_location, "Jungle Room") == 0) {
            solve_riddle(input + 7);
        }
        
        // OTHER COMMANDS
        else if (strcmp(input, "inventory") == 0) {
            if (inventory_count == 0) {
                printf("Your inventory is empty.\n");
                log_action("Check inventory", "Empty");
            } else {
                printf("Inventory:\n");
                for (int i = 0; i < inventory_count; i++) {
                    printf("- %s\n", inventory[i]);
                }
                log_action("Check inventory", "Listed items");
            }
        }
        
        else if (strcmp(input, "look") == 0 || strcmp(input, "look around") == 0) {
            describe_location();
        }
        
        else if (strcmp(input, "push crate") == 0 && strcmp(player_location, "Engine Room") == 0) {
            printf("You push the crate away from the wall.\nBehind it, you find a rucksack!\n");
            log_action("Push crate", "Found rucksack");
        }
        
        else {
            printf("I don't understand that command.\n");
            log_action("Unknown command", input);
        }
    }
    
    fclose(log_file);
    return 0;
}

/*******************************************
 * NAVIGATION FUNCTIONS
 *******************************************/
void describe_location() {
    if (strcmp(player_location, "Spawn Room") == 0) {
        printf("You are in the Spawn Room. There's a note on the ground, a fun hat, and a closed barrel.\n");
        printf("Exits: north (Engine Room), east (Jungle Room), west (Futuristic Room), golden door (locked)\n");
    }
    else if (strcmp(player_location, "Jungle Room") == 0) {
        printf("You are in the Jungle Room. There's a chest guarded by a wise jaguar.\n");
        if (riddle_solved) {
            printf("The jaguar has let you pass. The chest is open.\n");
        } else {
            printf("The jaguar watches you carefully. It asks: 'I speak without a mouth and hear without ears. What am I?'\n");
        }
        printf("Exits: west (Spawn Room)\n");
    }
    else if (strcmp(player_location, "Futuristic Room") == 0) {
        printf("You are in the Futuristic Room. There's a kitchen area with a blender.\n");
        if (!glass_broken) {
            printf("Behind a glass pane, you can see a sledgehammer.\n");
        }
        printf("Exits: east (Spawn Room)\n");
    }
    else if (strcmp(player_location, "Engine Room") == 0) {
        printf("You are in the Engine Room. There's a large machine with a missing part.\n");
        printf("A heavy crate is pushed against one wall.\n");
        printf("Exits: south (Spawn Room)\n");
    }
    else if (strcmp(player_location, "Treasure Room") == 0) {
        printf("Congratulations! You've reached the Treasure Room!\n");
        printf("Alejandro Zapata wins the crown!\n");
        printf("GAME OVER - YOU WIN!\n");
    }
    
    log_action("Location described", player_location);
}

void move(const char* direction) {
    char old_location[30];
    strcpy(old_location, player_location);
    
    if (strcmp(player_location, "Spawn Room") == 0) {
        if (strcmp(direction, "north") == 0) {
            strcpy(player_location, "Engine Room");
        }
        else if (strcmp(direction, "east") == 0) {
            strcpy(player_location, "Jungle Room");
        }
        else if (strcmp(direction, "west") == 0) {
            if (has_item("Keycard")) {
                strcpy(player_location, "Futuristic Room");
            } else {
                printf("The door to the west is locked. You need a keycard.\n");
                log_action("Move failed", "Keycard required");
                return;
            }
        }
        else if (strcmp(direction, "golden door") == 0 || strcmp(direction, "treasure") == 0) {
            if (has_item("Golden Key")) {
                strcpy(player_location, "Treasure Room");
            } else {
                printf("The golden door is locked. You need a special key.\n");
                log_action("Move failed", "Golden Key required");
                return;
            }
        }
        else {
            printf("You can't go that way.\n");
            log_action("Move failed", "Invalid direction");
            return;
        }
    }
    // Add other rooms' movement logic here...
    else if (strcmp(player_location, "Jungle Room") == 0) {
        if (strcmp(direction, "west") == 0) {
            strcpy(player_location, "Spawn Room");
        } else {
            printf("You can't go that way.\n");
            log_action("Move failed", "Invalid direction");
            return;
        }
    }
    else if (strcmp(player_location, "Futuristic Room") == 0) {
        if (strcmp(direction, "east") == 0) {
            strcpy(player_location, "Spawn Room");
        } else {
            printf("You can't go that way.\n");
            log_action("Move failed", "Invalid direction");
            return;
        }
    }
    else if (strcmp(player_location, "Engine Room") == 0) {
        if (strcmp(direction, "south") == 0) {
            strcpy(player_location, "Spawn Room");
        } else {
            printf("You can't go that way.\n");
            log_action("Move failed", "Invalid direction");
            return;
        }
    }
    
    char log_msg[100];
    sprintf(log_msg, "Moved from %s to %s", old_location, player_location);
    log_action("Move", log_msg);
    
    describe_location();
}

/*******************************************
 * INVENTORY AND ITEM FUNCTIONS
 *******************************************/
bool add_to_inventory(const char* item) {
    if (inventory_count < 10) {
        strcpy(inventory[inventory_count], item);
        inventory_count++;
        return true;
    }
    return false;
}

bool remove_from_inventory(const char* item) {
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i], item) == 0) {
            // Move all items after this one up one slot
            for (int j = i; j < inventory_count - 1; j++) {
                strcpy(inventory[j], inventory[j + 1]);
            }
            inventory_count--;
            return true;
        }
    }
    return false;
}

bool has_item(const char* item) {
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i], item) == 0) {
            return true;
        }
    }
    return false;
}

bool pickup_item(const char* item) {
    // Game over condition
    if (strcmp(item, "cog") == 0 && !has_item("Rucksack")) {
        printf("As you pick up the cog, you drop it on your foot.\n");
        printf("GAME OVER - You are injured and cannot continue.\n");
        log_action("Game over", "Picked up cog without rucksack");
        return false;
    }
    
    // Need rucksack for most items
    if (strcmp(item, "note") != 0 && strcmp(item, "rucksack") != 0 && !has_item("Rucksack")) {
        printf("You need something to carry items with.\n");
        log_action("Pickup failed", "No rucksack");
        return true;
    }
    
    // Check for puzzle conditions
    if (strcmp(item, "sledgehammer") == 0 && !glass_broken) {
        printf("The sledgehammer is behind the glass pane. You can't reach it.\n");
        log_action("Pickup failed", "Glass pane in the way");
        return true;
    }
    
    // Add the item to inventory
    char title_item[30];
    strcpy(title_item, item);
    title_item[0] = toupper(title_item[0]);  // Simple capitalization
    
    add_to_inventory(title_item);
    printf("You picked up the %s.\n", item);
    
    // Special case for rucksack
    if (strcmp(item, "rucksack") == 0) {
        has_rucksack = true;
        printf("Now you can carry more items!\n");
    }
    
    char log_msg[100];
    sprintf(log_msg, "Picked up %s", item);
    log_action("Pickup item", log_msg);
    
    return true;
}

void drop_item(const char* item) {
    char title_item[30];
    strcpy(title_item, item);
    title_item[0] = toupper(title_item[0]);  // Simple capitalization
    
    if (has_item(title_item)) {
        if (strcmp(title_item, "Rucksack") == 0) {
            printf("You can't drop your rucksack while it contains items.\n");
            log_action("Drop failed", "Rucksack contains items");
            return;
        }
        
        remove_from_inventory(title_item);
        printf("You dropped the %s.\n", item);
        
        char log_msg[100];
        sprintf(log_msg, "Dropped %s", item);
        log_action("Drop item", log_msg);
    } else {
        printf("You don't have that item.\n");
        log_action("Drop failed", "Item not in inventory");
    }
}

void examine_item(const char* item) {
    char title_item[30];
    strcpy(title_item, item);
    title_item[0] = toupper(title_item[0]);  // Simple capitalization
    
    if (has_item(title_item)) {
        if (strcmp(title_item, "Note") == 0) {
            printf("A cryptic note with a hint about jungle animals and riddles.\n");
        }
        else if (strcmp(title_item, "Rucksack") == 0) {
            printf("A sturdy rucksack that lets you carry items.\n");
        }
        else if (strcmp(title_item, "Cog") == 0) {
            if (strcmp(cog_state, "rusty") == 0) {
                printf("A rusty cog. It needs cleaning before it can be used.\n");
            } else {
                printf("A clean cog, ready to be used in machinery.\n");
            }
        }
        else if (strcmp(title_item, "Sledgehammer") == 0) {
            printf("A heavy sledgehammer that can break things.\n");
        }
        else if (strcmp(title_item, "Keycard") == 0) {
            printf("A keycard that can unlock electronic doors.\n");
        }
        else {
            printf("Just a regular %s.\n", item);
        }
        
        char log_msg[100];
        sprintf(log_msg, "Examined %s", item);
        log_action("Examine item", log_msg);
    } 
    else if (strcmp(player_location, "Jungle Room") && strcmp(item, "jaguar") == 0) {
        printf("A wise jaguar guarding the chest. It seems to be waiting for an answer.\n");
        log_action("Examine", "Looked at jaguar");
    }
    else {
        printf("You don't see that here.\n");
        log_action("Examine failed", "Item not found");
    }
}

/*******************************************
 * PUZZLE SOLVING FUNCTIONS
 *******************************************/
void solve_riddle(const char* answer) {
    // The riddle: "I speak without a mouth and hear without ears. What am I?"
    if (strcmp(answer, "echo") == 0) {
        printf("The jaguar nods approvingly. 'Correct. The echo speaks without a mouth and hears without ears.'\n");
        printf("The chest opens, revealing the first key part!\n");
        riddle_solved = true;
        add_to_inventory("First Key Part");
        
        log_action("Riddle solved", "Player received First Key Part");
    } else {
        printf("The jaguar growls. That's not the right answer.\n");
        log_action("Riddle attempt", "Incorrect answer");
    }
}

void use_item(const char* item, const char* target) {
    char title_item[30], title_target[30];
    strcpy(title_item, item);
    strcpy(title_target, target);
    title_item[0] = toupper(title_item[0]);
    title_target[0] = toupper(title_target[0]);
    
    if (!has_item(title_item)) {
        printf("You don't have the %s.\n", item);
        log_action("Use failed", "Item not in inventory");
        return;
    }
    
    // Key puzzle combinations
    if (strcmp(title_item, "First Key Part") == 0 && 
        strcmp(title_target, "Second Key Part") == 0 && 
        has_item("Third Key Part")) {
        
        printf("You combine all three key parts to form the Golden Key!\n");
        remove_from_inventory("First Key Part");
        remove_from_inventory("Second Key Part");
        remove_from_inventory("Third Key Part");
        add_to_inventory("Golden Key");
        
        log_action("Use success", "Combined key parts into Golden Key");
    }
    
    // Anti-rust puzzle
    else if (strcmp(title_item, "Anti-Rust Solution") == 0 && 
             strcmp(title_target, "Cog") == 0 && 
             has_item("Cog")) {
        
        printf("You apply the solution to the rusty cog. It's now clean and usable!\n");
        remove_from_inventory("Anti-Rust Solution");
        strcpy(cog_state, "clean");
        
        log_action("Use success", "Cleaned the cog");
    }
    
    // Glass breaking puzzle
    else if (strcmp(title_item, "Sledgehammer") == 0 && 
             strcmp(title_target, "Glass Pane") == 0 && 
             strcmp(player_location, "Futuristic Room") == 0) {
        
        printf("You smash the glass pane with the sledgehammer!\n");
        glass_broken = true;
        printf("Now you can take the sledgehammer.\n");
        
        log_action("Use success", "Broke glass pane");
    }
    
    // Default message
    else {
        printf("That doesn't seem to work.\n");
        
        char log_msg[100];
        sprintf(log_msg, "Tried to use %s on %s", item, target);
        log_action("Use failed", log_msg);
    }
}

/*******************************************
 * HELPER FUNCTIONS
 *******************************************/
void to_lower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}