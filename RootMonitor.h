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

#include"DescriptorsQueue.h"
#include"DescriptorsList.h"
#include"SomeDirectory.h"
#include"DirSnapshot.h"

//все потоки обработчиков дескрипторов обращаются только к объекту этого класса
class RootMonitor
{
    //корневая директория отслеживаемого проекта
    SomeDirectory *psdRootDirectory;
    //список всех дескрипторов открытых директорий отслеживаемого проекта
    DescriptorsList *pdlList;
    //список дескрипторов, ожидающих обработки
    DescriptorsQueue *pdqQueue;
public:
    RootMonitor();
    RootMonitor(char * const in_pRootPath);
    RootMonitor(FileData * const in_pfdData);
    RootMonitor(SomeDirectory * const in_psdRootDirectory);
    ~RootMonitor();

    //основные функции, требуемые потоками обработки дескрипторов
    //такие как обновление списка директорий, дерева файлов,
    //создание временных слепков для сравнения, само сравнение
    //...

    int SetRootPath(char const * const SetNewRootPath); //сменить путь к проекту
};
