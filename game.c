#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>



typedef struct Room Room;

// Item structure to hold item name and quantity
typedef struct {
    char name[50];
    int quantity;
    char description[500];
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
    char description[500];
    bool interacted;
    char message[500];
} Interactable;

typedef struct Room {
    char roomName[50];
    char description[500];
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

Item* CreateItem(const char* name, int quantity, const char* description, bool isCombinable, const char* combineWith) {
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

Interactable* CreateInteractable(const char* name, const char* description, const char* message) {
    Interactable* newInteractable = (Interactable*)malloc(sizeof(Interactable));
    if (!newInteractable) {
        perror("Failed to allocate memory for interactable");
        return NULL;
    }
    strcpy(newInteractable->name, name);
    strcpy(newInteractable->description, description);
    strcpy(newInteractable->message, message);
    newInteractable->interacted = false;
    return newInteractable;
}

void InspectItem(const Item* item) {
    printf("Inspecting %s: %s\n", item->name, item->description);
}

void Interact(Interactable* interactable) {
    interactable->interacted = true;
    printf(interactable->message);
}

void NewRoom(Room* room) {
    printf("You have entered the %s.\n", room->roomName);
    printf("%s\n", room->description);

    if (strcmp(room->roomName, "Spawn Room") == 0) {
        printf("You can go north, south, east, or west.\n");
    } else if (strcmp(room->roomName, "Jungle Room") == 0) {
        printf("You can go east to return to the Spawn Room.\n");
    } else if (strcmp(room->roomName, "Gold Room") == 0) {
        printf("You can go south to return to the Spawn Room, but why would you do that?.\n");
    } else if (strcmp(room->roomName, "Engine Room") == 0) {
        printf("You can go north to return to the Spawn Room.\n");
    } else if (strcmp(room->roomName, "Cyber Room") == 0) {
        printf("You can go east to return to the Spawn Room.\n");
    }
}

/*char* GetDirection(const char* command) {
    if (strcmp(command, "north") == 0) {
        return "north";
    } else if (strcmp(command, "south") == 0) {
        return "south";
    } else if (strcmp(command, "east") == 0) {
        return "east";
    } else if (strcmp(command, "west") == 0) {
        return "west";
    }
    return NULL;
}*/

Item* GetItemByName(const Inventory* inv, const char* itemName) {
    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].name, itemName) == 0) {
            return &inv->items[i];
        }
    }
    return NULL;
}

void CombineItems(Inventory* inv) {

    char item1Name[50];
    char item2Name[50];

    printf("Enter the name of the first item to combine: ");
    scanf(" %[^\n]", item1Name);
    printf("Enter the name of the second item to combine: ");
    scanf(" %[^\n]", item2Name);

    Item* item1 = GetItemByName(inv, item1Name);
    Item* item2 = GetItemByName(inv, item2Name);

    if (item1 == NULL || item2 == NULL) {
        printf("One or both items not found in inventory.\n");
        return;
    }

    // Find the items in the inventory
    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].name, item1Name) == 0) {
            item1 = &inv->items[i];
        }
        if (strcmp(inv->items[i].name, item2Name) == 0) {
            item2 = &inv->items[i];
        }
    }

    if (item1->combineWith == item2->name && item2->combineWith == item1->name) {
        printf("You combined %s with %s!\n", item1->name, item2->name);
        // Remove the items from the inventory
        item1->quantity--;
        item2->quantity--;

        // Add the new combined item to the inventory (since the only combination is the shiny cog)
        PickupItem(inv, "Shiny Cog", 1);
    } else {
        printf("You cannot combine %s with %s.\n", item1->name, item2->name);
    }
}

bool checkIfInventory(const Inventory* inv, const char* itemName) {
    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].name, itemName) == 0) {
            return true;
        }
    }
    return false;
}

void AwaitCommand(Inventory* inv, Room** currentRoomPtr) {
    
    char command[50];

    Room* room = *currentRoomPtr;
    Room* nextRoom = NULL;

    printf("What do you want to do?\n");
    scanf("%s", command);

    if (command == "north" || command == "south" || command == "east" || command == "west") {
        if (command != NULL) {
            if (strcmp(command, "north") == 0) {
                nextRoom = room->north;
            } else if (strcmp(command, "south") == 0) {
                nextRoom = room->south;
            } else if (strcmp(command, "east") == 0) {
                nextRoom = room->east;
            } else if (strcmp(command, "west") == 0) {
                nextRoom = room->west;
            }else {
                printf("You can't go that way.\n");
            }
        }

        if (nextRoom != NULL) {
            if (nextRoom->isLocked) {
                printf("The door is locked.\n");
            } else {
                *currentRoomPtr = nextRoom;
                NewRoom(nextRoom);
            }
        }

    }
    
    if (strcmp(command, "inventory") == 0) {
        PrintInventory(inv);
    }
    
    if (strcmp(command, "combine") == 0) {
        CombineItems(inv);
    }
    
    if (strcmp(command, "room items") == 0) {
        for (int i = 0; i < room->itemCount; i++) {
            printf("%s\n", room->items[i]->name);
        }
    } 

    // Check if the command is an item name
    for (int i = 0; i < room->itemCount; i++) {
        if (strcmp(command, room->items[i]->name) == 0) {
            InspectItem(room->items[i]);
            return;
        }
    }
    // Check if the command is an interactable name
    for (int i = 0; i < room->interactableCount; i++) {
        if (strcmp(command, room->interactables[i]->name) == 0) {
            Interact(room->interactables[i]);
            return;
        }
    }
    // Check if the command is an inventory item name
    for (int i = 0; i < inv->count; i++) {
        if (strcmp(command, inv->items[i].name) == 0) {
            InspectItem(&inv->items[i]);
            return;
        }
    }
    
}


