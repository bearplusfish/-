#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ymirdb.h"


entry cur_entry_h;
snapshot g_snapshot_h;
snapshot* cur_snapshot_h;

int isdigitstr(char* str)
{
    return (strspn(str, "0123456789") == strlen(str));
}



void entry_print_order(entry* head)
{
    entry* pnode;
    if (NULL == head) return;
    pnode = head->next;
    while (pnode != head)
    {
        printf("%s\n", pnode->key);
        pnode = pnode->next;
    }

}

void entry_print_rever(entry* head)
{
    entry* pnode;
    if (NULL == head) return;
    pnode = head->next;
    while (pnode != head)
    {
        printf("%s\n", pnode->key);
        pnode = pnode->next;
    }

}
void entry_head_init(entry* head)
{
    head->key[0] = '\0';
    head->next = head;
    head->prev = head;
}

entry* entry_create_node(entry* entry_h, char* key, char** values, int len)
{
    int i;
    entry* p = (entry*)malloc(sizeof(entry));
    strcpy(p->key, key);
    p->length = len;
    if (len > 0) {
        p->is_simple = 0;
        p->is_del = 0;
        p->values = (element*)malloc(sizeof(element) * len);
        p->forward = (entry**)malloc(sizeof(entry*) * len);
        p->backward = (entry**)malloc(sizeof(entry*) * len);
        p->forward_size = 0;
        p->backward_size = 0;
        for (i = 0; i < len; i++) {
            if (isdigitstr(values[i]))//����ֵΪ���ʾ������
            {
                p->values[i].value = atoi(values[i]);
                p->values[i].type = 0;
            }
            else {
                p->values[i].type = 1;
                p->values[i].entry = entry_find_node(entry_h, values[i]);
                p->values[i].entry->is_del = 1;
                if (p->values[i].entry == NULL) {
                    printf("no such key");
                    return NULL;
                }
                if (strcmp(p->values[i].entry->key, key) == 0) {
                    printf("not permitted");
                    return NULL;
                }
                p->is_simple++;
                p->forward[p->forward_size] = p->values[i].entry;
                p->forward_size++;
                p->values[i].entry->backward[p->values[i].entry->backward_size] = p;
                p->values[i].entry->backward_size++;

            }

        }
    }
    else {
        p->values = NULL;
    }
    p->next = p;//ֻ�Ǵ���
    p->prev = p;
    return p;
}


entry* entry_find_node(entry* head, char* key)
{
    entry* pnode = head->next;

    while (pnode != head && pnode->key[0] != '\0' && strcmp(pnode->key, key) != 0)
    {
        pnode = pnode->next;
    }
    if (pnode == head)  return NULL;
    return pnode;
}

entry* entry_insert_node(entry* head, entry* pnode)
{
    entry* q;
    if (head == NULL || pnode == NULL)return NULL;

    q = head->next;
    head->next = pnode;
    pnode->prev = head;
    pnode->next = q;
    q->prev = pnode;
    return head;

}

void entry_delete_node(entry* pnode)
{
    if (NULL == pnode) return;
    pnode->prev->next = pnode->next;
    pnode->next->prev = pnode->prev;
    //   if (pnode->values)
      //     free(pnode->values);
    if (pnode)
        free(pnode);
    return;
}

void entry_delete_list(entry* head)
{
    entry* pnode = head->next;
    entry* ptmp;
    if (NULL == head) return;

    while (pnode != head)
    {
        ptmp = pnode;
        pnode = pnode->next;
        //      if (ptmp->values)
         //         free(ptmp->values);
        if (ptmp)
            free(ptmp);
    }
    entry_head_init(head);
}

entry* entry_add_head(entry* new_head, entry* old_head)
{
    if (new_head == NULL || old_head == NULL)return NULL;

    entry_head_init(new_head);
    entry_insert_node(old_head, new_head);

    return new_head;
}

entry* entry_pick_off_head(entry* old_head)
{

    entry* new_head = old_head->next;
    entry* qnode = old_head->prev;

    new_head->prev = qnode;
    qnode->next = new_head;

    return new_head;
}

entry* entry_copy(entry* new_head, entry* old_head)
{

    //entry* pnode;
    entry* pnode = old_head->next;

    if (NULL == new_head || NULL == old_head) return NULL;
    entry_head_init(new_head);
    while (pnode != old_head) {
        entry* ptmp = (entry*)malloc(sizeof(entry));
        memcpy(ptmp, pnode, sizeof(entry));
        ptmp->values = (element*)malloc(sizeof(element) * pnode->length);//14
      //  ptmp->next = NULL;
        //ptmp->prev = NULL;
        memcpy(ptmp->values, pnode->values, sizeof(element) * pnode->length);
        entry_insert_node(new_head, ptmp);
        pnode = pnode->next;
    }
    return new_head;
}


void snapshot_head_init(snapshot* head)
{
    head->next = head;
    head->prev = head;
}

snapshot* snapshot_create_node(int id)
{
    snapshot* p = (snapshot*)malloc(sizeof(snapshot));
    p->id = id;
    p->entries = NULL;
    p->next = p;
    p->prev = p;

    return p;
}


snapshot* snapshot_find_node(snapshot* head, int id)
{
    //snapshot* pnode = head->next;
    snapshot* pnode = head->next;
    while (pnode != head) {
        if (pnode->id == id) {
            return pnode;
        }
        pnode = pnode->next;
    }
    if (pnode == head)  return NULL;
    return head;
}


