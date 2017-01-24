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
    char *pPath = NULL;
    FileData *pfdFile;
    struct dirent *pdeData;
    SomeDirectory *psdParent, *psdAdd;

    pfdFirst = NULL;

    //тут должен быть автоматически создан весь слепок
    //путём чтения текущей директории
    //параметр in_pName - имя этой директории

    //создаём список файлов (слепок)
    psdParent = (SomeDirectory *)in_psdParent;

    //небольшая проверка
    if((psdParent->GetFileData())->nType != IS_DIRECTORY)
	return;

    pPath = psdParent->GetFullPath();
    dFd = opendir(pPath);
    if(dFd == NULL)
    {
	if(pPath != NULL)
	    delete [] pPath;
	return;
    }

    pdeData = readdir(dFd);
    while(pdeData != NULL)
    {
	//исключаем "." и ".."
	if( !((strlen(pdeData->d_name) == 1 && strncmp(pdeData->d_name, ".", 1) == 0) ||
	    (strlen(pdeData->d_name) == 2 && strncmp(pdeData->d_name, "..", 2) == 0)) )
	{
	    pfdFile = AddFile(pdeData->d_name, in_fMakeHash); //сразу вычисляем хэш, если задано

	    if(pfdFile == NULL)
		continue;

//	    fprintf(stderr, "3:%s\t%s  \n", (pfdFile->nType==IS_DIRECTORY)?("DIR"):(""), pdeData->d_name); //отладка!!!

	    //если это директория - добавляем в список открытых дескрипторов
	    if(in_fUpdateDirList && pPath != NULL && pfdFile->nType == IS_DIRECTORY)
	    {
		//возможно, создание этого объекта следует перенести в pdlList->AddQueueElement() (?)
		psdAdd = new SomeDirectory(pfdFile, psdParent, false);
		//непосредственно добавление
		pthread_mutex_lock(&(RootMonitor::mDescListMutex));
		if(RootMonitor::pdlList != NULL)
		{
//		    fprintf(stderr, "добавляем новую директорию в список\n"); //отладка!!!
		    RootMonitor::pdlList->AddQueueElement(psdAdd);
//		    fprintf(stderr, "добавили новую директорию в список\n"); //отладка!!!
		}
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
FileData *DirSnapshot::AddFile(char const * const in_pName, bool in_fCalcHash)
{
    struct FileData *pfdList;

    //если имя файла указано неверно - выходим
    if(in_pName == NULL || strlen(in_pName) <= 0)
	return NULL;

    //если список ещё пуст
    if(pfdFirst == NULL)
    {
	pfdFirst = new FileData(in_pName, NULL, in_fCalcHash);
	return pfdFirst;
    }

    //ищем последний элемент списка
    pfdList = pfdFirst;
    while(pfdList->pfdNext != NULL)
    {
	pfdList = pfdList->pfdNext;
    }

    //добавляем файл в конец списка
    pfdList->pfdNext = new FileData(in_pName, pfdList, in_fCalcHash);

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

FileData::FileData(char const * const in_pName, struct FileData * const in_pfdPrev, bool in_fCalcHash)
{
    SetFileData(in_pName, in_fCalcHash);
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
void FileData::SetFileData(char const * const in_pName, bool in_fCalcHash)
{
    size_t stLen;
    struct stat st;

    //если имя указано неверно - создаём пустой элемент списка (?)
    //возможно, придётся восстанавливать полный путь к файлу для корректного вызова stat() (!)
    if(in_pName == NULL || ((stLen = strlen(in_pName)) <= 0) || (stat(in_pName, &st) < 0))
    {
	pName = NULL;
	pSafeName = NULL;
	nType = IS_NOTAFILE;

	pfdNext = NULL;
	pfdPrev = NULL;
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
}

char const * const FileData::GetName()
{
    return pName;
}
