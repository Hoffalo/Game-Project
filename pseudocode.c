#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Constants
#define MAX_INPUT 100
#define MAX_INVENTORY 10
#define MAX_KEY_PARTS 3

// Game state variables
char player_location[20] = "Spawn Room";
char inventory[MAX_INVENTORY][1];
int inventory_count = 0;
bool has_rucksack = false;
char cog_state[10] = "rusty"; // Can be "rusty", "clean", or "used"
bool riddle_solved = false;
bool glass_broken = false;
bool crate_broken = false;
bool engine_activated = false;
char key_parts[MAX_KEY_PARTS][30];
int key_parts_count = 0;

// Function prototypes
void describe_location();
void check_inventory();
void move(const char* destination);
bool pickup_item(const char* item);
void use_item(const char* item, const char* target);
void solve_riddle(const char* answer);
void push_crate();
void to_lower(char* str);
void to_title(char* str);
bool add_to_inventory(const char* item);
bool remove_from_inventory(const char* item);
bool has_item(const char* item);

// Helper function to convert string to lowercase
void to_lower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Helper function to convert first letter to uppercase (Title Case)
void to_title(char* str) {
    if (str[0] != '\0') {
        str[0] = toupper(str[0]);
    }
    for (int i = 1; str[i]; i++) {
        if (str[i-1] == ' ') {
            str[i] = toupper(str[i]);
        }
    }
}

// Add item to inventory
bool add_to_inventory(const char* item) {
    if (inventory_count < MAX_INVENTORY) {
        strcpy(inventory[inventory_count], item);
        inventory_count++;
        return true;
    }
    return false;
}

// Remove item from inventory
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

// Check if player has an item
bool has_item(const char* item) {
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i], item) == 0) {
            return true;
        }
    }
    return false;
}

// Description of current location
void describe_location() {
    if (strcmp(player_location, "Spawn Room") == 0) {
        printf("You are in the Spawn Room. There's a note on the ground, a fun hat, and a closed barrel.\n");
        printf("There are passages to the Futuristic Room, Jungle Room, Engine Room, and a locked golden door.\n");
    }
    else if (strcmp(player_location, "Jungle Room") == 0) {
        printf("You are in the Jungle Room. There's a chest guarded by a wise jaguar.\n");
        if (riddle_solved) {
            printf("The jaguar has let you pass. The chest is open.\n");
        } else {
            printf("The jaguar watches you carefully. It seems to be waiting for something...\n");
        }
        printf("There's a tree with something shiny in its branches.\n");
    }
    else if (strcmp(player_location, "Futuristic Room") == 0) {
        printf("You are in the Futuristic Room. There's a kitchen area with a blender.\n");
        if (!glass_broken) {
            printf("Behind a glass pane, you can see a sledgehammer.\n");
        } else {
            printf("The glass is broken, exposing the sledgehammer.\n");
        }
    }
    else if (strcmp(player_location, "Engine Room") == 0) {
        printf("You are in the Engine Room. There's a large machine with a missing part.\n");
        printf("A heavy crate is pushed against one wall.\n");
        if (engine_activated) {
            printf("The engine is humming nicely.\n");
        }
    }
    else if (strcmp(player_location, "Treasure Room") == 0) {
        printf("Congratulations! You've reached the Treasure Room!\n");
        printf("Alejandro Zapata wins the crown!\n");
        printf("GAME OVER - YOU WIN!\n");
    }
}

// Display inventory contents
void check_inventory() {
    if (inventory_count == 0) {
        printf("Your inventory is empty.\n");
    } else {
        printf("Inventory:\n");
        for (int i = 0; i < inventory_count; i++) {
            printf("- %s\n", inventory[i]);
        }
    }
}

// Move to a new location
void move(const char* destination) {
    char dest[30];
    strcpy(dest, destination);
    
    if (strcmp(dest, "Treasure Room") == 0 && !has_item("Golden Key")) {
        printf("The golden door is locked. You need a special key.\n");
        return;
    }
    
    if (strcmp(dest, "Futuristic Room") == 0 && !has_item("Keycard")) {
        printf("The door is locked. You need a keycard.\n");
        return;
    }
        
    strcpy(player_location, dest);
    describe_location();
}