snapshot* snapshot_insert_node(snapshot* head, snapshot* pnode)
{

    head->prev->next = pnode;
    pnode->next = head;

    pnode->prev = head->prev;
    head->prev = pnode;

    /*
    snapshot* newnode = pnode;
    snapshot* first = head->next;
    head->next = newnode;
    newnode->prev = head;
    newnode->next = first;
    first->prev = newnode;*/
    return head;
}

void snapshot_delete_node(snapshot* pnode)
{
    entry entry_head;
    snapshot* prev = pnode->prev;
    snapshot* next = pnode->next;

    if (NULL == pnode) return;
    /*
    pnode->prev->next = pnode->next;
    pnode->next->prev = pnode->prev;
    */
    entry_add_head(&entry_head, pnode->entries);
    entry_delete_list(&entry_head);

    prev->next = next;
    next->prev = prev;
    free(pnode);
    return;
}

void snapshot_delete_list(snapshot* head)
{
    snapshot* pnode = head->next;
    snapshot* ptmp;
    entry entry_head;

    if (NULL == head) return;

    while (pnode != head)
    {

        ptmp = pnode;
        pnode = pnode->next;
        entry_add_head(&entry_head, ptmp->entries);
        entry_delete_list(&entry_head);
        if (ptmp)
            free(ptmp);
        ptmp = NULL;
    }
    snapshot_head_init(head);
}


void snapshot_delete_more_than_ID(snapshot* head, int id)
{
    snapshot* pnode = head->next;
    snapshot* ptmp;

    if (NULL == head) return;

    while (pnode != head)
    {
        ptmp = pnode;
        pnode = pnode->next;
        if (ptmp->id > id) {
            snapshot_delete_node(ptmp);
            ptmp = NULL;
        }
    }
}




char* g_cmd_str[CMD_NUM] = {
    "BYE",
    "HELP",
    "LIST KEYS",
    "LIST ENTRIES",
    "LIST SNAPSHOTS",
    "GET",
    "DEL",
    "PURGE",
    "SET",
    "PUSH",
    "APPEND",
    "PICK",
    "PLUCK",
    "POP",
    "DROP",
    "ROLLBACK",
    "CHECKOUT",
    "SNAPSHOT",
    "MIN",
    "MAX",
    "SUM",
    "LEN",
    "REV",
    "UNIQ",
    "SORT",
    "FORWARD",
    "BACKWARD",
    "TYPE"
};

char* g_cmd_help[CMD_NUM] = {
    "BYE   clear database and exit",
    "HELP  display this help message\n",
    "LIST KEYS       displays all keys in current state",
    "LIST ENTRIES    displays all entries in current state",
    "LIST SNAPSHOTS  displays all snapshots in the database\n",
    "GET <key>    displays entry values",
    "DEL <key>    deletes entry from current state",
    "PURGE <key>  deletes entry from current state and snapshots\n",
    "SET <key> <value ...>     sets entry values",
    "PUSH <key> <value ...>    pushes values to the front",
    "APPEND <key> <value ...>  appends values to the back\n",
    "PICK <key> <index>   displays value at index",
    "PLUCK <key> <index>  displays and removes value at index",
    "POP <key>            displays and removes the front value\n",
    "DROP <id>      deletes snapshot",
    "ROLLBACK <id>  restores to snapshot and deletes newer snapshots",
    "CHECKOUT <id>  replaces current state with a copy of snapshot",
    "SNAPSHOT       saves the current state as a snapshot\n",
    "MIN <key>  displays minimum value",
    "MAX <key>  displays maximum value",
    "SUM <key>  displays sum of values",
    "LEN <key>  displays number of values\n",
    "REV <key>   reverses order of values (simple entry only)",
    "UNIQ <key>  removes repeated adjacent values (simple entry only)",
    "SORT <key>  sorts values in ascending order (simple entry only)\n",
    "FORWARD <key> lists all the forward references of this key",
    "BACKWARD <key> lists all the backward references of this key",
    "TYPE <key> displays if the entry of this key is simple or general"
};



void string_segmentation(char delimiter, char* string, int limit, string_segmentation_s* string_array)
{
    int count = 1;
    char* pchar, ** ptr;

    if (NULL != string_array) {
        memset(string_array, 0, sizeof(string_segmentation_s));
    }

    if (NULL == string || NULL == string_array || string[0] == '\0')
    {
        return;
    }

    if (0 == limit)
    {
        limit = 99999;
    }

    string_array->internal_buf = strdup(string);//�ַ�������
    if (NULL == string_array->internal_buf)
    {
        return;
    }

    pchar = string;
    while ('\0' != *pchar && (int)count < limit)
    {
        if (delimiter == *pchar)
        {
            count++;
        }
        pchar++;
    }
    string_array->strings = (char**)malloc(count * sizeof(char*));
    if (NULL == string_array->strings)
    {
        return;
    }
    string_array->len = count;

    ptr = string_array->strings;
    *ptr = string_array->internal_buf;
    pchar = string_array->internal_buf;
    while ('\0' != *pchar && count > 1)
    {
        if (delimiter == *pchar)
        {
            ptr++;
            *ptr = pchar + 1;
            *pchar = '\0';
            count--;
        }
        pchar++;
    }
}

void string_segmentation_free(string_segmentation_s* string_array)
{
    if (NULL == string_array)
    {
        return;
    }
    if (string_array->internal_buf)
    {
        free(string_array->internal_buf);
    }
    if (string_array->strings)
    {
        free(string_array->strings);
    }
}


int g_snapshot_id = 1;
extern char* gets(char* s);


