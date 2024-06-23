#include <unistd.h>
#include <cstdlib>
#include <cstdarg>
#include <cinttypes>
#include <cstdio>
#include <sys/time.h>

#include "json.h"
#include "log.h"


struct log_obj
{
    uint8_t type;
    char *key;
    union
    {
        unsigned int uint_val;
        int int_val;
        char *char_val;
    };
} log_obj;
int LogLevel = LOG_DEBUG;


void log_level(int logLevel) {
    LogLevel = logLevel;
}
int64_t currentTimeMillis() {
    struct timeval time{};
    gettimeofday(&time, nullptr);
    int64_t s1 = (int64_t)(time.tv_sec) * 1000;
    int64_t s2 = (time.tv_usec / 1000);
    return s1 + s2;
}

[[maybe_unused]] static enum {
    LOG_INT,
    LOG_UINT,
    LOG_STRING,
    LOG_BOOLEAN
} log_field_types;


static struct log_obj* log_field_new(const char* key) {
    auto* field = (struct log_obj*)malloc(sizeof(log_obj));
    memset(field, 0, sizeof(log_obj));
    field->key = (char*)malloc(strlen(key) + 1);
    strcpy(field->key, key);
    return field;
}

static void log_field_free(struct log_obj* sf) {
    if (sf == nullptr) return;
    if (sf->key != nullptr) free(sf->key);
    if ((sf->type == LOG_STRING) && (sf->char_val != nullptr)) free(sf->char_val);
    free(sf);
}

struct log_obj* lk_i(const char* key, int value) {
    struct log_obj* field = log_field_new(key);
    field->type = LOG_INT;
    field->int_val = value;
    return field;
}

struct log_obj* lk_ui(const char* key, unsigned int value) {
    struct log_obj* field = log_field_new(key);
    field->type = LOG_UINT;
    field->uint_val = value;
    return field;
}

struct log_obj* lk_s(const char* key, const char* value) {
    struct log_obj* field = log_field_new(key);
    field->type = LOG_STRING;

    unsigned int char_len = strlen(value);
    field->char_val = (char*)malloc(char_len + 1);

    for (int i = 0; i < char_len; i ++) {
        char ch = value[i];
        if (ch == '\\')  ch = '/';
        field->char_val[i] = ch;
    }
    field->char_val[char_len] = 0;
    return field;
}

struct log_obj* lk_b(const char* key, bool value) {
    if (value) return lk_s(key, "true");
    return lk_s(key, "false");
}

void log_process(int level, const char* fileName, int line, const char* msg, ...) {
    if (level < LogLevel) return;

    int64_t time = currentTimeMillis();
    fprintf(stdout, "{\"level\":%d,\"time\":%" PRId64 ",\"source\":\"%s:%d\",\"msg\":\"%s\"", level, time, fileName, line,  msg);

    va_list ap;
    va_start(ap, msg);

    for (int i = 1;; i++) {
        struct log_obj* arg = va_arg(ap, struct log_obj*);
        if (arg == nullptr) break;


        if (arg->type == LOG_INT) fprintf(stdout, ",\"%s\":%d", arg->key, arg->int_val);
        else if (arg->type == LOG_UINT) fprintf(stdout, R"(,"%s": "%#08x")", arg->key, arg->uint_val);
        else if (arg->type == LOG_STRING) fprintf(stdout, R"(,"%s":"%s")", arg->key, arg->char_val);

        log_field_free(arg);
    }

    fprintf(stdout, "}\n");
    fflush(stdout);
}