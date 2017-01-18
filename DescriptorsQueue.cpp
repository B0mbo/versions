//DescriptrorsQueue.cpp
//Author: Bombo
//18.01.2017
//Класс DescriptorsQueue сдержит очередь открытых дескрипторов директорий, в которых
//произошли некоторые изменения. Дескрипторы из этой очереди ожидают обрабтки в той
//же последовательности, в какой они были добавлены.

#include"DescriptorsQueue.h"

/*****************************************DescriptorsQueue**********************************/

DescriptorsQueue::DescriptorsQueue()
{
    pdnFirst = NULL;
    mFdQueueMutex = PTHREAD_MUTEX_INITIALIZER;
}

DescriptorsQueue::DescriptorsQueue(int in_nFd)
{
    pdnFirst = new DescriptorNum(in_nFd);
    mFdQueueMutex = PTHREAD_MUTEX_INITIALIZER;
}

//метод возращает первый в очереди дескриптор и удаляет элемент списка
int DescriptorsQueue::GetDescriptor(void)
{
    int nRetFd;
    DescriptorNum *pdnDel;

    if(pdnFirst == NULL)
	return -1;

    pthread_mutex_lock(&mFdQueueMutex);
    nRetFd = pdnFirst->nFd;
    pdnDel = pdnFirst;
    pdnFirst = pdnFirst->pdnNext;
    delete pdnDel;
    pthread_mutex_unlock(&mFdQueueMutex);

    return nRetFd;
}

DescriptorsQueue::~DescriptorsQueue()
{
    DescriptorNum *pdnDel;

    pthread_mutex_lock(&mFdQueueMutex);
    pdnDel = pdnFirst;
    while(pdnDel != NULL)
    {
	pdnFirst = pdnFirst->pdnNext;
	delete pdnDel;
    }
    pdnFirst = NULL;
    pthread_mutex_unlock(&mFdQueueMutex);
}

/*****************************************DescriptorNum*************************************/

DescriptorNum::DescriptorNum(int in_nFd)
{
    nFd = in_nFd;
    pdnNext = NULL;
}

DescriptorNum::~DescriptorNum()
{
}