void space_to_repeat(char* data)
{
    int new_len = strlen(data) - 1;

    char* p = data;
    char* q;
    while (new_len >= 0 && p[new_len] == ' ')p[new_len--] = '\0';
    while (*p != '\0' && *p == ' ') { p = p + 1; new_len--; }

    if (p != data)strcpy(data, p);

    p = strchr(p, ' ');
    while (p != NULL) {
        q = p + 1;
        if (*q == ' ') {
            while (*q != '\0' && *q == ' ')q++;
            strcpy(p + 1, q);
        }
        p = strchr(q, ' ');
    }

}

int cmd_get_str_from_file(char* cmd, int* cmd_len, FILE* fp)
{
    char* ret = NULL;
    int str_len = 0;

    if (feof(fp)) {
        strcpy(cmd, "BYE");
        *cmd_len = 3;
        return RET_YES;
    }

    ret = fgets(cmd, MAX_LINE, fp);
    if (ret == NULL) {
        printf("ERROR: fail to read file\n");
        strcpy(cmd, "BYE");
        *cmd_len = 3;
        return RET_YES;
    }

    str_len = strlen(cmd);
    while (cmd[str_len - 1] == '\r' || cmd[str_len - 1] == '\n') {
        cmd[str_len - 1] = '\0';
        str_len--;
    }
    space_to_repeat(cmd);
    *cmd_len = strlen(cmd);
    if (*cmd_len < 3 && *cmd_len >0) {
        printf("Invalid command, enter the HELP command to view the help.\n");
        return RET_NO;
    }
    return RET_YES;

}

int cmd_get_str_from_stdin(char* cmd, int* cmd_len)
{
    printf("> ");
    cmd[0] = 0;
    gets(cmd);
    space_to_repeat(cmd);
    *cmd_len = strlen(cmd);
    if (*cmd_len < 3 && *cmd_len >0) {
        printf("Invalid command, enter the HELP command to view the help.\n");
        return RET_NO;
    }

    return RET_YES;
}

int cmd_get_id(string_segmentation_s* string_array)
{
    int i;
    int cmd_len;

    for (i = 0; i < CMD_NUM; i++)
    {
        if (strcmp(string_array->strings[0], "LIST") == 0 && strcmp(string_array->strings[1], "ENTRIES") == 0)return 3;
        if (strcmp(string_array->strings[0], "LIST") == 0 && strcmp(string_array->strings[1], "SNAPSHOTS") == 0)return 4;
        if (strcmp(string_array->strings[0], "list") == 0 && strcmp(string_array->strings[1], "entries") == 0)return 3;
        if (strcmp(string_array->strings[0], "list") == 0 && strcmp(string_array->strings[1], "keys") == 0)return 2;
        if (strcmp(string_array->strings[0], "PURGE") == 0) return 6;
        if (strcmp(string_array->strings[0], "set") == 0) return 8;
        if (strcmp(string_array->strings[0], "APPEND") == 0) return 10;
        if (strcmp(string_array->strings[0], "PICK") == 0) return 11;
        if (strcmp(string_array->strings[0], "PLUCK") == 0) return 12;
        if (strcmp(string_array->strings[0], "POP") == 0) return 13;
        if (strcmp(string_array->strings[0], "DROP") == 0) return 14;
        if (strcmp(string_array->strings[0], "SNAPSHOT") == 0) return 17;
        if (strcmp(string_array->strings[0], "CHECKOUT") == 0) return 16;
        if (strcmp(string_array->strings[0], "ROLLBACK") == 0) return 15;
        if (strcmp(string_array->strings[0], "get") == 0) return 5;
        if (strcmp(string_array->strings[0], "bye") == 0) return 0;
        if (strcmp(string_array->strings[0], "FORWARD") == 0) return 25;
        if (strcmp(string_array->strings[0], "BACKWARD") == 0) return 26;

        cmd_len = strlen(string_array->strings[0]);
        if (memcmp(g_cmd_str[i], string_array->strings[0], cmd_len) == 0) {
            if (memcmp("LIST", string_array->strings[0], 4) == 0) {
                cmd_len = strlen(string_array->strings[1]);
                if (memcmp(g_cmd_str[i] + 5, string_array->strings[1], cmd_len) != 0) {
                    continue;
                }
            }
            return i;
        }
    }
    return i;
}

void cmd_help()
{
    int i;
    for (i = 0; i < CMD_NUM; i++)
        printf("%s\n", g_cmd_help[i]);
}
void cmd_list_keys(entry* entry_h)
{
    entry* pnode;
    if (NULL == entry_h || entry_h == entry_h->next) {
        printf("no keys\n");
        return;
    }
    pnode = entry_h->next;
    while (pnode != entry_h)
    {
        printf("%s\n", pnode->key);
        pnode = pnode->next;
    }

    return;
}
int cmd_maxx(entry* entry_h, char* key)
{
    int i;
    int max = 0;
    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        return 0;
    }

    if (p->length > 0) {

        for (i = 0; i < (int)p->length; i++) {
            if (!p->values[i].type) {
                if (max < p->values[i].value)max = p->values[i].value;
            }
            else
                if (max < p->values[i].entry->maxx) max = p->values[i].entry->maxx;
        }
        return max;
    }
    return 0;
}
int cmd_minn(entry* entry_h, char* key)
{
    int i;
    int min = 10007;
    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        return 0;
    }

    if (p->length > 0) {

        for (i = 0; i < (int)p->length; i++) {
            if (!p->values[i].type) {
                if (min > p->values[i].value)min = p->values[i].value;
            }
            else
                if (min > p->values[i].entry->minn) min = p->values[i].entry->minn;
        }
        return min;
    }
    return 0;
}
int cmd_summ(entry* entry_h, char* key)
{
    int i = 0;
    int sum = 0;
    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        return 0;
    }

    for (i = 0; i < (int)p->length; i++) {
        if (!p->values[i].type) {
            sum = sum + p->values[i].value;
        }
        else sum += p->values[i].entry->summ;
    }

    return sum;
}

