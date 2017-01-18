//DescriptorsList.cpp
//Author: Bombo
//13.01.2017
//Класс DescriptorsList содержит список всех открытых дескрипторов

#include"DescriptorsList.h"

/*********************************DescriptorQueue*********************************/

DescriptorsList::DescriptorsList()
{
    //инициализация блокировки списка директорий
    mQueueMutex = PTHREAD_MUTEX_INITIALIZER;
    //пустой первый элемент
    pthread_mutex_lock(&mQueueMutex);
    pdleFirst = NULL;
    pthread_mutex_unlock(&mQueueMutex);
}

DescriptorsList::DescriptorsList(SomeDirectory *in_psdRootDirectory)
{
    //инициализация блокировки списка директорий
    mQueueMutex = PTHREAD_MUTEX_INITIALIZER;
    //создаём список директорий с первым элементом
    pthread_mutex_lock(&mQueueMutex);
    pdleFirst = new DirListElement(in_psdRootDirectory, NULL);
    pthread_mutex_unlock(&mQueueMutex);
}

DescriptorsList::DescriptorsList(FileData *in_pfdData)
{
    //инициализация блокировки списка директорий
    mQueueMutex = PTHREAD_MUTEX_INITIALIZER;
    //создаём список директорий с первым элементом
    pthread_mutex_lock(&mQueueMutex);
    pdleFirst = new DirListElement(in_pfdData, NULL);
    pthread_mutex_unlock(&mQueueMutex);
}

DescriptorsList::~DescriptorsList()
{
    //удаляем ссписок директорий
    DirListElement *pdleList, *pdleDel;
    if(pdleFirst == NULL)
	return;
    pthread_mutex_lock(&mQueueMutex);
    pdleList = pdleFirst->pdleNext;
    while(pdleList != NULL)
    {
	pdleDel = pdleList;
	pdleList = pdleList->pdleNext;
	delete pdleDel;
    }
    delete pdleFirst;
    pdleFirst = NULL;
    pthread_mutex_unlock(&mQueueMutex);
}

void DescriptorsList::AddQueueElement(SomeDirectory const * const in_psdPtr)
{
    //добавляем элемент списка
    //...
}

void DescriptorsList::SubQueueElement(SomeDirectory const * const in_psdPtr)
{
    //удаляем элемент из очереди
    //...
}

void DescriptorsList::SubQueueElement(int in_nDirFd)
{
    //удаляем элемент из очереди
    //...
}


/*********************************DirListElement*********************************/

DirListElement::DirListElement()
{
    psdDirectory = NULL;
    pdleNext = NULL;
    pdlePrev = NULL;
}

//!!!Возможно, надо сделать в RootMonitor ссылку на SomeDirectory вместо наследования
//!!!для избежания создания ещё одной копии описания директории в этом конструкторе
//!!!т.к. создаётся ещё она копия слепка одной и той же директории
DirListElement::DirListElement(SomeDirectory *in_psdDirectory, DirListElement * const in_pdlePrev)
{
    psdDirectory = in_psdDirectory;
    //исключаем выпадение части элементов списка
    if(in_pdlePrev != NULL)
    {
        pdleNext = in_pdlePrev->pdleNext;
	//если за предшествующим элементом есть последующий, меняем у последующего предыдущий на this
        if(in_pdlePrev->pdleNext != NULL)
	    in_pdlePrev->pdleNext->pdlePrev = this;
	in_pdlePrev->pdleNext = this;
    }
    else
    {
	pdleNext = NULL;
    }

    if(in_psdDirectory->IsSnapshotNeeded()) //снимок обязан присутствовать в элементе очереди
	in_psdDirectory->GetSnapshot();

    //а вот тут и нужно подключить обработчик сигнала (когда слепок уже создан)
    //...
}

DirListElement::DirListElement(FileData *in_pfdData, DirListElement * const in_pdlePrev)
{
    psdDirectory = new SomeDirectory(in_pfdData, true); //снимок обязан присутствовать в элементе очереди
    //исключаем выпадение части элементов списка
    if(in_pdlePrev != NULL)
    {
        pdleNext = in_pdlePrev->pdleNext;
	//если за предшествующим элементом есть последующий, меняем у последующего предыдущий на this
        if(in_pdlePrev->pdleNext != NULL)
	    in_pdlePrev->pdleNext->pdlePrev = this;
	in_pdlePrev->pdleNext = this;
    }
    else
    {
	pdleNext = NULL;
    }

    //самое место для подключения обработчика сигнала (когда уже точно есть слепок)
    //...
}

DirListElement::~DirListElement()
{
    //возможно, придётся тут удалять FileData
    //...
}
