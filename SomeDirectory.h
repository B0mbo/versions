//SomeDirectory.h
//Author: Bombo
//12.01.2017
//Класс SomeDirectory хранит данные директории

#pragma once

#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

#include"DirSnapshot.h"

class SomeDirectory;

class SomeDirectory
{
    FileData *pfdData; //ссылка на полное описание файла
    SomeDirectory *pfdParent; //ссылка на директорию-владелец (для поиска полного пути)
    DirSnapshot *pdsSnapshot; //слепок текущей директории

public:
    SomeDirectory();
    //этот конструктор используется только классом RootMonitor,
    //поскольку он будет хранить ссылку на созданный объект
    SomeDirectory(char const * const in_pName, SomeDirectory * const in_pfdParent);
    SomeDirectory(FileData *in_pfdData, SomeDirectory * const in_pfdParent, bool in_fGetSnapshot);
    ~SomeDirectory();

    int GetDirFd(void); //получить дескриптор директории
    char *GetDirName(void); //получить путь к директории
    void GetSnapshot(void);
    bool IsSnapshotNeeded(void);

    int SetDirName(char const * const in_pNewDirName); //сменить путь к директории
};