int cmd_lenn(entry* entry_h, char* key)
{
    int i = 0;
    int ans = 0;
    entry* p = entry_find_node(entry_h, key);
    if (p == NULL) {
        return 0;
    }
    for (i = 0; i < (int)p->length; i++) {
        if (!p->values[i].type) {
            ans = ans + 1;
        }
        else ans += p->values[i].entry->lenn;
    }
    return ans;
}
void padd(entry* entry_h, char* key, entry* p)
{
    p->maxx = cmd_maxx(entry_h, key);
    p->minn = cmd_minn(entry_h, key);
    p->summ = cmd_summ(entry_h, key);
    p->lenn = cmd_lenn(entry_h, key);
}

void cmd_list_entries(entry* entry_h)
{
    int i = 0;
    entry* pnode = NULL;
    if (NULL == entry_h || entry_h == entry_h->next) {
        printf("no entries\n");
        return;
    }
    pnode = entry_h->next;
    while (pnode != entry_h)
    {
        printf("%s [", pnode->key);

        for (i = 0; i < (int)pnode->length; i++) {
            if (i != 0)printf(" ");
            if (!pnode->values[i].type)
                printf("%d", pnode->values[i].value);
            else
                printf("%s", pnode->values[i].entry->key);
        }
        printf("]\n");
        pnode = pnode->next;
    }

    return;
}

void cmd_list_snapshots(snapshot* snapshot_h)
{
    snapshot* pnode;
    if (NULL == snapshot_h || snapshot_h == snapshot_h->next) {
        printf("no snapshots\n");
        return;
    }
    pnode = snapshot_h->next;
    while (pnode != snapshot_h)
    {
        printf("%d\n", pnode->id);
        pnode = pnode->next;
    }

    return;
}

void cmd_get(entry* entry_h, char* key)
{


    int i = 0;
    entry* p = NULL;
    p = entry_find_node(entry_h, key);
    if (p == NULL) {
        printf("no such key\n");
        return;
    }

    printf("[");
    for (i = 0; i < (int)p->length; i++) {
        if (i != 0)printf(" ");
        if (!p->values[i].type)
            printf("%d", p->values[i].value);
        else
            printf("%s", p->values[i].entry->key);
    }
    printf("]\n");
    return;
}

void cmd_del(entry* entry_h, char* key, int isprintf)
{

    entry* p = entry_find_node(entry_h, key);
    if (p == NULL) {
        if (isprintf == RET_YES)
            printf("no such key\n");
        return;
    }
    if (p->is_del) {
        if (isprintf == RET_YES)
            printf("not permitted\n");
        return;
    }
    entry_delete_node(p);
    if (isprintf == RET_YES)
        printf("ok\n");
    return;
}

void cmd_purge(entry* entry_h, snapshot* snapshot_h, char* key)
{
    entry temp_entry_h;

    if (entry_h != NULL)
        cmd_del(entry_h, key, RET_NO);

    if (snapshot_h != NULL) {
        entry_add_head(&temp_entry_h, snapshot_h->entries);
        cmd_del(&temp_entry_h, key, RET_NO);
        snapshot_h->entries = entry_pick_off_head(&temp_entry_h);
    }

    printf("ok\n");
    return;
}

//Returns a node pointer to the newly added or modified
entry* cmd_set(entry* entry_h, char* key, char** values, int len)
{

    int i = 0;
    entry* p = NULL;


    if (key == NULL || values == NULL) {
        printf("invalid parameter\n");
        return NULL;
    }

    p = entry_find_node(entry_h, key);
    if (p == NULL) {
        p = entry_create_node(entry_h, key, values, len);//valus������԰����ַ����洢���Խ������3�����ڶ��鿴����ȷ�����ٿ�����������飬�Ҿ��ÿ��Ը�Ϊ������ǰ����
        if (p == NULL) return NULL;
        entry_insert_node(entry_h, p);//��˫��ѭ�������ĳ�˫������
        padd(entry_h, key, p);
    }
    else {
        p->length = len;
      //  free(p->values);
        p->values = NULL;
        if (len > 0) {
            p->is_simple = 0;
            p->is_del = 0;
            p->values = (element*)malloc(sizeof(element) * len);
            p->forward = (entry**)malloc(sizeof(entry*) * len);
            p->backward = (entry**)malloc(sizeof(entry*) * len);
            p->forward_size = 0;
            p->backward_size = 0;
            for (i = 0; i < len; i++) {
                if (isdigitstr(values[i]))//����ֵΪ���ʾ������
                {
                    p->values[i].value = atoi(values[i]);
                    p->values[i].type = 0;
                }
                else {
                    p->values[i].type = 1;
                    p->values[i].entry = entry_find_node(entry_h, values[i]);
                    p->values[i].entry->is_del = 1;
                    if (p->values[i].entry == NULL) {
                        printf("no such key");
                        return NULL;
                    }
                    if (strcmp(p->values[i].entry->key, key) == 0) {
                        printf("not permitted");
                        return NULL;
                    }
                    p->is_simple++;
                    p->forward[p->forward_size] = p->values[i].entry;
                    p->forward_size++;
                    p->values[i].entry->backward[p->values[i].entry->backward_size] = p;
                    p->values[i].entry->backward_size++;

                }

            }

        }
        padd(entry_h, key, p);
    }
    printf("ok\n");
    return p;
}

