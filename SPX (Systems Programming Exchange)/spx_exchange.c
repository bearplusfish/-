/**
* comp2017 - assignment 3
* <your name>
* <your unikey>
*/

#include "spx_exchange.h"



struct PRODUCT *productList = NULL;
int productCount = 0;

struct TRADER *traderList = NULL;
int traderCount = 0;

int *nextOrderIdList = NULL;

struct POSITION **positionList = NULL;

struct ORDER_BOOK **orderBookList = NULL;

struct ORDER *orderList = NULL;

struct SIGNAL *signalList = NULL;

int disconnectTraderCount = 0;

int isMarketOpening = 1;

int currentTime = 1;

long long exchangeFeesCollected = 0;


void sendCommand(int time, int traderId, const char *command)
{
    struct TRADER *trader = NULL;

    trader = &traderList[traderId];
    if (trader->exchangePipe >= 0)
    {
        if (!trader->isDisconnect)
        {
            write(trader->exchangePipe, command, strlen(command));
        }
        kill(trader->pid, SIGUSR1);
    }
}



void startTrader(char ** binaryFileNameList)
{
    int loop = 0;
    int pid = 0;
    char **argv = NULL;
    char exchangePipe[BUFFER_SIZE];
    char traderPipe[BUFFER_SIZE];
    struct TRADER *trader = NULL;
    char id[BUFFER_SIZE];

    for (loop = 0; loop < traderCount; loop++)
    {
        trader = &traderList[loop];
        sprintf(exchangePipe, FIFO_EXCHANGE, trader->traderId);
        unlink(exchangePipe);
        if (0 == mkfifo(exchangePipe, 0777))
        {
            printf("%s Created FIFO %s\n", LOG_PREFIX, exchangePipe);
        }
        unlink(traderPipe);
        sprintf(traderPipe, FIFO_TRADER, trader->traderId);
        if (0 == mkfifo(traderPipe, 0777))
        {
            printf("%s Created FIFO %s\n", LOG_PREFIX, traderPipe);
        }
        printf("%s Starting trader %d (%s)\n", LOG_PREFIX, trader->traderId, binaryFileNameList[loop]);
        pid = fork();
        if (0 == pid)
        {
            sprintf(id, "%d", trader->traderId);
            argv = (char **)allocateMemory(sizeof(char *) * 3);
            argv[0] = strdup(binaryFileNameList[loop]);
            argv[1] = strdup(id);
            argv[2] = NULL;
            execvp(argv[0], argv);
        }
        else
        {
            trader->pid = pid;
        }
        trader->exchangePipe = open(exchangePipe, O_WRONLY);
        if (trader->exchangePipe >= 0)
        {
            printf("%s Connected to %s\n", LOG_PREFIX, exchangePipe);
        }
        trader->traderPipe = open(traderPipe, O_RDONLY);
        if (trader->traderPipe >= 0)
        {
            printf("%s Connected to %s\n", LOG_PREFIX, traderPipe);
        }
        disconnectTraderCount--;
    }
}

void signalHandlerFunction(int signal, siginfo_t *info, void *p)
{
    struct SIGNAL *s = NULL;

    s = newSignal(signal, info->si_pid, currentTime, NULL, 0);

    appendSignal(&signalList, s);
    currentTime++;
}



void setSignalHandlerFunction()
{
    struct sigaction act;

    act.sa_sigaction = signalHandlerFunction;
    act.sa_flags = SA_SIGINFO;

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGCHLD);
    sigaction(SIGUSR1, &act, NULL);

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaction(SIGCHLD, &act, NULL);
}



void process_SIGUSR1_signal(struct SIGNAL *signal)
{
    int nSelectResult = 0;
    int contentLength = 0;
    char content[BUFFER_SIZE];
    struct TRADER *trader = NULL;
    fd_set readfd;
    struct timeval tv;

    trader = getTraderByPid(signal->pid);
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    FD_ZERO(&readfd);
    FD_SET(trader->traderPipe, &readfd);
    nSelectResult = select(trader->traderPipe + 1, &readfd, NULL, NULL, &tv);
    if (nSelectResult > 0)
    {
        if (FD_ISSET(trader->traderPipe, &readfd))
        {
            contentLength = read(trader->traderPipe, content, BUFFER_SIZE);
            if (contentLength >= 0)
            {
                content[contentLength] = '\0';
                processMultipleCommandRequest(trader->traderId,
                    content,
                    contentLength,
                    signal->pid,
                    signal->timeStamp);
            }
        }
    }
}

