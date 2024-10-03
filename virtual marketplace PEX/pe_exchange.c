/**
 * comp2017 - assignment 3
 * <your name>Jiayu Xiong
 * <your unikey>jxio5417
 */

#include "pe_exchange.h"

long long sumFee;
OrderBook* orderBook;
int orderBookLen;
Traders* peTrader;
int traderLen;

Node* CreateNode(int orderId, int orderType, int traderId, int orderBookId, int qty, int price) {
    Node* node = malloc(sizeof(Node));
    node->orderId = orderId;
    node->orderType = orderType;
    node->traderId = traderId;
    node->orderBookId = orderBookId;
    node->qty = qty;
    node->price = price;
    node->nextNode = NULL;
    node->prevNode = NULL;
    return node;
}
void DestroyNode(Node* node) {
    // free all nodes in the list
    Node *p = node;
    while (p != NULL) {
        Node *q = p->nextNode;
        free(p);
        p = q;
    }
}
void InsertNode(Node* node) {
    // higher price is in the front of the list
    Node *p = orderBook[node->orderBookId].ordersHead;
    if (p == NULL) {
        orderBook[node->orderBookId].ordersHead = node;
        orderBook[node->orderBookId].ordersTail = node;
        return;
    }
    while (p != NULL) {
        // if the price is the same, insert the node at the last of the same price
        // if the node->price is higher, insert the node before node p
        // the node will be inserted by the price from high to low
        // if same price, buy order insert to the back, sell order insert to the front
        if ((p->price < node->price && node->orderType == TYPE_BUY) || (p->price <= node->price && node->orderType == TYPE_SELL)) {
            if (p->prevNode == NULL) {
                orderBook[node->orderBookId].ordersHead = node;
            } else {
                p->prevNode->nextNode = node;
            }
            node->prevNode = p->prevNode;
            node->nextNode = p;
            p->prevNode = node;
            return;
        }
        p = p->nextNode;
    }
    // not find a price is smaller than the node->price, insert the node at the last
    orderBook[node->orderBookId].ordersTail->nextNode = node;
    node->prevNode = orderBook[node->orderBookId].ordersTail;
    orderBook[node->orderBookId].ordersTail = node;
}
int RemoveNode(int orderId, int traderId, int* orderBookId) {
    *orderBookId = -1;
    for (int i = 0; i < orderBookLen; i++) {
        Node *p = orderBook[i].ordersHead;
        while (p) {
            if (p->orderId != orderId) {
                p = p->nextNode;
            } else if(p->traderId != traderId) {
                p = p->nextNode;
            } else {
                // find the node, remove it
                if (p->nextNode != NULL) {
                    p->nextNode->prevNode = p->prevNode;
                } else {
                    orderBook[i].ordersTail = p->prevNode;
                }
                if (p->prevNode != NULL) {
                    p->prevNode->nextNode = p->nextNode;
                } else {
                    orderBook[i].ordersHead = p->nextNode;
                }
                int orderType = p->orderType;
                free(p);
                *orderBookId = i;
                // if remove success, return TYPE_BUY or TYPE_SELL
                return orderType;
            }
        }
    }
    return 0;
}
void MatchNode(Node* node) {
    Node *p = NULL;
    if (node->orderType == TYPE_SELL) {
        // compare with all the buy orders
        p = orderBook[node->orderBookId].ordersHead;
    } else if (node->orderType == TYPE_BUY) {
        // compare with all the sell orders, start from the last because we must search from the lowest price
        p = orderBook[node->orderBookId].ordersTail;
    }
    while (p != NULL) {
        if (p->orderType == node->orderType && node->orderType == TYPE_SELL) {
            p = p->nextNode;
            continue;
        } else if (p->orderType == node->orderType && node->orderType == TYPE_BUY) {
            // sell orders is search from tail to head
            p = p->prevNode;
            continue;
        }
        // if the sell price is higher than the buy price, break
        if (p->price < node->price && node->orderType == TYPE_SELL) {
            break;
        } else if (p->price > node->price && node->orderType == TYPE_BUY) {
            break;
        }
        // qty = min(p->qty, node->qty)
        int qty = p->qty < node->qty ? p->qty : node->qty;
        p->qty -= qty;
        node->qty -= qty;
        long long expense = (long long)qty * p->price;
        // rounded to the nearest
        long long fee = (double)(0.5 + expense * 0.01);
        sumFee = sumFee + fee;
        char infoFillBuy[MAX_INFO];
        char infoFillSell[MAX_INFO];
        if (node->orderType == TYPE_SELL) {
            peTrader[node->traderId].expense[node->orderBookId] = peTrader[node->traderId].expense[node->orderBookId] + expense - fee;
            peTrader[p->traderId].expense[p->orderBookId] = peTrader[p->traderId].expense[p->orderBookId] - expense;
            peTrader[node->traderId].qty[node->orderBookId] = peTrader[node->traderId].qty[node->orderBookId] - qty;
            peTrader[p->traderId].qty[p->orderBookId] = peTrader[p->traderId].qty[p->orderBookId] + qty;
            sprintf(infoFillBuy, "FILL %d %d;", p->orderId, qty);
            sprintf(infoFillSell, "FILL %d %d;", node->orderId, qty);
            // write the fill info to the fifo
            SendInfo(infoFillBuy, p->traderId);
            SendInfo(infoFillSell, node->traderId);
        } else {
            peTrader[node->traderId].expense[node->orderBookId] = peTrader[node->traderId].expense[node->orderBookId] - expense - fee;
            peTrader[p->traderId].expense[p->orderBookId] = peTrader[p->traderId].expense[p->orderBookId] + expense;
            peTrader[node->traderId].qty[node->orderBookId] = peTrader[node->traderId].qty[node->orderBookId] + qty;
            peTrader[p->traderId].qty[p->orderBookId] = peTrader[p->traderId].qty[p->orderBookId] - qty;
            sprintf(infoFillBuy, "FILL %d %d;", node->orderId, qty);
            sprintf(infoFillSell, "FILL %d %d;", p->orderId, qty);
            // write the fill info to the fifo
            SendInfo(infoFillBuy, node->traderId);
            SendInfo(infoFillSell, p->traderId);
        }
        printf(LOG_PREFIX" Match: Order %d [T%d], New Order %d [T%d], value: $%lld, fee: $%lld.\n", 
                            p->orderId, p->traderId, 
                            node->orderId, node->traderId, 
                            expense, fee);
        if (p->qty == 0) {
            // remove the order
            Node *q = NULL;
            if (node->orderType == TYPE_SELL) {
                q = p->nextNode;
            } else {
                // sell orders is search from tail to head
                q = p->prevNode;
            }
            if (p->nextNode != NULL) {
                p->nextNode->prevNode = p->prevNode;
            } else {
                orderBook[node->orderBookId].ordersTail = p->prevNode;
            }
            if (p->prevNode != NULL) {
                p->prevNode->nextNode = p->nextNode;
            } else {
                orderBook[node->orderBookId].ordersHead = p->nextNode;
            }
            free(p);
            p = q;
        }
        if (node->qty == 0) {
            // qty = 0, cannot match next order
            return;
        }
    }
}

