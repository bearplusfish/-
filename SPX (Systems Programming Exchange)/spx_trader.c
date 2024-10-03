#include "spx_trader.h"


int nextOrderId = 0;

int currentTime = 1;

int isMarketOpen = 1;

int hasOrder = 0;

unsigned long long lastSendTime = 0;

struct TRADER *trader = NULL;

struct SIGNAL *signalList = NULL;


unsigned long long getCurrentMillsecond()
{
    unsigned long long currentMillsecond = 0;
    struct timeval currentTimeval;

    gettimeofday(&currentTimeval, NULL);
    currentMillsecond = currentTimeval.tv_sec;
    currentMillsecond = currentMillsecond * 1000000;
    currentMillsecond = currentMillsecond + currentTimeval.tv_usec;

    return currentMillsecond;
}


void sendSignal()
{
    if (!trader->isDisconnect)
    {
        kill(trader->pid, SIGUSR1);
    }
    lastSendTime = getCurrentMillsecond();
}

void resendTimeOutOrderSignal()
{
    unsigned long long time = 0;

    if (hasOrder)
    {
        time = getCurrentMillsecond();
        if (lastSendTime + 1000000 < time)
        {
            sendSignal();
        }
    }
}

int writeCommand(const char *command)
{
    int numOfBytesToWrite = 0;
    int numOfBytesWritten = 0;

    numOfBytesToWrite = strlen(command);
    if (!trader->isDisconnect)
    {
        numOfBytesWritten = write(trader->traderPipe, command, numOfBytesToWrite);
    }

    return numOfBytesWritten;
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
    freeMemory(signal->content);
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


void deleteSignalList(struct SIGNAL **pSignalList)
{
    struct SIGNAL *cur = NULL;
    struct SIGNAL *next = NULL;

    cur = *pSignalList;
    while (NULL != cur)
    {
        next = cur->next;
        freeMemory(cur);
        cur = next;
    }
    *pSignalList = NULL;
}

struct TRADER *newTrader(int traderId)
{
    struct TRADER *trader = NULL;

    trader = (struct TRADER *)allocateMemory(sizeof(struct TRADER));
    trader->traderId = traderId;
    trader->pid = getppid();
    trader->exchangePipe = -1;
    trader->traderPipe = -1;

    return trader;
}

void startTrader()
{
    char exchangePipe[BUFFER_SIZE];
    char traderPipe[BUFFER_SIZE];

    sprintf(exchangePipe, FIFO_EXCHANGE, trader->traderId);
    trader->exchangePipe = open(exchangePipe, O_RDONLY);
    sprintf(traderPipe, FIFO_TRADER, trader->traderId);
    trader->traderPipe = open(traderPipe, O_WRONLY);
}

void stopTrader()
{
    close(trader->exchangePipe);
    close(trader->traderPipe);
    trader->exchangePipe = -1;
    trader->traderPipe = -1;
}


int isAcceptedCommand(char *str)
{
    if (0 == strncmp(str, "ACCEPTED", strlen("ACCEPTED")))
    {
        hasOrder = 0;
        return 1;
    }

    return 0;
}

void signalHandlerFunction(int signal, siginfo_t *info, void *p)
{
    int contentLength = 0;
    struct SIGNAL *s = NULL;
    char content[BUFFER_SIZE];
    int nSelectResult = 0;
    fd_set readfd;
    struct timeval tv;

    if (SIGUSR1 == signal)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        FD_ZERO(&readfd);
        FD_SET(trader->exchangePipe, &readfd);
        nSelectResult = select(trader->exchangePipe + 1, &readfd, NULL, NULL, &tv);
        if (nSelectResult > 0)
        {
            if (FD_ISSET(trader->exchangePipe, &readfd))
            {
                contentLength = read(trader->exchangePipe, content, BUFFER_SIZE);
                if (contentLength > 0)
                {
                    content[contentLength] = '\0';
                    if (!isAcceptedCommand(content))
                    {
                        s = newSignal(signal, info->si_pid, currentTime, content, contentLength);
                        appendSignal(&signalList, s);
                    }
                }
            }
        }
    }
    else if (SIGPIPE == signal)
    {
        trader->isDisconnect = 1;
        s = newSignal(signal, info->si_pid, currentTime, NULL, 0);
        appendSignal(&signalList, s);
    }
    currentTime++;
}



void setSignalHandlerFunction()
{
    struct sigaction act;

    act.sa_sigaction = signalHandlerFunction;
    act.sa_flags = SA_SIGINFO;

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGPIPE);
    sigaction(SIGUSR1, &act, NULL);

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaction(SIGPIPE, &act, NULL);
}


void init(int argc, char **argv)
{
    setSignalHandlerFunction();
    trader = newTrader(atoi(argv[1]));
    startTrader();
}


void unit()
{
    stopTrader();
    freeMemory(trader);
    deleteSignalList(&signalList);
}




void processCommand(int pid, char *str, int time)
{
    int quantity = 0;
    int price = 0;
    char productName[BUFFER_SIZE - 20];
    char commandName[BUFFER_SIZE];
    char marketName[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    if (0 == strncmp(str, "MARKET BUY", strlen("MARKET BUY")))
    {
        if (5 == sscanf(str, "%s %s %s %d %d",
            marketName,
            commandName,
            productName,
            &quantity,
            &price))
        {
            sprintf(command, "%s %d %s %d %d;",
                "SELL",
                nextOrderId,
                productName,
                quantity,
                price);
            nextOrderId ++;
            hasOrder = 1;
            writeCommand(command);
            sendSignal();
        }
    }
    else if (0 == strncmp(str, "MARKET SELL", strlen("MARKET SELL")))
    {
        if (5 == sscanf(str, "%s %s %s %d %d",
            marketName,
            commandName,
            productName,
            &quantity,
            &price))
        {
            sprintf(command, "%s %d %s %d %d;",
                "BUY",
                nextOrderId,
                productName,
                quantity,
                price);
            nextOrderId++;
            hasOrder = 1;
            writeCommand(command);
            sendSignal();
        }
    }
}

void process_SIGUSR1_signal(struct SIGNAL *signal)
{
    int index = 0;
    int command_length = 0;
    char command[BUFFER_SIZE];

    while (index < signal->contentLength)
    {
        command_length = 0;
        while (';' != signal->content[index])
        {
            command[command_length++] = signal->content[index++];
        }
        command[command_length++] = signal->content[index++];
        command[command_length] = '\0';
        processCommand(signal->pid, command, signal->timeStamp);
    }
}


void processSignal()
{
    struct SIGNAL *signal = NULL;

    while ((!hasOrder) && (NULL != signalList))
    {
        signal = signalList;
        removeSignal(&signalList, signal);
        if (SIGUSR1 == signal->signal)
        {
            process_SIGUSR1_signal(signal);
        }
        deleteSignal(signal);

        //        trader->disconnect = 1;
    }
}


int waitPipeSignal()
{
    sigset_t wait_mask;
    struct timespec time;

    time.tv_sec = 0;
    time.tv_nsec = 10000000;
    sigemptyset(&wait_mask);
    sigaddset(&wait_mask, SIGPIPE);
    return sigtimedwait(&wait_mask, NULL, &time);
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    init(argc, argv);
    while (!trader->isDisconnect)
    {
        if (SIGPIPE == waitPipeSignal())
        {
            trader->isDisconnect = 1;
        }
        else
        {
            processSignal();
            resendTimeOutOrderSignal();
        }
    }
    unit();
    
}
