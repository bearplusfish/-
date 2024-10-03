#ifndef SPX_TRADER_H
#define SPX_TRADER_H

#include "spx_common.h"



void *allocateMemory(size_t size);

char *strdup(const char *orgString);

struct SIGNAL *newSignal(int signal, int pid, int time, const char *content, int contentLength);

void deleteSignal(struct SIGNAL *signal);

void appendSignal(struct SIGNAL **pHead, struct SIGNAL *signal);

void removeSignal(struct SIGNAL **pHead, struct SIGNAL *signal);




#endif
