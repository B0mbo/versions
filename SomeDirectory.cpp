//SomeDirectory.cpp
//Author: Bombo
//12.01.2017
//Класс SomeDirectory хранит данные директории

#include"SomeDirectory.h"

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
    pfdData = new FileData(in_pName, NULL, false);
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

    //если путь не указан или пустой
    if(in_pfdData->pName == NULL || in_pfdData->nType != IS_DIRECTORY)
    {
	pfdData = NULL;
	pfdParent = NULL;
	pdsSnapshot = NULL;
	return;
    }

    if((in_pfdData->nDirFd = open(in_pfdData->pName, O_RDONLY)) < 0)
    {
        //если директория не найдена или не может быть открыта
	pfdData = NULL;
	pfdParent = NULL;
	pdsSnapshot = NULL;
        return;
    }

    pfdData = in_pfdData;
    if(in_pfdParent == NULL)
    {
	//ищем родительскую директорию своими силами
	//...
    }
    else
    {
	pfdParent = in_pfdParent;
    }

    if(in_fGetSnapshot)
    {
	//создаём слепок директории (но он не обязательно должен создаваться именно тут)
	//...
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
    return pfdData->pSafeName;
}

//получить путь к директории
//путь к директории каждый раз получается заново
//это сделано для упрощения переноса каталогов из одной ветки ФС в другую
//т.к. в таком случае достаточно лишь сменить родителя переносимой папки
char *SomeDirectory::GetFullPath(void)
{
    SomeDirectory *psdList;
    char *pcBuff, *pcRetName;
    size_t sPathLength;

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
	sPathLength = strlen(pcRetName)+strlen(psdList->GetDirName())+1; //+1 для '/'
	pcBuff = new char[sPathLength];
	memset(pcBuff, 0, sPathLength);
	//копируем в буфер новое имя директории и уже полученный участок пути
	strncpy(pcBuff, psdList->GetDirName(), sPathLength);
	if(strlen(pcRetName) > 0)
	    strncat(pcBuff, "/", sPathLength); //чтобы на конце пути не было '/'
	strncat(pcBuff, pcRetName, sPathLength);
	//обновляем возвращаемый путь
	delete [] pcRetName;
	pcRetName = new char[sPathLength];
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

void SomeDirectory::MakeSnapshot(void)
{
    if(pdsSnapshot != NULL)
	delete pdsSnapshot;
    pdsSnapshot = new DirSnapshot((void *) this);
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
