//DescriptorsList.h
//Author: Bombo
//13.01.2017
//Класс DescriptorsList содержит список всех открытых дескрипторов

#pragma once

#include<stdio.h>
#include<fcntl.h>
#include<signal.h>
#include<pthread.h>

#include"DirSnapshot.h"
#include"SomeDirectory.h"

struct DirListElement
{
    SomeDirectory *psdDirectory;
    DirListElement *pdleNext;
    DirListElement *pdlePrev;

    DirListElement();
    DirListElement(SomeDirectory *in_psdDirectory, DirListElement * const in_pdlePrev);
    DirListElement(FileData *in_pfdData, SomeDirectory * const in_psdParent, DirListElement * const in_pdlePrev);
    ~DirListElement();
};

class DescriptorsList
{
    DirListElement *pdleFirst;
    //блокировка доступа к списку директорий
    pthread_mutex_t mListMutex;
public:
    DescriptorsList();
    //для корневого каталога (возможно, тут нужен in_fSetSigHandler)
    DescriptorsList(SomeDirectory *in_psdRootDirectory);
    //директория не обязана быть открытой перед помещением в очередь
    DescriptorsList(FileData *in_psdDir);
    ~DescriptorsList();

    void AddQueueElement(SomeDirectory * const in_psdPtr);
    void SubQueueElement(SomeDirectory const * const in_psdPtr);
    void SubQueueElement(int in_nDirFd);

    void UpdateList(void);

    int GetFd(void); //отладка!!!
    void PrintList(void); //отладка!!!

    SomeDirectory *GetDirectory(int in_nDirFd);
};
