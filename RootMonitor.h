//RootMonitor.h
//Author: Bombo
//12.01.2017
//Класс RootMonitor отвечает за один отдельный проект для наблюдения

#pragma once

#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

class RootMonitor
{
    char *pRootPath; //путь к корневой директории
    char *pSafeRootPath; //возвращаемый путь
    int nRootFd; //дескриптор корневой директории
    
public:
    RootMonitor();
    ~RootMonitor();
    
    RootMonitor(char const * const in_pRootPath);
    
    int GetRootFd(); //получить дескриптор корневой директории
    char *GetRootPath(); //получить путь у отслеживаемому проекту
    
    int SetRootPath(char const * const SetNewRootPath); //сменить путь к проекту
};
