//DirSnapshot.cpp
//Author: Bombo
//12.01.2017
//Класс DirSnapshot является "слепком" каталога

#include"DirSnapshot.h"

/*************************************DirSnapshot**********************************/
DirSnapshot::DirSnapshot()
{
    pfdFirst = NULL;
}

DirSnapshot::DirSnapshot(char const * const in_pName)
{
    pfdFirst = new FileData(in_pName, NULL, NULL);
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
void DirSnapshot::AddFile(char const * const in_pName)
{
    struct FileData *pfdList;
    
    //если имя файла указано неверно - выходим
    if(in_pName == NULL || strlen(in_pName) <= 0)
	return;

    //если список ещё пуст
    if(pfdFirst == NULL)
    {
	pfdFirst = new FileData(in_pName, NULL, NULL);
	return;
    }
    
    //ищем последний элемент списка
    pfdList = pfdFirst;
    while(pfdList->pfdNext != NULL)
    {
	pfdList = pfdList->pfdNext;
    }
    
    //добавляем файл в конец списка
    pfdList->pfdNext = new FileData(in_pName, NULL, pfdList);
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
    nType = IS_NOTAFILE;
    
    pfdNext = NULL;
    pfdPrev = NULL;
}

FileData::FileData(char const * const in_pName, struct FileData * const in_pfdNext, struct FileData * const in_pfdPrev)
{
    SetFileData(in_pName);
    pfdNext = in_pfdNext;
    pfdPrev = in_pfdPrev;
        
    //исключение для next==prev
    //...
    
    if(pfdNext != NULL)
	pfdNext->pfdPrev = this;
    if(pfdPrev != NULL)
	pfdPrev->pfdNext = this;
}

FileData::~FileData()
{
    if(pfdPrev != NULL)
	pfdPrev->pfdNext = pfdNext;
    if(pfdNext != NULL)
	pfdNext->pfdPrev = pfdPrev;
    if(pName != NULL)
	delete [] pName;
}

//задать имя файла
void FileData::SetFileData(char const * const in_pName)
{
    size_t stLen;
    struct stat st;
    
    //если имя указано неверно - создаём пустой элемент списка (?)
    if(in_pName == NULL || (stLen = strlen(in_pName)) <= 0 || (stat(in_pName, &st) < 0))
    {
	pName = NULL;
        nType = IS_NOTAFILE;
    
	pfdNext = NULL;
        pfdPrev = NULL;
        return;
    }
    
    //инициализация имени файла
    pName = new char[stLen+1];
    memset(pName, 0, stLen+1);
    strncpy(pName, in_pName, stLen);
    
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
}