// Pick up an item
bool pickup_item(const char* item) {
    char item_name[30];
    strcpy(item_name, item);
    
    if (strcmp(item_name, "Cog") == 0 && !has_item("Rucksack")) {
        printf("As you pick up the cog, you drop it on your foot.\n");
        printf("GAME OVER - You are injured and cannot continue.\n");
        return false; // Game over
    }
    
    if (strcmp(item_name, "Note") != 0 && strcmp(item_name, "Rucksack") != 0 && !has_item("Rucksack")) {
        printf("You need something to carry items with.\n");
        return true;
    }
    
    if (strcmp(item_name, "Sledgehammer") == 0 && !glass_broken) {
        printf("The sledgehammer is behind the glass pane. You can't reach it.\n");
        return true;
    }
    
    if (strcmp(item_name, "First Key Part") == 0 && !riddle_solved) {
        printf("The jaguar blocks your access to the chest.\n");
        return true;
    }
    
    if (strcmp(item_name, "Keycard") == 0 && !has_item("Rucksack")) {
        printf("You spot the keycard in the tree, but have no way to reach it.\n");
        return true;
    }
    
    if (strcmp(item_name, "Third Key Part") == 0 && !crate_broken) {
        printf("The final key part seems to be inside the closed barrel.\n");
        return true;
    }
    
    add_to_inventory(item_name);
    printf("You picked up the %s.\n", item_name);
    
    if (strcmp(item_name, "Rucksack") == 0) {
        has_rucksack = true;
        printf("Now you can carry more items!\n");
    }
    
    if (strcmp(item_name, "First Key Part") == 0) {
        strcpy(key_parts[key_parts_count], "First Key Part");
        key_parts_count++;
    }
    
    return true;
}

// Use an item on a target
void use_item(const char* item, const char* target) {
    char item_name[30], target_name[30];
    strcpy(item_name, item);
    strcpy(target_name, target);
    
    if (!has_item(item_name)) {
        printf("You don't have the %s.\n", item_name);
        return;
    }
    
    if (strcmp(item_name, "Note") == 0 && strcmp(target_name, "Read") == 0) {
        printf("The note contains a cryptic message and a hint about jungle animals.\n");
        printf("It doesn't make much sense now, but might be useful later.\n");
    }
    
    else if (strcmp(item_name, "Anti-Rust Solution") == 0 && strcmp(target_name, "Cog") == 0 && has_item("Cog")) {
        printf("You apply the solution to the rusty cog. It's now clean and usable!\n");
        remove_from_inventory("Anti-Rust Solution");
        strcpy(cog_state, "clean");
    }
    
    else if (strcmp(item_name, "Cog") == 0 && strcmp(target_name, "Engine") == 0 && strcmp(cog_state, "clean") == 0) {
        printf("You install the cog in the engine. It starts humming!\n");
        remove_from_inventory("Cog");
        engine_activated = true;
        printf("A compartment opens, revealing the second part of the golden key!\n");
        add_to_inventory("Second Key Part");
        strcpy(key_parts[key_parts_count], "Second Key Part");
        key_parts_count++;
    }
    
    else if (strcmp(item_name, "Cog") == 0 && strcmp(target_name, "Glass Pane") == 0 && strcmp(cog_state, "clean") == 0) {
        printf("You throw the cog at the glass pane, but it bounces off harmlessly.\n");
        printf("You need something stronger to break the glass.\n");
    }
    
    else if (strcmp(item_name, "Poisonous Fruit") == 0 && strcmp(target_name, "Blender") == 0 && 
             strcmp(player_location, "Futuristic Room") == 0) {
        printf("You blend the fruit. It creates an anti-rust solution!\n");
        remove_from_inventory("Poisonous Fruit");
        add_to_inventory("Anti-Rust Solution");
    }
    
    else if (strcmp(item_name, "Sledgehammer") == 0 && strcmp(target_name, "Crate") == 0 && 
             strcmp(player_location, "Spawn Room") == 0) {
        printf("You smash the barrel with the sledgehammer! It breaks open!\n");
        printf("Inside, you find the third part of the golden key!\n");
        add_to_inventory("Third Key Part");
        strcpy(key_parts[key_parts_count], "Third Key Part");
        key_parts_count++;
        crate_broken = true;
    }
    
    else if (strcmp(item_name, "Sledgehammer") == 0 && strcmp(target_name, "Glass Pane") == 0 && 
             strcmp(player_location, "Futuristic Room") == 0) {
        printf("You smash the glass pane with the sledgehammer!\n");
        glass_broken = true;
        printf("Now you can take the sledgehammer.\n");
    }
    
    else if (strcmp(item_name, "First Key Part") == 0 && strcmp(target_name, "Second Key Part") == 0 && 
             has_item("Third Key Part")) {
        printf("You combine all three key parts to form the Golden Key!\n");
        remove_from_inventory("First Key Part");
        remove_from_inventory("Second Key Part");
        remove_from_inventory("Third Key Part");
        add_to_inventory("Golden Key");
    }
    
    else if (strcmp(item_name, "Golden Key") == 0 && strcmp(target_name, "Golden Door") == 0 && 
             strcmp(player_location, "Spawn Room") == 0) {
        printf("The key fits perfectly! The golden door opens...\n");
        move("Treasure Room");
    }
    
    else {
        printf("That doesn't seem to work.\n");
    }
}