void process_SIGCHLD_signal(struct SIGNAL *signal)
{
    char exchangePipe[BUFFER_SIZE];
    char traderPipe[BUFFER_SIZE];
    struct TRADER *trader = NULL;

    trader = getTraderByPid(signal->pid);
    trader->isDisconnect = 1;
    printf("%s Trader %d disconnected\n", LOG_PREFIX, trader->traderId);

    close(trader->exchangePipe);
    close(trader->traderPipe);
    trader->exchangePipe = -1;
    trader->traderPipe = -1;
    sprintf(exchangePipe, FIFO_EXCHANGE, trader->traderId);
    unlink(exchangePipe);
    sprintf(traderPipe, FIFO_TRADER, trader->traderId);
    unlink(traderPipe);

    disconnectTraderCount++;
    if (disconnectTraderCount == traderCount)
    {
        isMarketOpening = 0;
    }
}

void processSignal()
{
    struct SIGNAL *signal = NULL;

    while (NULL != signalList)
    {
        signal = signalList;
        removeSignal(&signalList, signal);
        if (SIGUSR1 == signal->signal)
        {
            process_SIGUSR1_signal(signal);
        }
        else if (SIGCHLD == signal->signal)
        {
            process_SIGCHLD_signal(signal);
        }
        deleteSignal(signal);
    }
}

void *allocateMemory(size_t size)
{
    void *memory = NULL;

    memory = malloc(size);
    memset(memory, 0, size);

    return memory;
}

void freeMemory(void *memory)
{
    if (NULL != memory)
    {
        free(memory);
        memory = NULL;
    }
}

char *strdup(const char *oldString)
{
    int nStringLength = 0;
    char *newString = NULL;

    nStringLength = (int)strlen(oldString);
    newString = malloc(nStringLength + 1);
    memcpy(newString, oldString, nStringLength);
    newString[nStringLength] = '\0';

    return newString;
}

void readLine(FILE *file, char *line)
{
    int index = 0;

    line[0] = '\0';
    fgets(line, BUFFER_SIZE, file);
    index = (int)(strlen(line) - 1);
    while (line[index])
    {
        if (('\r' != line[index])
            && ('\n' != line[index]))
        {
            break;
        }
        index--;
    }
    line[index + 1] = '\0';
}



void sendMarketOpenCommand()
{
    int loop = 0;

    for (loop = 0; loop < traderCount; ++loop)
    {
        sendCommand(0, traderList[loop].traderId, "MARKET OPEN;");
    }
}



struct SIGNAL *newSignal(int signal, int pid, int time, const char *content, int contentLength)
{
    struct SIGNAL *s = NULL;

    s = (struct SIGNAL *)allocateMemory(sizeof(struct SIGNAL));
    s->signal = signal;
    s->pid = pid;
    s->timeStamp = time;
    if (contentLength > 0)
    {
        s->content = allocateMemory(sizeof(char) * (contentLength + 1));
        memcpy(s->content, content, contentLength);
        s->contentLength = contentLength;
    }

    return s;
}

void deleteSignal(struct SIGNAL *signal)
{
    freeMemory(signal);
}

void appendSignal(struct SIGNAL **pHead, struct SIGNAL *signal)
{
    struct SIGNAL *last = NULL;
    struct SIGNAL *head = NULL;

    head = *pHead;
    if (NULL == head)
    {
        head = signal;
    }
    else
    {
        last = head;
        while (NULL != last->next)
        {
            last = last->next;
        }
        last->next = signal;
        signal->next = NULL;
    }
    *pHead = head;
}

void removeSignal(struct SIGNAL **pHead, struct SIGNAL *signal)
{
    struct SIGNAL *last = NULL;
    struct SIGNAL *head = NULL;

    head = *pHead;
    if (head == signal)
    {
        head = signal->next;
    }
    else
    {
        last = head;
        while (signal != last->next)
        {
            last = last->next;
        }
        last->next = signal->next;
    }
    *pHead = head;
}


