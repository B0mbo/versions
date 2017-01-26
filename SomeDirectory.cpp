//SomeDirectory.cpp
//Author: Bombo
//12.01.2017
//Класс SomeDirectory хранит данные директории

#include"SomeDirectory.h"
#include"RootMonitor.h"

SomeDirectory::SomeDirectory()
{
    pfdData = NULL;
    pfdParent = NULL;
    pdsSnapshot = NULL;
}

//этот конструктор автоматически открывает директорию
//и вешает обработчик сигнала на полученный дескриптор
//слепок директории создаётся по умолчанию, т.к. этот конструктор вызывается только
//объектом класса RootMonitor
SomeDirectory::SomeDirectory(char const * const in_pName, SomeDirectory * const in_pfdParent)
{
    size_t stLen;
    char *pPath = NULL;

    pdsSnapshot = NULL;

    //если путь не указан или пустой
    if(in_pName == NULL)
    {
	pfdData = NULL;
	pfdParent = NULL;
	//сюда бы исключение
	//...
	return;
    }

    //создаём описание корневой директории наблюдаемого проекта
   pPath = GetFullPath();
    pfdData = new FileData(in_pName, pPath, NULL, false);
    if(pPath != NULL)
      delete [] pPath;
    pfdParent = in_pfdParent;

    if((pfdData->nDirFd = open(pfdData->pName, O_RDONLY)) < 0)
    {
        //если директория не найдена или не может быть открыта
        return;
    }

    //создаём слепок директории (но не обязательно)
    //...
}

//этот конструктор автоматически открывает директорию
//и вешает обработчик сигнала на полученный дескриптор
//слепок директории создаётся по запросу (?)
SomeDirectory::SomeDirectory(FileData *in_pfdData, SomeDirectory * const in_pfdParent, bool in_fGetSnapshot)
{
    size_t stLen;
    char *pPath = NULL, *pFullPath = NULL;

    pdsSnapshot = NULL;

    //если путь не указан или пустой
    if(in_pfdData->pName == NULL || in_pfdData->nType != IS_DIRECTORY)
    {
	pfdData = NULL;
	pfdParent = NULL;
	return;
    }

    pfdData = in_pfdData;
    if(in_pfdParent == NULL)
    {
	//ищем родительскую директорию своими силами
	//...
	pfdParent = NULL;//заглушка (!)
    }
    else
    {
	pfdParent = in_pfdParent;
    }

    pPath = GetFullPath();
    if(pPath != NULL)
    {
      stLen = strlen(pPath) + strlen(in_pfdData->pName);
      pFullPath = new char[stLen+1];
      memset(pFullPath, 0, stLen+1);
      strncpy(pFullPath, pPath, stLen);
      strncat(pFullPath, "/", stLen);
      strncat(pFullPath, in_pfdData->pName, stLen);
    }

    if(pfdParent != NULL && pPath != NULL)
      in_pfdData->nDirFd = open(pFullPath, O_RDONLY);
    else
      in_pfdData->nDirFd = open(in_pfdData->pName, O_RDONLY);

    if(pPath != NULL)
      delete [] pPath;
    if(pFullPath != NULL)
      delete [] pFullPath;

    if(pfdParent == NULL && in_pfdData->nDirFd == -1)
    {
	fprintf(stderr, "SomeDirectory::SomeDirectory() невозможно открыть директорию и получить fd: %s\n", in_pfdData->pName); //отладка!!!
	//если директория не найдена или не может быть открыта
	pfdData = NULL;
	pfdParent = NULL;
        return;
    }

    if(in_fGetSnapshot)
    {
	//возможно, имеет смысл тут автоматом добавлять список директорий (?)
	//открываем и назначаем обработчик дескриптора
	//false - не запускать поток обработки списка директорий после создания слепка
	MakeSnapshot(false);
    }
}

SomeDirectory::~SomeDirectory()
{
    delete pdsSnapshot;
    delete pfdData;
}

//получить деcкриптор директории
int SomeDirectory::GetDirFd()
{
    return pfdData->nDirFd;
}

//получить имя директории
char *SomeDirectory::GetDirName()
{
    //возможно, имеет смысл заменить pName на pSafeName
    if(pfdData == NULL)
      return NULL;
    else
      return pfdData->pName;
}

