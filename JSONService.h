//JSONService.h
//Author: Bombo
//07.02.2017
//Класс JSONService обеспечивает формирование JSON запросов на обновление записей в базе данных.

#pragma once

#include<time.h>
#include<stdio.h>
#include<pthread.h>

#include"DirSnapshot.h"

enum ServiceType {NOT_READY = -1, NO_SERVICE = 0, INIT_SERVICE, CURRENT_SERVICE}; //тип очереди на отправку

//запись одного изменения
//при начальной инициализации списка файлов они вкладываются друг в друга, образуя дерево
//при дальнейших изменениях в файлах они передаются одним длинным списком
struct FSChange
{
  FileData *pfdFile; //ссылка на описание файла, который изменился (может быть NULL, если удалён)
  ResultOfCompare rocEvent; //событие (IS_EQUAL в случае первой инициализации)
  int nType; //тип файла
  ino_t itInode; //inode текущего файла
  time_t ttTime; //метка времени изменения
  char *pszChanges; //текстовое описание изменений в формате JSON

  FSChange *pfscNext; //ссылка на описание изменения в следующем файле

  FSChange();
  FSChange(ServiceType in_stType, FileData * const in_pfdFile, FileData const * const in_pfdParent, ResultOfCompare in_rocEvent, FSChange * const in_pfscPrev);
  ~FSChange();
};

//объект этого класса представляет собой список всех изменений в ФС, необходимых к отправке на сервер
class JSONService
{
  ServiceType stType; //тип, по которому ипределяется, что формировать - дерево, или список
  unsigned long ulSessionNumber; //номер сессии
  FSChange *pfscFirst; //ссылка на первый элемент в списке изменений

  JSONService *pjsNext; //сылка на следующий список изменений на отправку
  
  pthread_mutex_t mJSONServiceMutex; //блокировка доступа к списку

public:
  JSONService();
  JSONService(ServiceType in_stType, unsigned long in_ulSessionNumber);
  ~JSONService();

  //добавить изменения в список
  void AddChange(ServiceType in_stType, FileData * const in_pfdFile, FileData const * const in_pfdParent, ResultOfCompare in_rocEvent);
  //заменить уже существующую запись об изменениях в списке
  void UpdateChange(ServiceType in_stType, FileData * const in_pfdFile, ResultOfCompare in_rocEvent);
  char * const GetJSON(void); //получить список в формате JSON (полученную строку надо удалить после использования)

  unsigned long GetSessionNumber(void); //получить номер сессии сервиса
  ServiceType GetType(void); //получить тип списка

  JSONService * const GetNext(void); //получить ссылку на следующий сервис
  void SetNext(JSONService * const in_pjsNext); //инициализировать ссылку на следующий сервис

  void PrintService(void); //вывести содержимое списка
};