void DisplayOrderbook(int orderBookId) {
    // print the orders of one product in the order book
    Node *p = orderBook[orderBookId].ordersHead;
    // 999999 + 1 is a invalid price, so not same with orders price
    int samePrice = 999999 + 1;
    int numQty = 0;
    int numOrder = 0;
    int buyPrice = 999999 + 1;
    int sellPrice = 999999 + 1;
    int buyLevels = 0;
    int sellLevels = 0;
    while (p != NULL) {
        if (p->orderType == TYPE_BUY) {
            // price is diffrent from the previous one, increase the buy levels
            if (p->price != buyPrice) {
                buyPrice = p->price;
                buyLevels++;
            }
        } else {
            // price is diffrent from the previous one, increase the sell levels
            if (p->price != sellPrice) {
                sellPrice = p->price;
                sellLevels++;
            }
        }
        p = p->nextNode;
    }
    printf(LOG_PREFIX"\tProduct: %s; Buy levels: %d; Sell levels: %d\n", 
                         orderBook[orderBookId].productName, 
                         buyLevels, sellLevels);
    p = orderBook[orderBookId].ordersHead;
    while (p != NULL) {
        if (p->orderType == TYPE_BUY) {
            p = p->nextNode;
            continue;
        }
        if (p->price == samePrice) {
            numQty += p->qty;
            numOrder++;
        } else {
            // price is diffrent from the previous one, print the previous one's order price, qty and number of orders
            if (numOrder == 1 && samePrice != 999999 + 1) {
                printf(LOG_PREFIX"\t\tSELL %d @ $%d (%d order)\n", numQty, samePrice, numOrder);
            } else if (samePrice != 999999 + 1) {
                printf(LOG_PREFIX"\t\tSELL %d @ $%d (%d orders)\n", numQty, samePrice, numOrder);
            }
            samePrice = p->price;
            numQty = p->qty;
            numOrder = 1;
        }
        p = p->nextNode;
    }
    // the last one is not printed
    if (numOrder == 1 && samePrice != 999999 + 1) {
        printf(LOG_PREFIX"\t\tSELL %d @ $%d (%d order)\n", numQty, samePrice, numOrder);
    } else if (samePrice != 999999 + 1) {
        printf(LOG_PREFIX"\t\tSELL %d @ $%d (%d orders)\n", numQty, samePrice, numOrder);
    }
    p = orderBook[orderBookId].ordersHead;
    samePrice = 999999 + 1;
    numQty = 0;
    numOrder = 0;
    while (p != NULL) {
        if (p->orderType == TYPE_SELL) {
            p = p->nextNode;
            continue;
        }
        if (p->price == samePrice) {
            numQty += p->qty;
            numOrder++;
        } else {
            // price is diffrent from the previous one, print the previous one's order price, qty and number of orders
            if (numOrder == 1 && samePrice != 999999 + 1) {
                printf(LOG_PREFIX"\t\tBUY %d @ $%d (%d order)\n", numQty, samePrice, numOrder);
            } else if (samePrice != 999999 + 1){
                printf(LOG_PREFIX"\t\tBUY %d @ $%d (%d orders)\n", numQty, samePrice, numOrder);
            }
            samePrice = p->price;
            numQty = p->qty;
            numOrder = 1;
        }
        p = p->nextNode;
    }
    // the last one is not printed
    if (numOrder == 1 && samePrice != 999999 + 1) {
        printf(LOG_PREFIX"\t\tBUY %d @ $%d (%d order)\n", numQty, samePrice, numOrder);
    } else if (samePrice != 999999 + 1){
        printf(LOG_PREFIX"\t\tBUY %d @ $%d (%d orders)\n", numQty, samePrice, numOrder);
    }
}

