#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct Room Room;

// Item structure to hold item name and quantity
typedef struct {
    char name[50];
    int quantity;
    char description[200];
    bool isCombineable;
    char combineWith[50];
} Item;

//Inventory structure to hold items and total capacity
typedef struct {
    Item* items;
    int capacity;
    int count;
} Inventory;

typedef struct {
    char name[50];
    char description[200];
    bool interacted;
} Interactable;

typedef struct Room {
    char description[200];
    Item* items[10];
    int itemCount;

    Interactable* interactables[10];
    int interactableCount;
    bool isLocked;

    Room* north;
    Room* south;
    Room* east;
    Room* west;
} Room;

// Initializes the inventory with a default capacity of 1
void InitInventory(Inventory* inv) {
    inv->capacity = 1;
    inv->count = 0;
    inv->items = (Item*)malloc(sizeof(Item) * inv->capacity);
    if (inv->items == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

// Expands the inventory to 10 slots (when rucksack is picked up)
void ExpandInventory(Inventory* inv, int additional_space) {
    int new_capacity = inv->capacity + additional_space;
    Item* new_items = realloc(inv->items, sizeof(Item) * new_capacity);
    if (!new_items) {
        perror("Failed to expand inventory");
        return;
    }
    inv->items = new_items;
    inv->capacity = new_capacity;
}

void Rucksack(Inventory* inv, bool* hasRucksack) {
    printf("You picked up a rucksack! Now you can carry more items.\n");
    ExpandInventory(inv, 9);

    *hasRucksack = true;
}

void PickupItem(Inventory* inv, const char* itemName, int quantity) {
    
    // Check if the item already exists in the inventory, if so update the quantity
    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].name, itemName) == 0) {
            inv->items[i].quantity += quantity;
            return;
        }
    }

    //Check if there is space in the inventory
    if (inv->count >= inv->capacity) {
        printf("Inventory is full! Cannot pick up %s.\n", itemName);
        return;
    }

    // Add the new item to the inventory
    strcpy(inv->items[inv->count].name, itemName);
    inv->items[inv->count].quantity = quantity;
    inv->count++;
    printf("Picked up %s(s).\n", itemName);

}

void PrintInventory(const Inventory* inv) {
    printf("Inventory:\n");
    for (int i = 0; i < inv->count; i++) {
        printf("%s: %d\n", inv->items[i].name, inv->items[i].quantity);
    }
}

Item* CreateItem(const char* name, int quantity, const char* description, bool* isCombinable, const char* combineWith) {
    Item* newItem = (Item*)malloc(sizeof(Item));
    if (!newItem) {
        perror("Failed to allocate memory for item");
        return NULL;
    }
    strcpy(newItem->name, name);
    newItem->quantity = quantity;
    strcpy(newItem->description, description);
    strcpy(newItem->combineWith, combineWith);
    return newItem;
}

Interactable* CreateInteractable(const char* name, const char* description) {
    Interactable* newInteractable = (Interactable*)malloc(sizeof(Interactable));
    if (!newInteractable) {
        perror("Failed to allocate memory for interactable");
        return NULL;
    }
    strcpy(newInteractable->name, name);
    strcpy(newInteractable->description, description);
    newInteractable->interacted = false;
    return newInteractable;
}