struct TRADER *getTraderByPid(int pid)
{
    int loop = 0;

    for (loop = 0; loop < traderCount; ++ loop)
    {
        if (pid == traderList[loop].pid)
        {
            return &traderList[loop];
        }
    }

    return NULL;
}


void readProductFromFile(const char *filename)
{
    int loop = 0;
    FILE *file = NULL;
    char line[BUFFER_SIZE + 1];

    file = fopen(filename, "rb");

    readLine(file, line);
    productCount = atoi(line);
    productList = (struct PRODUCT *)allocateMemory(sizeof(struct PRODUCT) * productCount);
    for (loop = 0; loop < productCount; ++loop)
    {
        readLine(file, line);
        productList[loop].productId = loop;
        strcpy(productList[loop].name, line);
    }

    fclose(file);
}

void printAllProductName(FILE *file)
{
    int loop = 0;

    fprintf(file, "%s Trading %d products:", LOG_PREFIX, productCount);
    for (loop = 0; loop < productCount; ++loop)
    {
        fprintf(file, " %s", productList[loop].name);
    }
    fprintf(file, "\n");
}


struct PRODUCT *getProductByName(const char *name)
{
    int loop = 0;

    for (loop = 0; loop < productCount; ++loop)
    {
        if (0 == strcmp(productList[loop].name, name))
        {
            return &productList[loop];
        }
    }

    return NULL;
}

struct ORDER *newOrder(const char *orderName,
    int orderId,
    int traderId,
    int quantity,
    int price,
    int time,
    const char *productName)
{
    struct ORDER *order = NULL;

    order = (struct ORDER *)allocateMemory(sizeof(struct ORDER));
    order->traderId = traderId;
    order->orderId = orderId;
    order->quantity = quantity;
    order->price = price;
    order->time = time;
    strcpy(order->orderName, orderName);
    strcpy(order->productName, productName);

    return order;
}


struct ORDER *copyOrder(struct ORDER *o)
{
    struct ORDER *order = NULL;

    order = (struct ORDER *)allocateMemory(sizeof(struct ORDER));
    order->traderId = o->traderId;
    order->orderId = o->orderId;
    order->quantity = o->quantity;
    order->price = o->price;
    order->time = o->time;
    strcpy(order->orderName, o->orderName);
    strcpy(order->productName, o->productName);

    return order;
}

void appendOrder(struct ORDER **pHead, struct ORDER *order, int isAppendToOrderBook)
{
    struct ORDER *last = NULL;
    struct ORDER *head = NULL;

    head = *pHead;
    if (NULL == head)
    {
        head = order;
    }
    else
    {
        last = head;
        if (isAppendToOrderBook)
        {
            while (NULL != last->nextOrderInOrderBook)
            {
                last = last->nextOrderInOrderBook;
            }
            last->nextOrderInOrderBook = order;
            order->nextOrderInOrderBook = NULL;
        }
        else
        {
            while (NULL != last->nextOrderInOrderList)
            {
                last = last->nextOrderInOrderList;
            }
            last->nextOrderInOrderList = order;
            order->nextOrderInOrderList = NULL;
        }
    }
    *pHead = head;
}


void removeOrder(struct ORDER **pHead, struct ORDER *order, int isRemoveFromOrderBook)
{
    struct ORDER *last = NULL;
    struct ORDER *head = NULL;

    head = *pHead;
    if (head == order)
    {
        if (isRemoveFromOrderBook)
        {
            head = order->nextOrderInOrderBook;
        }
        else
        {
            head = order->nextOrderInOrderList;
        }
    }
    else
    {
        last = head;
        if (isRemoveFromOrderBook)
        {
            while (order != last->nextOrderInOrderBook)
            {
                last = last->nextOrderInOrderBook;
            }
            last->nextOrderInOrderBook = order->nextOrderInOrderBook;
        }
        else
        {
            while (order != last->nextOrderInOrderList)
            {
                last = last->nextOrderInOrderList;
            }
            last->nextOrderInOrderList = order->nextOrderInOrderList;
        }
    }
    *pHead = head;
}