void DisplayPosition(int traderId) {
    // print trader's position information
    printf(LOG_PREFIX"\tTrader %d: ", traderId);
    for (int i = 0; i < orderBookLen - 1; i++) {
        printf("%s %d ($%lld), ", orderBook[i].productName, peTrader[traderId].qty[i], peTrader[traderId].expense[i]);
    }
    printf("%s %d ($%lld)\n", orderBook[orderBookLen - 1].productName, peTrader[traderId].qty[orderBookLen - 1], peTrader[traderId].expense[orderBookLen - 1]);
}

void Display() {
    printf(LOG_PREFIX"\t--ORDERBOOK--\n");
    // print the orders of all products in the order book
    for (int i = 0; i < orderBookLen; i++) {
        DisplayOrderbook(i);
    }
    printf(LOG_PREFIX"\t--POSITIONS--\n");
    // print the positions of all peTrader
    for (int i = 0; i < traderLen; i++) {
        DisplayPosition(i);
    }
}

void ListDebug() {
    // debug function, test the correctness of the linked list
    // test InsertNode, MatchNode and DeleteNode
    sumFee = 0;
    orderBookLen = 2;
    traderLen = 2;
    orderBook = malloc(sizeof(OrderBook) * orderBookLen);
    memset(orderBook, 0, sizeof(OrderBook) * orderBookLen);
    peTrader = malloc(sizeof(Traders) * traderLen);
    memset(peTrader, 0, sizeof(Traders) * traderLen);
    for (int i = 0; i < traderLen; i++) {
        peTrader[i].expense = malloc(sizeof(long long) * orderBookLen);
        peTrader[i].expense[0] = 0;
        peTrader[i].expense[1] = 0;
        peTrader[i].qty = malloc(sizeof(int) * orderBookLen);
        peTrader[i].qty[0] = 0;
        peTrader[i].qty[1] = 0;
        peTrader[i].type = TRADER_CLOSE;
        peTrader[i].orderLen = 0;
    }
    orderBook[0].ordersHead = NULL;
    orderBook[0].ordersTail = NULL;
    strcpy(orderBook[0].productName, "GPU");
    orderBook[1].ordersHead = NULL;
    orderBook[1].ordersTail = NULL;
    strcpy(orderBook[1].productName, "Router");
    Node* node0 = CreateNode(0, TYPE_BUY, 0, 0, 30, 500);
    InsertNode(node0);
    Display();
    printf("\n");
    Node* node1 = CreateNode(1, TYPE_BUY, 0, 0, 30, 501);
    InsertNode(node1);
    Display();
    printf("\n");
    Node* node2 = CreateNode(2, TYPE_BUY, 0, 0, 30, 501);
    InsertNode(node2);
    Display();
    printf("\n");
    Node* node3 = CreateNode(3, TYPE_BUY, 0, 0, 30, 502);
    InsertNode(node3);
    Display();
    printf("\n");
    Node* node4 = CreateNode(0, TYPE_SELL, 1, 0, 99, 511);
    InsertNode(node4);
    Display();
    printf("\n");
    Node* node5 = CreateNode(1, TYPE_SELL, 1, 0, 99, 402);
    MatchNode(node5);
    if (node5->qty > 0) {
        InsertNode(node5);
    } else {
        free(node5);
    }
    Display();
    printf("\n");
    int orderBookId = 0;
    RemoveNode(node4->orderId, node4->traderId, &orderBookId);
    Display();
    printf("\n");
    node4 = CreateNode(0, TYPE_SELL, 1, 0, 99, 511);
    InsertNode(node4);
    Display();
    printf("\n");
    printf(LOG_PREFIX" Exchange fees collected: $%lld\n", sumFee);
    DestroyNode(orderBook[0].ordersHead);
    free(orderBook);
    free(peTrader[0].expense);
    free(peTrader[0].qty);
    free(peTrader[1].expense);
    free(peTrader[1].qty);
    free(peTrader);
}

