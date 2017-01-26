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

enum ResultOfCompare {NO_SNAPSHOT = -1, IS_EMPTY = 0, INPUT_IS_EMPTY, OUTPUT_IS_EMPTY, IS_CREATED, IS_DELETED, NEW_NAME, NEW_TIME, IS_EQUAL};

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
    FileData(char const * const in_pName, char *in_pPath, struct FileData * const in_pfdPrev, bool in_fCalcHash);
    FileData(FileData const * const in_pfdFile, bool in_fCalcHash);
    ~FileData();

    void CalcHash(void); //вычислить хэш файла

private:
    //задать имя файла и определить его тип
    void SetFileData(char const * const in_pName, char *in_pPath, bool in_fCalcHash);
    //получить имя файла/директории
    char const * const GetName(void);
};

struct SnapshotComparison
{
    ResultOfCompare rocResult;
    FileData *pfdData;
    
    SnapshotComparison();
};

// "слепок" директории (двунаправленный список всех файлов данной директории)
class DirSnapshot
{
    FileData *pfdFirst; //первый файл в списке
//     bool fIsActual; (?)

public:
    DirSnapshot();
    DirSnapshot(char const * const in_pName);
    DirSnapshot(FileData * const in_pfdParent);
    DirSnapshot(void * const in_psdParent, bool in_fMakeHash, bool in_fUpdateDirList);
    ~DirSnapshot();

    FileData *AddFile(char const * const in_pName, char *in_pPath, bool in_fCaclHash); //добавить файл в список
    FileData *AddFile(FileData const * const in_pName, bool in_fCaclHash); //добавить файл в список
    void SubFile(char const * const in_pName); //удалить файл из списка

    ResultOfCompare CompareSnapshots(DirSnapshot *in_pdsRemake, SnapshotComparison *out_pscResult);
    FileData *IsDataIncluded(DirSnapshot *in_pdsSubset, DirSnapshot *in_pdsSet);

    void PrintSnapshot(void); //отладка!!!
};