struct ORDER *getOrder(struct ORDER *head, int traderId, int orderId)
{
    struct ORDER *cur = NULL;

    cur = head;
    while (NULL != cur)
    {
        if ((orderId == cur->orderId)
            && (traderId == cur->traderId))
        {
            return cur;
        }
        cur = cur->nextOrderInOrderList;
    }

    return NULL;
}

unsigned long long calcOrderBookId(int level, const char *orderName)
{
    unsigned long long id = 0;

    id = level;
    id = id << 32;
    if (0 == strcmp(orderName, "BUY"))
    {
        id = id | 1;
    }

    return id;
}

struct ORDER_BOOK *newOrderBook(const char *orderName, int level)
{
    struct ORDER_BOOK *orderBook = NULL;

    orderBook = (struct ORDER_BOOK *)allocateMemory(sizeof(struct ORDER_BOOK));
    orderBook->id = calcOrderBookId(level, orderName);
    orderBook->level = level;
    strcpy(orderBook->orderName, orderName);

    return orderBook;
}


void removeOrderBook(struct ORDER_BOOK **pHead, struct ORDER_BOOK *orderBook)
{
    struct ORDER_BOOK *last = NULL;
    struct ORDER_BOOK *head = NULL;

    head = *pHead;
    if (head == orderBook)
    {
        head = orderBook->next;
    }
    else
    {
        last = head;
        while (orderBook != last->next)
        {
            last = last->next;
        }
        last->next = orderBook->next;
    }
    *pHead = head;
}



void insertOrderToOrderBook(struct ORDER_BOOK **pHead, struct ORDER *order, struct PRODUCT *product)
{
    int has_insert_order_book = 1;
    unsigned long long order_book_id = 0;
    struct ORDER_BOOK *head;
    struct ORDER_BOOK *next;
    struct ORDER_BOOK *prev;
    struct ORDER_BOOK *curOrderBook;

    head = *pHead;
    if (NULL == head)
    {
        curOrderBook = newOrderBook(order->orderName, order->price);
        head = curOrderBook;
    }
    else
    {
        order_book_id = calcOrderBookId(order->price, order->orderName);
        if (order_book_id > head->id)
        {
            curOrderBook = newOrderBook(order->orderName, order->price);
            curOrderBook->next = head;
            head = curOrderBook;
        }
        else
        {
            prev = head;
            while (NULL != prev->next)
            {
                if ((order_book_id <= prev->id) && (order_book_id > prev->next->id))
                {
                    break;
                }
                prev = prev->next;
            }
            if ((0 != strcmp(order->orderName, prev->orderName))
                || (order->price != prev->level))
            {
                curOrderBook = newOrderBook(order->orderName, order->price);
                next = prev->next;
                prev->next = curOrderBook;
                curOrderBook->next = next;
            }
            else
            {
                has_insert_order_book = 0;
                curOrderBook = prev;
            }
        }
    }
    *pHead = head;
    if (has_insert_order_book)
    {
        if (0 == strcmp(order->orderName, "BUY"))
        {
            product->buyCount++;
        }
        else if (0 == strcmp(order->orderName, "SELL"))
        {
            product->sellCount++;
        }
    }
    curOrderBook->length++;
    curOrderBook->quantity = curOrderBook->quantity + order->quantity;
    appendOrder(&curOrderBook->orderList, order, 1);
}


void removeOrderFromOrderBook(struct ORDER_BOOK **pHead, struct ORDER_BOOK *orderBook, struct ORDER *order, struct PRODUCT *product)
{
    orderBook->length--;
    orderBook->quantity = orderBook->quantity - order->quantity;
    removeOrder(&orderBook->orderList, order, 1);
    if (0 == orderBook->length)
    {
        if (0 == strcmp(orderBook->orderName, "BUY"))
        {
            product->buyCount--;
        }
        else if (0 == strcmp(orderBook->orderName, "SELL"))
        {
            product->sellCount--;
        }
        removeOrderBook(pHead, orderBook);
        freeMemory(orderBook);
    }
}


long long calcValue(int quantity, int price)
{
    long long value = 0;

    value = price;
    value = value * quantity;

    return value;
}

