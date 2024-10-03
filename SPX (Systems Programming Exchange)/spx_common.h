#ifndef SPX_COMMON_H
#define SPX_COMMON_H

#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200809L


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>


#define FIFO_EXCHANGE "/tmp/spx_exchange_%d"
#define FIFO_TRADER "/tmp/spx_trader_%d"
#define FEE_PERCENTAGE 1
#define ORDER_NAME_LENGTH   4
#define BUFFER_SIZE         1024
#define PRODUCT_NAME_LENGTH 16




struct SIGNAL
{
    int signal;
    int pid;
    int timeStamp;
    char *content;
    int contentLength;
    struct SIGNAL *next;
};

struct ORDER
{
    int orderId;
    int traderId;
    int quantity;
    int price;
    unsigned long long time;
    char productName[PRODUCT_NAME_LENGTH + 1];
    char orderName[ORDER_NAME_LENGTH + 1];
    struct ORDER *nextOrderInOrderBook;
    struct ORDER *nextOrderInOrderList;

};


struct TRADER
{
    int traderId;
    int pid;
    int exchangePipe;
    int traderPipe;
    int isDisconnect;

};

#endif
