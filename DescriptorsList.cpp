//DescriptorsList.cpp
//Author: Bombo
//13.01.2017
//Класс DescriptorsList содержит список всех открытых дескрипторов

#include"DescriptorsList.h"

/*********************************DescriptorQueue*********************************/

DescriptorsList::DescriptorsList()
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    //пустой первый элемент
    pthread_mutex_lock(&mListMutex);
    pdleFirst = NULL;
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::DescriptorsList(SomeDirectory *in_psdRootDirectory)
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_unlock(&mListMutex);
    //создаём список директорий с первым элементом
    pthread_mutex_lock(&mListMutex);
    pdleFirst = new DirListElement(in_psdRootDirectory, NULL);
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::DescriptorsList(FileData *in_pfdData)
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_unlock(&mListMutex);
    //создаём список директорий с первым элементом
    pthread_mutex_lock(&mListMutex);
    pdleFirst = new DirListElement(in_pfdData, NULL, NULL);
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::~DescriptorsList()
{
    //удаляем ссписок директорий
    DirListElement *pdleList, *pdleDel;
    if(pdleFirst == NULL)
	return;
    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst->pdleNext;
    while(pdleList != NULL)
    {
	pdleDel = pdleList;
	pdleList = pdleList->pdleNext;
	delete pdleDel;
    }
    delete pdleFirst;
    pdleFirst = NULL;
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::AddQueueElement(SomeDirectory * const in_psdPtr)
{
    DirListElement *pdleList;

    if(in_psdPtr == NULL)
    {
//	fprintf(stderr, "error in_psdPtr\n"); //отладка!!!
	return;
    }

    //добавляем элемент списка
    if(pdleFirst == NULL)
    {
	pthread_mutex_lock(&mListMutex);
	//если список пуст - назначаем первый элемент
	pdleFirst = new DirListElement(in_psdPtr, NULL);
	pthread_mutex_unlock(&mListMutex);
	return;
    }

    pthread_mutex_lock(&mListMutex);
    //ищем конец списка
    pdleList = pdleFirst;
    while(pdleList->pdleNext != NULL)
    {
	pdleList = pdleList->pdleNext;
    }

    //добавляем директорию в конец списка
    pdleList->pdleNext = new DirListElement(in_psdPtr, pdleList);
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::SubQueueElement(SomeDirectory const * const in_psdPtr)
{
    DirListElement *pdleList;

    //если список пуст - выходим
    if(pdleFirst == NULL)
	return;

    pthread_mutex_lock(&mListMutex);
    //ищем заданный элемент
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
	if(pdleList->psdDirectory == in_psdPtr)
	{
	    //удаляем найденный элемент из списка
	    delete pdleList;
	    pthread_mutex_unlock(&mListMutex);
	    return;
	}
    }
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::SubQueueElement(int in_nDirFd)
{
    DirListElement *pdleList;

    //если список пуст - выходим
    if(pdleFirst == NULL)
	return;

    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
	if(pdleList->psdDirectory->GetDirFd() == in_nDirFd)
	{
	    //удаляем элемент из очереди
	    delete pdleList;
	    pthread_mutex_unlock(&mListMutex);
	    return;
	}
    }
    pthread_mutex_unlock(&mListMutex);
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
    pdlePrev = in_pdlePrev;

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

    //рекурсия (!)
    //надо создание слепка повесить на поток обработчика списка директорий
//    if(in_psdDirectory->IsSnapshotNeeded()) //снимок обязан присутствовать в элементе очереди
//	in_psdDirectory->MakeSnapshot();

    //а вот тут и нужно подключить обработчик сигнала (когда слепок уже создан)
    //...

    //освобождение мьютекса обработчика очереди потока (?) возможно, лучше освобождать после создания слепка
    //...
}

DirListElement::DirListElement(FileData *in_pfdData, SomeDirectory * const in_psdParent, DirListElement * const in_pdlePrev)
{
    //осторожно! Родительский каталок не ищется! Надо делать! Краш!!!
    psdDirectory = new SomeDirectory(in_pfdData, in_psdParent, true); //снимок обязан присутствовать в элементе очереди
    pdlePrev = in_pdlePrev;
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