void SendInfo(char* info, int traderId) {
    // send the information to the trader
    if (peTrader[traderId].type == TRADER_RUN) {
        write(peTrader[traderId].exchangeFifo, info, strlen(info));
        kill(peTrader[traderId].traderPid, SIGUSR1);
    }
}

int StartWith(char* str, char* prefix) {
    // if str is start with prefix, return 1, else return 0
    int i = 0;
    while (str[i] != 0 && prefix[i] != 0) {
        if (str[i] != prefix[i]) {
            return 0;
        }
        i++;
    }
    if (prefix[i] != 0) {
        return 0;
    } else {
        return 1;
    }
}

int MarketOperation(char* traderInfo, int traderId) {
    printf(LOG_PREFIX" [T%d] Parsing command: <%s>\n", 
                       traderId, traderInfo);
    if (StartWith(traderInfo, "BUY") == 1) {
        return BuyOperation(traderInfo, traderId);
    } else if (StartWith(traderInfo, "SELL") == 1) {
        return SellOperation(traderInfo, traderId);
    } else if (StartWith(traderInfo, "AMEND") == 1) {
        return AmendOperation(traderInfo, traderId);
    } else if (StartWith(traderInfo, "CANCEL") == 1) {
        return CancelOperation(traderInfo, traderId);
    } else {
        // printf(LOG_PREFIX" Invalid operation\n");
        return 0;
    }
}
int BuyOperation(char* traderInfo, int traderId) {
    // parse the trader information
    int delimNum = 0;
    for (int i = 0; i < strlen(traderInfo); i++) {
        if (traderInfo[i] == ' ') {
            delimNum++;
        }
    }
    // BUY <orderId> <product> <qty> <price>
    if (delimNum != 4) {
        // printf(LOG_PREFIX" Invalid buy operation\n");
        return 0;
    }
    char delims[] = " ";
    char* ignore;
    char* productStr;
    char* qtyStr;
    char* priceStr;
    char* orderIdStr;
    int qty = 0;
    int price = 0;
    int orderId = 0;
    // ignore the first word
    ignore = strtok(traderInfo, delims);
    if (ignore == NULL) {
        // printf(LOG_PREFIX" Invalid buy operation\n");
        return 0;
    }
    // get orde r id
    orderIdStr = strtok(NULL, delims);
    orderId = atoi(orderIdStr);
    // get product name
    productStr = strtok(NULL, delims);
    // get qty
    qtyStr = strtok(NULL, delims);
    qty = atoi(qtyStr);
    // get price
    priceStr = strtok(NULL, delims);
    price = atoi(priceStr);
    // check
    if (qty < 1 || qty >= 999999 + 1 || price < 1 || price >= 999999 + 1) {
        // printf(LOG_PREFIX" Invalid amend operation\n");
        return 0;
    }
    // check order id
    if (orderId != peTrader[traderId].orderLen) {
        // printf(LOG_PREFIX" Invalid amend operation\n");
        return 0;
    }
    // get the index of the order book
    int orderBookId = GetOrderBookId(productStr);
    if (orderBookId == -1) {
        // printf(LOG_PREFIX" Invalid buy operation\n");
        return 0;
    }
    peTrader[traderId].orderLen++;
    // write to fifo and signal the trader
    char infoAccept[MAX_INFO];
    char infoMarketBuy[MAX_INFO];
    sprintf(infoAccept, "ACCEPTED %d;", orderId);
    sprintf(infoMarketBuy, "MARKET BUY %s %d %d;", orderBook[orderBookId].productName, qty, price);
    SendInfo(infoAccept, traderId);
    for (int i = 0; i < traderLen; i++) {
        if (i == traderId) {
            continue;
        }
        SendInfo(infoMarketBuy, i);
    }
    // create a new node
    Node* node = CreateNode(orderId, TYPE_BUY, traderId, orderBookId, qty, price);
    // match with the sell orders
    MatchNode(node);
    // if the qty is not 0, insert the node into the order book
    if (node->qty > 0) {
        InsertNode(node);
    } else {
        // release the memory
        free(node);
    }
    return 1;
}
int SellOperation(char* traderInfo, int traderId) {
    // parse the trader information
    int delimNum = 0;
    for (int i = 0; i < strlen(traderInfo); i++) {
        if (traderInfo[i] == ' ') {
            delimNum++;
        }
    }
    // SELL <orderId> <product> <qty> <price>
    if (delimNum != 4) {
        // printf(LOG_PREFIX" Invalid sell operation\n");
        return 0;
    }
    char delims[] = " ";
    char* ignore;
    char* productStr;
    char* qtyStr;
    char* priceStr;
    char* orderIdStr;
    int qty = 0;
    int price = 0;
    int orderId = 0;
    // ignore the first word
    ignore = strtok(traderInfo, delims);
    if (ignore == NULL) {
        // printf(LOG_PREFIX" Invalid sell operation\n");
        return 0;
    }
    // get order id
    orderIdStr = strtok(NULL, delims);
    orderId = atoi(orderIdStr);
    // get product name
    productStr = strtok(NULL, delims);
    // get qty
    qtyStr = strtok(NULL, delims);
    qty = atoi(qtyStr);
    // get price
    priceStr = strtok(NULL, delims);
    price = atoi(priceStr);
    // check
    if (qty < 1 || qty >= 999999 + 1 || price < 1 || price >= 999999 + 1) {
        // printf(LOG_PREFIX" Invalid amend operation\n");
        return 0;
    }
    // check order id
    if (orderId != peTrader[traderId].orderLen) {
        // printf(LOG_PREFIX" Invalid amend operation\n");
        return 0;
    }
    // get the index of the order book
    int orderBookId = GetOrderBookId(productStr);
    if (orderBookId == -1) {
        // printf(LOG_PREFIX" Invalid sell operation\n");
        return 0;
    }
    peTrader[traderId].orderLen++;
    // write to fifo and signal the trader
    char infoAccept[MAX_INFO];
    char infoMarketSell[MAX_INFO];
    sprintf(infoAccept, "ACCEPTED %d;", orderId);
    sprintf(infoMarketSell, "MARKET SELL %s %d %d;", orderBook[orderBookId].productName, qty, price);
    SendInfo(infoAccept, traderId);
    for (int i = 0; i < traderLen; i++) {
        if (i == traderId) {
            continue;
        }
        SendInfo(infoMarketSell, i);
    }
    // create a new node
    Node* node = CreateNode(orderId, TYPE_SELL, traderId, orderBookId, qty, price);
    // match with the buy orders
    MatchNode(node);
    // if the qty is not 0, insert the node into the order book
    if (node->qty > 0) {
        InsertNode(node);
    } else {
        // release the memory
        free(node);
    }
    return 1;
}
int AmendOperation(char* traderInfo, int traderId) {
    // parse the trader information
    int delimNum = 0;
    for (int i = 0; i < strlen(traderInfo); i++) {
        if (traderInfo[i] == ' ') {
            delimNum++;
        }
    }
    // AMEND <orderId> <qty> <price>
    if (delimNum != 3) {
        // printf(LOG_PREFIX" Invalid amend operation\n");
        return 0;
    }
    char delims[] = " ";
    char* ignore;
    char* qtyStr;
    char* priceStr;
    char* orderIdStr;
    int qty = 0;
    int price = 0;
    int orderId = 0;
    int orderBookId = 0;
    // ignore the first word
    ignore = strtok(traderInfo, delims);
    if (ignore == NULL) {
        // printf(LOG_PREFIX" Invalid amend operation\n");
        return 0;
    }
    // get order id
    orderIdStr = strtok(NULL, delims);
    orderId = atoi(orderIdStr);
    // get qty
    qtyStr = strtok(NULL, delims);
    qty = atoi(qtyStr);
    // get price
    priceStr = strtok(NULL, delims);
    price = atoi(priceStr);
    if (qty < 1 || qty >= 999999 + 1 || price < 1 || price >= 999999 + 1) {
        // printf(LOG_PREFIX" Invalid amend operation\n");
        return 0;
    }
    // remove the node
    int orderType = RemoveNode(orderId, traderId, &orderBookId);
    if (orderType == 0) {
        // printf(LOG_PREFIX" Invalid amend operation\n");
        return 0;
    }
    // write to fifo and signal the trader
    char infoAmend[MAX_INFO];
    char infoMarketAmend[MAX_INFO];
    sprintf(infoAmend, "AMENDED %d;", orderId);
    if (orderType == TYPE_BUY) {
        sprintf(infoMarketAmend, "MARKET BUY %s %d %d;", orderBook[orderBookId].productName, qty, price);
    } else {
        sprintf(infoMarketAmend, "MARKET SELL %s %d %d;", orderBook[orderBookId].productName, qty, price);
    }
    SendInfo(infoAmend, traderId);
    for (int i = 0; i < traderLen; i++) {
        if (i == traderId) {
            continue;
        }
        SendInfo(infoMarketAmend, i);
    }
    // create a new node
    Node* node = CreateNode(orderId, orderType, traderId, orderBookId, qty, price);
    // match with the orders
    MatchNode(node);
    // if the qty is not 0, insert the node into the order book
    if (node->qty > 0) {
        InsertNode(node);
    } else {
        // release the memory
        free(node);
    }
    return 1;
}
int CancelOperation(char* traderInfo, int traderId) {
    // parse the trader information
    int delimNum = 0;
    for (int i = 0; i < strlen(traderInfo); i++) {
        if (traderInfo[i] == ' ') {
            delimNum++;
        }
    }
    // CANCEL <orderId>
    if (delimNum != 1) {
        // printf(LOG_PREFIX" Invalid cancel operation\n");
        return 0;
    }
    char delims[] = " ";
    char* ignore;
    char* orderIdStr;
    int orderId = 0;
    int orderBookId = 0;
    // ignore the first word
    ignore = strtok(traderInfo, delims);
    if (ignore == NULL) {
        // printf(LOG_PREFIX" Invalid cancel operation\n");
        return 0;
    }
    // get order id
    orderIdStr = strtok(NULL, delims);
    orderId = atoi(orderIdStr);
    // remove the node
    int orderType = RemoveNode(orderId, traderId, &orderBookId);
    if (orderType == 0) {
        // printf(LOG_PREFIX" Invalid cancel operation\n");
        return 0;
    }
    // write to fifo and signal the trader
    char infoCancel[MAX_INFO];
    char infoMarketCancel[MAX_INFO];
    sprintf(infoCancel, "CANCELLED %d;", orderId);
    if (orderType == TYPE_BUY) {
        sprintf(infoMarketCancel, "MARKET BUY %s %d %d;", orderBook[orderBookId].productName, 0, 0);
    } else {
        sprintf(infoMarketCancel, "MARKET SELL %s %d %d;", orderBook[orderBookId].productName, 0, 0);
    }
    SendInfo(infoCancel, traderId);
    for (int i = 0; i < traderLen; i++) {
        if (i == traderId) {
            continue;
        }
        SendInfo(infoMarketCancel, i);
    }
    return 1;
}

