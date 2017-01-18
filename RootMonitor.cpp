//RootMonitor.cpp
//Author: Bombo
//12.01.2017
//Класс RootMonitor отвечает за один отдельный проект для наблюдения

#include"RootMonitor.h"

RootMonitor::RootMonitor()
{
    psdRootDirectory = NULL;
    pdlQueue = NULL;
}

RootMonitor::RootMonitor(char * const pRootPath)
{
    if(pRootPath == NULL)
    {
	psdRootDirectory = NULL;
	pdlQueue = NULL;
	return;
    }
    psdRootDirectory = new SomeDirectory(pRootPath, NULL);
    pdlQueue = new DescriptorsList(psdRootDirectory);
}

RootMonitor::RootMonitor(FileData * const in_pfdData)
{
    if(in_pfdData == NULL)
    {
	psdRootDirectory = NULL;
	pdlQueue = NULL;
	return;
    }

    psdRootDirectory = new SomeDirectory(in_pfdData, NULL, true);
    pdlQueue = new DescriptorsList(psdRootDirectory);
}

RootMonitor::RootMonitor(SomeDirectory * const in_psdRootDirectory)
{
    if(in_psdRootDirectory == NULL)
    {
	psdRootDirectory = NULL;
	pdlQueue = NULL;
	return;
    }

    psdRootDirectory = in_psdRootDirectory;
    pdlQueue = new DescriptorsList(in_psdRootDirectory);
}

RootMonitor::~RootMonitor()
{
    if(pdlQueue != NULL)
        delete pdlQueue;
    if(psdRootDirectory != NULL)
        delete psdRootDirectory;
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
