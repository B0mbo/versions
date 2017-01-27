//DirSnapshot.cpp
//Author: Bombo
//12.01.2017
//Класс DirSnapshot является "слепком" каталога

#include"DirSnapshot.h"
#include"SomeDirectory.h"
#include"RootMonitor.h"

/*************************************DirSnapshot**********************************/

DirSnapshot::DirSnapshot()
{
    pfdFirst = NULL;
}

//функция не подходит для создания конечного слепка директории, т.к.
//ненайденным директориям не задаются родительские
DirSnapshot::DirSnapshot(char const * const in_pName)
{
    DIR *dFd;
    FileData *pfdFile;
    struct dirent *pdeData;

    pfdFirst = NULL;

    //тут должен быть автоматически создан весь слепок
    //путём чтения текущей директории
    //параметр in_pName - имя этой директории

    //создаём список файлов (слепок)
    if(in_pName == NULL)
	return;

/*
    dFd = opendir(in_pName);

    if(dFd == NULL)
	return;

    pdeData = readdir(dFd);

    while(pdeData != NULL)
    {
	//исключаем "." и ".."
	if( !((strlen(pdeData->d_name) == 1 && strncmp(pdeData->d_name, ".", 1) == 0) ||
	    (strlen(pdeData->d_name) == 2 && strncmp(pdeData->d_name, "..", 2) == 0)) )
	{
	    pfdFile = AddFile(pdeData->d_name, true); //сразу вычисляем хэш

	    if(pfdFile != NULL)
		fprintf(stderr, "1:%s\t%s  \n", (pfdFile->nType==IS_DIRECTORY)?("DIR"):(""), pdeData->d_name); //отладка!!!

	    //т.к. этим конструктором создаётся слепок исключительно для сравнения с другими,
	    //отслежывать найденные в нём директории не требуется
	}
	pdeData = readdir(dFd);
    }
*/
}

DirSnapshot::DirSnapshot(FileData * const in_pfdParent)
{
    DIR *dFd;
    char *pPath;
    FileData *pfdFile;
    struct dirent *pdeData;

    pfdFirst = NULL;

    //тут должен быть автоматически создан весь слепок
    //путём чтения текущей директории
    //параметр in_pName - имя этой директории

/*
    //создаём список файлов (слепок)
    dFd = opendir(in_pfdParent->pName);
    if(dFd < 0)
	return;

    pdeData = readdir(dFd);
    while(pdeData != NULL)
    {
	//исключаем "." и ".."
	if( !((strlen(pdeData->d_name) == 1 && strncmp(pdeData->d_name, ".", 1) == 0) ||
	    (strlen(pdeData->d_name) == 2 && strncmp(pdeData->d_name, "..", 2) == 0)) )
	{
	    pfdFile = AddFile(pdeData->d_name, true); //сразу вычисляем хэш

	    if(pfdFile != NULL)
		fprintf(stderr, "2:%s\t%s  \n", (pfdFile->nType==IS_DIRECTORY)?("DIR"):(""), pdeData->d_name); //отладка!!!

	    //если это директория - добавляем в список
	    //ищем полный путь к директории
//	    pPath = in_pfdParent->GetFullPath();
	    if(pPath != NULL)
	    {
//		fprintf(stderr, "path: %s\n", pPath); //отладка!!!
//		delete [] pPath;
	    }
	}
	pdeData = readdir(dFd);
    }
*/
}