int GetOrderBookId(char* productName) {
    // get the index of the order book for the given product name
    for (int i = 0; i < orderBookLen; i++) {
        if (strcmp(orderBook[i].productName, productName) == 0) {
            return i;
        }
    }
    return -1;
}

void OperationDebug() {
    // debug function, test the correctness of the linked list
    // test InsertNode, MatchNode and DeleteNode
    sumFee = 0;
    orderBookLen = 2;
    traderLen = 2;
    orderBook = malloc(sizeof(OrderBook) * orderBookLen);
    memset(orderBook, 0, sizeof(OrderBook) * orderBookLen);
    peTrader = malloc(sizeof(Traders) * traderLen);
    memset(peTrader, 0, sizeof(Traders) * traderLen);
    for (int i = 0; i < traderLen; i++) {
        peTrader[i].expense = malloc(sizeof(long long) * orderBookLen);
        peTrader[i].expense[0] = 0;
        peTrader[i].expense[1] = 0;
        peTrader[i].qty = malloc(sizeof(int) * orderBookLen);
        peTrader[i].qty[0] = 0;
        peTrader[i].qty[1] = 0;
        peTrader[i].type = TRADER_CLOSE;
        peTrader[i].orderLen = 0;
    }
    orderBook[0].ordersHead = NULL;
    orderBook[0].ordersTail = NULL;
    strcpy(orderBook[0].productName, "GPU");
    orderBook[1].ordersHead = NULL;
    orderBook[1].ordersTail = NULL;
    strcpy(orderBook[1].productName, "Router");
    char operation0[] = "BUY 0 GPU 4 100";
    if (MarketOperation(operation0, 0) != 0) {
        Display();
        printf("\n");
    }
    char operation1[] = "CANCEL 0";
    if (MarketOperation(operation1, 0) != 0) {
        Display();
        printf("\n");
    }
    char operation2[] = "SELL 0 GPU 3 100";
    if (MarketOperation(operation2, 1) != 0) {
        Display();
        printf("\n");
    }
    // char operation3[] = "BUY 3 GPU 30 502";
    // if (MarketOperation(operation3, 0) != 0) {
    //     Display();
    //     printf("\n");
    // }
    // char operation4[] = "SELL 0 GPU 99 511";
    // if (MarketOperation(operation4, 1) != 0) {
    //     Display();
    //     printf("\n");
    // }
    // char operation5[] = "SELL 1 GPU 99 402";
    // if (MarketOperation(operation5, 1) != 0) {
    //     Display();
    //     printf("\n");
    // }
    printf(LOG_PREFIX" Exchange fees collected: $%lld\n", sumFee);
    DestroyNode(orderBook[0].ordersHead);
    free(orderBook);
    free(peTrader[0].expense);
    free(peTrader[0].qty);
    free(peTrader[1].expense);
    free(peTrader[1].qty);
    free(peTrader);
}

