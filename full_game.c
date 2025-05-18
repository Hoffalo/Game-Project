#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

int stricmp(const char *a, const char *b)
{
    for (;; a++, b++)
    {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

typedef struct Room Room;

// Item structure to hold item name and quantity
typedef struct
{
    char name[50];
    int quantity;
    char description[200];
    bool isCombinable;
    char combineWith[50];
    char resultItem[50];
} Item;

// Inventory structure to hold items and total capacity
typedef struct
{
    Item *items;
    int capacity;
    int count;
} Inventory;

typedef struct
{
    char name[50];
    char description[200];
    bool interacted;
    char riddle[200];
    char answer[50];
} Interactable;

typedef struct Room
{
    char name[50];
    char description[200];
    Item *items[10];
    int itemCount;

    Interactable *interactables[10];
    int interactableCount;
    bool isLocked;
    char keyName[50];

    Room *north;
    Room *south;
    Room *east;
    Room *west;
} Room;

// Function declarations
void InitInventory(Inventory *inv);
void ExpandInventory(Inventory *inv, int additional_space);
void PickupItem(Inventory *inv, Room *currentRoom, const char *itemName);
void DropItem(Inventory *inv, Room *currentRoom, const char *itemName);
void ExamineItem(const Inventory *inv, const char *itemName);
void PrintInventory(const Inventory *inv);
Item *CreateItem(const char *name, int quantity, const char *description, bool isCombinable, const char *combineWith, const char *resultItem);
Interactable *CreateInteractable(const char *name, const char *description, const char *riddle, const char *answer);
void ProcessCommand(char *command, Inventory *inv, Room **currentRoom, bool *gameRunning, bool *hasWon, FILE *logFile);
void LogAction(FILE *logFile, const char *action, const char *result);
bool CombineItems(Inventory *inv, const char *item1, const char *item2);
void InteractWith(Room *currentRoom, Inventory *inv, const char *objectName);
void UseItem(Room *currentRoom, Inventory *inv, const char *itemName, const char *targetName);
bool HasItem(const Inventory *inv, const char *itemName);
void RemoveItemFromInventory(Inventory *inv, const char *itemName);
void AddItemToRoom(Room *room, Item *item);

// Initializes the inventory with a default capacity of 1
void InitInventory(Inventory *inv)
{
    inv->capacity = 1;
    inv->count = 0;
    inv->items = (Item *)malloc(sizeof(Item) * inv->capacity);
    if (inv->items == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

// Expands the inventory to have additional slots
void ExpandInventory(Inventory *inv, int additional_space)
{
    int new_capacity = inv->capacity + additional_space;
    Item *new_items = realloc(inv->items, sizeof(Item) * new_capacity);
    if (!new_items)
    {
        perror("Failed to expand inventory");
        return;
    }
    inv->items = new_items;
    inv->capacity = new_capacity;
    printf("Your inventory capacity is now %d slots.\n", inv->capacity);
}

// Picks up an item from the current room and adds it to inventory
void PickupItem(Inventory *inv, Room *currentRoom, const char *itemName)
{
    // Find item in the room
    int itemIndex = -1;
    for (int i = 0; i < currentRoom->itemCount; i++)
    {
        if (stricmp(currentRoom->items[i]->name, itemName) == 0)
        {
            itemIndex = i;
            break;
        }
    }

    if (itemIndex == -1)
    {
        printf("There is no %s here to pick up.\n", itemName);
        return;
    }

    // Special case for Rusty Cog without rucksack
    if (stricmp(itemName, "Rusty Cog") == 0 && inv->capacity == 1)
    {
        printf("You tried to pick up the rusty cog but dropped it on your foot! Game over!\n");
        exit(0);
    }

    // Special case for Rucksack: expand inventory first, and allow pickup even if full
    if (stricmp(itemName, "Rucksack") == 0)
    {
        ExpandInventory(inv, 9);
    }
    else if (inv->count >= inv->capacity)
    {
        printf("Inventory is full! Cannot pick up %s.\n", itemName);
        return;
    }

    // Add item to inventory
    strcpy(inv->items[inv->count].name, currentRoom->items[itemIndex]->name);
    strcpy(inv->items[inv->count].description, currentRoom->items[itemIndex]->description);
    inv->items[inv->count].quantity = currentRoom->items[itemIndex]->quantity;
    inv->items[inv->count].isCombinable = currentRoom->items[itemIndex]->isCombinable;
    strcpy(inv->items[inv->count].combineWith, currentRoom->items[itemIndex]->combineWith);
    strcpy(inv->items[inv->count].resultItem, currentRoom->items[itemIndex]->resultItem);
    inv->count++;

    if (stricmp(itemName, "Rucksack") == 0)
    {
        printf("You picked up a rucksack! Now you can carry more items.\n");
        printf("Your inventory capacity is now %d slots.\n", inv->capacity);
    }
    else
    {
        printf("Picked up %s.\n", itemName);
    }

    // Remove item from room
    free(currentRoom->items[itemIndex]);
    for (int i = itemIndex; i < currentRoom->itemCount - 1; i++)
    {
        currentRoom->items[i] = currentRoom->items[i + 1];
    }
    currentRoom->itemCount--;
}

// Drops an item from inventory to the current room
void DropItem(Inventory *inv, Room *currentRoom, const char *itemName)
{
    int itemIndex = -1;
    for (int i = 0; i < inv->count; i++)
    {
        if (stricmp(inv->items[i].name, itemName) == 0)
        {
            itemIndex = i;
            break;
        }
    }
    if (itemIndex == -1)
    {
        printf("You don't have %s in your inventory.\n", itemName);
        return;
    }
    // Don't allow dropping the rucksack
    if (stricmp(itemName, "Rucksack") == 0)
    {
        printf("You can't drop the rucksack, it's too useful!\n");
        return;
    }
    // Add item to room
    if (currentRoom->itemCount < 10)
    {
        Item *droppedItem = CreateItem(
            inv->items[itemIndex].name,
            inv->items[itemIndex].quantity,
            inv->items[itemIndex].description,
            inv->items[itemIndex].isCombinable,
            inv->items[itemIndex].combineWith,
            inv->items[itemIndex].resultItem);
        currentRoom->items[currentRoom->itemCount] = droppedItem;
        currentRoom->itemCount++;
        // Remove from inventory
        for (int i = itemIndex; i < inv->count - 1; i++)
        {
            inv->items[i] = inv->items[i + 1];
        }
        inv->count--;
        printf("Dropped %s.\n", itemName);
    }
    else
    {
        printf("The room is too cluttered to drop anything else.\n");
    }
}

// Examines an item in the inventory
void ExamineItem(const Inventory *inv, const char *itemName)
{
    for (int i = 0; i < inv->count; i++)
    {
        if (stricmp(inv->items[i].name, itemName) == 0)
        {
            printf("%s: %s\n", inv->items[i].name, inv->items[i].description);
            return;
        }
    }
    printf("You don't have %s in your inventory.\n", itemName);
}

// Prints inventory contents
void PrintInventory(const Inventory *inv)
{
    if (inv->count == 0)
    {
        printf("Your inventory is empty.\n");
        return;
    }
    printf("Inventory (%d/%d slots):\n", inv->count, inv->capacity);
    for (int i = 0; i < inv->count; i++)
    {
        printf("- %s (%d)\n", inv->items[i].name, inv->items[i].quantity);
    }
}

// Creates a new item
Item *CreateItem(const char *name, int quantity, const char *description, bool isCombinable, const char *combineWith, const char *resultItem)
{
    Item *newItem = (Item *)malloc(sizeof(Item));
    if (!newItem)
    {
        perror("Failed to allocate memory for item");
        return NULL;
    }
    strcpy(newItem->name, name);
    newItem->quantity = quantity;
    strcpy(newItem->description, description);
    newItem->isCombinable = isCombinable;
    strcpy(newItem->combineWith, combineWith);
    strcpy(newItem->resultItem, resultItem);
    return newItem;
}

// Creates a new interactable object
Interactable *CreateInteractable(const char *name, const char *description, const char *riddle, const char *answer)
{
    Interactable *newInteractable = (Interactable *)malloc(sizeof(Interactable));
    if (!newInteractable)
    {
        perror("Failed to allocate memory for interactable");
        return NULL;
    }
    strcpy(newInteractable->name, name);
    strcpy(newInteractable->description, description);
    strcpy(newInteractable->riddle, riddle);
    strcpy(newInteractable->answer, answer);
    newInteractable->interacted = false;
    return newInteractable;
}

// Checks if the inventory contains a specific item
bool HasItem(const Inventory *inv, const char *itemName)
{
    for (int i = 0; i < inv->count; i++)
    {
        if (stricmp(inv->items[i].name, itemName) == 0)
        {
            return true;
        }
    }
    return false;
}

// Removes an item from inventory
void RemoveItemFromInventory(Inventory *inv, const char *itemName)
{
    int itemIndex = -1;
    for (int i = 0; i < inv->count; i++)
    {
        if (stricmp(inv->items[i].name, itemName) == 0)
        {
            itemIndex = i;
            break;
        }
    }
    if (itemIndex == -1)
    {
        return;
    }
    for (int i = itemIndex; i < inv->count - 1; i++)
    {
        inv->items[i] = inv->items[i + 1];
    }
    inv->count--;
}

// Adds an item to a room
void AddItemToRoom(Room *room, Item *item)
{
    if (room->itemCount < 10)
    {
        room->items[room->itemCount] = item;
        room->itemCount++;
    }
}

// Combines two items in the inventory
bool CombineItems(Inventory *inv, const char *item1, const char *item2)
{
    int index1 = -1, index2 = -1;
    for (int i = 0; i < inv->count; i++)
    {
        if (stricmp(inv->items[i].name, item1) == 0)
        {
            index1 = i;
        }
        if (stricmp(inv->items[i].name, item2) == 0)
        {
            index2 = i;
        }
    }
    if (index1 == -1 || index2 == -1)
    {
        printf("You don't have both items to combine.\n");
        return false;
    }
    // Check if items can be combined
    if (inv->items[index1].isCombinable &&
        (stricmp(inv->items[index1].combineWith, inv->items[index2].name) == 0))
    {
        strcpy(inv->items[inv->count].name, inv->items[index1].resultItem);
        if (strcmp(inv->items[index1].resultItem, "Clean Cog") == 0)
            strcpy(inv->items[inv->count].description, "A clean, rust-free cog that can be used in machinery.");
        else if (strcmp(inv->items[index1].resultItem, "Combined Key Parts") == 0)
            strcpy(inv->items[inv->count].description, "Two key parts combined. Maybe there's a third?");
        else if (strcmp(inv->items[index1].resultItem, "Golden Key") == 0)
            strcpy(inv->items[inv->count].description, "A magnificent golden key that looks important.");
        else
            strcpy(inv->items[inv->count].description, "Combined item");
        inv->items[inv->count].quantity = 1;
        inv->items[inv->count].isCombinable = false;
        inv->count++;
        RemoveItemFromInventory(inv, item1);
        RemoveItemFromInventory(inv, item2);
        printf("Combined %s and %s to create %s!\n", item1, item2, inv->items[inv->count - 1].name);
        return true;
    }
    else if (inv->items[index2].isCombinable &&
             (stricmp(inv->items[index2].combineWith, inv->items[index1].name) == 0))
    {
        strcpy(inv->items[inv->count].name, inv->items[index2].resultItem);
        strcpy(inv->items[inv->count].description, "Combined item");
        inv->items[inv->count].quantity = 1;
        inv->items[inv->count].isCombinable = false;
        inv->count++;
        RemoveItemFromInventory(inv, item1);
        RemoveItemFromInventory(inv, item2);
        printf("Combined %s and %s to create %s!\n", item1, item2, inv->items[inv->count - 1].name);
        return true;
    }
    else
    {
        printf("These items cannot be combined.\n");
        return false;
    }
}

void InteractWith(Room *currentRoom, Inventory *inv, const char *objectName)
{
    for (int i = 0; i < currentRoom->interactableCount; i++)
    {
        if (stricmp(currentRoom->interactables[i]->name, objectName) == 0)
        {
            printf("You interact with %s.\n", objectName);

            // Jaguar logic
            if (stricmp(objectName, "Jaguar") == 0 && !currentRoom->interactables[i]->interacted)
            {
                printf("The jaguar looks at you with ancient wisdom in its eyes and speaks:\n");
                printf("\"%s\"\n", currentRoom->interactables[i]->riddle);

                char answer[50];
                printf("What is your answer? ");
                scanf("%49s", answer);
                int c;
                while ((c = getchar()) != '\n' && c != EOF)
                {
                }

                char correctAnswer[50];
                strcpy(correctAnswer, currentRoom->interactables[i]->answer);
                for (int j = 0; correctAnswer[j]; j++)
                {
                    correctAnswer[j] = tolower(correctAnswer[j]);
                }
                for (int j = 0; answer[j]; j++)
                {
                    answer[j] = tolower(answer[j]);
                }

                if (strcmp(answer, correctAnswer) == 0)
                {
                    printf("The jaguar nods approvingly. \"You have proven your wisdom.\"\n");
                    printf("The jaguar moves aside, revealing a gleaming key part in the chest.\n");
                    for (int j = 0; j < currentRoom->interactableCount; j++)
                    {
                        if (strcmp(currentRoom->interactables[j]->name, "Chest") == 0)
                        {
                            strcpy(currentRoom->interactables[j]->description,
                                   "A chest containing the first part of a golden key.");
                            break;
                        }
                    }
                    currentRoom->interactables[i]->interacted = true;
                }
                else
                {
                    printf("The jaguar growls. \"That is incorrect. Prove your wisdom or leave.\"\n");
                }
                return;
            }
            // --- CHEST LOGIC ---
            else if (stricmp(objectName, "Chest") == 0)
            {
                // Check if jaguar has been satisfied
                bool jaguarSatisfied = false;
                for (int j = 0; j < currentRoom->interactableCount; j++)
                {
                    if (strcmp(currentRoom->interactables[j]->name, "Jaguar") == 0)
                    {
                        jaguarSatisfied = currentRoom->interactables[j]->interacted;
                        break;
                    }
                }

                // Use static flag for this function only
                static bool keyPartTaken = false;

                if (jaguarSatisfied)
                {
                    if (!keyPartTaken)
                    {
                        printf("You open the chest and find the first part of a golden key!\n");
                        
                        // Check if there's space in inventory
                        if (inv->count < inv->capacity)
                        {
                            // Add key part directly to inventory
                            strcpy(inv->items[inv->count].name, "Key Part 1");
                            strcpy(inv->items[inv->count].description, "The first part of a three-part golden key.");
                            inv->items[inv->count].quantity = 1;
                            inv->items[inv->count].isCombinable = true;
                            strcpy(inv->items[inv->count].combineWith, "Key Part 2");
                            strcpy(inv->items[inv->count].resultItem, "Combined Key Parts");
                            inv->count++;
                            
                            printf("You automatically take the key part and put it in your inventory.\n");
                            
                            // Update chest description
                            strcpy(currentRoom->interactables[i]->description,
                                "An open chest. You've already taken the key part.");
                            
                            keyPartTaken = true;
                        }
                        else
                        {
                            printf("Your inventory is full! You can't take the key part.\n");
                            // Create the key part and add it to the room instead
                            Item *keyPart = CreateItem("Key Part 1", 1,
                                                "The first part of a three-part golden key.",
                                                true, "Key Part 2", "Combined Key Parts");
                            AddItemToRoom(currentRoom, keyPart);
                        }
                    }
                    else
                    {
                        printf("The chest is empty. You've already taken the key part.\n");
                    }
                }
                else
                {
                    printf("The chest is guarded by the jaguar. You need to satisfy it first.\n");
                }
                return;
            }
            // --- TREE LOGIC ---
            else if (stricmp(objectName, "Tree") == 0)
            {
                // Check if jaguar has been satisfied first
                bool jaguarSatisfied = false;
                for (int j = 0; j < currentRoom->interactableCount; j++)
                {
                    if (strcmp(currentRoom->interactables[j]->name, "Jaguar") == 0)
                    {
                        jaguarSatisfied = currentRoom->interactables[j]->interacted;
                        break;
                    }
                }

                // Use static flags for this function only
                static bool keycardTaken = false;
                static bool fruitDropped = false;

                if (!jaguarSatisfied)
                {
                    printf("The jaguar is blocking your way to examine the tree fully. You need to solve its riddle first.\n");
                    return;
                }

                // If neither has happened, do both at once
                if (!keycardTaken && !fruitDropped)
                {
                    printf("You shake the tree. A suspicious fruit drops to the ground, and you spot a shiny keycard embedded in the trunk!\n");

                    // Drop fruit to the ground
                    Item *fruit = CreateItem("Suspicious fruit", 1,
                                             "A strange glowing fruit that doesn't look edible but might have other uses.",
                                             false, "", "");
                    AddItemToRoom(currentRoom, fruit);
                    fruitDropped = true;

                    // Add keycard to inventory if possible, else drop to ground
                    if (inv->count < inv->capacity)
                    {
                        strcpy(inv->items[inv->count].name, "Keycard");
                        strcpy(inv->items[inv->count].description, "A high-tech keycard that can unlock electronic doors.");
                        inv->items[inv->count].quantity = 1;
                        inv->items[inv->count].isCombinable = false;
                        strcpy(inv->items[inv->count].combineWith, "");
                        strcpy(inv->items[inv->count].resultItem, "");
                        inv->count++;
                        printf("You automatically take the keycard and put it in your inventory.\n");
                    }
                    else
                    {
                        printf("Your inventory is full! You can't take the keycard.\n");
                        Item *keycard = CreateItem("Keycard", 1,
                                                   "A high-tech keycard that can unlock electronic doors.",
                                                   false, "", "");
                        AddItemToRoom(currentRoom, keycard);
                    }
                    keycardTaken = true;

                    // Update tree description
                    strcpy(currentRoom->interactables[i]->description,
                           "An unusual tree with metal components embedded in its trunk. The fruit has already fallen and the keycard is gone.");
                    return;
                }

                // If only fruit not dropped
                if (!fruitDropped)
                {
                    printf("You shake the tree and a suspicious fruit drops to the ground!\n");
                    Item *fruit = CreateItem("Suspicious fruit", 1,
                                             "A strange glowing fruit that doesn't look edible but might have other uses.",
                                             false, "", "");
                    AddItemToRoom(currentRoom, fruit);
                    fruitDropped = true;
                    strcpy(currentRoom->interactables[i]->description,
                           "An unusual tree with metal components embedded in its trunk. The fruit has already fallen.");
                    return;
                }

                // If only keycard not taken
                if (!keycardTaken)
                {
                    printf("With the jaguar out of the way, you can now examine the tree more closely...\n");
                    printf("There's something shiny embedded in the trunk. It looks like a keycard!\n");
                    if (inv->count < inv->capacity)
                    {
                        strcpy(inv->items[inv->count].name, "Keycard");
                        strcpy(inv->items[inv->count].description, "A high-tech keycard that can unlock electronic doors.");
                        inv->items[inv->count].quantity = 1;
                        inv->items[inv->count].isCombinable = false;
                        strcpy(inv->items[inv->count].combineWith, "");
                        strcpy(inv->items[inv->count].resultItem, "");
                        inv->count++;
                        printf("You automatically take the keycard and put it in your inventory.\n");
                    }
                    else
                    {
                        printf("Your inventory is full! You can't take the keycard.\n");
                        Item *keycard = CreateItem("Keycard", 1,
                                                   "A high-tech keycard that can unlock electronic doors.",
                                                   false, "", "");
                        AddItemToRoom(currentRoom, keycard);
                    }
                    keycardTaken = true;
                    strcpy(currentRoom->interactables[i]->description,
                           "An unusual tree with metal components embedded in its trunk. The keycard is gone.");
                    return;
                }

                // If both already done
                printf("There's nothing else of interest in the tree.\n");
                return;
            }
            // Default: print description
            printf("%s\n", currentRoom->interactables[i]->description);
            return;
        }
    }
    printf("There is no %s here to interact with.\n", objectName);
}

// Use an item on a target
void UseItem(Room *currentRoom, Inventory *inv, const char *itemName, const char *targetName)
{
    if (!HasItem(inv, itemName))
    {
        printf("You don't have %s in your inventory.\n", itemName);
        return;
    }
    // Special case for Golden Key on the golden door
    if (stricmp(itemName, "Golden Key") == 0 && stricmp(targetName, "golden door") == 0)
    {
        if (currentRoom->north && stricmp(currentRoom->north->name, "Gold Room") == 0)
        {
            printf("You use the Golden Key on the golden door. It unlocks!\n");
            currentRoom->north->isLocked = false;
            RemoveItemFromInventory(inv, "Golden Key");
            return;
        }
    }
    // Special case for Keycard on cyber room door
    if (stricmp(itemName, "Keycard") == 0 && stricmp(targetName, "metal door") == 0)
    {
        if (currentRoom->west && stricmp(currentRoom->west->name, "Cyber Room") == 0)
        {
            printf("You swipe the keycard on the door's reader. The door slides open!\n");
            currentRoom->west->isLocked = false;
            RemoveItemFromInventory(inv, "Keycard");
            return;
        }
    }
    // Blend suspicious fruit in kitchen to get anti-rust solution
    if (stricmp(itemName, "Suspicious fruit") == 0 && stricmp(targetName, "kitchen") == 0)
    {
        if (currentRoom->interactableCount > 0) {
            bool foundKitchen = false;
            for (int i = 0; i < currentRoom->interactableCount; i++) {
                if (stricmp(currentRoom->interactables[i]->name, "Kitchen") == 0) {
                    foundKitchen = true;
                    break;
                }
            }
            if (foundKitchen) {
                printf("You blend the suspicious fruit in the kitchen. It produces a potent anti-rust solution!\n");
                RemoveItemFromInventory(inv, "Suspicious fruit");
                Item *antiRust = CreateItem("Anti-Rust Solution", 1,
                    "A chemical solution that can remove rust from metal objects.",
                    true, "Rusty Cog", "Clean Cog");
                AddItemToRoom(currentRoom, antiRust);
                return;
            }
        }
        printf("There is no kitchen here to use the fruit with.\n");
        return;
    }
    // Use cog or clean cog to break glass pane and get crowbar
    if (
        (stricmp(itemName, "Rusty Cog") == 0 || stricmp(itemName, "Clean Cog") == 0) &&
        stricmp(targetName, "glass pane") == 0
    )
    {
        if (currentRoom->interactableCount > 0) {
            bool foundGlass = false;
            static bool crowbarTaken = false;
            for (int i = 0; i < currentRoom->interactableCount; i++) {
                if (stricmp(currentRoom->interactables[i]->name, "Glass Pane") == 0) {
                    foundGlass = true;
                    if (!crowbarTaken) {
                        printf("You smash the glass pane with the cog. Inside, you find a crowbar!\n");
                        Item *crowbar = CreateItem("Crowbar", 1,
                            "A sturdy crowbar, perfect for prying things open.",
                            false, "", "");
                        AddItemToRoom(currentRoom, crowbar);
                        crowbarTaken = true;
                        strcpy(currentRoom->interactables[i]->description,
                            "A shattered glass pane. The crowbar is gone.");
                    } else {
                        printf("The glass pane is already broken and the crowbar is gone.\n");
                    }
                    break;
                }
            }
            if (!foundGlass) {
                printf("There is no glass pane here to break.\n");
            }
            return;
        }
    }
    // Use crowbar on crate in Entrance Hall
    if (stricmp(itemName, "Crowbar") == 0 && stricmp(targetName, "crate") == 0)
    {
        if (stricmp(currentRoom->name, "Entrance Hall") == 0)
        {
            static bool crateOpened = false;
            if (!crateOpened)
            {
                printf("You pry open the crate with the crowbar! Inside, you find the second part of the golden key.\n");
                Item *keyPart2 = CreateItem("Key Part 2", 1,
                    "The second part of a three-part golden key.",
                    true, "Key Part 1", "Combined Key Parts");
                AddItemToRoom(currentRoom, keyPart2);
                crateOpened = true;
                // Optionally update crate description
                for (int i = 0; i < currentRoom->interactableCount; i++)
                {
                    if (stricmp(currentRoom->interactables[i]->name, "Crate") == 0)
                    {
                        strcpy(currentRoom->interactables[i]->description, "An empty crate, now pried open.");
                        break;
                    }
                }
            }
            else
            {
                printf("The crate is already open and empty.\n");
            }
            return;
        }
    }
    // Use clean cog on machine in Engine Room to get Key Part 3
    if (stricmp(itemName, "Clean Cog") == 0 && stricmp(targetName, "machine") == 0)
    {
        if (stricmp(currentRoom->name, "Engine Room") == 0)
        {
            static bool machineUsed = false;
            if (!machineUsed)
            {
                printf("You insert the clean cog into the machine. The machinery whirs to life and a hidden compartment opens, revealing the third part of the golden key!\n");
                Item *keyPart3 = CreateItem("Key Part 3", 1,
                    "The third part of a three-part golden key.",
                    true, "Combined Key Parts", "Golden Key");
                AddItemToRoom(currentRoom, keyPart3);
                machineUsed = true;
            }
            else
            {
                printf("The machine is already running and the compartment is empty.\n");
            }
            return;
        }
    }
    printf("You can't use %s on %s.\n", itemName, targetName);
}

// Log player actions
void LogAction(FILE *logFile, const char *action, const char *result)
{
    if (!logFile)
        return;
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    fprintf(logFile, "[%s] %s: %s\n", timestamp, action, result);
    fflush(logFile);
}

// Process player commands
void ProcessCommand(char *command, Inventory *inv, Room **currentRoom, bool *gameRunning, bool *hasWon, FILE *logFile)
{
    char cmd[100];
    char param1[100] = "";
    char param2[100] = "";
    // Convert command to lowercase
    for (int i = 0; command[i]; i++)
    {
        command[i] = tolower(command[i]);
    }
    // Parse command
    int params = sscanf(command, "%s %s %[^\n]", cmd, param1, param2);
    char result[256] = "";

    // Navigation commands
    if (strcmp(cmd, "north") == 0 || strcmp(cmd, "n") == 0)
    {
        if ((*currentRoom)->north != NULL)
        {
            if ((*currentRoom)->north->isLocked)
            {
                sprintf(result, "The door to the north is locked.");
                printf("%s\n", result);
            }
            else
            {
                *currentRoom = (*currentRoom)->north;
                sprintf(result, "Moved north to %s", (*currentRoom)->name);
                printf("%s\n", (*currentRoom)->description);
            }
        }
        else
        {
            sprintf(result, "You can't go north from here.");
            printf("%s\n", result);
        }
    }
    else if (strcmp(cmd, "south") == 0 || strcmp(cmd, "s") == 0)
    {
        if ((*currentRoom)->south != NULL)
        {
            if ((*currentRoom)->south->isLocked)
            {
                sprintf(result, "The door to the south is locked.");
                printf("%s\n", result);
            }
            else
            {
                *currentRoom = (*currentRoom)->south;
                sprintf(result, "Moved south to %s", (*currentRoom)->name);
                printf("%s\n", (*currentRoom)->description);
            }
        }
        else
        {
            sprintf(result, "You can't go south from here.");
            printf("%s\n", result);
        }
    }
    else if (strcmp(cmd, "east") == 0 || strcmp(cmd, "e") == 0)
    {
        if ((*currentRoom)->east != NULL)
        {
            if ((*currentRoom)->east->isLocked)
            {
                sprintf(result, "The door to the east is locked.");
                printf("%s\n", result);
            }
            else
            {
                *currentRoom = (*currentRoom)->east;
                sprintf(result, "Moved east to %s", (*currentRoom)->name);
                printf("%s\n", (*currentRoom)->description);
            }
        }
        else
        {
            sprintf(result, "You can't go east from here.");
            printf("%s\n", result);
        }
    }
    else if (strcmp(cmd, "west") == 0 || strcmp(cmd, "w") == 0)
    {
        if ((*currentRoom)->west != NULL)
        {
            if ((*currentRoom)->west->isLocked)
            {
                sprintf(result, "The door to the west is locked.");
                printf("%s\n", result);
            }
            else
            {
                *currentRoom = (*currentRoom)->west;
                sprintf(result, "Moved west to %s", (*currentRoom)->name);
                printf("%s\n", (*currentRoom)->description);
            }
        }
        else
        {
            sprintf(result, "You can't go west from here.");
            printf("%s\n", result);
        }
    }
    // Inventory commands
    else if (strcmp(cmd, "inventory") == 0 || strcmp(cmd, "i") == 0)
    {
        PrintInventory(inv);
        sprintf(result, "Displayed inventory");
    }
    else if (strcmp(cmd, "pick") == 0 && strcmp(param1, "up") == 0)
    {
        char *itemName = command + strlen("pick up ");
        while (*itemName == ' ') itemName++;
        PickupItem(inv, *currentRoom, itemName);
        sprintf(result, "Attempted to pick up %s", itemName);
    }
    else if (strcmp(cmd, "take") == 0 && params >= 2)
    {
        char *itemName = command + strlen("take ");
        while (*itemName == ' ') itemName++;
        PickupItem(inv, *currentRoom, itemName);
        sprintf(result, "Attempted to take %s", itemName);
    }
    else if (strcmp(cmd, "drop") == 0 && params >= 2)
    {
        char *itemName = command + strlen("drop ");
        while (*itemName == ' ') itemName++;
        DropItem(inv, *currentRoom, itemName);
        sprintf(result, "Attempted to drop %s", itemName);
    }
    else if (strcmp(cmd, "examine") == 0 && params >= 2)
    {
        char *itemName = command + strlen("examine ");
        while (*itemName == ' ') itemName++;
        ExamineItem(inv, itemName);
        sprintf(result, "Examined %s", itemName);
    }
    else if (strcmp(cmd, "interact") == 0 && params >= 2)
    {
        char *objectName = command + strlen("interact ");
        while (*objectName == ' ') objectName++;
        InteractWith(*currentRoom, inv, objectName);
        sprintf(result, "Interacted with %s", objectName);
    }
    else if (strcmp(cmd, "use") == 0 && params >= 3)
    {
        char *args = command + strlen("use ");
        while (*args == ' ') args++;
        char *lastSpace = strrchr(args, ' ');
        if (lastSpace)
        {
            char *itemName, *targetName;
            *lastSpace = '\0';
            itemName = args;
            targetName = lastSpace + 1;
            while (*targetName == ' ') targetName++;

            // Fix for known multi-word targets
            if (
                (strcmp(targetName, "door") == 0 || strcmp(targetName, "pane") == 0)
            )
            {
                // Move the last word from itemName to targetName
                char *secondLastSpace = strrchr(itemName, ' ');
                if (secondLastSpace)
                {
                    *secondLastSpace = '\0';
                    // Prepend the last word to targetName
                    static char fixedTarget[100];
                    snprintf(fixedTarget, sizeof(fixedTarget), "%s %s", secondLastSpace + 1, targetName);
                    targetName = fixedTarget;
                }
            }

            UseItem(*currentRoom, inv, itemName, targetName);
            sprintf(result, "Used %s on %s", itemName, targetName);
        }
        else
        {
            printf("Usage: use [item] [target]\n");
            sprintf(result, "Incorrect use command");
        }
    }
    else if (strcmp(cmd, "combine") == 0 && params >= 3)
    {
        char *args = command + strlen("combine ");
        while (*args == ' ') args++;

        // Find the split point (the space between the two items)
        // We'll split at the first space that is followed by a word that matches an item in inventory
        // But for simplicity, split at the middle space
        char *split = NULL;
        int spaceCount = 0;
        for (char *p = args; *p; ++p) {
            if (*p == ' ') spaceCount++;
        }
        if (spaceCount == 0) {
            printf("Usage: combine [item1] [item2]\n");
            sprintf(result, "Incorrect combine command");
        } else {
            int mid = spaceCount / 2;
            int count = 0;
            for (char *p = args; *p; ++p) {
                if (*p == ' ') {
                    if (count == mid) {
                        split = p;
                        break;
                    }
                    count++;
                }
            }
            if (split) {
                *split = '\0';
                char *item1 = args;
                char *item2 = split + 1;
                while (*item2 == ' ') item2++;
                CombineItems(inv, item1, item2);
                sprintf(result, "Combined %s with %s", item1, item2);
            } else {
                printf("Usage: combine [item1] [item2]\n");
                sprintf(result, "Incorrect combine command");
            }
        }
    }
    else if (strcmp(cmd, "push") == 0 && params >= 2)
    {
        char *objectName = command + strlen("push ");
        while (*objectName == ' ') objectName++;
        if (stricmp(objectName, "crate") == 0 && strcmp((*currentRoom)->name, "Engine Room") == 0)
        {
            printf("You push the crate aside, revealing a rucksack hidden behind it!\n");
            Item *rucksack = CreateItem("Rucksack", 1,
                                    "A sturdy rucksack that allows you to carry more items.",
                                    false, "", "");
            AddItemToRoom(*currentRoom, rucksack);
            sprintf(result, "Pushed crate, revealed rucksack");
        }
        else
        {
            printf("You can't push that here.\n");
            sprintf(result, "Attempted to push %s", objectName);
        }
    }
    else if (strcmp(cmd, "look") == 0)
    {
        printf("You are in %s.\n", (*currentRoom)->name);
        printf("%s\n", (*currentRoom)->description);
        printf("Exits: ");
        bool hasExits = false;
        if ((*currentRoom)->north)
        {
            printf("north");
            hasExits = true;
        }
        if ((*currentRoom)->south)
        {
            if (hasExits)
                printf(", ");
            printf("south");
            hasExits = true;
        }
        if ((*currentRoom)->east)
        {
            if (hasExits)
                printf(", ");
            printf("east");
            hasExits = true;
        }
        if ((*currentRoom)->west)
        {
            if (hasExits)
                printf(", ");
            printf("west");
            hasExits = true;
        }
        printf("\n");
        if ((*currentRoom)->itemCount > 0)
        {
            printf("Items in the room:\n");
            for (int i = 0; i < (*currentRoom)->itemCount; i++)
            {
                printf("- %s\n", (*currentRoom)->items[i]->name);
            }
        }
        if ((*currentRoom)->interactableCount > 0)
        {
            printf("You can interact with:\n");
            for (int i = 0; i < (*currentRoom)->interactableCount; i++)
            {
                printf("- %s\n", (*currentRoom)->interactables[i]->name);
            }
        }
        sprintf(result, "Looked around");
    }
    // Help command
    else if (strcmp(cmd, "help") == 0)
    {
        printf("Available commands:\n");
        printf("- north/n, south/s, east/e, west/w: Move in a direction\n");
        printf("- look: Look around the room\n");
        printf("- inventory/i: Check your inventory\n");
        printf("- take [item] or pick up [item]: Take an item from the room\n");
        printf("- drop [item]: Drop an item from your inventory\n");
        printf("- examine [item]: Look at an item in your inventory\n");
        printf("- interact [object]: Interact with an object in the room\n");
        printf("- use [item] [target]: Use an item on a target\n");
        printf("- combine [item1] [item2]: Combine two items in your inventory\n");
        printf("- push [object]: Push an object in the room\n");
        printf("- quit: Exit the game\n");
        sprintf(result, "Displayed help");
    }
    // Quit commands
    else if (strcmp(cmd, "quit") == 0)
    {
        printf("Thanks for playing!\n");
        *gameRunning = false;
        sprintf(result, "Quit game");
    }
    // Special case for winning
    else if (strcmp(cmd, "win") == 0 && strcmp((*currentRoom)->name, "Gold Room") == 0)
    {
        printf("Congratulations! You've unlocked the secrets of the temple and won the game!\n");
        *hasWon = true;
        *gameRunning = false;
        sprintf(result, "Won the game");
    }
    else
    {
        printf("Unknown command. Type 'help' for a list of commands.\n");
        sprintf(result, "Unknown command");
    }
    LogAction(logFile, command, result);
}

int main()
{
    // Initialize game
    bool gameRunning = true;
    bool hasWon = false;
    FILE *logFile = fopen("game_log.txt", "w");
    if (!logFile)
    {
        perror("Failed to open log file");
        return EXIT_FAILURE;
    }
    Inventory playerInventory;
    InitInventory(&playerInventory);

    // Create rooms
    Room *startingRoom = (Room *)malloc(sizeof(Room));
    Room *jungleRoom = (Room *)malloc(sizeof(Room));
    Room *engineRoom = (Room *)malloc(sizeof(Room));
    Room *cyberRoom = (Room *)malloc(sizeof(Room));
    Room *goldRoom = (Room *)malloc(sizeof(Room));
    if (!startingRoom || !jungleRoom || !engineRoom || !cyberRoom || !goldRoom)
    {
        perror("Failed to allocate memory for rooms");
        return EXIT_FAILURE;
    }

    // Setup starting room
    strcpy(startingRoom->name, "Entrance Hall");
    strcpy(startingRoom->description, "A dimly lit entrance hall with ancient stone walls. A golden door is visible to the north.");
    startingRoom->itemCount = 0;
    startingRoom->interactableCount = 0;
    startingRoom->isLocked = false;
    strcpy(startingRoom->keyName, "");
    // Add note to starting room
    Item *note = CreateItem("Note", 1,
                            "A faded note that reads: 'The guardian of the jungle seeks wisdom. The answer is Time.'",
                            false, "", "");
    startingRoom->items[startingRoom->itemCount++] = note;
    // Add crate to starting room
    Interactable *crate = CreateInteractable("Crate",
                                             "A heavy wooden crate. It looks like it needs a tool to open it.",
                                             "", "");
    startingRoom->interactables[startingRoom->interactableCount++] = crate;

    // Setup jungle room
    strcpy(jungleRoom->name, "Jungle Room");
    strcpy(jungleRoom->description, "A room filled with lush vegetation and the sounds of jungle creatures.");
    jungleRoom->itemCount = 0;
    jungleRoom->interactableCount = 0;
    jungleRoom->isLocked = false;
    strcpy(jungleRoom->keyName, "");
    // Add Rusty Cog to jungle room
    Item *rustyCog = CreateItem("Rusty Cog", 1,
    "A heavily rusted metal cog. Looks like it could fit into some machinery if it wasn't so rusty.",
    true, "Anti-Rust Solution", "Clean Cog");
    jungleRoom->items[jungleRoom->itemCount++] = rustyCog;
    // Add jaguar to jungle room
    Interactable *jaguar = CreateInteractable("Jaguar",
                                              "A majestic stone jaguar statue with emerald eyes.",
                                              "I am always coming but never arrive. What am I?", "Tomorrow");
    jungleRoom->interactables[jungleRoom->interactableCount++] = jaguar;
    // Add chest to jungle room
    Interactable *chest = CreateInteractable("Chest",
                                             "A wooden chest guarded by the jaguar statue.",
                                             "", "");
    jungleRoom->interactables[jungleRoom->interactableCount++] = chest;
    // Add tree to jungle room
    Interactable *tree = CreateInteractable("Tree",
                                            "An unusual tree with metal components embedded in its trunk.",
                                            "", "");
    jungleRoom->interactables[jungleRoom->interactableCount++] = tree;

    // Setup engine room
    strcpy(engineRoom->name, "Engine Room");
    strcpy(engineRoom->description, "A room filled with strange machinery. There's a large control panel in the center.");
    engineRoom->itemCount = 0;
    engineRoom->interactableCount = 0;
    engineRoom->isLocked = false;
    strcpy(engineRoom->keyName, "");
    // Add crate to engine room
    Interactable *engineCrate = CreateInteractable("Crate",
                                                   "A heavy crate pushed against the wall. Maybe there's something behind it?",
                                                   "", "");
    engineRoom->interactables[engineRoom->interactableCount++] = engineCrate;
    // Add machine to engine room
    Interactable *machine = CreateInteractable("Machine",
                                               "A complex machine with a slot that seems to fit a cog.",
                                               "", "");
    engineRoom->interactables[engineRoom->interactableCount++] = machine;

    // Setup cyber room
    strcpy(cyberRoom->name, "Cyber Room");
    strcpy(cyberRoom->description, "A futuristic room with blinking lights and high-tech equipment.");
    cyberRoom->itemCount = 0;
    cyberRoom->interactableCount = 0;
    cyberRoom->isLocked = true;
    strcpy(cyberRoom->keyName, "Keycard");
    // Add crowbar container to cyber room
    Interactable *glassPane = CreateInteractable("Glass Pane",
                                                 "A reinforced glass pane with a crowbar behind it.",
                                                 "", "");
    cyberRoom->interactables[cyberRoom->interactableCount++] = glassPane;
    // Add kitchen to cyber room
    Interactable *kitchen = CreateInteractable("Kitchen",
                                               "A hi-tech kitchen with various appliances, including a futuristic blender.",
                                               "", "");
    cyberRoom->interactables[cyberRoom->interactableCount++] = kitchen;

    // Setup gold room (win condition)
    strcpy(goldRoom->name, "Gold Room");
    strcpy(goldRoom->description, "A magnificent room filled with golden treasures! You have won the game!");
    goldRoom->itemCount = 0;
    goldRoom->interactableCount = 0;
    goldRoom->isLocked = true;
    strcpy(goldRoom->keyName, "Golden Key");

    // Connect rooms
    startingRoom->north = goldRoom;
    startingRoom->south = jungleRoom;
    startingRoom->east = engineRoom;
    startingRoom->west = cyberRoom;

    jungleRoom->north = startingRoom;
    jungleRoom->south = NULL;
    jungleRoom->east = NULL;
    jungleRoom->west = NULL;

    engineRoom->north = NULL;
    engineRoom->south = NULL;
    engineRoom->east = NULL;
    engineRoom->west = startingRoom;

    cyberRoom->north = NULL;
    cyberRoom->south = NULL;
    cyberRoom->east = startingRoom;
    cyberRoom->west = NULL;

    goldRoom->north = NULL;
    goldRoom->south = startingRoom;
    goldRoom->east = NULL;
    goldRoom->west = NULL;

    // Set the current room
    Room *currentRoom = startingRoom;

    // Print welcome message
    printf("===========================================\n");
    printf("      TEMPLE OF SECRETS - TEXT ADVENTURE   \n");
    printf("===========================================\n\n");
    printf("You are an explorer seeking the treasures of an ancient temple.\n");
    printf("Navigate through the rooms, solve puzzles, and find the golden key to win!\n");
    printf("Type 'help' for a list of commands.\n\n");
    printf("You are in %s.\n", currentRoom->name);
    printf("%s\n", currentRoom->description);

    // Main game loop
    char command[100];
    while (gameRunning)
    {
        printf("\n> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;
        ProcessCommand(command, &playerInventory, &currentRoom, &gameRunning, &hasWon, logFile);
        if (strcmp(currentRoom->name, "Gold Room") == 0 && !hasWon)
        {
            printf("Congratulations! You've made it to the Gold Room and found the treasure!\n");
            printf("YOU WIN!\n");
            hasWon = true;
            gameRunning = false;
        }
    }
    fclose(logFile);

    // Free allocated memory
    for (int i = 0; i < startingRoom->itemCount; i++)
        free(startingRoom->items[i]);
    for (int i = 0; i < jungleRoom->itemCount; i++)
        free(jungleRoom->items[i]);
    for (int i = 0; i < engineRoom->itemCount; i++)
        free(engineRoom->items[i]);
    for (int i = 0; i < cyberRoom->itemCount; i++)
        free(cyberRoom->items[i]);
    for (int i = 0; i < goldRoom->itemCount; i++)
        free(goldRoom->items[i]);
    for (int i = 0; i < startingRoom->interactableCount; i++)
        free(startingRoom->interactables[i]);
    for (int i = 0; i < jungleRoom->interactableCount; i++)
        free(jungleRoom->interactables[i]);
    for (int i = 0; i < engineRoom->interactableCount; i++)
        free(engineRoom->interactables[i]);
    for (int i = 0; i < cyberRoom->interactableCount; i++)
        free(cyberRoom->interactables[i]);
    for (int i = 0; i < goldRoom->interactableCount; i++)
        free(goldRoom->interactables[i]);
    free(startingRoom);
    free(jungleRoom);
    free(engineRoom);
    free(cyberRoom);
    free(goldRoom);
    free(playerInventory.items);
    return 0;
}