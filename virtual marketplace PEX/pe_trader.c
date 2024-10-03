#include "pe_trader.h"

int exchangeFifo;
int traderFifo;
int state;
int totalOrder;

void InitNamePipes(int pipeId) {
    // init the name pipes
    char exchangeFifoName[MAX_PIPE_NAME];
    char traderFifoName[MAX_PIPE_NAME];
    
    sprintf(exchangeFifoName, FIFO_EXCHANGE, pipeId);
    sprintf(traderFifoName, FIFO_TRADER, pipeId);

    // open the pipes
    exchangeFifo = open(exchangeFifoName, O_RDONLY);
    traderFifo = open(traderFifoName, O_WRONLY);
}

void DestroyNamePipes() {
    // close the pipes
    close(exchangeFifo);
    close(traderFifo);
}

void SignalHandler(int signo, siginfo_t* sigInfo, void* voidPtr) {
    if (signo != SIGUSR1) {
        return;
    }
    char exchangeInfo[MAX_INFO];
    read(exchangeFifo, exchangeInfo, MAX_INFO);
    // remove ';' from the end, then it's easier to use strtok
    int i = 0;
    while (exchangeInfo[i] != ';' && i < MAX_INFO) {
        i++;
    }
    exchangeInfo[i] = 0;
    // state machine
    traderStateMachine(exchangeInfo);
}

int StartWith(char* str, char* prefix) {
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

void SendOrder(int orderId, char* product, int qty, int price) {
    // update trader info
    char traderInfo[MAX_INFO];
    sprintf(traderInfo, "BUY %d %s %d %d;", orderId, product, qty, price);
    // send order
    write(traderFifo, traderInfo, strlen(traderInfo));
    // signal exchange
    kill(getppid(), SIGUSR1);
}

void traderStateMachine(char* exchangeInfo) {
    char delims[] = " ";
    char* ignore;
    char* productStr;
    char* qtyStr;
    char* priceStr;
    char* orderIdStr;
    int qty = 0;
    int price = 0;
    int orderId = 0;
    
    switch (state) {
        case STATE_OPEN:
            if (StartWith(exchangeInfo, "MARKET") == 0) {
                // wrong message
                break;
            }
            ignore = strtok(exchangeInfo, delims);
            ignore = strtok(NULL, delims);
            // check second word is OPEN
            if (ignore == NULL || strcmp(ignore, "OPEN") != 0) {
                // wrong message
                break;
            }
            state = STATE_WAITING;
            break;
        case STATE_WAITING:
            // wait for exchange update (MARKET message)
            if (StartWith(exchangeInfo, "MARKET") == 0) {
                // wrong message
                break;
            }
            // ignore first and second word
            ignore = strtok(exchangeInfo, delims);
            ignore = strtok(NULL, delims);
            // check second word is SELL
            if (ignore == NULL || StartWith(ignore, "SELL") == 0) {
                // wrong message
                break;
            }
            // get product
            productStr = strtok(NULL, delims);
            // get qty
            qtyStr = strtok(NULL, delims);
            qty = atoi(qtyStr);
            // get price
            priceStr = strtok(NULL, delims);
            price = atoi(priceStr);
            // check qty
            if (qty >= 1000) {
                state = STATE_CLOSED;
                break;
            } else {
                state = STATE_CONFIRMED;
            }
            // send order
            SendOrder(totalOrder, productStr, qty, price);
            totalOrder++;
            break;
        case STATE_CONFIRMED:
            // wait for exchange confirmation (ACCEPTED message)
            if (StartWith(exchangeInfo, "ACCEPTED") == 0) {
                // wrong message
                break;
            }
            // ignore first word
            ignore = strtok(exchangeInfo, delims);
            // get order id
            orderIdStr = strtok(NULL, delims);
            orderId = atoi(orderIdStr);
            // check order id
            if (orderId != totalOrder - 1) {
                // wrong order id
                break;
            } else {
                state = STATE_WAITING;
            }
            break;
        default:
            break;
    }
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    // init the signal handler, set sa_handler to SignalHandler
    struct sigaction traderSigaction;
    sigemptyset(&traderSigaction.sa_mask);
    traderSigaction.sa_flags = SA_SIGINFO;
    traderSigaction.sa_sigaction = SignalHandler;
    sigaction(SIGUSR1, &traderSigaction, NULL);

    // connect to named pipes
    InitNamePipes(atoi(argv[1]));

    state = STATE_OPEN;
    totalOrder = 0;

    // event loop:
    while (state != STATE_CLOSED) {
        // wait for sigusr1 signal
        pause();
        // update state of trader
    }

    // disconnect from named pipes
    DestroyNamePipes();
    
    return 0;
}
