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
    // Check if there is space in the inventory
    if (inv->count >= inv->capacity)
    {
        printf("Inventory is full! Cannot pick up %s.\n", itemName);
        return;
    }

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

    // Add item to inventory
    strcpy(inv->items[inv->count].name, currentRoom->items[itemIndex]->name);
    strcpy(inv->items[inv->count].description, currentRoom->items[itemIndex]->description);
    inv->items[inv->count].quantity = currentRoom->items[itemIndex]->quantity;
    inv->items[inv->count].isCombinable = currentRoom->items[itemIndex]->isCombinable;
    strcpy(inv->items[inv->count].combineWith, currentRoom->items[itemIndex]->combineWith);
    strcpy(inv->items[inv->count].resultItem, currentRoom->items[itemIndex]->resultItem);
    inv->count++;

    // Special case for Rucksack
    if (stricmp(itemName, "Rucksack") == 0)
    {
        printf("You picked up a rucksack! Now you can carry more items.\n");
        ExpandInventory(inv, 9);
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
            else if (strcmp(objectName, "Chest") == 0)
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

                // Use a file-scope static variable to track if key part has been shown
                static bool keyPartRevealed = false;
                // Track if key has been taken separately
                static bool keyPartTaken = false;

                if (jaguarSatisfied)
                {
                    if (!keyPartRevealed)
                    {
                        printf("You open the chest and find the first part of a golden key!\n");
                        printf("You can now take the key part.\n");

                        // Create the key part and add it to the room
                        Item *keyPart = CreateItem("Key Part 1", 1,
                                                   "The first part of a three-part golden key.",
                                                   true, "Key Part 2", "Combined Key Parts");

                        // Add to room instead of inventory
                        AddItemToRoom(currentRoom, keyPart);
                        keyPartRevealed = true;

                        // Update chest description
                        strcpy(currentRoom->interactables[i]->description,
                               "An open chest. The key part is available to take.");
                    }
                    else if (keyPartTaken)
                    {
                        printf("The chest is empty. You've already taken the key part.\n");
                    }
                    else
                    {
                        printf("The chest is open, revealing the first part of a golden key. You can take it.\n");
                    }
                }
                else
                {
                    printf("The chest is guarded by the jaguar. You need to satisfy it first.\n");
                }
                return;
            }
            // --- TREE LOGIC ---
            else if (strcmp(objectName, "Tree") == 0)
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

                // Use a file-scope static variable for tracking if keycard revealed
                static bool keycardRevealed = false;
                // Track if keycard has been taken separately
                static bool keycardTaken = false;

                if (!jaguarSatisfied)
                {
                    printf("The jaguar is blocking your way to examine the tree fully. You need to solve its riddle first.\n");
                    return;
                }

                if (!keycardRevealed)
                {
                    printf("With the jaguar out of the way, you can now examine the tree more closely...\n");
                    printf("There's something shiny embedded in the trunk. It looks like a keycard!\n");
                    printf("You can now take the keycard.\n");

                    // Create keycard and add it to the room
                    Item *keycard = CreateItem("Keycard", 1,
                                               "A high-tech keycard that can unlock electronic doors.",
                                               false, "", "");

                    // Add keycard to room instead of inventory
                    AddItemToRoom(currentRoom, keycard);
                    keycardRevealed = true;

                    // Update tree description
                    strcpy(currentRoom->interactables[i]->description,
                           "An unusual tree with metal components embedded in its trunk. A keycard is visible in the trunk.");
                }
                else if (keycardTaken)
                {
                    printf("You already took the keycard from the tree.\n");
                }
                else
                {
                    printf("The tree trunk has a keycard embedded in it. You can take it.\n");
                }
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
    if (strcmp(itemName, "Golden Key") == 0 && strcmp(targetName, "golden door") == 0)
    {
        if (currentRoom->north && strcmp(currentRoom->north->name, "Gold Room") == 0)
        {
            printf("You use the Golden Key on the golden door. It unlocks!\n");
            currentRoom->north->isLocked = false;
            RemoveItemFromInventory(inv, "Golden Key");
            return;
        }
    }
    // Special case for Keycard on cyber room door
    if (strcmp(itemName, "Keycard") == 0 && strcmp(targetName, "metal door") == 0)
    {
        if (currentRoom->west && strcmp(currentRoom->west->name, "Cyber Room") == 0)
        {
            printf("You swipe the keycard on the door's reader. The door slides open!\n");
            currentRoom->west->isLocked = false;
            RemoveItemFromInventory(inv, "Keycard");
            return;
        }
    }
    // Special case for Cog with Anti-Rust Solution
    if (strcmp(itemName, "Anti-Rust Solution") == 0 && strcmp(targetName, "Rusty Cog") == 0)
    {
        if (HasItem(inv, "Rusty Cog"))
        {
            printf("You apply the Anti-Rust Solution to the Rusty Cog.\n");
            RemoveItemFromInventory(inv, "Anti-Rust Solution");
            RemoveItemFromInventory(inv, "Rusty Cog");
            Item *cleanCog = CreateItem("Clean Cog", 1,
                                        "A clean, rust-free cog that can be used in machinery.",
                                        false, "", "");
            if (inv->count < inv->capacity)
            {
                strcpy(inv->items[inv->count].name, cleanCog->name);
                strcpy(inv->items[inv->count].description, cleanCog->description);
                inv->items[inv->count].quantity = cleanCog->quantity;
                inv->items[inv->count].isCombinable = cleanCog->isCombinable;
                strcpy(inv->items[inv->count].combineWith, "");
                strcpy(inv->items[inv->count].resultItem, "");
                inv->count++;
                printf("You created %s!\n", cleanCog->name);
                free(cleanCog);
            }
            else
            {
                printf("Your inventory is full! You can't take the clean cog.\n");
                AddItemToRoom(currentRoom, cleanCog);
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
    char param1[50] = "";
    char param2[50] = "";
    // Convert command to lowercase
    for (int i = 0; command[i]; i++)
    {
        command[i] = tolower(command[i]);
    }
    // Parse command
    int params = sscanf(command, "%s %s %s", cmd, param1, param2);
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
        PickupItem(inv, *currentRoom, itemName);
        sprintf(result, "Attempted to pick up %s", itemName);
    }
    else if (strcmp(cmd, "take") == 0)
    {
        char *itemName = command + strlen("take ");
        PickupItem(inv, *currentRoom, itemName);
        sprintf(result, "Attempted to take %s", itemName);
    }
    else if (strcmp(cmd, "drop") == 0 && params >= 2)
    {
        DropItem(inv, *currentRoom, param1);
        sprintf(result, "Attempted to drop %s", param1);
    }
    else if (strcmp(cmd, "examine") == 0 && params >= 2)
    {
        ExamineItem(inv, param1);
        sprintf(result, "Examined %s", param1);
    }
    // Interaction commands
    else if (strcmp(cmd, "interact") == 0 && params >= 2)
    {
        InteractWith(*currentRoom, inv, param1);
        sprintf(result, "Interacted with %s", param1);
    }
    else if (strcmp(cmd, "use") == 0 && params >= 3)
    {
        UseItem(*currentRoom, inv, param1, param2);
        sprintf(result, "Used %s on %s", param1, param2);
    }
    else if (strcmp(cmd, "combine") == 0 && params >= 3)
    {
        CombineItems(inv, param1, param2);
        sprintf(result, "Combined %s with %s", param1, param2);
    }
    else if (strcmp(cmd, "push") == 0 && params >= 2)
    {
        if (strcmp(param1, "crate") == 0 && strcmp((*currentRoom)->name, "Engine Room") == 0)
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
            sprintf(result, "Attempted to push %s", param1);
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
    // Add sledgehammer container to cyber room
    Interactable *glassPane = CreateInteractable("Glass Pane",
                                                 "A reinforced glass pane with a sledgehammer behind it.",
                                                 "", "");
    cyberRoom->interactables[cyberRoom->interactableCount++] = glassPane;
    // Add kitchen to cyber room
    Interactable *kitchen = CreateInteractable("Kitchen",
                                               "A hi-tech kitchen with various appliances, including a futuristic blender.",
                                               "", "");
    cyberRoom->interactables[cyberRoom->interactableCount++] = kitchen;
    // Add fruit to cyber room
    Item *fruit = CreateItem("Suspicious fruit", 1,
                             "A strange glowing fruit that doesn't look edible but might have other uses.",
                             false, "", "");
    cyberRoom->items[cyberRoom->itemCount++] = fruit;

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