// Solve the jaguar's riddle
void solve_riddle(const char* answer) {
    char player_answer[50];
    strcpy(player_answer, answer);
    to_lower(player_answer);
    
    // The correct answer would depend on the actual riddle
    if (strcmp(player_answer, "correct answer") == 0) {
        printf("The jaguar nods approvingly. The chest opens!\n");
        printf("You find the first part of the golden key inside!\n");
        riddle_solved = true;
        add_to_inventory("First Key Part");
    } else {
        printf("The jaguar growls. That's not the right answer.\n");
    }
}

// Push the crate in the engine room
void push_crate() {
    if (strcmp(player_location, "Engine Room") == 0) {
        printf("You push the crate away from the wall.\n");
        printf("Behind it, you find a rucksack!\n");
    }
}

// Main function
int main() {
    char input[MAX_INPUT];
    char command[MAX_INPUT];
    char item[30];
    char target[30];
    bool game_over = false;
    
    printf("Welcome to the Adventure Game!\n");
    describe_location();
    
    while (!game_over) {
        printf("> ");
        fgets(input, MAX_INPUT, stdin);
        
        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;
        
        // Convert input to lowercase for easier parsing
        to_lower(input);
        
        // Basic commands
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            game_over = true;
        }
        
        // Movement command
        else if (strncmp(input, "go to ", 6) == 0) {
            strcpy(command, input + 6);
            to_title(command);
            move(command);
        }
        
        // Pickup commands
        else if (strncmp(input, "get ", 4) == 0) {
            strcpy(item, input + 4);
            to_title(item);
            if (!pickup_item(item)) {
                game_over = true;
            }
        }
        else if (strncmp(input, "pick up ", 8) == 0) {
            strcpy(item, input + 8);
            to_title(item);
            if (!pickup_item(item)) {
                game_over = true;
            }
        }
        
        // Use commands
        else if (strncmp(input, "use ", 4) == 0) {
            char *token;
            token = strtok(input + 4, " on ");
            
            if (token != NULL) {
                strcpy(item, token);
                to_title(item);
                
                token = strtok(NULL, "");
                if (token != NULL) {
                    strcpy(target, token);
                    to_title(target);
                    use_item(item, target);
                } else {
                    use_item(item, "Read");
                }
            } else {
                printf("Use what?\n");
            }
        }
        
        // Inventory command
        else if (strcmp(input, "inventory") == 0 || strcmp(input, "check inventory") == 0) {
            check_inventory();
        }
        
        // Look command
        else if (strcmp(input, "look") == 0 || strcmp(input, "look around") == 0) {
            describe_location();
        }
        
        // Riddle solution
        else if (strncmp(input, "answer ", 7) == 0 && strcmp(player_location, "Jungle Room") == 0) {
            solve_riddle(input + 7);
        }
        
        // Push crate command
        else if (strcmp(input, "push crate") == 0 && strcmp(player_location, "Engine Room") == 0) {
            push_crate();
        }
        
        // Unknown command
        else {
            printf("I don't understand that command.\n");
        }
    }
    
    return 0;
}