#pragma once
#ifndef YMIRDB_H
#define YMIRDB_H

#define MAX_KEY 16
#define MAX_LINE 1024

#define RET_YES 0
#define RET_NO 1



enum item_type {
    INTEGER = 0,
    ENTRY = 1
};
typedef struct element element;
typedef struct entry entry;
typedef struct snapshot snapshot;

struct element {
    enum item_type type;
    union {
        int value;
        struct entry* entry;
    };
};

struct entry {
    char key[MAX_KEY];
    element* values;//���
    size_t length;
    entry* next;
    entry* prev;
    int maxx;
    int minn;
    int summ;
    int lenn;

    size_t forward_size;
    size_t backward_size;
    entry** forward;
    entry** backward;

    int is_simple;
    int is_del;

};
struct snapshot {
    int id;
    entry* entries;
    snapshot* next;
    snapshot* prev;
};


extern entry cur_entry_h;
extern snapshot g_snapshot_h;
extern snapshot* cur_snapshot_h;

extern void entry_head_init(entry* head);
extern entry* entry_create_node(entry* entry_h, char* key, char** values, int len);
extern entry* entry_find_node(entry* head, char* key);
extern entry* entry_insert_node(entry* head, entry* pnode);
extern void entry_delete_node(entry* pnode);
extern void entry_delete_list(entry* head);
extern entry* entry_add_head(entry* new_head, entry* old_head);
extern entry* entry_pick_off_head(entry* old_head);
extern entry* entry_copy(entry* new_head, entry* old_head);


extern void snapshot_head_init(snapshot* head);
extern snapshot* snapshot_create_node(int id);
extern snapshot* snapshot_find_node(snapshot* head, int id);
extern snapshot* snapshot_insert_node(snapshot* head, snapshot* pnode);
extern void snapshot_delete_node(snapshot* pnode);
extern void snapshot_delete_list(snapshot* head);
extern void snapshot_delete_more_than_ID(snapshot* head, int id);
extern int isdigitstr(char* str);
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