entry* cmd_push(entry* entry_h, char* key, char** values, int len)
{

    int i;
    entry* p = NULL;


    if (key == NULL || values == NULL) {
        printf("invalid parameter\n");
        return NULL;
    }

    p = entry_find_node(entry_h, key);//�ؼ�����
    if (p == NULL) {
        printf("no such key\n");
        return NULL;

    }
    else {
        //for (int i = 0; i < values; i++)
         //       printf("%d", atoi(values[i]));

        int new_len = len + p->length;
        if (new_len > 0) {
            element* t_v = (element*)malloc(sizeof(element) * (new_len));
            p->forward = (entry**)malloc(sizeof(entry*) * len);
            p->backward = (entry**)malloc(sizeof(entry*) * len);
            p->forward_size = 0;
            p->backward_size = 0;
            int j;
            for (i = 0, j = len - 1; i < len; i++, j--) {
                if (isdigitstr(values[i]))//����ֵΪ���ʾ������
                {
                    t_v[j].value = atoi(values[i]);
                    t_v[j].type = 0;
                }
                else {
                    t_v[j].type = 1;
                    t_v[j].entry = entry_find_node(entry_h, values[i]);
                    t_v[j].entry->is_del = 1;
                    if (t_v[j].entry == NULL) {
                        printf("no such key");
                        return NULL;
                    }
                    if (strcmp(t_v[j].entry->key, key) == 0) {
                        printf("not permitted");
                        return NULL;
                    }
                    p->forward[p->forward_size] = t_v[j].entry;
                    p->forward_size++;
                    p->is_simple++;
                    t_v[j].entry->backward[t_v[j].entry->backward_size] = p;
                    t_v[j].entry->backward_size++;
                }

            }
            // t_v[j] = atoi(values[i]);//�����������ж�values[i]�ǲ��Ǹ���ֵ�����ǵĻ���ȥ�ؼ�����ȥ�����ǲ��ǹؼ��֣�����ж������⣬�������ȷ���ӵ���key����value�ͺ���
            memcpy(t_v + len, p->values, sizeof(element) * p->length);

            p->length = new_len;
        //    free(p->values);
            p->values = t_v;//1
        }
        else {
            p->values = NULL;
        }


        padd(entry_h, key, p);
    }
    printf("ok\n");
    return p;
}

entry* cmd_append(entry* entry_h, char* key, char** values, int len)
{

    int i;
    entry* p = NULL;


    if (key == NULL || values == NULL) {
        printf("invalid parameter\n");
        return NULL;
    }

    p = entry_find_node(entry_h, key);
    if (p == NULL) {
        p = entry_create_node(entry_h, key, values, len);
        entry_insert_node(entry_h, p);
        padd(entry_h, key, p);
    }
    else {
        int new_len = len + p->length;
        if (new_len > 0) {
            element* t_v = (element*)malloc(sizeof(element) * (new_len));
            p->forward = (entry**)malloc(sizeof(entry*) * len);
            p->backward = (entry**)malloc(sizeof(entry*) * len);
            p->forward_size = 0;
            p->backward_size = 0;
            memcpy(t_v, p->values, sizeof(element) * p->length);
            for (i = p->length; i < new_len; i++) {
                if (isdigitstr(values[i - p->length]))//����ֵΪ���ʾ������
                {
                    t_v[i].value = atoi(values[i - p->length]);
                    t_v[i].type = 0;
                }
                else {
                    t_v[i].type = 1;
                    t_v[i].entry = entry_find_node(entry_h, values[i - p->length]);
                    t_v[i].entry->is_del = 1;
                    if (t_v[i].entry == NULL) {
                        printf("no such key");
                        return NULL;
                    }
                    if (strcmp(t_v[i].entry->key, key) == 0) {
                        printf("not permitted");
                        return NULL;
                    }
                    p->is_simple++;
                    p->forward[p->forward_size] = t_v[i].entry;
                    p->forward_size++;
                    t_v[i].entry->backward[t_v[i].entry->backward_size] = p;
                    t_v[i].entry->backward_size++;
                }

            }


            p->length = new_len;
     //       free(p->values);
            p->values = t_v;
        }
        else {
            p->values = NULL;
        }
        padd(entry_h, key, p);
    }
    printf("ok\n");
    return p;
}


entry* cmd_pick(entry* entry_h, char* key, char* values, int len)
{

    int i;
    entry* p = NULL;


    if (key == NULL || values == NULL) {
        printf("Invalid parameter, enter the HELP command to view the help.\n");
        return NULL;
    }

    if (len != 1) {
        printf("Invalid parameter, enter the HELP command to view the help.\n");
        return NULL;
    }

    p = entry_find_node(entry_h, key);
    if (p == NULL) {
        printf("no such key\n");
    }
    else {
        i = atoi(values);
        if (i<1 || i>(int)p->length)
            printf("index out of range\n");
        else
        {
            if (p->is_simple != 0)
            {
                printf("%s\n", p->values[i - 1].entry->key);//3
                return p;
            }
            else {
                printf("%d\n", p->values[i - 1].value);//3
                return p;
            }
        }
    }
    return p;
}

