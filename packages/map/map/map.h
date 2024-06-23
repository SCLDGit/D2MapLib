#pragma once

int map_max_x();
int map_max_y();

int map_offset(int x, int y);
int map_value(int x, int y);
void map_set(unsigned int x, unsigned int y, int value);
void map_reset();
