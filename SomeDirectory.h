//SomeDirectory.h
//Author: Bombo
//12.01.2017
//Класс SomeDirectory отвечает за один отдельный проект для наблюдения

#pragma once

#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

class SomeDirectory
{
    char *pDirName; //путь к директории
    char *pSafeDirName; //возвращаемый путь
    int nDirFd; //дескриптор директории

        
public:
    SomeDirectory();
    SomeDirectory(char const * const in_pDirName);
    ~SomeDirectory();

    int GetDirFd(); //получить дескриптор директории
    char *GetDirName(); //получить путь к директории

    int SetDirName(char const * const in_pNewDirName); //сменить путь к директории
};
