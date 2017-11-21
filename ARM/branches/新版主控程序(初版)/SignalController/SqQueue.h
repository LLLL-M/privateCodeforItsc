#ifndef _SQ_QUEUE_H
#define _SQ_QUEUE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
typedef char QElemType ;

typedef struct
{
    QElemType *base;
    int front;
    int rear;
    int TotalSize;//size of the buf , like 188*10
	int BlockSize;//size of each block in this queue , like 188
	int CurrentSize;//size of the buf in current cond
}SqQueue;



int InitSqQueue(SqQueue *S);


int InsertQueue(SqQueue *S, QElemType *e);

int DeleteQueue(SqQueue *S, QElemType * e);

void PrintQueue(SqQueue *S);

void DestoryQueue(SqQueue *S);

int QueueLength(SqQueue *S);

int IsSqQueueEmpty(SqQueue *S);


#endif
