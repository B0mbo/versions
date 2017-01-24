//RootMonitor.h
//Author: Bombo
//12.01.2017
//Класс RootMonitor отвечает за один отдельный проект для наблюдения

#pragma once

#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>
#include<sys/types.h>

#include"DescriptorsQueue.h"
#include"DescriptorsList.h"
#include"SomeDirectory.h"
#include"DirSnapshot.h"

//все потоки обработчиков дескрипторов обращаются только к объекту этого класса (?)
class RootMonitor
{
    //корневая директория отслеживаемого проекта
    SomeDirectory *psdRootDirectory;

public:
    //список всех дескрипторов открытых директорий отслеживаемого проекта
    static DescriptorsList *pdlList;
    //список дескрипторов, ожидающих обработки
    static DescriptorsQueue *pdqQueue;

    //блокировки доступа к списку директорий и очереди декрипторов на обработку
    static pthread_mutex_t mDescListMutex;
    static pthread_mutex_t mDescQueueMutex;
    //блокировка потока обработчика списка найденных директорий
    static pthread_mutex_t mDirThreadMutex;
    //блокировка потока обработки очереди дескрипторов
    static pthread_mutex_t mDescThreadMutex;

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