void Sigusr1Handler(int signo, siginfo_t* sigInfo, void* voidPtr) {
    for (int i = 0; i < traderLen; i++) {
        if (peTrader[i].traderPid == sigInfo->si_pid) {
            char tradeInfo[MAX_INFO];
            read(peTrader[i].traderFifo, tradeInfo, MAX_INFO);
            int j = 0;
            // remove ';' or '\n' from the end
            while (j < MAX_INFO - 1 && tradeInfo[j] != ';' && tradeInfo[j] != '\n') {
                j++;
            }
            tradeInfo[j] = 0;
            int valid = MarketOperation(tradeInfo, i);
            if (valid == 1) {
                Display();
            } else {
                char info[] = "INVALID;";
                SendInfo(info, i);
            }
            return;
        }
    }
}
void SigchldHandler(int signo, siginfo_t* sigInfo, void* voidPtr) {
    // remove the trader from the trader array
    for (int i = 0; i < traderLen; i++) {
        if (peTrader[i].traderPid == sigInfo->si_pid) {
            // close two fifo
            close(peTrader[i].exchangeFifo);
            close(peTrader[i].traderFifo);
            char exchangeFifoName[MAX_PIPE_NAME];
            char traderFifoName[MAX_PIPE_NAME];
            sprintf(exchangeFifoName, FIFO_EXCHANGE, i);
            sprintf(traderFifoName, FIFO_TRADER, i);
            unlink(exchangeFifoName);
            unlink(traderFifoName);
            printf(LOG_PREFIX" Trader %d disconnected\n", i);
            peTrader[i].type = TRADER_CLOSE;
            peTrader[i].traderPid = 0;
            return;
        }
    }
}