int main() {

    Room spawnRoom;
    Room goldRoom;
    Room engineRoom;
    Room jungleRoom;
    Room cyberRoom;

    //  === Spawn Room ===
    strcpy(spawnRoom.roomName, "Spawn Room");
    strcpy(spawnRoom.description, "You are in a dark room. The walls are damp, made of stone bricks with moss growing on them.\n"
        "There is a golden door to the north, a wooden door to the east, a rusty door to the south, and a metal sliding door to the west.\n");
    spawnRoom.itemCount = 1;
    spawnRoom.interactableCount = 1;
    spawnRoom.isLocked = false;
    spawnRoom.north = &goldRoom;
    spawnRoom.south = &engineRoom;
    spawnRoom.east = &jungleRoom;
    spawnRoom.west = &cyberRoom;

    spawnRoom.items[0] = CreateItem("Note", 1, "A note with a cryptic message: 'The wise guardian seeks that which is seen to the north'", false, "");
    spawnRoom.interactables[0] = CreateInteractable("Crate", "A large wooden crate. It looks like it can be pried open.", "You pried open the crate and found a key piece inside!");

    //  === Jungle Room ===
    strcpy(jungleRoom.roomName, "Jungle Room");
    strcpy(jungleRoom.description, "You are in a dense jungle. The air is humid and filled with the sounds of wildlife. \n There is a wise-looking jaguar laying on a chest in front of a tree\n");
    jungleRoom.itemCount = 2;
    jungleRoom.interactableCount = 3;
    jungleRoom.isLocked = false;
    jungleRoom.north = NULL;
    jungleRoom.south = NULL;
    jungleRoom.east = &spawnRoom;
    jungleRoom.west = NULL;

    jungleRoom.items[0] = CreateItem("Rusty Cog", 1, "A large rusty cog. It looks like it could have been part of a machine.", true, "Suspicious fruit");
    jungleRoom.items[1] = CreateItem("Suspicious fruit", 1, "A fruit that looks like it could be poisonous. It has a strange glow to it.", false, "");
    jungleRoom.interactables[0] = CreateInteractable("Jaguar", "A wise-looking jaguar. It seems to be guarding a chest.", "The jaguar looks at you and says: riddle");
    jungleRoom.interactables[1] = CreateInteractable("Chest", "A large chest. It looks like it can be opened.", "You opened the chest and found a golden key piece inside!");
    jungleRoom.interactables[2] = CreateInteractable("Tree", "A large tree with thick branches. There looks to be a faint glint at the top of its trunk.", "You climbed the tree and found a keycard stabbed ");

    //  === Gold Room ===
    strcpy(goldRoom.roomName, "Gold Room");
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
    strcpy(engineRoom.roomName, "Engine Room");
    strcpy(engineRoom.description, "You are in a room filled with machinery. The walls are made of metal, and the floor is covered in oil.\n"
        "There is a large machine in the center of the room.\n");
    engineRoom.itemCount = 1;
    engineRoom.interactableCount = 1;
    engineRoom.isLocked = false;
    engineRoom.north = &spawnRoom;
    engineRoom.south = NULL;
    engineRoom.east = NULL;
    engineRoom.west = NULL;

    engineRoom.interactables[0] = CreateInteractable("Rucksack", "A large leather rucksack. You can see some mold on it.", "You picked up a rucksack! Now you can carry more items.\n");
    engineRoom.interactables[0] = CreateInteractable("Machine", "A large machine. It looks like it can be repaired.", "You repaired the machine and it started working again! A key piece popped out!");

    //  === Cyber Room ===
    strcpy(cyberRoom.roomName, "Cyber Room");
    strcpy(cyberRoom.description, "You are in a room filled with computers and servers. The walls are made of glass, and the floor is covered in wires.\n"
        "There is a hi-tech kitchen area on the corner.\n");
    cyberRoom.itemCount = 1;
    cyberRoom.interactableCount = 1;
    cyberRoom.isLocked = true;
    cyberRoom.north = NULL;
    cyberRoom.south = NULL;
    cyberRoom.east = &spawnRoom;
    cyberRoom.west = NULL;

    cyberRoom.items[0] = CreateItem("Crowbar", 1, "A large crowbar. It is behind a glass case.", false, "");
    cyberRoom.interactables[0] = CreateInteractable("Glass case", "A large glass case. It looks fragile.", "You broke the glass case and found a crowbar inside!");
    cyberRoom.interactables[1] = CreateInteractable("Kitchen", "A hi-tech kitchen area. There are many gadgets and appliances." , "You put the fruit in the blender and made an anti-corrosive smoothie!");


    // Game loop
    Inventory inv;
    bool hasRucksack = false;
    InitInventory(&inv);

    Room* currentRoom = &spawnRoom;

    printf("Welcome to the game!\n");
    printf(spawnRoom.description);
    printf("You can go north, south, east, or west.\n");
    printf("Type 'inventory' to access your inventory.\n");
    printf("Type an item name to inspect it, or type \"combine\" to join two items\n");

    while (true) {
        AwaitCommand(&inv, &currentRoom);
    }



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