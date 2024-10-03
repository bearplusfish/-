#ifndef SPX_EXCHANGE_H
#define SPX_EXCHANGE_H

#include "spx_common.h"

#define LOG_PREFIX "[SPX]"


struct POSITION
{
    int quantity;
    long long value;

};


struct ORDER_BOOK
{
    unsigned long long id;
    int level;
    int quantity;
    int length;
    struct ORDER * orderList;
    char orderName[ORDER_NAME_LENGTH + 1];
    struct ORDER_BOOK *next;

};


struct PRODUCT
{
    int productId;
    int buyCount;
    int sellCount;
    char name[PRODUCT_NAME_LENGTH + 1];
};


struct MATCH {
    int traderId1;
    int orderId1;
    int traderId2;
    int orderId2;
    int quantity;
    long long value;
    long long fee;
    struct MATCH *next;
};

void *allocateMemory(size_t size);

char *strdup(const char *orgString);

struct SIGNAL *newSignal(int signal, int pid, int time, const char *content, int contentLength);

void deleteSignal(struct SIGNAL *signal);

void appendSignal(struct SIGNAL **pHead, struct SIGNAL *signal);

void removeSignal(struct SIGNAL **pHead, struct SIGNAL *signal);

struct TRADER *getTraderByPid(int pid);

void processMultipleCommandRequest(int trader_id,
    const char *content,
    int contentLength,
    int pid,
    int time);


#endif