long long calcFee(int quantity, int price)
{
    long long value = 0;
    long long fee = 0;

    value = price;
    value = value * quantity;
    fee = (value * FEE_PERCENTAGE + 50) / 100;

    return fee;
}


void findOrderBook(struct ORDER_BOOK **pOrderBook,
                struct ORDER_BOOK *head,
                struct ORDER *order)
{
    struct ORDER_BOOK *cur = NULL;

    cur = head;
    while (NULL != cur)
    {
        if (0 == strcmp(order->orderName, cur->orderName)
            && (order->price == cur->level))
        {
            *pOrderBook = cur;
            break;
        }
        cur = cur->next;
    }
}

void removeOrderAndOrderBook(struct ORDER *order)
{
    struct PRODUCT *product = NULL;
    struct ORDER_BOOK *orderBook = NULL;

    product = getProductByName(order->productName);
    findOrderBook(&orderBook, orderBookList[product->productId], order);
    removeOrderFromOrderBook(&orderBookList[product->productId],
        orderBook,
        order,
        product);
    removeOrder(&orderList, order, 0);
}

struct MATCH *newMatch(struct ORDER *order1, struct ORDER *order2)
{
    struct MATCH *match = NULL;

    match = (struct MATCH *)allocateMemory(sizeof(struct MATCH));
    match->traderId1 = order1->traderId;
    match->orderId1 = order1->orderId;
    match->traderId2 = order2->traderId;
    match->orderId2 = order2->orderId;
    match->quantity = order1->quantity < order2->quantity ? order1->quantity : order2->quantity;
    match->value = calcValue(match->quantity, order1->price);
    match->fee = calcFee(match->quantity, order1->price);

    return match;
}

void deleteOrder(struct ORDER_BOOK **pHead,
                struct ORDER_BOOK *orderBook,
                struct ORDER *order,
                struct PRODUCT *product)
{
    removeOrderFromOrderBook(pHead, orderBook, order, product);
    removeOrder(&orderList, order, 0);
    freeMemory(order);
}

struct MATCH *findMatchOrder(struct ORDER_BOOK **pHead, struct ORDER *order2, struct PRODUCT *product)
{
    struct ORDER *order1 = NULL;
    struct ORDER_BOOK *cur = NULL;
    struct ORDER_BOOK *orderBook = NULL;
    struct MATCH *match = NULL;

    if (0 == strcmp(order2->orderName, "SELL"))
    {
        cur = *pHead;
        while (NULL != cur)
        {
            if ((0 == strcmp(cur->orderName, "BUY"))
                && (cur->level >= order2->price))
            {
                orderBook = cur;
                break;
            }
            cur = cur->next;
        }
    }
    else if (0 == strcmp(order2->orderName, "BUY"))
    {
        cur = *pHead;
        while (NULL != cur)
        {
            if ((0 == strcmp(cur->orderName, "SELL"))
                && (cur->level <= order2->price))
            {
                orderBook = cur;
            }
            cur = cur->next;
        }
    }
    if (NULL != orderBook)
    {
        order1 = orderBook->orderList;
        match = newMatch(order1, order2);
        orderBook->quantity = orderBook->quantity - match->quantity;
        order1->quantity = order1->quantity - match->quantity;
        order2->quantity = order2->quantity - match->quantity;
        if (0 == order1->quantity)
        {
            deleteOrder(pHead, orderBook, order1, product);
        }
        return match;
    }

    return match;
}



int getTokenCount(const char *str)
{
    int index = 0;
    int tokenCount = 0;

    while (str[index] != '\0')
    {
        if ((' ' == str[index]) || ('\t' == str[index]))
        {
            ++tokenCount;
            while ((' ' == str[index]) || ('\t' == str[index]))
            {
                ++index;
            }
            --index;
        }
        ++index;
    }

    return tokenCount + 1;
}


