#include "SqQueue.h"


int InitSqQueue(SqQueue *S)
{
    S->base = (QElemType *)malloc(sizeof(QElemType)*(S->TotalSize));

   // log_debug("Init %p\n",S->base);
    if(!S->base)
    {
    	printf("%s  error : %s   bufSize   %d\n",__func__,strerror(errno),S->TotalSize);
        return 0;
    }
    memset(S->base,0,sizeof(QElemType)*(S->TotalSize));
    S->front = S->rear = 0;
    S->CurrentSize = 0;

    return 1;
}

int InsertQueue(SqQueue *S, QElemType *e)
{
    if((S->rear + S->BlockSize)%(S->TotalSize) == S->front)
    {
        printf("full e:  %s  !!!\n","InsertQueue");
        return 0;
    }

//	AppendEmptyData(S,e);
   
	memcpy((S->base + S->rear),e,S->BlockSize);
	//log_debug("insert  %p : %s\n",(S->base + S->rear),e);

    S->rear = (S->rear + S->BlockSize)%(S->TotalSize);
	S->CurrentSize += 1;
	
   return 1;
}

int DeleteQueue(SqQueue *S, QElemType * e)
{
    if(S->front == S->rear)
    {
    	//log_debug("stream[%d]----del error\n",nProcessIndex);
        return 0;

    }

    //*e = *(S->base + S->front);
    memcpy(e,(S->base + S->front),S->BlockSize);
  //  log_debug("del  %p : %s\n",(S->base + S->front),e);
    S->front = (S->front + S->BlockSize)%(S->TotalSize);
	S->CurrentSize -= 1;
    return 1;
}


void PrintQueue(SqQueue *S)
{
    QElemType *a = S->base;

    int front = S->front;
    int rear = S->rear;
    
    while(front != rear)
    {
        printf("%s\t",a+front);
        front ++;
    }

    printf("\n");

}

void DestoryQueue(SqQueue *S)
{
    free(S->base);
}

int QueueLength(SqQueue *S)
{

	return (S->rear - S->front + (S->TotalSize))%(S->TotalSize);
}

int IsSqQueueEmpty(SqQueue *S)
{

	if(QueueLength(S) > 0)
	{
		printf("sq not null\n");
		return -1;
	}
	else
	{
		printf("sq null\n");
		return 1;
	}

}

#if 0
int main(int argc,char **argv)
{
	SqQueue S;
	S.BlockSize = 8;
	S.TotalSize = 8*5;
	InitSqQueue(&S);

	InsertQueue(&S,"xiao");
	InsertQueue(&S,"wen");
	InsertQueue(&S,"hu");
	InsertQueue(&S,"tiger");

	char buf[8];

	DeleteQueue(&S,buf);

	DestoryQueue(&S);
	
	return -1;
}
#endif