entry* cmd_pluck(entry* entry_h, char* key, char* values, int len)
{

    int i;
    entry* p = NULL;

    if (key == NULL || values == NULL) {
        printf("Invalid parameter, enter the HELP command to view the help.\n");
        return NULL;
    }

    if (len != 1) {
        printf("Invalid parameter, enter the HELP command to view the help.\n");
        return NULL;
    }

    p = entry_find_node(entry_h, key);
    if (p == NULL) {
        printf("no such key\n");
    }
    else {
        i = atoi(values);
        if (i<1 || i>(int)p->length)
            printf("index out of range\n");
        else {
            int newlen = p->length - 1;
            element* newvalue = (element*)malloc(newlen * sizeof(element));
            int new_len=0;
            for (int j = 0;j < (p->length);++j) {
                if (j != (i-1)) {
                    newvalue[new_len] = p->values[j];
                    new_len++;
                }
                else {
                    if (p->values[j].type != 0)
                    {
                        printf("%s\n", p->values[j].entry->key);
                    }
                    else {
                        printf("%d\n", p->values[j].value);
                    }
                }
            }
            p->values = newvalue;
            p->length--;
           
        }
    }
    return p;
}
entry* cmd_pop(entry* entry_h, char* key)
{

    entry* p = NULL;


    if (key == NULL) {
        printf("Invalid parameter, enter the HELP command to view the help.\n");
        return NULL;
    }

    p = entry_find_node(entry_h, key);
    if (p == NULL) {
        printf("no such key\n");
    }
    else {
        if (p->length > 0) {
            if (!p->values[0].type)
                printf("%d\n", p->values[0].value);
            else {
                printf("%s\n", p->values[0].entry->key);
                p->is_simple--;
            }

            p->length--;
            p->values++;
            // memcpy(p->values, p->values + 1, sizeof(int) * p->length);
        }
        else {
            printf("nil\n");
        }
    }
    return p;
}

snapshot* cmd_drop(snapshot* snapshot_h, int id)
{
    snapshot* p = NULL;

    p = snapshot_find_node(snapshot_h, id);
    if (p == NULL) {
        printf("no such snapshot\n");
    }
    else {
        snapshot_delete_node(p);
        printf("ok\n");
    }
    return p;

}
snapshot* cmd_rollback(snapshot* snapshot_h, int id)
{
    snapshot* p = NULL;

    p = snapshot_find_node(snapshot_h, id);
    if (p == NULL) {
        printf("no such snapshot\n");
    }
    else {
        snapshot_delete_more_than_ID(snapshot_h, id);
        printf("ok\n");
    }
    return p;
}

snapshot* cmd_checkout(snapshot* snapshot_h, int id)
{
    snapshot* p = NULL;
    entry temp_head;
    p = snapshot_find_node(snapshot_h, id);
    cur_snapshot_h = p;
    if (p == NULL) {
        printf("no such snapshot\n");
    }
    else {
        entry_delete_list(&cur_entry_h);

        entry_head_init(&temp_head);
        entry_add_head(&temp_head, p->entries);

        entry_copy(&cur_entry_h, &temp_head);

        entry_pick_off_head(&temp_head);
        printf("ok\n");
    }
    return p;
}


snapshot* cmd_snapshot(snapshot* snapshot_h)
{
    snapshot* p = NULL;
    entry temp_head;


    p = snapshot_create_node(g_snapshot_id);
    cur_snapshot_h = p;
    if (p == NULL) {
        printf("no such snapshot\n");
    }
    else {
        g_snapshot_id++;

        entry_head_init(&temp_head);
        entry_copy(&temp_head, &cur_entry_h);
        p->entries = entry_pick_off_head(&temp_head);

        snapshot_insert_node(snapshot_h, p);
        printf("saved as snapshot %d\n", p->id);

    }
    return p;
}




void cmd_min(entry* entry_h, char* key)
{
    int i;
    int min;
    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        printf("no such key\n");
        return;
    }
    if (p->length > 0) {
        if (p->is_simple != 0) {
            printf("%d\n", p->minn);
            return;
        }
        else {
            min = p->values[0].value;//6
            for (i = 1; i < (int)p->length; i++) {
                if (min > p->values[i].value)min = p->values[i].value;//7
            }
            printf("%d\n", min);
            return;
        }
    }
    else {
        printf("key values number is 0\n");
    }

    return;
}

void cmd_max(entry* entry_h, char* key)
{
    int i;
    int max;
    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        printf("no such key\n");
        return;
    }

    if (p->length > 0) {
        if (p->is_simple != 0) {
            printf("%d\n", p->maxx);
            return;
        }
        else {
            max = p->values[0].value;
            for (i = 1; i < (int)p->length; i++) {
                if (max < p->values[i].value)max = p->values[i].value;//8
            }
            printf("%d\n", max);
            return;
        }

    }
    else {
        printf("key values number is 0\n");
    }

    return;
}




void cmd_sum(entry* entry_h, char* key)
{
    int i = 0;
    int sum = 0;
    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        printf("no such key\n");
        return;
    }
    if (p->length > 0) {
        if (p->is_simple != 0) {
            printf("%d\n", p->summ);
            return;
        }
        else {
            for (i = 0; i < (int)p->length; i++) {
                sum = sum + p->values[i].value;//9
            }
            printf("%d\n", sum);
            return;
        }
    }
    return;
}


void cmd_len(entry* entry_h, char* key)
{


    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        printf("no such key\n");
        return;
    }

    printf("%d\n", p->lenn);
    return;
}

