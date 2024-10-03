#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ymirdb.h"
#include "ymirdb_cmd.h"
#include "ymirdb_list.h"



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
extern void entry_head_init(entry* head)
{
    head->key[0] = '\0';
    head->next = head;
    head->prev = head;
}
int isdigitstr(char* str)
{
    return (strspn(str, "0123456789") == strlen(str));
}
extern entry* entry_create_node(entry* entry_h, char* key, char** values, int len)
{
    int i;
    entry* p = (entry*)malloc(sizeof(entry));
    strcpy(p->key, key);
    p->length = len;
    if (len > 0) {
        p->is_simple = 0;
        (element*)p->values = (element*)malloc(sizeof(element) * len);
        (entry*)p->forward = (entry*)malloc(sizeof(entry) * len);
        (entry*)p->backward = (entry*)malloc(sizeof(entry) * len);
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
                if (p->values[i].entry == NULL) {
                    printf("no such key");
                    return NULL;
                }
                if (strcmp(p->values[i].entry->key, key) == 0) {
                    printf("not permitted");
                    return NULL;
                }
                p->is_simple = 1;
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
    if (pnode->values)
        free(pnode->values);
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
        if (ptmp->values)
            free(ptmp->values);
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




