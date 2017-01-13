//SomeDirectory.cpp
//Author: Bombo
//12.01.2017
//Класс SomeDirectory хранит данные директории

#include"SomeDirectory.h"

SomeDirectory::SomeDirectory()
{
    pDirName = NULL;
    pSafeDirName = NULL;
    nDirFd = -1;
}

SomeDirectory::~SomeDirectory()
{
    if(pDirName != NULL)
        delete [] pDirName;
    if(pSafeDirName != NULL)
        delete [] pSafeDirName;
}

SomeDirectory::SomeDirectory(char const * const in_pDirName)
{
    size_t stLen;
    
    //если путь не указан или пустой
    if(in_pDirName == NULL || (stLen = strlen(in_pDirName)) <= 0 )
    {
        pDirName = NULL;
        pSafeDirName = NULL;
	nDirFd = -1;
	return;
    }

    if((nDirFd = open(in_pDirName, O_RDONLY)) < 0)
    {
        //если директория не найдена или не может быть открыта
        return;
    }    

    pDirName = new char[stLen+1];
    memset(pDirName, 0, stLen+1);
    //копируем путь к директории
    strncpy(pDirName, in_pDirName, stLen);

    pSafeDirName = new char[stLen+1];
    memset(pSafeDirName, 0, stLen+1);
    //копируем путь к директории
    strncpy(pSafeDirName, in_pDirName, stLen);
}

//получить деcкриптор директории
int SomeDirectory::GetDirFd()
{
    return nDirFd;
}

//получить путь к директории
char *SomeDirectory::GetDirName()
{
    return pSafeDirName;
}

//поменять/установить путь к корневой директории
int SomeDirectory::SetDirName(char const * const in_pNewDirName)
{
    size_t stLen;
    int nNewDirFd;
    
    //если путь указан неверно
    if(in_pNewDirName == NULL || (stLen = strlen(in_pNewDirName)) <= 0)
    {
	return -1;
    }
    
    if((nNewDirFd = open(in_pNewDirName, O_RDONLY)) < 0)
    {
	return -2;
    }

    //закрываем имеющийся дескриптор (если он открыт)
    if(nDirFd >= 0)
        close(nDirFd);
    //удаляем прежний путь
    if(pDirName != NULL)
	delete [] pDirName;
    //удаляем копию пути
    if(pSafeDirName != NULL)
	delete [] pSafeDirName;
    
    //задаём дескриптор
    nDirFd = nNewDirFd;
    //создаём новый путь
    pDirName = new char[stLen+1];
    memset(pDirName, 0, stLen+1);
    strncpy(pDirName, in_pNewDirName, stLen);
    //копируем путь
    pSafeDirName = new char[stLen+1];
    memset(pSafeDirName, 0, stLen+1);
    strncpy(pSafeDirName, in_pNewDirName, stLen);

    return 0;
}