//only simple
void cmd_rev(entry* entry_h, char* key)
{
    int i;
    int len;
    entry* p = entry_find_node(entry_h, key);
    int t;

    if (p == NULL) {
        printf("no such key\n");
        return;
    }
    if (p->is_simple != 0) {
        printf("the type is general.");
        return;
    }
    else {
        len = p->length;
        for (i = 0; i < len / 2; i++) {
            t = p->values[i].value;//10
            p->values[i].value = p->values[len - i - 1].value;//11
            p->values[len - i - 1].value = t;//12
        }
        printf("ok\n");
    }

    return;
}

//only simple
int original_order_to_repeat(element* data, int len)
{
    int i;

    int t_len = 0;
    for (i = 1; i < len; i++) {
        if (data[i].value != data[i - 1].value) {
            data[t_len] = data[i - 1];
            t_len++;
        }
    }
    if (t_len == 0) {
        data[t_len] = data[i - 1];
        t_len++;
    }
    if (data[len - 1].value != data[t_len - 1].value || t_len == 0) {
        data[t_len] = data[i - 1];
        t_len++;
    }
    return t_len;

}

//only simple
void cmd_uniq(entry* entry_h, char* key)
{

    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        printf("no such key\n");
        return;
    }
    if (p->is_simple != 0) {
        printf("The type is general.\n");
        return;
    }

    else {
        p->length = original_order_to_repeat(p->values, p->length);//13   
        printf("ok\n");
    }
    return;
}

//only simple
int cmp_asc(const void* a, const void* b)
{
    int val1 = ((struct element*)a)->value;
    int val2 = ((struct element*)b)->value;
    return val1 > val2;
    //return *(int*)a > *(int*)b;
}

//only simple
void cmd_sort(entry* entry_h, char* key)
{

    entry* p = entry_find_node(entry_h, key);

    if (p == NULL) {
        printf("no such key\n");
        return;
    }
    if (p->is_simple == 0) {

        qsort(p->values, p->length, sizeof(struct element), cmp_asc);
        printf("ok\n");
    }
    else {
        printf("the type is general\n");
        return;
    }
    return;
}




int qsort_to_repeat(int* data, int len)
{
    int new_len = 0;
    int i;

    qsort((void*)data, len, sizeof(int), cmp_asc);


    for (i = 1; i < len; i++) {
        if (data[new_len] != data[i]) {
            new_len++;
            data[new_len] = data[i];
        }
    }
    return new_len + 1;
}





void cmd_type(entry* entry_h, char* key)
{
    entry* p = NULL;
    p = entry_find_node(entry_h, key);
    if (p == NULL) {
        printf("no such key\n");
        return;
    }
    if (p->is_simple != 0) {
        printf("general\n");
        return;
    }

    printf("simple\n");
    return;
}

void sort2(char** arr, int x) {
    int i, j, k;
    char* temp;

    for (i = 0; i < x; i++) {
        k = i;
        for (j = i + 1; j < x; j++) {
            if (strcmp(arr[k], arr[j]) > 0) {
                k = j;
            }
        }
        if (k != i) {
            temp = arr[k];
            arr[k] = arr[i];
            arr[i] = temp;
        }
    }
    return;
}
void cmd_forward(entry* entry_h, char* key)
{
    entry* pnode;
    if (NULL == entry_h || entry_h == entry_h->next) {
        printf("no keys\n");
        return;
    }
    pnode = entry_find_node(entry_h, key);
    if (NULL == pnode) {
        printf("no keys\n");
        return;
    }
    if (pnode->forward_size == 0) {
        printf("nil\n");
        return;
    }
    //   malloc(sizeof)//
    char** str = NULL; int x = 0;
    str = (char**)malloc(sizeof(char*) * 10);
    for (int i = 0; i < pnode->forward_size; ++i) {
        entry* p;
        p = pnode->forward[i];
        while (p != entry_h && p != pnode)
        {
            //printf("%s\n", pnode->key);
            str[x] = malloc(sizeof(char) * 10);
            strcpy(str[x], p->key);
            x++;
            p = p->next;
        }
    }
    sort2(str, x);
    for (int i = 0; i < x - 1; ++i) {
        printf("%s, ", str[i]);
    }
    printf("%s\n", str[x - 1]);
    free(str);
    return;
}

void cmd_backward(entry* entry_h, char* key)
{
    entry* pnode;
    if (NULL == entry_h || entry_h == entry_h->next) {
        printf("no keys\n");
        return;
    }
    pnode = entry_find_node(entry_h, key);
    if (NULL == pnode) {
        printf("no keys\n");
        return;
    }
    if (pnode->backward_size == 0) {
        printf("nil\n");
        return;
    }
    //   malloc(sizeof)//
    char** str = NULL; int x = 0;
    str = (char**)malloc(sizeof(char*) * 10);
    for (int i = 0; i < pnode->backward_size; ++i) {
        entry* p;
        p = pnode->backward[i];
        while (p != entry_h && p != pnode)
        {
            //printf("%s\n", pnode->key);
            str[x] = malloc(sizeof(char) * 10);
            strcpy(str[x], p->key);
            x++;
            p = p->prev;
        }
    }
    sort2(str, x);
    for (int i = 0; i < x - 1; ++i) {
        printf("%s, ", str[i]);
    }
    printf("%s\n", str[x - 1]);
    free(str);
    return;
}


