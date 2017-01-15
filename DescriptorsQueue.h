//DescriptorsQueue.h
//Author: Bombo
//13.01.2017
//Класс DescriptorQueue содержит очередь дескрипторов на обработку

#pragma once

#include<pthread.h>

#include"SomeDirectory.h"

struct DirQueueElement;

struct DirQueueElement
{
    SomeDirectory *psdPtr;
    DirQueueElement *pdqeNext;
    DirQueueElement *pdqePrev;

    DirQueueElement();
    DirQueueElement(SomeDirectory *in_psdPtr, DirQueueElement *in_pdqePrev);
    ~DirQueueElement();
};

class DescriptorsQueue
{
    DirQueueElement *pdqeFirst;
    pthread_mutex_t mQueueMutex;
public:
    DescriptorsQueue();
    //директория не обязана быть открытой перед помещением в очередь
    DescriptorsQueue(SomeDirectory const * const in_psdDir);
    ~DescriptorsQueue();

    void AddQueueElement(int in_nFd);
    void SubQueueElement(int in_nFd);
};
