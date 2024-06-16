#include "services/plant.h"

PLANT plants[] = {
    {"tomato", 60.0, 150.0, 300.0},
    {"Lettuce", 80.0, 150.0, 300.0},
    {"apple_mint", 80.0, 150.0, 300.0}
};

int plant_count = sizeof(plants) / sizeof(plants[0]);
int plant_index = 0;
int plant_quantity = 0;