DirSnapshot::DirSnapshot(void * const in_psdParent, bool in_fMakeHash, bool in_fUpdateDirList)
{
    DIR *dFd;
    int nFd;
    char *pPath = NULL;
    FileData *pfdFile, *pfdData;
    struct dirent *pdeData;
    SomeDirectory *psdParent, *psdAdd;

    pfdFirst = NULL;

    //тут должен быть автоматически создан весь слепок
    //путём чтения текущей директории
    //параметр in_pName - имя этой директории

    //создаём список файлов (слепок)
    
    //получаем переданный параметр - родительскую директорию
    psdParent = (SomeDirectory *)in_psdParent;

    //небольшая проверка
    if(psdParent == NULL || ((pfdData = psdParent->GetFileData())==NULL) || pfdData->nType != IS_DIRECTORY)
    {
	fprintf(stderr, "DirSnapshot::DirSnapshot() Can not create snapshot (file \"%s\" is not a directory)\n", (pfdData==NULL)?"NULL!!!":pfdData->pName);
	return;
    }

    pPath = psdParent->GetFullPath();

    if(in_fUpdateDirList)
    {
      nFd = psdParent->GetDirFd();
      //если директория ещё не открыта - открываем
      if(nFd == -1)
      {
//  	fprintf(stderr, "DirSnapshot::DirSnapshot() открывается этот файл: pPath=\"%s\"\n", pPath); //отладка!!!
	nFd = open(pPath, O_RDONLY);

// 	if(nFd == -1)
// 	  perror("DirSnapshot::DirSnapshot() open directory");

	  //учесть при инкапсуляции (!)
	psdParent->GetFileData()->nDirFd = nFd;
      }
//       fprintf(stderr, "DirSnapshot::DirSnapshot() nFd=%d\n", nFd); //отладка!!!
      dFd = fdopendir(nFd);
      if(dFd == NULL)
      {
// 	fprintf(stderr, "DirSnapshot::DirSnapshot() dFd == NULL\n");
	if(pPath != NULL)
	  delete [] pPath;
	return;
      }
    }
    else
    {
      //тут требуется переоткрыть директорию, чтобы получить обновлённые данные о ней
      nFd = psdParent->GetDirFd();
      dFd = fdopendir(nFd);
      rewinddir(dFd); //вот что надо было сделать!
    }

    pdeData = readdir(dFd);

    while(pdeData != NULL)
    {
	//исключаем "." и ".."
	if( !((strlen(pdeData->d_name) == 1 && strncmp(pdeData->d_name, ".", 1) == 0) ||
	    (strlen(pdeData->d_name) == 2 && strncmp(pdeData->d_name, "..", 2) == 0)) )
	{
	    pfdFile = AddFile(pdeData->d_name, pPath, in_fMakeHash); //сразу вычисляем хэш, если задано

//  	    if(in_fUpdateDirList)
//  	      fprintf(stderr, "DirSnapshot::DirSnapshot() 3:path: %s, %s\t%s, fd=%d, inode=%d\n", pPath, (pfdFile->nType==IS_DIRECTORY)?("DIR"):(""), pdeData->d_name, pfdFile->nDirFd, (int)pfdFile->stData.st_ino); //отладка!!!

	    if(pfdFile == NULL)
		continue;

	    //если это директория - добавляем в список открытых дескрипторов
	    if(in_fUpdateDirList && pPath != NULL && pfdFile->nType == IS_DIRECTORY)
	    {
		//возможно, создание этого объекта следует перенести в pdlList->AddQueueElement() (?)
		psdAdd = new SomeDirectory(pfdFile, psdParent, false);
		//непосредственно добавление
		pthread_mutex_lock(&(RootMonitor::mDescListMutex));
		if(RootMonitor::pdlList != NULL)
		    RootMonitor::pdlList->AddQueueElement(psdAdd);
		pthread_mutex_unlock(&(RootMonitor::mDescListMutex));
	    }
	}
	pdeData = readdir(dFd);
    }

    if(pPath != NULL)
        delete [] pPath;

    //освобождение мьютекса потока обработчика списка директорий (?)
    //возможно, лучше это делать после завершения данного метода
    //для избежания рекурсии
    //...
}

DirSnapshot::~DirSnapshot()
{
    //если список файлов пуст - выходим
    if(pfdFirst == NULL)
	return;

    //удаляем элементы списка
    while(pfdFirst->pfdNext != NULL)
    {
	delete pfdFirst->pfdNext;
    }
    //удаляем первый элемент списка
    delete pfdFirst;
    pfdFirst = NULL;
}

//добавляем файл в список
FileData *DirSnapshot::AddFile(char const * const in_pName, char *in_pPath, bool in_fCalcHash)
{
    struct FileData *pfdList;

    //если имя файла указано неверно - выходим
    if(in_pName == NULL || strlen(in_pName) <= 0)
	return NULL;

    //если список ещё пуст
    if(pfdFirst == NULL)
    {
	pfdFirst = new FileData(in_pName, in_pPath, NULL, in_fCalcHash);
	return pfdFirst;
    }

    //ищем последний элемент списка
    pfdList = pfdFirst;
    while(pfdList->pfdNext != NULL)
    {
	pfdList = pfdList->pfdNext;
    }

    //добавляем файл в конец списка
    pfdList->pfdNext = new FileData(in_pName, in_pPath, pfdList, in_fCalcHash);

    return (pfdList->pfdNext);
}

//добавляем файл в список
FileData *DirSnapshot::AddFile(FileData const * const in_pfdFile, bool in_fCalcHash)
{
    struct FileData *pfdList;
    FileData *pfdCopy;

    //если список ещё пуст
    if(pfdFirst == NULL)
    {
	pfdFirst = new FileData(in_pfdFile, true);
	return pfdFirst;
    }

    //ищем последний элемент списка
    pfdList = pfdFirst;
    while(pfdList->pfdNext != NULL)
    {
	pfdList = pfdList->pfdNext;
    }

    //тута надо сделать копию (!)
    pfdCopy = new FileData(in_pfdFile, true);
	 
    //добавляем файл в конец списка
    pfdList->pfdNext = pfdCopy;

    pfdCopy->pfdPrev = pfdList;
    pfdCopy->pfdNext = NULL;

    return pfdList->pfdNext;
}

