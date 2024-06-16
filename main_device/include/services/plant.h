#ifndef PLANT_H
#define PLANT_H

typedef struct plant {
    char* name;
    float min_humidity;
    float min_water_level;
    float min_light_intensity;
} PLANT;

extern PLANT plants[];
extern int plant_count;
extern int plant_index;
extern int plant_quantity;

#endif // PLANT_H
