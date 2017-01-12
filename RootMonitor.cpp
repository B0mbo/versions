//RootMonitor.cpp
//Author: Bombo
//12.01.2017
//Класс RootMonitor отвечает за один отдельный проект для наблюдения

#include"RootMonitor.h"

RootMonitor::RootMonitor()
{
    pRootPath = NULL;
    pSafeRootPath = NULL;
    nRootFd = -1;
}

RootMonitor::~RootMonitor()
{
    if(pRootPath != NULL)
        delete [] pRootPath;
    if(pSafeRootPath != NULL)
        delete [] pSafeRootPath;
}

RootMonitor::RootMonitor(char const * const in_pRootPath)
{
    size_t szLen;
    
    //если путь не указан или пустой
    if(in_pRootPath == NULL || (szLen = strlen(in_pRootPath)) <= 0 )
    {
        pRootPath = NULL;
        pSafeRootPath = NULL;
	nRootFd = -1;
	return;
    }

    if((nRootFd = open(in_pRootPath, O_RDONLY)) < 0)
    {
        //если директория не найдена или не может быть открыта
        return;
    }    

    pRootPath = new char[szLen+1];
    memset(pRootPath, 0, szLen+1);
    //копируем путь к корневой директории отслеживаемого проекта
    strncpy(pRootPath, in_pRootPath, szLen);

    pSafeRootPath = new char[szLen+1];
    memset(pSafeRootPath, 0, szLen+1);
    //копируем путь к корневой директории отслеживаемого проекта
    strncpy(pSafeRootPath, in_pRootPath, szLen);
}

//получить девкриптор корневой директории
int RootMonitor::GetRootFd()
{
    return nRootFd;
}

//получить путь к корневой директории
char *RootMonitor::GetRootPath()
{
    return pSafeRootPath;
}

//поменять/установить путь к корневой директории
int RootMonitor::SetRootPath(char const * const in_pNewRootPath)
{
    size_t szLen;
    int nNewRootFd;
    
    //если путь указан неверно
    if(in_pNewRootPath == NULL || (szLen = strlen(in_pNewRootPath)) <= 0)
    {
	return -1;
    }
    
    if((nNewRootFd = open(in_pNewRootPath, O_RDONLY)) < 0)
    {
	return -2;
    }

    //сюда бы добавить остановку всех прикрученных потоков
    //...
    
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
    pRootPath = new char[szLen+1];
    memset(pRootPath, 0, szLen+1);
    strncpy(pRootPath, in_pNewRootPath, szLen);
    //копируем путь
    pSafeRootPath = new char[szLen+1];
    memset(pSafeRootPath, 0, szLen+1);
    strncpy(pSafeRootPath, in_pNewRootPath, szLen);

    //тут надо бы возобновить потоки
    //...
    
    return 0;
}
