//JSONService.h
//Author: Bombo
//07.02.2017
//Класс JSONService обеспечивает формирование JSON запросов на обновление записей в базе данных.

#include"JSONService.h"

/***********************************JSONService*************************************/

JSONService::JSONService()
{
  stType = NOT_READY;
  ulSessionNumber = -1;
  pfscFirst = NULL;
  pjsNext = NULL;
  mJSONServiceMutex = PTHREAD_MUTEX_INITIALIZER;
}

JSONService::JSONService(ServiceType in_stType, unsigned long in_ulSessionNumber)
{
  stType = in_stType;
  ulSessionNumber = in_ulSessionNumber;
  pfscFirst = NULL;
  pjsNext = NULL;
  mJSONServiceMutex = PTHREAD_MUTEX_INITIALIZER;
}

JSONService::~JSONService()
{
  FSChange *pfscList, *pfscDel;

  //возможно, при удалении имеет смысл попытаться отправить список через сокет (?)
  pthread_mutex_lock(&mJSONServiceMutex);
  pfscList = pfscDel = pfscFirst;
  while(pfscList != NULL)
  {
    pfscList = pfscList->pfscNext;
    delete pfscDel;
    pfscDel = pfscList;
  }
  pthread_mutex_unlock(&mJSONServiceMutex);
  
  pthread_mutex_destroy(&mJSONServiceMutex);
}

char * const JSONService::GetJSON(void)
{
  char *pszRet = NULL;
  size_t stLen;
  FSChange *pfscList, *pfscLast;

  if(pfscFirst == NULL)
    return NULL;

  pthread_mutex_lock(&mJSONServiceMutex);
  //подсчёт размера
  stLen = 0L;
  pfscList = pfscFirst;
  while(pfscList != NULL)
  {
    if(pfscList->rocEvent == IS_EMPTY || pfscList->pszChanges == NULL)
    {
      pfscList = pfscList->pfscNext;
      continue;
    }
    if(pfscList->pszChanges != NULL)
      stLen = stLen + strlen(pfscList->pszChanges) + 1; //+1 на запятую
    else
    {
      stLen++;
      //если это начальная инициализация
      if(stType == INIT_SERVICE && pfscList->rocEvent == DIRECTORY_END)
      {
	fprintf(stderr, "JSONService::GetJSON() error: wrong JSON format!!!\n");
      }
    }
    pfscList = pfscList->pfscNext;
  }
  stLen++; //для '\0' в конце

  //создаём возвращаемую строку
  pszRet = new char[stLen+1];
  memset(pszRet, 0, stLen+1);

  //формируем окончательный JSON запрос
  pfscList = pfscLast = pfscFirst;
  while(pfscList != NULL)
  {
    if(pfscList->pszChanges != NULL)
    {
      if( pfscList->rocEvent != DIRECTORY_END &&
	  pfscLast->nType != IS_DIRECTORY &&
	  pfscLast->rocEvent != INIT_PROJECT &&
	  pfscList != pfscFirst )
	strncat(pszRet, ",", stLen);
      strncat(pszRet, pfscList->pszChanges, stLen);
    }
    pfscLast = pfscList;
    pfscList = pfscList->pfscNext;
  }
  pthread_mutex_unlock(&mJSONServiceMutex);
  return pszRet;
}

//добавить в список запись об изменении файла
void JSONService::AddChange(ServiceType in_stType, FileData * const in_pfdFile, ResultOfCompare in_rocEvent, ino_t in_itParentInode)
{
  FSChange *pfscList;

  if(in_pfdFile == NULL || in_pfdFile->pName == NULL)
    return;

  pthread_mutex_lock(&mJSONServiceMutex);
  if(pfscFirst == NULL)
  {
    pfscFirst = new FSChange(in_stType, in_pfdFile, in_rocEvent, NULL);
    pthread_mutex_unlock(&mJSONServiceMutex);
    return;
  }

  //ищем нужный элемент списка
  pfscList = pfscFirst;
  while(pfscList != NULL)
  {
    if(in_stType == INIT_SERVICE && pfscList->pfdFile != NULL)
    {
      if((pfscList->pfdFile->stData.st_ino) == in_itParentInode)
	break;
    }
    else
    {
      if(pfscList->pfscNext == NULL)
	break;
    }
    pfscList = pfscList->pfscNext;
  }
  //если нужная директория не найдена - выходим
  if(pfscList == NULL)
  {
    pthread_mutex_unlock(&mJSONServiceMutex);
    return;
  }

  //добавляем событие в список
  new FSChange(in_stType, in_pfdFile, in_rocEvent, pfscList);
  pthread_mutex_unlock(&mJSONServiceMutex);
}

unsigned long JSONService::GetSessionNumber(void)
{
  return ulSessionNumber;
}

ServiceType JSONService::GetType(void)
{
  return stType;
}

//вернуть следующий список
JSONService * const JSONService::GetNext(void)
{
  return pjsNext;
}

void JSONService::SetNext(JSONService * const in_pjsNext)
{
  pjsNext = in_pjsNext;
}

//вывести содержимое списка
void JSONService::PrintService(void)
{
  FSChange *pfscList;
  
  pfscList = pfscFirst;
  while(pfscList != NULL)
  {
    fprintf(stderr, "Session: %5ld, pfdFile=%ld,rocEvent=%d,itInode=%ld,ttTime=%ld,pszChanges=%s,pfscNext=%ld\n",
	    ulSessionNumber,
	    (unsigned long)pfscList->pfdFile,
	    pfscList->rocEvent,
	    (unsigned long)pfscList->itInode,
	    pfscList->ttTime,
	    pfscList->pszChanges,
	    (unsigned long)pfscList->pfscNext);
    pfscList = pfscList->pfscNext;
  }
}

