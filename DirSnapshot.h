//DirSnapshot.h
//Author: Bombo
//12.01.2017
//Класс DirSnapshot является "слепком" директории

#pragma once

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/types.h>

enum {IS_NOTAFILE = 0, IS_DIRECTORY, IS_FILE, IS_LINK}; //виды файлов

struct FileData;

//элемент списка файлов, находящихся в отслеживаемой директории
//обязательно должен хранить всю структуру stat для данного файла
//и его хэш
struct FileData
{
    char *pName; //имя файла
    char *pSafeName; //имя для возврата при запросе
    int nType; //тип файла (каталог, обычный, ссылка)
    struct stat stData; //данные файла
    int nDirFd; //дескриптор (для директории)
    char szHash[32]; //хэш (для обычного файла)

    struct FileData *pfdNext;
    struct FileData *pfdPrev;

    FileData();
    FileData(char const * const in_pName, struct FileData * const in_pfdPrev, bool in_fCalcHash);
    ~FileData();

    void CalcHash(void); //вычислить хэш файла

private:
    //задать имя файла и определить его тип
    void SetFileData(char const * const in_pName, bool in_fCalcHash);
    //получить имя файла/директории
    char const * const GetName(void);
};

// "слепок" директории (двунаправленный список всех файлов данной директории)
class DirSnapshot
{
    FileData *pfdFirst; //первый файл в списке
public:
    DirSnapshot();
    DirSnapshot(char const * const in_pName);
    DirSnapshot(FileData * const in_pfdParent);
    DirSnapshot(void * const in_psdParent, bool in_fMakeHash, bool in_fUpdateDirList);
    ~DirSnapshot();

    FileData *AddFile(char const * const in_pName, bool in_fCaclHash); //добавить файл в список
    void SubFile(char const * const in_pName); //удалить файл из списка
};