//получить путь к директории
//путь к директории каждый раз получается заново
//это сделано для упрощения переноса каталогов из одной ветки ФС в другую
//т.к. в таком случае достаточно лишь сменить родителя переносимой папки
char *SomeDirectory::GetFullPath(void)
{
    SomeDirectory *psdList;
    char *pcBuff, *pcRetName;
    char *pDirName;
    size_t sPathLength, stRetLen, stNameLen;

    pcBuff = NULL;

    //обнуляем путь
    pcRetName = new char[1];
    memset(pcRetName, 0, sizeof(char));

    psdList = this;
    while(psdList != NULL)
    {
	//очищаем буфер
	if(pcBuff != NULL)
	    delete [] pcBuff;

	//увеличиваем буфер до размеров формируемого пути
	(pcRetName == NULL)
	?stRetLen = 0
	:stRetLen = strlen(pcRetName);

	pDirName = psdList->GetDirName();
	(pDirName == NULL)
	?stNameLen = 0
	:stNameLen = strlen(pDirName);

	sPathLength = stRetLen + stNameLen + 1; //+1 для '/'

	//копируем в буфер новое имя директории и уже полученный участок пути
	pcBuff = new char[sPathLength + 1];
	memset(pcBuff, 0, sPathLength + 1);
	strncpy(pcBuff, (pDirName==NULL)?"":pDirName, sPathLength);

	if(strlen(pcRetName) > 0)
	    strncat(pcBuff, "/", sPathLength); //чтобы на конце пути не было '/'

	strncat(pcBuff, pcRetName, sPathLength);

	//обновляем возвращаемый путь
	delete [] pcRetName;
	pcRetName = new char[sPathLength + 1];
	memset(pcRetName, 0, sPathLength + 1);
	strncpy(pcRetName, pcBuff, sPathLength);

	psdList = psdList->GetParent();
    }
    if(pcBuff != NULL)
	delete [] pcBuff;

    return pcRetName;
}

SomeDirectory *SomeDirectory::GetParent(void)
{
    return pfdParent;
}

FileData *SomeDirectory::GetFileData(void)
{
    return pfdData;
}

void SomeDirectory::MakeSnapshot(bool in_fStartDirThread)
{
    if(pdsSnapshot != NULL)
      delete pdsSnapshot;
    pdsSnapshot = new DirSnapshot((void *) this, true, true);
//    pdsSnapshot->PrintSnapshot(); //отладка!!!
    //освобождение мьютекса потока обработчика списка директорий (?)
    //возможно, имеет смысл вручную запускать поток там, где это нужно,
    //во избежании рекурсии (!)
    if(in_fStartDirThread)
      pthread_mutex_unlock(&(RootMonitor::mDirThreadMutex));
}

