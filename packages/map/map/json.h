#pragma once

#include <cstdint>

/** Really stupid JSON encoder which dumps to stdout/stderr */
void json_start();
void json_start(FILE* fp);

void json_comma();
void json_value(int value);
void json_key_raw(const char *key);
void json_key_value(const char *key, int value);
void json_key_value(const char *key, unsigned int value);
void json_key_value(const char *key, int64_t value);
void json_key_value(const char *key, char *value);
void json_key_value(const char* key, bool value);
void json_array_start(const char *key);
void json_array_start();
void json_array_end();
void json_object_start();
void json_object_start(const char *key);
void json_object_end();
void json_end();
void json_end(bool force);