//удаляем файл из списка
void DirSnapshot::SubFile(char const * const in_pName)
{
    struct FileData *pfdList;

    if(pfdFirst == NULL)
	return;

    if(in_pName == NULL || strlen(in_pName) <= 0)
	return;

    //ищем файл в списке по указанному имени
    pfdList = pfdFirst;
    while(pfdList != NULL && strcmp(pfdList->pName, in_pName) != 0)
    {
	pfdList = pfdList->pfdNext;
    }

    //если такой файл не найден в списке - выходим
    if(pfdList == NULL)
	return;

    //удаляем файл из списка
    delete pfdList;
}

ResultOfCompare DirSnapshot::CompareSnapshots(DirSnapshot *in_pdsRemake, SnapshotComparison *out_pscResult)
{
  FileData *pfdResult;

  if(pfdFirst == NULL)
  {
    if(in_pdsRemake != NULL && in_pdsRemake->pfdFirst != NULL)
    {
      pfdResult = IsDataIncluded(in_pdsRemake, this);
      if(pfdResult != NULL)
      {
	//если файл создан
	out_pscResult->rocResult = IS_CREATED;
	out_pscResult->pfdData = pfdResult;
	return IS_CREATED;
      }
      else
	return IS_EMPTY;
    }
    else
      return IS_EMPTY;
  }

  if(in_pdsRemake == NULL)
    return INPUT_IS_EMPTY;

  if(out_pscResult == NULL)
    return OUTPUT_IS_EMPTY;

  //ищем исчезнувшие файлы из старого слепка
  pfdResult = IsDataIncluded(this, in_pdsRemake);
  if(pfdResult != NULL)
  {
    //если файл удалён
    out_pscResult->rocResult = IS_DELETED;
    out_pscResult->pfdData = pfdResult;
    return IS_DELETED;
  }

  //ищем новые файлы в новом слепке
  pfdResult = IsDataIncluded(in_pdsRemake, this);
  if(pfdResult != NULL)
  {
    //если файл создан
    out_pscResult->rocResult = IS_CREATED;
    out_pscResult->pfdData = pfdResult;
    return IS_CREATED;
  }

  return IS_EQUAL;
}

//если Subset является частью Set, возвращаем NULL
//если Subset больше Set, возвращается ссылка на файл, которого нету в Set
FileData *DirSnapshot::IsDataIncluded(DirSnapshot *in_pdsSubset, DirSnapshot *in_pdsSet)
{
  FileData *pfdListSubset, *pfdListSet;

  //перебираем все файлы подмножества
  pfdListSubset = in_pdsSubset->pfdFirst;
  while(pfdListSubset != NULL)
  {
    //перебираем все файлы множества
    pfdListSet = in_pdsSet->pfdFirst;
    while(pfdListSet != NULL)
    {
      //если имена файлов совпадает - переключаем файл подмножества на следующий
//       if(strcmp(pfdListSubset->pName, pfdListSet->pName) == 0)
// 	break;
      //если совпадают inode - переключаем файл подмножества на следующий
      if(pfdListSubset->stData.st_ino == pfdListSet->stData.st_ino)
	break;
      pfdListSet = pfdListSet->pfdNext;
    }
    //если достигнут конец подмножества и файл не найден в множестве, возвращаем ссылку на него
    if(pfdListSet == NULL)
      return pfdListSubset;
    //берём следующий файл из подмножества для поиска в множестве
    pfdListSubset = pfdListSubset->pfdNext;
  }
  
  return NULL;
}

//вывести содержимое директории
void DirSnapshot::PrintSnapshot(void)
{
  FileData *pfdList;

  pfdList = pfdFirst;
  while(pfdList != NULL)
  {
    fprintf(stderr, "DirSnapshot::PrintSnapshot() : Directory snapshot: %s\n", pfdList->pName);
    pfdList = pfdList->pfdNext;
  }
}

/****************************************FileData**********************************/

FileData::FileData()
{
    pName = NULL;
    pSafeName = NULL;
    nType = IS_NOTAFILE;
    memset(&stData, 0, sizeof(struct stat));
    nDirFd = -1; //инициализация дескриптора как пустого
    memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла

    pfdNext = NULL;
    pfdPrev = NULL;
}

FileData::FileData(char const * const in_pName, char *in_pPath, struct FileData * const in_pfdPrev, bool in_fCalcHash)
{
    SetFileData(in_pName, in_pPath, in_fCalcHash);
    pfdPrev = in_pfdPrev;

    if(in_pfdPrev != NULL)
    {
	pfdNext = in_pfdPrev->pfdNext;
	if(in_pfdPrev->pfdNext != NULL)
	    in_pfdPrev->pfdNext->pfdPrev = this;
	in_pfdPrev->pfdNext = this;
    }
    else
    {
	pfdNext = NULL;
    }
}