struct ORDER *parseCommand(char *commandName,
    char *str,
    int traderId,
    int time)
{
    int orderId = 0;
    int quantity = 0;
    int price = 0;
    char productName[BUFFER_SIZE];
    struct ORDER *order = NULL;

    sscanf(str, "%s", commandName);
    if ((0 == strcmp(commandName, "BUY"))
        || (0 == strcmp(commandName, "SELL")))
    {
        if (5 != getTokenCount(str))
        {
            return order;
        }
        sscanf(str, "%s %d %s %d %d;",
            commandName,
            &orderId,
            productName,
            &quantity,
            &price);
        if (NULL == getProductByName(productName))
        {
            return order;
        }
        if ((orderId != nextOrderIdList[traderId])
            || (quantity < 1)
            || (quantity > 999999)
            || (price < 1)
            || (price > 999999))
        {
            return order;
        }
        order = newOrder(commandName,
            orderId,
            traderId,
            quantity,
            price,
            time,
            productName);
        nextOrderIdList[traderId]++;
    }
    else if (0 == strcmp(commandName, "AMEND"))
    {
        if (4 != getTokenCount(str))
        {
            return order;
        }
        sscanf(str, "%s %d %d %d;",
            commandName,
            &orderId,
            &quantity,
            &price);
        if ((quantity < 1)
            || (quantity > 999999)
            || (price < 1)
            || (price > 999999))
        {
            return order;
        }
        order = getOrder(orderList, traderId, orderId);
        if (NULL != order)
        {
            removeOrderAndOrderBook(order);
            order->quantity = quantity;
            order->price = price;
            order->time = time;
        }
    }
    else if (0 == strcmp(commandName, "CANCEL"))
    {
        if (2 != getTokenCount(str))
        {
            return order;
        }
        sscanf(str, "%s %d;",
            commandName,
            &orderId);
        order = getOrder(orderList, traderId, orderId);
        if (NULL != order)
        {
            removeOrderAndOrderBook(order);
        }
    }

    return order;
}




void printOrderBook(FILE *file)
{
    int loop = 0;
    struct ORDER_BOOK *orderBook = NULL;

    fprintf(file, "%s\t--ORDERBOOK--\n", LOG_PREFIX);
    for (loop = 0; loop < productCount; ++loop)
    {
        fprintf(file, "%s\tProduct: %s; Buy levels: %d; Sell levels: %d\n",
            LOG_PREFIX,
            productList[loop].name,
            productList[loop].buyCount,
            productList[loop].sellCount);
        orderBook = orderBookList[loop];
        while (NULL != orderBook)
        {
            fprintf(file, "%s\t\t", LOG_PREFIX);
            fprintf(file, "%s", orderBook->orderName);
            fprintf(file, " %d @ $%d (%d order",
                orderBook->quantity,
                orderBook->level,
                orderBook->length);
            if (orderBook->length > 1)
            {
                fprintf(file, "s");
            }
            fprintf(file, ")\n");
            orderBook = orderBook->next;
        }
    }
}

void printPositions(FILE *file)
{
    int i = 0;
    int j = 0;

    fprintf(file, "%s\t--POSITIONS--\n", LOG_PREFIX);
    for (i = 0; i < traderCount; ++i)
    {
        fprintf(file, "%s\tTrader %d: ", LOG_PREFIX, i);
        for (j = 0; j < productCount; ++j)
        {
            if (j > 0)
            {
                fprintf(file, ", ");
            }
            fprintf(file, "%s %d ($%lld)",
                productList[j].name,
                positionList[i][j].quantity,
                positionList[i][j].value);
        }
        fprintf(file, "\n");
    }
}

void printMatch(FILE *file, struct MATCH *match_list)
{
    struct MATCH *cur = NULL;

    cur = match_list;
    while (NULL != cur)
    {
        fprintf(file, "%s Match: Order %d [T%d], New Order %d [T%d], value: $%lld, fee: $%lld.\n",
            LOG_PREFIX,
            cur->orderId1,
            cur->traderId1,
            cur->orderId2,
            cur->traderId2,
            cur->value,
            cur->fee);
        cur = cur->next;
    }
}

void printInfo(FILE *file, struct MATCH *match_list)
{
    printMatch(file, match_list);
    printOrderBook(file);
    printPositions(file);
}


void sendResponseCommand(int time,
    int traderId,
    const char *commandName,
    struct ORDER *order,
    struct MATCH *match_list)
{
    int loop = 0;
    char responseCcommand[BUFFER_SIZE];
    char marketCcommand[BUFFER_SIZE];
    char fillCcommand[BUFFER_SIZE];
    struct TRADER *trader = NULL;
    struct MATCH *cur = NULL;