int main(int argc, char **argv) {
    // ListDebug();
    // OperationDebug();
    printf(LOG_PREFIX" Starting\n");
    // init the signal handler, set sa_handler to SignalHandler
    struct sigaction exchangeSigaction, traderExitSigaction;
    sigemptyset(&exchangeSigaction.sa_mask);
    sigemptyset(&traderExitSigaction.sa_mask);
    exchangeSigaction.sa_flags = SA_SIGINFO;
    traderExitSigaction.sa_flags = SA_SIGINFO;
    exchangeSigaction.sa_sigaction = Sigusr1Handler;
    traderExitSigaction.sa_sigaction = SigchldHandler;
    sigaction(SIGUSR1, &exchangeSigaction, NULL);
    sigaction(SIGCHLD, &traderExitSigaction, NULL);
    // init the order book and trader array
    FILE *fp = fopen(argv[1], "r");
    fscanf(fp, "%d", &orderBookLen);
    orderBook = malloc(sizeof(OrderBook) * orderBookLen);
    printf(LOG_PREFIX" Trading %d products:", orderBookLen);
    char productName[MAX_PRODUCT_NAME];
    int i = 0;
    while (i < orderBookLen && fscanf(fp, "%s", productName) == 1) {
        orderBook[i].ordersHead = NULL;
        orderBook[i].ordersTail = NULL;
        strcpy(orderBook[i].productName, productName);
        i++;
    }
    for (int i = 0; i < orderBookLen; i++) {
        printf(" %s", orderBook[i].productName);
        if (i == orderBookLen - 1) {
            printf("\n");
        }
    }
    traderLen = argc - 2;
    peTrader = malloc(sizeof(Traders) * traderLen);
    for (int i = 0; i < traderLen; i++) {
        peTrader[i].type = TRADER_CLOSE;
        char exchangeFifoName[MAX_PIPE_NAME];
        char traderFifoName[MAX_PIPE_NAME];
        sprintf(exchangeFifoName, FIFO_EXCHANGE, i);
        sprintf(traderFifoName, FIFO_TRADER, i);
        if (mkfifo(exchangeFifoName, 0777) == -1) {
            printf(LOG_PREFIX" Failed to create FIFO %s\n", exchangeFifoName);
            return 1;
        }
        if (mkfifo(traderFifoName, 0777) == -1) {
            printf(LOG_PREFIX" Failed to create FIFO %s\n", traderFifoName);
            return 1;
        }
        printf(LOG_PREFIX" Created FIFO %s\n", exchangeFifoName);
        printf(LOG_PREFIX" Created FIFO %s\n", traderFifoName);
        printf(LOG_PREFIX" Starting trader %d (%s)\n", i, argv[i + 2]);
        peTrader[i].traderPid = fork();
        if (peTrader[i].traderPid != 0) {
            peTrader[i].type = TRADER_RUN;
            peTrader[i].exchangeFifo = open(exchangeFifoName, O_WRONLY);
            peTrader[i].traderFifo = open(traderFifoName, O_RDONLY);
            if (peTrader[i].exchangeFifo == -1) {
                printf(LOG_PREFIX" Failed to open FIFO %s\n", exchangeFifoName);
                return 1;
            }
            if (peTrader[i].traderFifo == -1) {
                printf(LOG_PREFIX" Failed to open FIFO %s\n", traderFifoName);
                return 1;
            }
            printf(LOG_PREFIX" Connected to %s\n", exchangeFifoName);
            printf(LOG_PREFIX" Connected to %s\n", traderFifoName);
            peTrader[i].expense = malloc(sizeof(long long) * orderBookLen);
            memset(peTrader[i].expense, 0, sizeof(long long) * orderBookLen);
            peTrader[i].qty = malloc(sizeof(int) * orderBookLen);
            memset(peTrader[i].qty, 0, sizeof(int) * orderBookLen);
            peTrader[i].orderLen = 0;
        } else {
            // child process, use execl to run the trader
            char traderId[20];
            sprintf(traderId, "%d", i);
            execl(argv[i + 2], argv[i + 2], traderId, NULL);
        }
    }
    char infoOpen[] = "MARKET OPEN;";
    for (int i = 0; i < traderLen; i++) {
        SendInfo(infoOpen, i);
    }
    while (1) {
        // wait for the signal
        pause();
        // check if all peTrader are closed
        int numClosed = 0;
        for (int i = 0; i < traderLen; i++) {
            if (peTrader[i].type == TRADER_CLOSE) {
                numClosed++;
            } else {
                break;
            }
        }
        if (numClosed == traderLen) {
            // release the memory
            for (int i = 0; i < orderBookLen; i++) {
                DestroyNode(orderBook[i].ordersHead);
            }
            // release the memory, when trader close we cannot release these two arrays
            for (int i = 0; i < traderLen; i++) {
                free(peTrader[i].expense);
                free(peTrader[i].qty);
            }
            free(orderBook);
            free(peTrader);
            break;
        }
    }

    printf(LOG_PREFIX" Trading completed\n");
    printf(LOG_PREFIX" Exchange fees collected: $%lld\n", sumFee);

    return 0;
}
