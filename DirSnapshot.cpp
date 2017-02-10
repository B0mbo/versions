//DirSnapshot.cpp
//Author: Bombo
//12.01.2017
//Класс DirSnapshot является "слепком" каталога

#include"DirSnapshot.h"
#include"SomeDirectory.h"
#include"RootMonitor.h"
#include"JSONService.h"

extern RootMonitor *rmProject;

/*************************************DirSnapshot**********************************/

DirSnapshot::DirSnapshot()
{
    pfdFirst = new FileData();
    pscFirst = NULL;
    mComparisonResultList = PTHREAD_MUTEX_INITIALIZER;
    mSnapshotList = PTHREAD_MUTEX_INITIALIZER;
}

//функция не подходит для создания конечного слепка директории, т.к.
//ненайденным директориям не задаются родительские
DirSnapshot::DirSnapshot(char const * const in_pName)
{
    DIR *dFd;
    FileData *pfdFile;
    struct dirent *pdeData;

    mComparisonResultList = PTHREAD_MUTEX_INITIALIZER;
    mSnapshotList = PTHREAD_MUTEX_INITIALIZER;

    pfdFirst = new FileData();

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

    mComparisonResultList = PTHREAD_MUTEX_INITIALIZER;
    mSnapshotList = PTHREAD_MUTEX_INITIALIZER;

    pfdFirst = new FileData();
    pscFirst = NULL;

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

//in_psdParent - та директория, для которой создаётся слепок
DirSnapshot::DirSnapshot(void * const in_psdParent, bool in_fMakeHash, bool in_fUpdateDirList)
{
    DIR *dFd;
    int nFd;
    char *pPath = NULL;
    FileData *pfdFile, *pfdData;
    struct dirent *pdeData;
    SomeDirectory *psdParent, *psdAdd;

    mComparisonResultList = PTHREAD_MUTEX_INITIALIZER;
    mSnapshotList = PTHREAD_MUTEX_INITIALIZER;

    pfdFirst = new FileData();
    pscFirst = NULL;

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
	switch(errno)
	{
	  case EACCES:
	    psdParent->GetFileData()->nType = IS_NOTAFILE;
	    break;
	}
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
      //тут не требуется переоткрывать директорию, чтобы получить обновлённые данные о ней (!)
      //т.к. эта часть кода выполняется для создания временного списка уже давно открытой директории (!)
      
      //получаем дескриптор директории, для которой собираемся создать временный слепок
      nFd = psdParent->GetDirFd();
      dFd = fdopendir(nFd);
      if(dFd == NULL)
      {
	fprintf(stderr, "DirSnapshot::DirSnapshot() : This directory is closed!!!\n");
	return;
      }
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

	    //добавляем полученный файл или директорию в инициализирующий список
	    //в обычный список будут добавляться файлы непосредственно из обработчика отличий
	    if(in_fUpdateDirList)
	    {
	      if(rmProject != NULL && psdParent->GetFileData() != NULL)
		rmProject->AddInitChange(pfdFile, psdParent->GetFileData()->stData.st_ino);
	      else
		fprintf(stderr, "DirSnapshot::DirSnapshot() : error! rmProject=%ld\n", (unsigned long)rmProject);
	    }

//  	    if(in_fUpdateDirList)
//  	      fprintf(stderr, "DirSnapshot::DirSnapshot() 3:path: %s, %s\t%s, fd=%d, inode=%d\n", pPath, (pfdFile->nType==IS_DIRECTORY)?("DIR"):(""), pdeData->d_name, pfdFile->nDirFd, (int)pfdFile->stData.st_ino); //отладка!!!
	    if(pfdFile == NULL)
	    {
		pdeData = readdir(dFd);
		continue;
	    }
	    //если это директория - добавляем в список открытых дескрипторов
	    //добавление происходит только при первичном создании слепка
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
    SnapshotComparison *pscList, *pscDel;

    //если список файлов пуст - выходим
    if(pfdFirst == NULL)
	return;

    pthread_mutex_lock(&mSnapshotList);
    //удаляем элементы списка
    while(pfdFirst->pfdNext != NULL)
    {
	delete pfdFirst->pfdNext;
    }
    //удаляем первый элемент списка
    delete pfdFirst;
    pfdFirst = NULL;
    pthread_mutex_unlock(&mSnapshotList);

    pthread_mutex_lock(&mComparisonResultList);
    //удаляем результат сравнения слепков (если он есть)
    pscList = pscDel = pscFirst;
    while(pscList != NULL)
    {
      pscList = pscDel->pscNext;
      delete pscDel;
      pscDel = pscList;
    }
    pscFirst = NULL;
    pthread_mutex_unlock(&mComparisonResultList);

    pthread_mutex_destroy(&mSnapshotList);
    pthread_mutex_destroy(&mComparisonResultList);
}

//добавляем файл в список
FileData *DirSnapshot::AddFile(char const * const in_pName, char *in_pPath, bool in_fCalcHash)
{
    struct FileData *pfdList;
    char *pFullPath;
    size_t stName, stPath;
    struct stat sBuff;

    //если имя файла указано неверно - выходим
    if(in_pName == NULL || strlen(in_pName) <= 0)
	return NULL;

    pthread_mutex_lock(&mSnapshotList);
    //если список ещё пуст
    if(pfdFirst == NULL)
	pfdFirst = new FileData();

    //ищем последний элемент списка
    pfdList = pfdFirst;
    while(pfdList->pfdNext != NULL)
    {
	pfdList = pfdList->pfdNext;
    }

    //проверяем, есть ли доступ к файлу
    stName = strlen(in_pName);
    if(in_pPath != NULL)
      stPath = strlen(in_pPath);
    else
      stPath = 0;
    pFullPath = new char[stName + stPath + 2]; // 1 для '/' и 1 для '\0'
    memset(pFullPath, 0, stName+stPath+2);
    if(stPath > 0)
    {
      strncpy(pFullPath, in_pPath, stPath+stName+1);
      strncat(pFullPath, "/", stPath+stName+1);
    }
    strncat(pFullPath, in_pName, stPath+stName+1);

    //при успехе доступа к файлу добавляем его в список
    if(stat(pFullPath, &sBuff) == 0)
    {
      //добавляем файл в конец списка
      pfdList->pfdNext = new FileData(in_pName, in_pPath, pfdList, in_fCalcHash);
      delete [] pFullPath;
      pthread_mutex_unlock(&mSnapshotList);
      return (pfdList->pfdNext);
    }
    else
    {
//       fprintf(stderr, "DirSnapshot::AddFile() : \"%s\"\n", pFullPath); //отладка!!!
      delete [] pFullPath;
      pthread_mutex_unlock(&mSnapshotList);
      return NULL;
    }
    pthread_mutex_unlock(&mSnapshotList);
}

//добавляем файл в список
FileData *DirSnapshot::AddFile(FileData const * const in_pfdFile, char const * const in_pPath, bool in_fCalcHash)
{
    struct FileData *pfdList;
    FileData *pfdCopy;

    pthread_mutex_lock(&mSnapshotList);
    //если список ещё пуст
    if(pfdFirst == NULL)
	pfdFirst = new FileData();

    //проверка
    if(in_pPath == NULL)
    {
      pthread_mutex_unlock(&mSnapshotList);
      return NULL;
    }

    //ищем последний элемент списка
    pfdList = pfdFirst;
    while(pfdList->pfdNext != NULL)
    {
	pfdList = pfdList->pfdNext;
    }

    //тута надо сделать копию (!)
    pfdCopy = new FileData(in_pfdFile, in_pPath, true);
	 
    //добавляем файл в конец списка
    pfdList->pfdNext = pfdCopy;

    pfdCopy->pfdPrev = pfdList;
    pfdCopy->pfdNext = NULL;

    pthread_mutex_unlock(&mSnapshotList);
    return pfdList->pfdNext;
}

//удаляем файл из слепка
//можно удалять по одному только имени, т.к. два файла с одинаковыми именами в одной
//директории находиться не могут
void DirSnapshot::SubFile(char const * const in_pName)
{
    struct FileData *pfdList;

    if(pfdFirst == NULL)
	return;

    if(in_pName == NULL || strlen(in_pName) <= 0)
	return;

    pthread_mutex_lock(&mSnapshotList);
    //ищем файл в списке по указанному имени
    pfdList = pfdFirst;
    while(pfdList != NULL && strcmp(pfdList->pName, in_pName) != 0)
    {
	pfdList = pfdList->pfdNext;
    }

    //если такой файл не найден в списке - выходим
    if(pfdList == NULL)
    {
	pthread_mutex_unlock(&mSnapshotList);
	return;
    }

    //удаляем файл из списка
    delete pfdList;

    pthread_mutex_unlock(&mSnapshotList);
}

//удаляем файл из слепка по его inode
void DirSnapshot::SubFile(int in_nInode)
{
    struct FileData *pfdList;

    if(pfdFirst == NULL)
	return;

    pthread_mutex_lock(&mSnapshotList);
    //ищем файл в списке по указанному имени
    pfdList = pfdFirst;
    while(pfdList != NULL && pfdList->stData.st_ino != in_nInode)
    {
	pfdList = pfdList->pfdNext;
    }

    //если такой файл не найден в списке - выходим
    if(pfdList == NULL)
    {
	pthread_mutex_unlock(&mSnapshotList);
	return;
    }

    //удаляем файл из списка
    delete pfdList;

    pthread_mutex_unlock(&mSnapshotList);
}

//переименовать файл в слепке
void DirSnapshot::RenameFile(FileData const * const in_pfdData)
{
    struct FileData *pfdList;

    if(pfdFirst == NULL)
	return;

    pthread_mutex_lock(&mSnapshotList);
    pfdList = pfdFirst;
    while(pfdList != NULL && pfdList->stData.st_ino != in_pfdData->stData.st_ino)
    {
	pfdList = pfdList->pfdNext;
    }

    //если не найден
    if(pfdList == NULL)
    {
	pthread_mutex_unlock(&mSnapshotList);
	return;
    }
    //переименовываем
    pfdList->SetName(in_pfdData->pName);
    pthread_mutex_unlock(&mSnapshotList);
}

//требуется переработать для поиска всех отличий сразу, а не по одному за вызов (!)
void DirSnapshot::CompareSnapshots(DirSnapshot *in_pdsRemake, bool in_fHash)
{
//   fprintf(stderr, "DirSnapshot::CompareSnapshots() : main snapshot:\n"); //отладка!!!
//   PrintSnapshot(); //отладка!!!
//   fprintf(stderr, "DirSnapshot::CompareSnapshots() : remake snapshot:\n"); //отладка!!!
//   in_pdsRemake->PrintSnapshot(); //отладка!!!

  IsDataIncluded(this, in_pdsRemake, in_fHash);

//   fprintf(stderr, "DirSnapshot::CompareSnapshots() : result of compare:\n"); //отладка!!!
//   PrintComparison(); //отладка!!!
}

//инициализируем список отличий в текущем объекте слепка
void DirSnapshot::IsDataIncluded(DirSnapshot * const in_pdsSubset, DirSnapshot * const in_pdsSet, bool in_fHash)
{
  FileData *pfdListSubset = NULL, *pfdListSet = NULL, *pfdLastSubset = NULL;
  bool fNotALink;

  //начальная проверка
  if(in_pdsSubset == NULL)
    return;

  //удаляем прежний результат сравнения
  in_pdsSubset->ClearResult();

  //если новый слепок не создался
  if(in_pdsSet == NULL)
  {
    //добавляем сообщение об ошибке и выходим
    in_pdsSubset->AddResult(NULL, NO_SNAPSHOT);
    return;
  }

  //если прежний слепок не создан, возвращаем ошибку "нет слепка"
  if(in_pdsSubset->pfdFirst == NULL)
  {
    //добавляем сообщение об ошибке и выходим
    in_pdsSubset->AddResult(NULL, IS_EMPTY);
    return;
  }
  //если новый слепок пустой, сообщаем об ошибке
  if(in_pdsSet->pfdFirst == NULL)
  {
    //сообщаем о том, что новый слепок не был создан
    in_pdsSubset->AddResult(NULL, INPUT_IS_EMPTY);
    return;
  }

  //1) получаем результат вычетания второго слепка из первого (IS_DELETED)

  //перебираем все файлы первого слепка
  pfdListSubset = in_pdsSubset->pfdFirst->pfdNext;
  while(pfdListSubset != NULL)
  {
    fNotALink = false;
    pfdLastSubset = NULL;
    //перебираем все файлы второго слепка
    pfdListSet = in_pdsSet->pfdFirst->pfdNext;
    while(pfdListSet != NULL)
    {
      //если совпадают inode (либо ulCrc) - переключаем файл первого слепка на следующий
      //нужно ещё добавить сравнение по именам+inode (для поиска переименований) и как-то сообщать о результате (!)
      if( pfdListSubset->nType != IS_NOTAFILE && pfdListSet->nType != IS_NOTAFILE )
      {
	//проверяем, не директории ли
	if( in_fHash && pfdListSubset->nType != IS_DIRECTORY )
	{
	  //сравниваем хэши этих файлов
	  if( pfdListSet->nType != IS_DIRECTORY && pfdListSubset->stData.st_ino == pfdListSet->stData.st_ino )
	  {
	    if( pfdListSubset->ulCrc != pfdListSet->ulCrc )
	    {
	      //хэши старого и нового слепков отличаются - файл изменён
	      //добавляем новый файл в список отличий
	      in_pdsSubset->AddResult(pfdListSet, NEW_HASH);
	    }
	    //переходим к следующему файлу в первом слепке
	    break;
	  }
	}
	else
	{
	  //сравниваем inode файлов
	  if( pfdListSubset->stData.st_ino == pfdListSet->stData.st_ino )
	  {
	    //если имена файлов не совпадают
	    if(strcmp(pfdListSubset->pName, pfdListSet->pName) != 0)
	    {
	      //если разные имена - возможно, файл переименован, а возможно это ссылка на него
	      fNotALink = true;
	      pfdLastSubset = pfdListSubset;
	    }
	    else
	    {
	      //файл найден - значит, прежде была найдена ссылка
	      fNotALink = false;
	      break;
	    }
	    //переключаем файл первого слепка на следующий
	  }
	}
      }
      pfdListSet = pfdListSet->pfdNext;
    }
    if(fNotALink && pfdLastSubset != NULL)
    {
      //добавляем файл из нового слепка в список отличий
      in_pdsSubset->AddResult(pfdLastSubset, NEW_NAME);
    }

    //если достигнут конец второго слепка, и какой-то файл из первого слепка в нём не найден
    if(pfdListSet == NULL && pfdListSubset != NULL && pfdListSubset->nType != IS_NOTAFILE)
    {
      //добавляем этот файл в список отличий
      in_pdsSubset->AddResult(pfdListSubset, IS_DELETED);
    }
    //берём следующий файл из первого слепка для поиска во втором слепке
    pfdListSubset = pfdListSubset->pfdNext;
  }

  //2) получаем результат вычетания первого слепка из второго (IS_CREATED)
  //   хэш при этом проверять уже не имеет смысла

  //перебираем все файлы второго слепка
  pfdListSet = in_pdsSet->pfdFirst->pfdNext;
  while(pfdListSet != NULL)
  {
    //перебираем все файлы первого слепка
    pfdListSubset = in_pdsSubset->pfdFirst->pfdNext;
    while(pfdListSubset != NULL)
    {
      //если сравниваются файлы или директории
      if( pfdListSet->nType != IS_NOTAFILE && pfdListSubset->nType != IS_NOTAFILE )
      {
	//сравниваем inode файлов
	if( pfdListSet->stData.st_ino == pfdListSubset->stData.st_ino )
	{
	  break;
	}
      }

      //переходим к следующему файлу из первого слепка для сравнения с ним
      pfdListSubset = pfdListSubset->pfdNext;
    }

    //если достигли конца первого слепка, но файла из второго слепка так и не нашли - значит, это новый файл
    if(pfdListSubset == NULL && pfdListSet != NULL && pfdListSet->nType!= IS_NOTAFILE)
    {
      //добавляем этот файл к списку отличий
      in_pdsSubset->AddResult(pfdListSet, IS_CREATED);
    }

    //берём следующий файл из второго слепка
    pfdListSet = pfdListSet->pfdNext;
  }

  if(in_pdsSubset->IsResultEmpty())
    in_pdsSubset->AddResult(NULL, IS_EQUAL);
}

//удаляем всё из списка отличий двух слепков
void DirSnapshot::ClearResult(void)
{
  SnapshotComparison *pscList, *pscDel;

  pthread_mutex_lock(&mComparisonResultList);
  pscList = pscDel = pscFirst;
  while(pscList != NULL)	
  {
    pscList = pscList->pscNext;
    delete pscDel;
    pscDel = pscList;
  }
  pscFirst = NULL;
  pthread_mutex_unlock(&mComparisonResultList);
}

//добавить результат сравнения двух файлов
void DirSnapshot::AddResult(FileData * const in_pfdFile, ResultOfCompare in_rocResult)
{
  SnapshotComparison *pscList;

  pthread_mutex_lock(&mComparisonResultList);
  if(pscFirst == NULL)
    pscFirst = new SnapshotComparison();

  pscList = pscFirst;
  while(pscList->pscNext != NULL)
  {
    pscList = pscList->pscNext;
  }

  pscList->pscNext = new SnapshotComparison(in_pfdFile, in_rocResult);

  pthread_mutex_unlock(&mComparisonResultList);
}

//получить последнее отличие
void DirSnapshot::GetResult(SnapshotComparison * const out_pscResult)
{
  SnapshotComparison *pscDel;

  if(out_pscResult == NULL)
  {
    fprintf(stderr, "DirSnapshot::GetResult() : out_pscResult is NULL!\n");
    return;
  }
  //если список не инициализирован или пустой - возвращаем NULL
  if(pscFirst == NULL || pscFirst->pscNext == NULL)
  {
    out_pscResult->CopyResult(NULL);
    return;
  }

  pthread_mutex_lock(&mComparisonResultList);
  //возвращаем копию первого отличия сравнения двух слепков
  out_pscResult->CopyResult(pscFirst->pscNext);
  pscDel = pscFirst->pscNext;
  if(pscDel != NULL)
  {
    pscFirst->pscNext = pscDel->pscNext;
    delete pscDel;
  }
  pthread_mutex_unlock(&mComparisonResultList);
}

//пустой результат сравнения или нет
bool DirSnapshot::IsResultEmpty(void)
{
  if(pscFirst == NULL || pscFirst->pscNext == NULL)
    return true;
  else
    return false;
}

//вывести содержимое директории
void DirSnapshot::PrintSnapshot(void)
{
  FileData *pfdList;

  pfdList = pfdFirst;
  while(pfdList != NULL)
  {
    if(pfdList->nType == IS_DIRECTORY)
      fprintf(stderr, "DirSnapshot::PrintSnapshot() : %s, inode=%d\n", pfdList->pName, (int)pfdList->stData.st_ino);
    else
      fprintf(stderr, "DirSnapshot::PrintSnapshot() : %s, inode=%d, crc=0x%x\n", pfdList->pName, (int)pfdList->stData.st_ino, (int)pfdList->ulCrc);
    pfdList = pfdList->pfdNext;
  }
}

//вывести результат сравнения слепков
void DirSnapshot::PrintComparison(void)
{
  SnapshotComparison *pscList;

  pscList = pscFirst;
  while(pscList != NULL)
  {
    if(pscList->pfdData != NULL && pscList->pfdData->pName != NULL)
      fprintf(stderr, "DirSnapshot::PrintComparison() : \"%s\", %d\n", pscList->pfdData->pName, pscList->rocResult);
    else
      fprintf(stderr, "DirSnapshot::PrintComparison() : NULL, %d\n", pscList->rocResult);
    pscList = pscList->pscNext;
  }
}

/****************************************FileData**********************************/

FileData::FileData()
{
    pName = new char[1];
    memset(pName, 0, sizeof(char));
    pSafeName = NULL;
    nType = IS_NOTAFILE;
    memset(&stData, 0, sizeof(struct stat));
    nDirFd = -1; //инициализация дескриптора как пустого
    //memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла
    ulCrc = crc32(0L, Z_NULL, 0);

    pfdNext = NULL;
    pfdPrev = NULL;
}

FileData::FileData(char const * const in_pName, char *in_pPath, struct FileData * const in_pfdPrev, bool in_fCalcHash)
{
    pfdNext = NULL;
    pfdPrev = NULL;

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
FileData::FileData(FileData const * const in_pfdFile, char const * const in_pPath, bool in_fCalcHash)
{
  size_t stSize;

  //обязательно обнуляем эти указатели
  pfdNext = NULL;
  pfdPrev = NULL;

  nDirFd = -1; //инициализация дескриптора как пустого
  
  if(in_pfdFile == NULL)
  {
    pName = new char[1];
    memset(pName, 0, sizeof(char));
    pSafeName = NULL;
    nType = IS_NOTAFILE;
    memset(&stData, 0, sizeof(struct stat));
    //memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла
    ulCrc = crc32(0L, Z_NULL, 0);
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
//     pName = new char[1];
//     memset(pName, 0, sizeof(char));
    pName = NULL;
  }
  pSafeName = NULL;
  nType = in_pfdFile->nType;
  memcpy(&stData, &(in_pfdFile->stData), sizeof(struct stat));
  nDirFd = in_pfdFile->nDirFd;
  if(in_fCalcHash)
  {
    CalcHash(in_pPath);
  }
  else
  {
    //memcpy(szHash, in_pfdFile->szHash, sizeof(szHash));
    ulCrc = in_pfdFile->ulCrc;
  }
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

void FileData::CalcHash(char const * const in_pPath)
{
    int nFd;
    char *pcBuff;
    ssize_t sstRead;
    char *pFullPath;
    const long BUFF_SIZE = 1048510;
    size_t stSize, stSizePath;

    ulCrc = crc32(0L, Z_NULL, 0);

    //обнуляем хэш
    //memset(szHash, 0, sizeof(szHash));

    //проверка
    if(nType != IS_FILE)
      return;

    if(in_pPath == NULL || pName == NULL || strlen(pName) == 0)
      return;

    //получаем полный путь к файлу
    stSize = strlen(in_pPath);
    stSizePath = strlen(pName);
    pFullPath = new char[stSize + stSizePath + 2];
    memset(pFullPath, 0, stSize + stSizePath + 2);
    strncpy(pFullPath, in_pPath, stSize + stSizePath + 1);
    strncat(pFullPath, "/", stSize + stSizePath + 1);
    strncat(pFullPath, pName, stSize + stSizePath + 1);
    
    nFd = open(pFullPath, O_RDONLY);
    if(pFullPath != NULL)
      delete [] pFullPath;
    if(nFd != -1)
    {
      pcBuff = new char[BUFF_SIZE]; //1M
      stSize = 0;
      while(stSize < stData.st_size)
      {
	memset(pcBuff, 0, BUFF_SIZE);
	stSizePath = read(nFd, pcBuff, BUFF_SIZE);
	stSize = stSize + stSizePath;
	//вычисляем хэш
	ulCrc = crc32(ulCrc, (const unsigned char *)pcBuff, stSizePath);
      }
      close(nFd);
      delete [] pcBuff;
    }
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
      if(strlen(in_pPath) > 0)
      {
	memcpy(pPath, in_pPath, stLen);
	strncat(pPath, "/", stLen);
      }
      strncat(pPath, in_pName, stLen);
    }

    //если имя указано неверно - создаём пустой элемент списка (?)
    //возможно, придётся восстанавливать полный путь к файлу для корректного вызова stat() (!)
    if(in_pName == NULL || ((stLen = strlen(in_pName)) <= 0) || (stat(pPath, &st) < 0))
    {
	//осторожно, может сильно вырасти список пустышек (!)
	//возможно, стоит вызывать stat до создания объекта класса FileData
	perror("error in stat, FileData::SetFileData()");
 	fprintf(stderr, "Can not get stat FileData::SetFileData() : \"%s\" !\n", in_pName); //отладка!!!
	pName = new char[1];
	memset(pName, 0, sizeof(char));
	pSafeName = NULL;
	nType = IS_NOTAFILE;
	memset(&stData, 0, sizeof(struct stat));
	//memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла	
	ulCrc = crc32(0L, Z_NULL, 0);

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

    //хэш
    //memset(szHash, 0, sizeof(szHash));
    ulCrc = crc32(0L, Z_NULL, 0);

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
	CalcHash(in_pPath);
    }

    if(pPath != NULL)
      delete [] pPath;
}

void FileData::SetName(char const * const in_pName)
{
    size_t stLen;

    if(in_pName == NULL || (stLen = strlen(in_pName)) == 0)
	return;

    if(pName != NULL)
	delete [] pName;

    pName = new char[stLen + 1];
    memset(pName, 0, stLen + 1);
    strncpy(pName, in_pName, stLen);
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
    pscNext = NULL;
}

SnapshotComparison::SnapshotComparison(FileData * const in_pfdFile, ResultOfCompare in_rocResult)
{
    rocResult = in_rocResult;
    pfdData = in_pfdFile;
    pscNext = NULL;
}

SnapshotComparison::~SnapshotComparison()
{
    rocResult = IS_EMPTY;
    pfdData = NULL;
}

//копия
void SnapshotComparison::CopyResult(SnapshotComparison const * const in_pscSnapshotResult)
{
    if(in_pscSnapshotResult == NULL)
    {
	rocResult = IS_EMPTY;
	pfdData = NULL;
	pscNext = NULL;
	return;
    }
    rocResult = in_pscSnapshotResult->rocResult;
    pfdData = in_pscSnapshotResult->pfdData;
    pscNext = NULL;
}
