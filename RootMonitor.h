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

#include"SomeDirectory.h"
#include"DirSnapshot.h"

class RootMonitor : public SomeDirectory
{
public:
    RootMonitor() : SomeDirectory() {};
    RootMonitor(char *in_pRootPath) : SomeDirectory(in_pRootPath) {};
    ~RootMonitor();

    int SetRootPath(char const * const SetNewRootPath); //сменить путь к проекту
};
