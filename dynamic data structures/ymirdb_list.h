#pragma once
#ifndef YMIRDB_LIST_H
#define YMIRDB_LIST_H

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


#endif
