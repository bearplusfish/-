#pragma once
#ifndef YMIRDB_CMD_H
#define YMIRDB_CMD_H



enum g_cmd {
    CMD_BYE,
    CMD_HELP,
    CMD_LIST_KEYS,
    CMD_LIST_ENTRIES,
    CMD_LIST_SNAPSHOTS,
    CMD_GET,
    CMD_DEL,
    CMD_PURGE,
    CMD_SET,
    CMD_PUSH,
    CMD_APPEND,
    CMD_PICK,
    CMD_PLUCK,
    CMD_POP,
    CMD_DROP,
    CMD_ROLLBACK,
    CMD_CHECKOUT,
    CMD_SNAPSHOT,
    CMD_MIN,
    CMD_MAX,
    CMD_SUM,
    CMD_LEN,
    CMD_REV,
    CMD_UNIQ,
    CMD_SORT,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_TYPE,
    CMD_NUM
};

extern char* g_cmd_str[CMD_NUM];
extern char* g_cmd_help[CMD_NUM];

typedef struct string_segmentation_struct
{
    char** strings;
    int len;
    char* internal_buf;
}string_segmentation_s;

extern void string_segmentation(char delimiter, char* string, int limit, string_segmentation_s* string_array);
extern void string_segmentation_free(string_segmentation_s* string_array);


extern void snapshot_free(snapshot* snapshot_h);

extern int cmd_get_str_from_file(char* cmd, int* cmd_len, FILE* fp);
extern int cmd_get_str_from_stdin(char cmd[], int* cmd_len);
extern int cmd_get_id(string_segmentation_s* string_array);


extern void cmd_dispatch(int cmd_id, string_segmentation_s* string_array);

#endif