void cmd_dispatch(int cmd_id, string_segmentation_s* string_array)
{
    switch (cmd_id) {
        //CMD_BYE:
    case CMD_HELP:
        if (string_array->len != 1) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_help();
        break;
    case CMD_LIST_KEYS:
        cmd_list_keys(&cur_entry_h);
        break;
    case CMD_LIST_ENTRIES:
        cmd_list_entries(&cur_entry_h);
        break;
    case CMD_LIST_SNAPSHOTS:
        cmd_list_snapshots(&g_snapshot_h);
        break;
    case CMD_GET:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_get(&cur_entry_h, string_array->strings[1]);
        break;
    case CMD_DEL:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_del(&cur_entry_h, string_array->strings[1], RET_YES);
        break;
    case CMD_PURGE:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_purge(&cur_entry_h, cur_snapshot_h, string_array->strings[1]);
        break;
    case CMD_SET:
        if (string_array->len < 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_set(&cur_entry_h, string_array->strings[1], string_array->strings + 2, string_array->len - 2);
        break;
    case CMD_PUSH:
        if (string_array->len <= 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_push(&cur_entry_h, string_array->strings[1], string_array->strings + 2, string_array->len - 2);
        break;
    case CMD_APPEND:
        if (string_array->len <= 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_append(&cur_entry_h, string_array->strings[1], string_array->strings + 2, string_array->len - 2);
        break;
    case CMD_PICK:
        if (string_array->len != 3) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_pick(&cur_entry_h, string_array->strings[1], string_array->strings[2], string_array->len - 2);
        break;
    case CMD_PLUCK:
        if (string_array->len != 3) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_pluck(&cur_entry_h, string_array->strings[1], string_array->strings[2], string_array->len - 2);
        break;
    case CMD_POP:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_pop(&cur_entry_h, string_array->strings[1]);
        break;
    case CMD_DROP:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_drop(&g_snapshot_h, atoi(string_array->strings[1]));
        break;
    case CMD_ROLLBACK:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_rollback(&g_snapshot_h, atoi(string_array->strings[1]));
        break;
    case CMD_CHECKOUT:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_checkout(&g_snapshot_h, atoi(string_array->strings[1]));
        break;
    case CMD_SNAPSHOT:
        if (string_array->len != 1) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_snapshot(&g_snapshot_h);
        break;
    case CMD_MIN:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_min(&cur_entry_h, string_array->strings[1]);
        break;
    case CMD_MAX:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_max(&cur_entry_h, string_array->strings[1]);
        break;
    case CMD_SUM:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_sum(&cur_entry_h, string_array->strings[1]);
        break;
    case CMD_LEN:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_len(&cur_entry_h, string_array->strings[1]);
        break;
    case CMD_REV:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_rev(&cur_entry_h, string_array->strings[1]);
        break;
    case CMD_UNIQ:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_uniq(&cur_entry_h, string_array->strings[1]);
        break;
    case CMD_SORT:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_sort(&cur_entry_h, string_array->strings[1]);
        break;

    case CMD_FORWARD:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_forward(&cur_entry_h, string_array->strings[1]);
        break;

    case CMD_BACKWARD:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_backward(&cur_entry_h, string_array->strings[1]);
        break;

    case CMD_TYPE:
        if (string_array->len != 2) {
            printf("Invalid parameter, enter the HELP command to view the help.\n");
            break;
        }
        cmd_type(&cur_entry_h, string_array->strings[1]);
        break;


    default:
        break;
        //CMD_NUM
    };

    return;

}










void run_environment_init()
{
    entry_head_init(&cur_entry_h);
    snapshot_head_init(&g_snapshot_h);
    cur_snapshot_h = NULL;
}


void run_environment_destroy()
{
    entry_delete_list(&cur_entry_h);
    snapshot_delete_list(&g_snapshot_h);
    return;
}


/*
int cmp_asc(const void *a, const void *b)
{
    return *(int *)a>*(int *)b;
}
void qsort_to_repeat(int *data, int *len)
{
    int new_len = 0;
    int i,j;

    qsort(data, *len, sizeof(int), cmp_asc);

    for(i=1;i<*len;i++){
        if(data[new_len]!=data[i]){
            new_len++;
            data[new_len]=data[i];
        }
    }
    *len = new_len+1;
}*/

int main(int args, char** argv)
{
    char get_cmd[MAX_LINE];
    int get_cmd_len = 0;
    int get_cmd_id;
    int ret;
    string_segmentation_s string_array;
    FILE* fp = NULL;



    /*
        int data[10]={
            1,2,3,4,5,2,7,8,9,20
        };
        printf("%d\n", qsort_to_repeat(data, 10));
        return 0;*/

    run_environment_init();
    if (args > 1) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            printf("ERROR: fail to read file [%s].\n", argv[1]);
            return 0;
        }
    }


    while (1) {
        if (args > 1) {
            ret = cmd_get_str_from_file(get_cmd, &get_cmd_len, fp);
            printf(">%s\n", get_cmd);
        }
        else {
            ret = cmd_get_str_from_stdin(get_cmd, &get_cmd_len);
        }
        if (get_cmd[0] == '\0')continue;
        if (ret == RET_YES) {
            string_segmentation(' ', get_cmd, 0, &string_array);
            get_cmd_id = cmd_get_id(&string_array);
            if (get_cmd_id >= CMD_NUM) {
                printf("Invalid command, enter the HELP command to view the help.\n\n");
                string_segmentation_free(&string_array);
                continue;
            }
            if (get_cmd_id == CMD_BYE) {
                run_environment_destroy();
                printf("bye\n");
                string_segmentation_free(&string_array);
                //cmd_bye();
                break;
            }
            cmd_dispatch(get_cmd_id, &string_array);
            string_segmentation_free(&string_array);
        }
        printf("\n");
    }

    if (fp != NULL)fclose(fp);
    return 0;
}
