#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_NAME_LENGTH 50

// Item structure to hold item name and quantity
typedef struct {
    char name[MAX_NAME_LENGTH];
    int quantity;
} Item;

//Inventory structure to hold items and total capacity
typedef struct {
    Item* items;
    int capacity;
    int count;
} Inventory;

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

void Rucksack(Inventory* inv) {
    printf("You picked up a rucksack! Now you can carry more items.\n");
    ExpandInventory(inv, 9);
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

}

void main() {

    return 0;
}