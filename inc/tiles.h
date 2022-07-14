#include <stdint.h>

typedef struct {
int tilew, tileh;
int numtw, numth;
int numlayers;
int wrapX, wrapY;
int *layerplx;
uint16_t **layers;
} dtilemap_t;

extern const uint16_t donna_layer00[];

extern const uint16_t sonic_layer00[];
extern const uint16_t sonic_layer01[];

extern const uint16_t* donna_tmxl[];
extern const uint16_t* sonic_tmxl[];

extern const int donna_tmxlplx[][2];
extern const int sonic_tmxlplx[][2];

extern const dtilemap_t donna_tmx;
extern const dtilemap_t sonic_tmx;