//сравнить слепки и обработать результат
//это ключевая функция всей системы мониторинга
void SomeDirectory::CompareSnapshots(void)
{
    char *pPath = NULL;
    FileData *pfdCopy;
    DirSnapshot *pdsRemake;
    ResultOfCompare rocResult;
    SnapshotComparison scResult;
    SomeDirectory *psdNewDirectory;

    if(pdsSnapshot == NULL)
    {
	//если слепка ещё нет, создаём и выходим, т.к. это исключительная ситуация
	//поскольку при вызове этой функции слепок уже должен существовать
	pdsSnapshot = new DirSnapshot((void *) this, true, true);
	return;
    }

    //создаём слепок для сравнения (без хэшей и без добавления в список директорий)
    pdsRemake = new DirSnapshot((void *) this, false, false);

    //производим сравнение
    rocResult = pdsSnapshot->CompareSnapshots(pdsRemake, &scResult);
//     fprintf(stderr, "SomeDirectory::CompareSnapshots() scResult.pfdData->pName: \"%s\"\n", scResult.pfdData->pName); //отладка!!!
    //обрабатываем разницу между новым и старым слепками
    switch(rocResult)
    {
      case NO_SNAPSHOT:
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : No snapshot.\n"); //отладка!!!
	break;
      case IS_EMPTY:
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : Old snapshot is empty.\n"); //отладка!!!
	break;
      case INPUT_IS_EMPTY:
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : Input snapshot is empty.\n"); //отладка!!!
	break;
      case OUTPUT_IS_EMPTY:
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : The result is empty.\n"); //отладка!!!
	break;
      case IS_CREATED:
	//добавляем файл в прежний слепок
	pfdCopy = pdsSnapshot->AddFile(scResult.pfdData, true);
	//если это директория - добавляем в список директорий, вызываем обработчик списка
	//в список директорий файл попадает автоматически при создании слепка
	if(scResult.pfdData->nType==IS_DIRECTORY)
	{
	  //false - не создаём слепок
	  psdNewDirectory = new SomeDirectory(pfdCopy, this, false);
	  pthread_mutex_lock(&(RootMonitor::mDescListMutex));
	  RootMonitor::pdlList->AddQueueElement(psdNewDirectory);
	  pthread_mutex_unlock(&(RootMonitor::mDescListMutex));

	  //запускаем поток обработки списка директорий
	  pthread_mutex_unlock(&(RootMonitor::mDirThreadMutex));
	}
	pPath = GetFullPath();
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : %s \"%s%s%s\" has been created.\n",
			  (scResult.pfdData->nType==IS_DIRECTORY)?"Directory":"File",
			  (pPath==NULL)?"":pPath,
			  (pPath==NULL)?"":"/",
			  scResult.pfdData->pName); //отладка!!!
	if(pPath != NULL)
	  delete [] pPath;
	break;
      case IS_DELETED:
	pPath = GetFullPath();
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : %s \"%s%s%s\" has been deleted.\n",
			  (scResult.pfdData->nType==IS_DIRECTORY)?"Directory":"File",
			  (pPath==NULL)?"":pPath, (pPath==NULL)?"":"/", scResult.pfdData->pName); //отладка!!!
	if(pPath != NULL)
	  delete [] pPath;
	//если это директория - удаляем из списка директорий
	//из слепка он при этом удалится автоматом
	if(scResult.pfdData->nType==IS_DIRECTORY)
	{
	  //удаляем директорию из списка директорий (вместе с описанием файла)
	  pthread_mutex_lock(&(RootMonitor::mDescListMutex));
	  RootMonitor::pdlList->SubQueueElement(scResult.pfdData->nDirFd);
	  pthread_mutex_unlock(&(RootMonitor::mDescListMutex));
	}
	else
	{
	  //удаляем файл из старого слепка
	  pdsSnapshot->SubFile(scResult.pfdData->pName);
	}
	break;
      case NEW_NAME:
	pPath = GetFullPath();
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : Some file has been renamed to \"%s%s%s\".\n", (pPath==NULL)?"":pPath, (pPath==NULL)?"":"/", scResult.pfdData->pName); //отладка!!!
	if(pPath != NULL)
	  delete [] pPath;
	//заменяем pfdData в старом слепке
	//...
	//если это директория - переоткрываем её, вешаем обработчик на новый дескриптор;
	//вызов обработчика списка директорий при этом не требуется (?)
	//...
	break;
      case NEW_TIME:
	pPath = GetFullPath();
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : A time of file \"%s%s%s\" has been changed.\n", (pPath==NULL)?"":pPath, (pPath==NULL)?"":"/", scResult.pfdData->pName); //отладка!!!
	if(pPath != NULL)
	  delete [] pPath;
	//scResult.pfdData содержит данные изменившегося файла. Всё, кроме хэша
	scResult.pfdData->CalcHash();
	//сравниваем новый и старый хэши файлов. Если они разные - заменяем старую
	//структуру pfdData на новую (в слепке директории); старую удаляем.
	//...
	//если же это директория - переоткрываем, вешаем обработчик
	//...
	break;
      case IS_EQUAL:
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : Hash is needed.\n"); //отладка!!!
	delete pdsRemake;
	//создаём слепок с хэшем всех файлов
	pdsRemake = new DirSnapshot((void *) this, true, false);
	//сравниваем файлы по содержимому
	//...
	break;
    }

    delete pdsRemake;
}

bool SomeDirectory::IsSnapshotNeeded(void)
{
    //если слепок не создан - возвращаем true
    return (pdsSnapshot == NULL);
}

//поменять/установить путь к директории
//сомнительная функция, т.к. не меняет FileData и не обновляет слепок
int SomeDirectory::SetDirName(char const * const in_pNewDirName)
{
/*
    size_t stLen;
    int nNewDirFd;

    if(pfdData->nType != IS_DIRECTORY)
	return;

    //если путь указан неверно
    if(in_pNewDirName == NULL || (stLen = strlen(in_pNewDirName)) <= 0)
    {
	return -1;
    }

    if((nNewDirFd = open(in_pNewDirName, O_RDONLY)) < 0)
    {
	return -2;
    }

    //закрываем имеющийся дескриптор (если он открыт)
    if(pfdData->nDirFd >= 0)
	close(pfdData->nDirFd);
    //удаляем прежний путь
    if(pfdData->pName != NULL)
	delete [] pfdData->pName;
    //удаляем копию пути
    if(pSafeName != NULL)
	delete [] pfdData->pSafeName;

    //задаём дескриптор
    pfdData->nDirFd = nNewDirFd;
    //создаём новый путь
    pDirName = new char[stLen+1];
    memset(pDirName, 0, stLen+1);
    strncpy(pDirName, in_pNewDirName, stLen);
    //копируем путь
    pSafeDirName = new char[stLen+1];
    memset(pSafeDirName, 0, stLen+1);
    strncpy(pSafeDirName, in_pNewDirName, stLen);
*/
    return 0;
}

void SomeDirectory::PrintSnapshot(void)
{
  if(pdsSnapshot != NULL)
  {
    fprintf(stderr, "Snapshot for \"%s\"\n", pfdData->pName);
    pdsSnapshot->PrintSnapshot();
  }
  else
    fprintf(stderr, "Snapshot is NULL!\n");
}