    if ((0 == strcmp(commandName, "BUY"))
        || (0 == strcmp(commandName, "SELL")))
    {
        sprintf(responseCcommand, "ACCEPTED %d;", order->orderId);
    }
    else if (0 == strcmp(commandName, "AMEND"))
    {
        sprintf(responseCcommand, "AMENDED %d;", order->orderId);
    }
    else if (0 == strcmp(commandName, "CANCEL"))
    {
        sprintf(responseCcommand, "CANCELLED %d;", order->orderId);
    }

    if (0 == strcmp(commandName, "CANCEL"))
    {
        sprintf(marketCcommand, "MARKET %s %s %d %d;",
            order->orderName,
            order->productName,
            0,
            0);
    }
    else
    {
        sprintf(marketCcommand, "MARKET %s %s %d %d;",
            order->orderName,
            order->productName,
            order->quantity,
            order->price);
    }
    for (loop = 0; loop < traderCount; loop++)
    {
        trader = &traderList[loop];
        if (traderId == trader->traderId)
        {
            sendCommand(time, trader->traderId, responseCcommand);
        }
        else
        {
            sendCommand(time, trader->traderId, marketCcommand);
        }
    }
    cur = match_list;
    while (NULL != cur)
    {
        sprintf(fillCcommand, "FILL %d %d;", cur->orderId1, cur->quantity);
        sendCommand(time, cur->traderId1, fillCcommand);
        sprintf(fillCcommand, "FILL %d %d;", cur->orderId2, cur->quantity);
        sendCommand(time, cur->traderId2, fillCcommand);
        cur = cur->next;
    }
}

void appendMatch(struct MATCH **pHead, struct MATCH *match)
{
    struct MATCH *last = NULL;
    struct MATCH *head = NULL;

    head = *pHead;
    if (NULL == head)
    {
        head = match;
    }
    else
    {
        last = head;
        while (NULL != last->next)
        {
            last = last->next;
        }
        last->next = match;
        match->next = NULL;
    }
    *pHead = head;
}



void deleteMatchList(struct MATCH **pMatchList)
{
    struct MATCH *cur = NULL;
    struct MATCH *next = NULL;

    cur = *pMatchList;
    while (NULL != cur)
    {
        next = cur->next;
        freeMemory(cur);
        cur = next;
    }
    *pMatchList = NULL;
}


void match(struct MATCH **pMatchList,
    struct ORDER *order2)
{
    struct MATCH *match = NULL;
    struct PRODUCT *product = NULL;

    product = getProductByName(order2->productName);
    while (order2->quantity > 0)
    {
        match = findMatchOrder(&orderBookList[product->productId],
            order2,
            product);
        if (NULL == match)
        {
            break;
        }
        if (0 == strcmp(order2->orderName, "BUY"))
        {
            positionList[match->traderId1][product->productId].quantity -= match->quantity;
            positionList[match->traderId1][product->productId].value += match->value;
            positionList[match->traderId2][product->productId].quantity += match->quantity;
            positionList[match->traderId2][product->productId].value -= match->value;
        }
        else if (0 == strcmp(order2->orderName, "SELL"))
        {
            positionList[match->traderId1][product->productId].quantity += match->quantity;
            positionList[match->traderId1][product->productId].value -= match->value;
            positionList[match->traderId2][product->productId].quantity -= match->quantity;
            positionList[match->traderId2][product->productId].value += match->value;
        }
        positionList[match->traderId2][product->productId].value -= match->fee;
        exchangeFeesCollected += match->fee;
        appendMatch(pMatchList, match);
    }
}



