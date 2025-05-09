#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_NAME_LENGTH 50

typedef struct {
    char name[MAX_NAME_LENGTH];
    int quantity;
} Item;

typedef struct {
    Item* items;
    int capacity;
    int count;
} Inventory;

void InitInventory(Inventory* inv) {
    inv->capacity = 1;
    inv->count = 0;
    inv->items = (Item*)malloc(sizeof(Item) * inv->capacity);
    if (inv->items == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

void expandInventory(Inventory* inv, int additional_space) {
    int new_capacity = inv->capacity + additional_space;
    Item* new_items = realloc(inv->items, sizeof(Item) * new_capacity);
    if (!new_items) {
        perror("Failed to expand inventory");
        return;
    }
    inv->items = new_items;
    inv->capacity = new_capacity;
}


void main() {

    return 0;
}

//lororirjur