//конструктор копии
FileData::FileData(FileData const * const in_pfdFile, bool in_fCalcHash)
{
  size_t stSize;

  //обязательно обнуляем эти указатели
  pfdNext = NULL;
  pfdPrev = NULL;

  nDirFd = -1; //инициализация дескриптора как пустого
  
  if(in_pfdFile == NULL)
  {
    pName = NULL;
    pSafeName = NULL;
    nType = IS_NOTAFILE;
    memset(&stData, 0, sizeof(struct stat));
    memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла
    return;    
  }
  if(in_pfdFile->pName != NULL)
  {
    stSize = strlen(in_pfdFile->pName);
    pName = new char[stSize+1];
    memset(pName, 0, stSize+1);
    strncpy(pName, in_pfdFile->pName, stSize);
  }
  else
  {
    pName = new char[1];
    memset(pName, 0, sizeof(char));
  }
  pSafeName = NULL;
  nType = in_pfdFile->nType;
  memcpy(&stData, &(in_pfdFile->stData), sizeof(struct stat));
  nDirFd = in_pfdFile->nDirFd;
  if(in_fCalcHash)
    CalcHash();
  else
    memcpy(szHash, in_pfdFile->szHash, sizeof(szHash));
}

FileData::~FileData()
{
    if(pfdPrev != NULL)
	pfdPrev->pfdNext = pfdNext;
    if(pfdNext != NULL)
	pfdNext->pfdPrev = pfdPrev;
    if(pName != NULL)
    {
	delete [] pName;
	pName = NULL;
    }
    if(pSafeName != NULL)
    {
	delete [] pSafeName;
	pSafeName = NULL;
    }
}

void FileData::CalcHash()
{
    //обнуляем хэш
    memset(szHash, 0, sizeof(szHash));
    //вычисляем хэш
    //...
}

//задать имя файла и определить его тип
//по запросу можно сразу вычислить хэш
void FileData::SetFileData(char const * const in_pName, char *in_pPath, bool in_fCalcHash)
{
    char *pPath = NULL;
    size_t stLen;
    struct stat st;

    nDirFd = -1; //инициализация дескриптора как пустого

    if(in_pPath != NULL && in_pName != NULL)
    {
      stLen = strlen(in_pPath)+strlen(in_pName);
      pPath = new char[stLen+2];
      memset(pPath, 0, stLen+2);
      memcpy(pPath, in_pPath, stLen);
      strncat(pPath, "/", stLen);
      strncat(pPath, in_pName, stLen);
//       fprintf(stderr, "FileData::SetFileData() : %s\n", pPath); //отладка!!!
    }

    //если имя указано неверно - создаём пустой элемент списка (?)
    //возможно, придётся восстанавливать полный путь к файлу для корректного вызова stat() (!)
    if(in_pName == NULL || ((stLen = strlen(in_pName)) <= 0) || (stat(pPath, &st) < 0))
    {
	fprintf(stderr, "Can not get stat for: %s!\n", in_pName); //отладка!!!
	pName = NULL;
	pSafeName = NULL;
	nType = IS_NOTAFILE;
	memset(&stData, 0, sizeof(struct stat));
	memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла	

	pfdNext = NULL;
	pfdPrev = NULL;
	
	if(pPath != NULL)
	  delete [] pPath;
	return;
    }

    //инициализация имени файла
    pName = new char[stLen+1];
    memset(pName, 0, stLen+1);
    strncpy(pName, in_pName, stLen);

    //инициализация имени файла
    pSafeName = new char[stLen+1];
    memset(pSafeName, 0, stLen+1);
    strncpy(pSafeName, in_pName, stLen);

    //описание файла
    memcpy(&stData, &st, sizeof(st));

//     fprintf(stderr, "FileData::SetFileData() : %s, inode=%d, %d\n", pPath, (int)st.st_ino, (int)stData.st_ino); //отладка!!!

    //хэш
    memset(szHash, 0, sizeof(szHash));

    //инициализация типа файла
    switch(st.st_mode & (S_IFDIR | S_IFREG | S_IFLNK))
    {
	case S_IFDIR:
	    nType = IS_DIRECTORY;
	    break;
	case S_IFREG:
	    nType = IS_FILE;
	    break;
	case S_IFLNK:
	    nType = IS_LINK;
	    break;
	default: nType = IS_NOTAFILE;
    }

    if(in_fCalcHash && nType == IS_FILE)
    {
	//получаем хэш файла, если это обычный файл
	CalcHash();
    }

    if(pPath != NULL)
      delete [] pPath;
}

char const * const FileData::GetName()
{
    return pName;
}

/****************************************SnapshotComparison*********************/

SnapshotComparison::SnapshotComparison()
{
    rocResult = IS_EMPTY;
    pfdData = NULL;
}