int main() {

    Room spawnRoom;
    Room goldRoom;
    Room engineRoom;
    Room jungleRoom;
    Room cyberRoom;

    //  === Spawn Room ===
    strcpy(spawnRoom.description, "You are in a dark room. The walls are damp, made of stone bricks with moss growing on them.\n"
        "There is a golden door to the north, a wooden door to the east, a rusty door to the south, and a metal sliding door to the west.\n");
    spawnRoom.itemCount = 1;
    spawnRoom.interactableCount = 1;
    spawnRoom.isLocked = false;
    spawnRoom.north = &goldRoom;
    spawnRoom.south = &engineRoom;
    spawnRoom.east = &jungleRoom;
    spawnRoom.west = &cyberRoom;

    spawnRoom.items[0] = CreateItem("Note", 1, "A note with a cryptic message: 'The wise guardian seeks that which is seen to the north'");
    spawnRoom.interactables[0] = CreateInteractable("Crate", "A large wooden crate. It looks like it can be pried open.");

    //  === Jungle Room ===

    strcpy(jungleRoom.description, "You are in a dense jungle. The air is humid and filled with the sounds of wildlife. \n There is a wise-looking jaguar laying on a chest in front of a tree\n");
    jungleRoom.itemCount = 2;
    jungleRoom.interactableCount = 3;
    jungleRoom.isLocked = false;
    jungleRoom.north = NULL;
    jungleRoom.south = NULL;
    jungleRoom.east = &spawnRoom;
    jungleRoom.west = NULL;

    jungleRoom.items[0] = CreateItem("Rusty Cog", 1, "A large rusty cog. It looks like it could have been part of a machine.");
    jungleRoom.items[1] = CreateItem("Suspicious fruit", 1, "A fruit that looks like it could be poisonous. It has a strange glow to it.");
    jungleRoom.interactables[0] = CreateInteractable("Jaguar", "A wise-looking jaguar. It seems to be guarding a chest.");
    jungleRoom.interactables[1] = CreateInteractable("Chest", "A large chest. It looks like it can be opened.");
    jungleRoom.interactables[2] = CreateInteractable("Tree", "A large tree with thick branches. There looks to be a faint glint at the top of its trunk.");

    //  === Gold Room ===
    
    strcpy(goldRoom.description, "You are in a room filled with gold. The walls are covered in gold leaf, and the floor is made of gold bricks.\n"
        "There is a pile of treasure in front of you. You win!\n");

    goldRoom.itemCount = 0;
    goldRoom.interactableCount = 0;
    goldRoom.isLocked = true;
    goldRoom.north = NULL;
    goldRoom.south = &spawnRoom;
    goldRoom.east = NULL;
    goldRoom.west = NULL;

    //  === Engine Room ===
    strcpy(engineRoom.description, "You are in a room filled with machinery. The walls are made of metal, and the floor is covered in oil.\n"
        "There is a large machine in the center of the room.\n");

    engineRoom.itemCount = 1;
    engineRoom.interactableCount = 1;
    engineRoom.isLocked = false;
    engineRoom.north = &spawnRoom;
    engineRoom.south = NULL;
    engineRoom.east = NULL;
    engineRoom.west = NULL;

    engineRoom.items[0] = CreateItem("Rucksack", 1, "A large rucksack. It looks like it can hold more items.");
    engineRoom.interactables[0] = CreateInteractable("Machine", "A large machine. It looks like it can be repaired.");

    //  === Cyber Room ===
    strcpy(cyberRoom.description, "You are in a room filled with computers and servers. The walls are made of glass, and the floor is covered in wires.\n"
        "There is a hi-tech kitchen area on the corner.\n");
    cyberRoom.itemCount = 1;
    cyberRoom.interactableCount = 1;
    cyberRoom.isLocked = true;
    cyberRoom.north = NULL;
    cyberRoom.south = NULL;
    cyberRoom.east = &spawnRoom;
    cyberRoom.west = NULL;

    cyberRoom.items[0] = CreateItem("Crowbar", 1, "A large crowbar. It is behind a glass case.");
    cyberRoom.interactables[0] = CreateInteractable("Kitchen", "A hi-tech kitchen area. There are many gadgets and appliances.");


    // Game loop
    Inventory playerInventory;
    bool hasRucksack = false;
    InitInventory(&playerInventory);

    char currentRoom = "Spawn Room";

    printf("Welcome to the game!\n");
    printf(spawnRoom.description);
    printf("You can go north, south, east, or west.\n");
    printf("Type 'inventory' to access your inventory.\n");
    //printf('When in the inventory, type an item name to inspect it or type "combine" to \n');



    // --------

    for (int i = 0; i < spawnRoom.itemCount; i++) {
        free(spawnRoom.items[i]);
    }
    for (int i = 0; i < spawnRoom.interactableCount; i++) {
        free(spawnRoom.interactables[i]);
    }

    for (int i = 0; i < jungleRoom.itemCount; i++) {
        free(jungleRoom.items[i]);
    }
    for (int i = 0; i < jungleRoom.interactableCount; i++) {
        free(jungleRoom.interactables[i]);
    }
    
    for (int i = 0; i < goldRoom.itemCount; i++) {
        free(goldRoom.items[i]);
    }
    for (int i = 0; i < goldRoom.interactableCount; i++) {
        free(goldRoom.interactables[i]);
    }

    for (int i = 0; i < engineRoom.itemCount; i++) {
        free(engineRoom.items[i]);
    }
    for (int i = 0; i < engineRoom.interactableCount; i++) {
        free(engineRoom.interactables[i]);
    }

    for (int i = 0; i < cyberRoom.itemCount; i++) {
        free(cyberRoom.items[i]);
    }
    for (int i = 0; i < cyberRoom.interactableCount; i++) {
        free(cyberRoom.interactables[i]);
    }

    return 0;
}