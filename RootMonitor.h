//RootMonitor.h
//Author: Bombo
//12.01.2017
//Класс RootMonitor отвечает за один отдельный проект для наблюдения

#pragma once

#include<netdb.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>

#include"DescriptorsQueue.h"
#include"DescriptorsList.h"
#include"SomeDirectory.h"
#include"DirSnapshot.h"
#include"JSONService.h"

//все потоки обработчиков дескрипторов обращаются только к объекту этого класса (?)
class RootMonitor
{
    //корневая директория отслеживаемого проекта
    SomeDirectory *psdRootDirectory;

    //список очередей изменений в формате JSON. Отправляются на удалённый сервер строго по порядку! (!)
    JSONService *pjsFirst;

    //номер последнего созданного списка событий
    unsigned long ulLastSessionNumber;
    unsigned long ulRegularSessionNumber;

    char *pszServerURL; //URL-адрес сервера
    struct addrinfo *aiRes; //результат поиска адреса

    pthread_mutex_t mSocketMutex; //блокировка сокета
    pthread_mutex_t mEventsQueueMutex; //блокировка списка событий

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

    //добавить изменение в список
    void AddChange(ServiceType in_stType, unsigned long in_ulSessionNumber, FileData * const in_pfdFile, ResultOfCompare in_rocEvent, ino_t in_itParentInode);
    //добавить изменение в инициализирующий список
    void AddInitChange(FileData * const in_pfdFile, ino_t in_itParentInode);
    //добавить новый список изменений
    void AddJSONService(ServiceType in_stType, unsigned long in_ulSessionNumber);
    //получить готовый запрос в формате JSON
    char * const GetJSON(unsigned long in_ulSessionNumber);

    void SetServerURL(char const * const in_pszServerURL); //задать URL сервера
    char const * const GetServerURL(void); //получить адрес сервера
    int SendData(char * const in_pData, size_t in_stLen, bool in_fDeleteString); //отправить что-либо на сервер
    void SendChangesToServer(void); //отправить изменения на сервер

    unsigned long GetLastSessionNumber(void); //получить номер последнего инициализирующего (?) списка
    unsigned long GetRegularSessionNumber(void); //получить последний номер обычной сессии
    void IncRegularSessionNumber(void); //увеличить номер последней обычной сессии

    int SetRootPath(char const * const SetNewRootPath); //сменить путь к проекту
    void DeleteJSONServices(void); //удалить очереди изменений на отправку
    
    void PrintSession(unsigned long in_ulSessionNumber);
    void PrintServices(void);
};
