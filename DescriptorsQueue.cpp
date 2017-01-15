//DescriptorsQueue.cpp
//Author: Bombo
//13.01.2017
//Класс DescriptorQueue содержит очередь дескрипторов на обработку

#include"DescriptorsQueue.h"

/*********************************DescriptorQueue*********************************/

DescriptorsQueue::DescriptorsQueue()
{
    pdqeFirst = NULL;
    mQueueMutex = PTHREAD_MUTEX_INITIALIZER;
}

/*********************************DirQueueElement*********************************/

DirQueueElement::DirQueueElement()
{
    psdPtr = NULL;
    pdqeNext = NULL;
    pdqePrev = NULL;
}

DirQueueElement::DirQueueElement(SomeDirectory *in_psdPtr, DirQueueElement *in_pdqePrev)
{
    psdPtr = in_psdPtr;

    pdqePrev = in_pdqePrev;
    
    //исключаем выпадение части элементов списка    
    if(in_pdqePrev != NULL)
    {
        pdqeNext = in_pdqePrev->pdqeNext;
	//если за предшествующим элементом есть последующий, меняем у последующего предыдущий на this
        if(in_pdqePrev->pdqeNext != NULL)
    	    in_pdqePrev->pdqeNext->pdqePrev = this;
	in_pdqePrev->pdqeNext = this;
    }
    else
    {
	pdqeNext = NULL;
    }
}

DirQueueElement::~DirQueueElement()
{
    psdPtr = NULL;
}
