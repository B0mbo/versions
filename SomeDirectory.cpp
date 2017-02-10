//SomeDirectory.cpp
//Author: Bombo
//12.01.2017
//Класс SomeDirectory хранит данные директории

#include"SomeDirectory.h"
#include"RootMonitor.h"
#include"JSONService.h"

extern RootMonitor *rmProject;

SomeDirectory::SomeDirectory()
{
    pfdData = NULL;
    psdParent = NULL;
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
	psdParent = NULL;
	//сюда бы исключение
	//...
	return;
    }

    //создаём описание корневой директории наблюдаемого проекта
    pPath = GetFullPath();
    pfdData = new FileData(in_pName, pPath, NULL, false);
    psdParent = in_pfdParent;

    if((pfdData->nDirFd = open(pPath, O_RDONLY)) < 0)
    {
      if((pfdData->nDirFd = open(in_pName, O_RDONLY)) < 0)
      {
	fprintf(stderr, "SomeDirectory::SomeDirectory() : Can not open directory \"%s\"!!!\n", pPath);
        //если директория не найдена или не может быть открыта
	if(pPath != NULL)
	  delete [] pPath;
        return;
      }
    }

    if(pPath != NULL)
      delete [] pPath;
}

//этот конструктор автоматически открывает директорию
//и вешает обработчик сигнала на полученный дескриптор
//слепок директории создаётся по запросу (?)
SomeDirectory::SomeDirectory(FileData *in_pfdData, SomeDirectory * const in_pfdParent, bool in_fGetSnapshot)
{
    size_t stLen;
    char *pPath = NULL;

    pdsSnapshot = NULL;

    //если путь не указан или пустой
    if(in_pfdData->pName == NULL || in_pfdData->nType != IS_DIRECTORY)
    {
	pfdData = NULL;
	psdParent = NULL;
	return;
    }

    pfdData = in_pfdData;
    if(in_pfdParent == NULL)
    {
	//ищем родительскую директорию своими силами
	//...
	psdParent = NULL;//заглушка (!)
    }
    else
    {
	psdParent = in_pfdParent;
    }

    pPath = GetFullPath();
    if(psdParent != NULL && pPath != NULL)
      in_pfdData->nDirFd = open(pPath, O_RDONLY);
    else
      in_pfdData->nDirFd = open(in_pfdData->pName, O_RDONLY);

    if(pPath != NULL)
      delete [] pPath;

    if(psdParent == NULL && in_pfdData->nDirFd == -1)
    {
	fprintf(stderr, "SomeDirectory::SomeDirectory() невозможно открыть директорию и получить fd: %s\n", in_pfdData->pName); //отладка!!!
	//если директория не найдена или не может быть открыта
	pfdData = NULL;
	psdParent = NULL;
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
    //удаляем слепок текущей директории
    if(pdsSnapshot != NULL)
      delete pdsSnapshot;
    //удаляем директорию из слепка родительской директории
    if(pfdData != NULL)
    {
	//необходимо исправить двойное удаление (!)
	//второй раз в DescriptorsList::SubQueueElement()
	delete pfdData;
    }
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
    {
      if(pfdData->pSafeName == NULL)
	delete [] pfdData->pSafeName;
      pfdData->pSafeName = new char[strlen(pfdData->pName)+1];
      memset(pfdData->pSafeName, 0, strlen(pfdData->pName)+1);
      strncpy(pfdData->pSafeName, pfdData->pName, strlen(pfdData->pName));
      return pfdData->pSafeName;
    }
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

	if( (strlen(pcRetName) > 0) &&
	    (pDirName != NULL) &&
	    (strlen(pDirName) > 0) &&
	    (pDirName[strlen(pDirName)-1] != '/') &&
	    (pcBuff != NULL) &&
	    (strlen(pcBuff) > 0) &&
	    (pcBuff[strlen(pcBuff)-1] != '/') )
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

SomeDirectory * const SomeDirectory::GetParent(void)
{
    return psdParent;
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
    FileData *pfdCopy = NULL;
    bool fSecondResult;
    DirSnapshot *pdsRemake = NULL;
    SnapshotComparison scResult;
    SomeDirectory *psdNewDirectory = NULL;
    unsigned long ulSessionNumber;

    if(pdsSnapshot == NULL)
    {
	//если слепка ещё нет, создаём и выходим, т.к. это исключительная ситуация
	//поскольку при вызове этой функции слепок уже должен существовать
	pdsSnapshot = new DirSnapshot((void *) this, true, true);
	fprintf(stderr, "SomeDirectory::CompareSnapshots() : Позднее создание слепка!\n");
	return;
    }

    //создаём слепок для сравнения (без хэшей и без добавления в список директорий)
    pdsRemake = new DirSnapshot((void *) this, false, false);

    //первый проход функции сравнения слепков
    fSecondResult = false;
    //производим сравнение
    pdsSnapshot->CompareSnapshots(pdsRemake, false);

    //получаем первое отличие
    pdsSnapshot->GetResult(&scResult);

    ulSessionNumber = rmProject->GetRegularSessionNumber();
//     fprintf(stderr, "SomeDirectory::CompareSnapshots() : %ld\n", ulSessionNumber); //отладка!!!
    rmProject->IncRegularSessionNumber();

    //обрабатываем каждое отличие в отдельности
    while(scResult.rocResult != IS_EMPTY)
    {
	//обрабатываем разницу между новым и старым слепками
	switch(scResult.rocResult)
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
	    //получаем путь к родительской директории
	    pPath = GetFullPath();
	    //добавляем файл в прежний слепок
	    pfdCopy = pdsSnapshot->AddFile(scResult.pfdData, pPath, true);
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
	    fprintf(stderr, "%s \"%s%s%s\" has been created.\n",
			      (scResult.pfdData->nType==IS_DIRECTORY)?"Directory":"File",
			      (pPath==NULL)?"":pPath,
			      (pPath==NULL||( (strlen(pPath) > 0) && (pPath[strlen(pPath)-1] == '/') ))?"":"/",
			      scResult.pfdData->pName); //отладка!!!
	    if(pPath != NULL)
	      delete [] pPath;
	    //добавляем запись в список событий
	    rmProject->AddChange(CURRENT_SERVICE, ulSessionNumber, scResult.pfdData, IS_CREATED, GetFileData()->stData.st_ino);
	    break;
	  case IS_DELETED:
	    pPath = GetFullPath();
	    fprintf(stderr, "%s \"%s%s%s\" (inode=%d) has been deleted.\n",
			      (scResult.pfdData->nType==IS_DIRECTORY)?"Directory":"File",
			      (pPath==NULL)?"":pPath,
			      (pPath==NULL||( (strlen(pPath) > 0) && (pPath[strlen(pPath)-1] == '/') ))?"":"/",
			      scResult.pfdData->pName,
			      (int)scResult.pfdData->stData.st_ino); //отладка!!!
	    if(pPath != NULL)
	      delete [] pPath;
	    //добавляем запись в список событий
	    rmProject->AddChange(CURRENT_SERVICE, ulSessionNumber, scResult.pfdData, IS_DELETED, GetFileData()->stData.st_ino);
	    //если это директория - удаляем из списка директорий
	    //из слепка он при этом удалится автоматом
	    if(scResult.pfdData->nType==IS_DIRECTORY)
	    {
	      //удаляем директорию из списка директорий (вместе с описанием файла)
	      pthread_mutex_lock(&(RootMonitor::mDescListMutex));
// 	      fprintf(stderr, "SomeDirectory::CompareComparisons() : n = %d\n", scResult.pfdData->nDirFd); //отладка!!!
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
	    fprintf(stderr, "Some file has been renamed to \"%s%s%s\".\n",
			    (pPath==NULL)?"":pPath,
			    (pPath==NULL||( (strlen(pPath) > 0) && (pPath[strlen(pPath)-1] == '/') ))?"":"/",
			    scResult.pfdData->pName); //отладка!!!
	    if(pPath != NULL)
	      delete [] pPath;
	    //переимновываем файл
	    pdsSnapshot->RenameFile(scResult.pfdData);
	    //добавляем запись в список событий
	    rmProject->AddChange(CURRENT_SERVICE, ulSessionNumber, scResult.pfdData, NEW_NAME, GetFileData()->stData.st_ino);
	    break;
	  case NEW_TIME:
	    pPath = GetFullPath();
	    fprintf(stderr, "A time of file \"%s%s%s\" has been changed.\n", (pPath==NULL)?"":pPath, (pPath==NULL)?"":"/", scResult.pfdData->pName); //отладка!!!
	    //scResult.pfdData содержит данные изменившегося файла. Всё, кроме хэша
	    scResult.pfdData->CalcHash(pPath);
	    if(pPath != NULL)
	      delete [] pPath;
	    //сравниваем новый и старый хэши файлов. Если они разные - заменяем старую
	    //структуру pfdData на новую (в слепке директории); старую удаляем.
	    //...
	    //если же это директория - переоткрываем, вешаем обработчик
	    //...
// 	    //добавляем запись в список событий
// 	    rmProject->AddChange(CURRENT_SERVICE, ulSessionNumber, scResult.pfdData, NEW_TIME, GetFileData()->stData.st_ino);
	    break;
	  case NEW_HASH:
	    pPath = GetFullPath();
	    fprintf(stderr, "%s \"%s%s%s\" (inode=%d) has been changed.\n",
			      (scResult.pfdData->nType==IS_DIRECTORY)?"Directory":"File",
			      (pPath==NULL)?"":pPath,
			      (pPath==NULL||( (strlen(pPath) > 0) && (pPath[strlen(pPath)-1] == '/') ))?"":"/",
			      scResult.pfdData->pName,
			      (int)scResult.pfdData->stData.st_ino); //отладка!!!

	    //обновляем данные в базе
	    //удаляем файл из прежнего слепка
	    pdsSnapshot->SubFile(scResult.pfdData->pName);
	    //добавляем файл с изменениями в прежний слепок
	    pdsSnapshot->AddFile(scResult.pfdData, pPath, false);

	    if(pPath != NULL)
	      delete [] pPath;
	    //добавляем запись в список событий
	    rmProject->AddChange(CURRENT_SERVICE, ulSessionNumber, scResult.pfdData, NEW_HASH, GetFileData()->stData.st_ino);
	    break;
	  case IS_EQUAL:
	    break;
	}

	//если это первый "проход" и различия не найдены - повтор
	if(!fSecondResult && scResult.rocResult == IS_EQUAL)
	{
	  //второй проход функции сравнения слепков
	  fSecondResult = true;
	  if(pdsRemake != NULL)
	  {
	    //удаляем прежний результат сравнения
	    delete pdsRemake;
	  }
	  //создаём слепок с хэшем всех файлов
	  pdsRemake = new DirSnapshot((void *) this, true, false);
	  //сравниваем файлы по содержимому (по хэшу)
	  pdsSnapshot->CompareSnapshots(pdsRemake, true);

	  //повторяем поиск различий
	  continue;
	}

	//получаем следующее отличие
	pdsSnapshot->GetResult(&scResult);
    }

    char *list = rmProject->GetJSON(ulSessionNumber);
    if(list != NULL)
    {
      fprintf(stderr, "%s\n", list); //отладка!!!
      delete [] list;
    }

//     rmProject->PrintServices(); //отладка!!!

    delete pdsRemake;
}

bool SomeDirectory::IsSnapshotNeeded(void)
{
    //если слепок не создан - возвращаем true
    return (pdsSnapshot == NULL);
}

//поменять/установить путь к директории
//сомнительная функция, т.к. не меняет FileData и не обновляет слепок
//проверить, нужно ли после переименования переоткрывать директорию (!)
int SomeDirectory::SetDirName(char const * const in_pNewDirName)
{
    size_t stNameLen;

    if(pfdData == NULL)
      return 1;

    if(pfdData->pName != NULL)
      delete [] pfdData->pName;

    stNameLen = strlen(in_pNewDirName);
    pfdData->pName = new char[stNameLen + 1];
    memset(pfdData->pName, 0, stNameLen + 1);
    strncpy(pfdData->pName, in_pNewDirName, stNameLen);

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
    fprintf(stderr, "SomeDirectory::PrintSnapshot() : snapshot for \"%s\"\n", pfdData->pName);
    pdsSnapshot->PrintSnapshot();
  }
  else
    fprintf(stderr, "Snapshot is NULL!\n");
}