struct ORDER *process_command(char *commandName,
    struct MATCH **pMatchList,
    int traderId,
    char *str,
    int time)
{
    struct ORDER *order = NULL;
    struct ORDER *orderParsed = NULL;
    struct PRODUCT *product = NULL;

    *pMatchList = NULL;
    order = parseCommand(commandName, str, traderId, time);
    if (NULL != order)
    {
        orderParsed = copyOrder(order);
        if ((0 == strcmp(commandName, "BUY"))
            || (0 == strcmp(commandName, "SELL"))
            || (0 == strcmp(commandName, "AMEND")))
        {
            match(pMatchList, order);
            if (order->quantity > 0)
            {
                product = getProductByName(order->productName);
                appendOrder(&orderList, order, 0);
                insertOrderToOrderBook(&orderBookList[product->productId], order, product);
            }
            else
            {
                freeMemory(order);
            }
        }
        else if (0 == strcmp(commandName, "CANCEL"))
        {
            freeMemory(order);
        }
    }

    return orderParsed;
}


void processMultipleCommandRequest(int traderId,
    const char *content,
    int contentLength,
    int pid,
    int time)
{
    int index = 0;
    int command_length = 0;
    char command[BUFFER_SIZE];
    char commandName[BUFFER_SIZE];
    struct ORDER *order = NULL;
    struct MATCH *matchList = NULL;

    while (index < contentLength)
    {
        command_length = 0;
        while (';' != content[index])
        {
            command[command_length++] = content[index++];
        }
        command[command_length] = '\0';
        ++index;
//            fprintf(stderr, "%d %d %s\n", trader->trader_id, signal->time, command);
        order = process_command(commandName,
            &matchList,
            traderId,
            command,
            time);
        if (NULL != order)
        {
            fprintf(stdout, "%s [T%d] Parsing command: <%s>\n",
                LOG_PREFIX,
                traderId,
                command);
            printInfo(stdout, matchList);
            sendResponseCommand(time,
                traderId,
                commandName,
                order,
                matchList);
            freeMemory(order);
            deleteMatchList(&matchList);
        }
        else
        {
            fprintf(stdout, "%s [T%d] Parsing command: <%s>\n",
                LOG_PREFIX,
                traderId,
                command);
            sendCommand(time, traderId, "INVALID;");
        }
    }

}


void startExchange(const char *productFileName, int trader_count)
{
    int loop = 0;

    readProductFromFile(productFileName);
    traderCount = trader_count;
    disconnectTraderCount = traderCount;
    orderBookList = (struct ORDER_BOOK **)allocateMemory(sizeof(struct ORDER_BOOK *) * productCount);
    traderList = (struct TRADER *)allocateMemory(sizeof(struct TRADER) * trader_count);
    positionList = (struct POSITION **)allocateMemory(sizeof(struct POSITION *) * trader_count);
    nextOrderIdList = (int *)allocateMemory(sizeof(int) * trader_count);
    for (loop = 0; loop < traderCount; ++loop)
    {
        traderList[loop].traderId = loop;
        positionList[loop] = (struct POSITION *)allocateMemory(sizeof(struct POSITION) * productCount);
    }
}


void stopExchange()
{
    int loop = 0;
    struct ORDER *cur = NULL;
    struct ORDER *next = NULL;

    cur = orderList;
    while (NULL != cur)
    {
        next = cur->nextOrderInOrderList;
        removeOrderAndOrderBook(cur);
        freeMemory(cur);
        cur = next;
    }
    for (loop = 0; loop < traderCount; ++loop)
    {
        freeMemory(positionList[loop]);
        positionList[loop] = NULL;
    }
    freeMemory(orderBookList);
    freeMemory(nextOrderIdList);
    freeMemory(positionList);
    freeMemory(traderList);
    freeMemory(productList);

    orderBookList = NULL;
    nextOrderIdList = NULL;
    positionList = NULL;
    traderList = NULL;
    productList = NULL;
    traderCount = 0;
    productCount = 0;
}


int main(int argc, char **argv) {
    
    startExchange(argv[1], argc - 2);
    setSignalHandlerFunction();
    printf("%s Starting\n", LOG_PREFIX);
    printAllProductName(stdout);
    startTrader(argv + 2);
    sendMarketOpenCommand();
    while (isMarketOpening)
    {
        processSignal();
    }
    stopExchange();
    fprintf(stdout, "%s Trading completed\n", LOG_PREFIX);
    fprintf(stdout, "%s Exchange fees collected: $%lld\n", LOG_PREFIX, exchangeFeesCollected);


    return 0;
}





