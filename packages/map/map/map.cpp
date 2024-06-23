int MAP_SIZE = 1500;
int MAP_ARRAY_SIZE = 1500 * 1500;
int MAP[1500 * 1500];
unsigned int MAX_SIZE_X = 0;
unsigned int MAX_SIZE_Y = 0;

#define MAP_DATA_INVALID (-1)       // Invalid
#define MAP_DATA_CLEANED 11110    // Cleaned for start/end positions
#define MAP_DATA_FILLED 11111     // Filled gaps
#define MAP_DATA_THICKENED 11113  // Wall thickened
#define MAP_DATA_AVOID 11115      // Should be avoided

unsigned int map_max_x() {
    return MAX_SIZE_X;
}
unsigned int map_max_y() {
    return MAX_SIZE_Y;
}
unsigned int map_offset(unsigned int x, unsigned int y) {
    return x * MAP_SIZE + y;
}

int map_value(int x, int y) {
    return MAP[map_offset(x, y)];
}

void map_set(unsigned int x, unsigned int y, int value) {
    if (value % 2 == 0) {
        if (x > MAX_SIZE_X) MAX_SIZE_X = x;
        if (y > MAX_SIZE_Y) MAX_SIZE_Y = y;
    }
    MAP[map_offset(x, y)] = value;
}

void map_reset() {
    MAX_SIZE_X = 0;
    MAX_SIZE_Y = 0;
    for (int x = 0; x < MAP_ARRAY_SIZE; x++) MAP[x] = MAP_DATA_INVALID;
}
