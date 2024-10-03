#ifndef PE_TRADER_H
#define PE_TRADER_H

#include "pe_common.h"

#define MAX_INFO 200
#define MAX_PIPE_NAME 60

#define STATE_OPEN 0
#define STATE_CLOSED 1
#define STATE_WAITING 2
#define STATE_CONFIRMED 3

void InitNamePipes(int pipeId);
void SignalHandler(int signo, siginfo_t* sigInfo, void* voidPtr);
void DestroyNamePipes();
int StartWith(char* str, char* prefix);
void traderStateMachine(char* exchangeInfo);

#endif
