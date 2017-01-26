//RootMonitor.cpp
//Author: Bombo
//12.01.2017
//Класс RootMonitor отвечает за один отдельный проект для наблюдения

#include"RootMonitor.h"

DescriptorsList *RootMonitor::pdlList = NULL;
DescriptorsQueue *RootMonitor::pdqQueue = NULL;
pthread_mutex_t RootMonitor::mDescListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RootMonitor::mDescQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RootMonitor::mDirThreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RootMonitor::mDescThreadMutex = PTHREAD_MUTEX_INITIALIZER;

RootMonitor::RootMonitor()
{
    psdRootDirectory = NULL;
    pdlList = NULL;
}

RootMonitor::RootMonitor(char * const pRootPath)
{
    if(pRootPath == NULL)
    {
	psdRootDirectory = NULL;
	pdlList = NULL;
	return;
    }

    if(pdqQueue == NULL)
	pdqQueue = new DescriptorsQueue();

    //создаём описание корневой директории
    psdRootDirectory = new SomeDirectory(pRootPath, NULL);
    //открываем корневую директорию и добавляем полученный дескриптор в список открытых
    //этот список существует для упрощения поиска директории по её дескриптору
    if(pdlList == NULL)
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList = new DescriptorsList(psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
    else
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList->AddQueueElement(psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
}

RootMonitor::RootMonitor(FileData * const in_pfdData)
{
    if(in_pfdData == NULL)
    {
	psdRootDirectory = NULL;
	pdlList = NULL;
	return;
    }

    if(pdqQueue == NULL)
	pdqQueue = new DescriptorsQueue();

    //создаём описание корневой дирекстории
    psdRootDirectory = new SomeDirectory(in_pfdData, NULL, true);
    //открываем корневую директорию и добавляем полученный дескриптор в список открытых
    if(pdlList == NULL)
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList = new DescriptorsList(psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
    else
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList->AddQueueElement(psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
}

RootMonitor::RootMonitor(SomeDirectory * const in_psdRootDirectory)
{
    if(in_psdRootDirectory == NULL)
    {
	psdRootDirectory = NULL;
	pdlList = NULL;
	return;
    }

    if(pdqQueue == NULL)
	pdqQueue = new DescriptorsQueue();

    //инициализируем ссылку на описание корневой директории отслеживаемого проекта
    psdRootDirectory = in_psdRootDirectory;
    //открываем корневую директорию и добавляем полученный дескриптор в список открытых
    if(pdlList == NULL)
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList = new DescriptorsList(in_psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
    else
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList->AddQueueElement(in_psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
}

RootMonitor::~RootMonitor()
{
    //т.к. эти данные теперь статические, их не следует удалять
//    if(pdlList != NULL)
//        delete pdlList;
//    if(psdRootDirectory != NULL)
//        delete psdRootDirectory;
}

//поменять/установить путь к корневой директории
int RootMonitor::SetRootPath(char const * const in_pNewRootPath)
{
    //останавливаем сопровождающие потоки
    //...

//    SetDirName(in_pNewRootPath); //функцию необходимо переделать, поэтому закомментировано

    //запускаем потоки
    //...

/*
    size_t stLen;
    int nNewRootFd;

    //если путь указан неверно
    if(in_pNewRootPath == NULL || (stLen = strlen(in_pNewRootPath)) <= 0)
    {
	return -1;
    }

    if((nNewRootFd = open(in_pNewRootPath, O_RDONLY)) < 0)
    {
	return -2;
    }

    //закрываем имеющийся дескриптор (если он открыт)
    if(nRootFd >= 0)
        close(nRootFd);
    //удаляем прежний путь
    if(pRootPath != NULL)
	delete [] pRootPath;
    //удаляем копию пути
    if(pSafeRootPath != NULL)
	delete [] pSafeRootPath;

    //задаём дескриптор
    nRootFd = nNewRootFd;
    //создаём новый путь
    pRootPath = new char[stLen+1];
    memset(pRootPath, 0, stLen+1);
    strncpy(pRootPath, in_pNewRootPath, stLen);
    //копируем путь
    pSafeRootPath = new char[stLen+1];
    memset(pSafeRootPath, 0, stLen+1);
    strncpy(pSafeRootPath, in_pNewRootPath, stLen);
*/
    return 0;
}