/************************************FSChange***************************************/

FSChange::FSChange()
{
  pfdFile = NULL;
  rocEvent = IS_EMPTY;
  nType = IS_NOTAFILE;
  itInode = 0;
  ttTime = 0;
  pszChanges = NULL;

  pfscNext = NULL;
}

//перед добавлением записи необходимо отыскать место, куда её вставить
FSChange::FSChange(ServiceType in_stType, FileData * const in_pfdFile, ResultOfCompare in_rocEvent, FSChange * const in_pfscPrev)
{
  char szBuff[2048]; //буфер под запись
  char szType[32], szEvent[32], szCrc[32];
  size_t stLen;

  if(rocEvent == IS_DELETED)
    pfdFile = NULL;
  else
    pfdFile = in_pfdFile;

  if(in_pfdFile == NULL || in_pfdFile->pName == NULL)
  {
    rocEvent = IS_EMPTY;
    nType = IS_NOTAFILE;
    itInode = 0;
    ttTime = 0;
    pszChanges = NULL;

    pfscNext = NULL;
  }
  rocEvent = in_rocEvent;
  nType = in_pfdFile->nType;
  itInode = in_pfdFile->stData.st_ino;
  ttTime = time(NULL);

  //непосредственно создание записи в формате JSON
  memset(szBuff, 0, sizeof(szBuff));
  memset(szType, 0, sizeof(szType));
  memset(szCrc, 0, sizeof(szCrc));
  switch(in_pfdFile->nType)
  {
    case IS_FILE:
      strncpy(szType, "file", sizeof(szType));
      if(in_rocEvent != IS_DELETED)
	snprintf(szCrc, sizeof(szCrc), ",\"crc\":\"%ld\"", in_pfdFile->ulCrc);
      break;
    case IS_DIRECTORY:
      strncpy(szType, "folder", sizeof(szType));
      break;
    case IS_LINK:
      strncpy(szType, "link", sizeof(szType));
      if(in_rocEvent != IS_DELETED)
	snprintf(szCrc, sizeof(szCrc), ",\"crc\":\"%ld\"", in_pfdFile->ulCrc);
      break;
  }

  switch(in_rocEvent)
  {
    case IS_EQUAL:
      strncpy(szEvent, "IS_EQUAL", sizeof(szEvent));
      break;
    case IS_CREATED:
      strncpy(szEvent, "IS_CREATED", sizeof(szEvent));
      break;
    case IS_DELETED:
      strncpy(szEvent, "IS_DELETED", sizeof(szEvent));
      break;
    case NEW_NAME:
      strncpy(szEvent, "NEW_NAME", sizeof(szEvent));
      break;
    case NEW_TIME:
      strncpy(szEvent, "NEW_TIME", sizeof(szEvent));
      break;
    case NEW_HASH:
      strncpy(szEvent, "NEW_HASH", sizeof(szEvent));
      break;
    case INIT_PROJECT:
      strncpy(szEvent, "INIT_PROJECT", sizeof(szEvent));
      break;
    default:
      strncpy(szEvent, "IS_EQUAL", sizeof(szEvent));
  }

  snprintf(szBuff, sizeof(szBuff)-1, "{\"type\":\"%s\",\"event\":\"%s\",\"name\":\"%s\",\"inode\":\"%ld\",\"time\":\"%ld\"%s%s",
				     szType,
				     szEvent,
				     in_pfdFile->pName,
				     itInode,
				     ttTime,
				     szCrc,
				     (in_pfdFile->nType==IS_DIRECTORY && in_stType == INIT_SERVICE)?",\"content\":[":"}");
  
  stLen = strlen(szBuff);
  pszChanges = new char[stLen + 1];
  memset(pszChanges, 0, stLen + 1);
  //формируем начало записи
  strncpy(pszChanges, szBuff, stLen);

  if(in_stType == INIT_SERVICE && in_pfdFile->nType == IS_DIRECTORY)
  {
    pfscNext = new FSChange();
    if(in_pfscPrev != NULL)
    {
      pfscNext->pfscNext = in_pfscPrev->pfscNext;
      in_pfscPrev->pfscNext = this;
    }
    pfscNext->rocEvent = DIRECTORY_END;
    memset(szBuff, 0, sizeof(szBuff));
    strncpy(szBuff, "]}", sizeof(szBuff)-1);
    pfscNext->pszChanges = new char[stLen + 1];
    memset(pfscNext->pszChanges, 0, stLen);
    //формируем конец записи о директории
    strncpy(pfscNext->pszChanges, szBuff, stLen);
  }
  else
  {
    if(in_pfscPrev != NULL)
    {
      pfscNext = in_pfscPrev->pfscNext;
      in_pfscPrev->pfscNext = this;
    }
    else
      pfscNext = NULL;
  }
//   fprintf(stderr, "FSChange::FSChange() : %s\n", pszChanges); //отладка!!!
}

FSChange::~FSChange()
{
  pfdFile = NULL;
  rocEvent = IS_EMPTY;
  nType = IS_NOTAFILE;
  itInode = 0;
  ttTime = 0;
  if(pszChanges != NULL)
    delete [] pszChanges;
  pfscNext = NULL;
}
