#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"

#define LOG_PREFIX "[PEX]"
#define MAX_INFO 100
#define MAX_PIPE_NAME 60
#define MAX_PRODUCT_NAME 20
#define TYPE_BUY 1
#define TYPE_SELL 2
#define TRADER_CLOSE 1
#define TRADER_RUN 2

typedef struct Node {
    int orderId;
    int orderType;
    int traderId;
    int orderBookId;
    int qty;
    int price;
    // double linked list
    struct Node* nextNode;
    struct Node* prevNode;
} Node;

typedef struct {
    char productName[MAX_PRODUCT_NAME];
    // store the orders in a double linked list
    Node* ordersHead;
    Node* ordersTail;
} OrderBook;

typedef struct {
    int type;
    int traderPid;
    int orderLen;
    long long* expense;
    int* qty;
    int exchangeFifo;
    int traderFifo;
} Traders;

Node* CreateNode(int orderId, int orderType, int traderId, int orderBookId, int qty, int price);
void DestroyNode(Node* node);
void InsertNode(Node* node);
int RemoveNode(int orderId, int traderId, int* orderBookId);
void MatchNode(Node* node);
void DisplayOrderbook(int orderBookId);
void DisplayPosition(int traderId);
void Display();
int StartWith(char* str, char* prefix);
int MarketOperation(char* traderInfo, int traderId);
int BuyOperation(char* traderInfo, int traderId);
int SellOperation(char* traderInfo, int traderId);
int AmendOperation(char* traderInfo, int traderId);
int CancelOperation(char* traderInfo, int traderId);
int GetOrderBookId(char* productName);
void SendInfo(char* info, int traderId);
void Sigusr1Handler(int signo, siginfo_t* sigInfo, void* voidPtr);
void SigchldHandler(int signo, siginfo_t* sigInfo, void* voidPtr);

#endif
