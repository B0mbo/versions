//DirSnapshot.h
//Author: Bombo
//12.01.2017
//Класс DirSnapshot является "слепком" директории

#pragma once

#include<zlib.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<strings.h>
#include<sys/stat.h>
#include<sys/types.h>

enum {IS_NOTAFILE = 0, IS_DIRECTORY, IS_FILE, IS_LINK}; //виды файлов

enum ResultOfCompare {NO_SNAPSHOT = -1, IS_EMPTY = 0, INPUT_IS_EMPTY, OUTPUT_IS_EMPTY,
  IS_CREATED, IS_DELETED, NEW_NAME, NEW_TIME, NEW_HASH, IS_EQUAL, DIRECTORY_END,
  INIT_PROJECT, START_CONTENT, END_CONTENT, START_FILE_LIST, END_FILE_LIST};

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
    //char szHash[32]; //хэш (для обычного файла)
    unsigned long ulCrc;

    struct FileData *pfdNext;
    struct FileData *pfdPrev;

    FileData();
    FileData(char const * const in_pName, char *in_pPath, struct FileData * const in_pfdPrev, bool in_fCalcHash);
    FileData(FileData const * const in_pfdFile, char const * const in_pPath, bool in_fCalcHash);
    ~FileData();

    void CalcHash(char const * const in_pPath); //вычислить хэш файла
    void SetName(char const * const in_pName); //сменить имя файла

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

    struct SnapshotComparison *pscNext;

    SnapshotComparison();
    SnapshotComparison(FileData * const in_pfdFile, ResultOfCompare in_rocResult);
    ~SnapshotComparison();

    void CopyResult(SnapshotComparison const * const in_pscSnapshotResult);
};

// "слепок" директории (двунаправленный список всех файлов данной директории)
class DirSnapshot
{
    FileData *pfdFirst; //первый файл в списке слепка
    SnapshotComparison *pscFirst; //первый файл в списке результата сравнения двух слепков

    pthread_mutex_t mSnapshotList;
    pthread_mutex_t mComparisonResultList;
//     bool fIsActual; (?) //вместо этого всегда инициализируется pfdFirst != NULL

public:
    DirSnapshot();
    DirSnapshot(char const * const in_pName);
    DirSnapshot(FileData * const in_pfdParent);
    DirSnapshot(void * const in_psdParent, bool in_fMakeHash, bool in_fUpdateDirList);
    ~DirSnapshot();

    FileData *AddFile(char const * const in_pName, char *in_pPath, bool in_fCaclHash); //добавить файл в список
    FileData *AddFile(FileData const * const in_pName, char const * const in_pPath, bool in_fCaclHash); //добавить файл в список
    void SubFile(char const * const in_pName); //удалить файл из списка
    void SubFile(int nInode); //удалить файл из списка по его inode
    void RenameFile(FileData const * const in_pfdData); //переименовать файл

    void CompareSnapshots(DirSnapshot *in_pdsRemake, bool in_fHash);
    void IsDataIncluded(DirSnapshot * const in_pdsSubset, DirSnapshot * const in_pdsSet, bool in_fHash);
    void ClearResult(void); //удалить прежний результат сравнения слепков

    void AddResult(FileData * const in_pfdFile, ResultOfCompare in_rocResult); //добавить результат сравнения в список
    void GetResult(SnapshotComparison * const out_pscResult); //получить следующий результат сравнения и удалить его из списка
    bool IsResultEmpty(void); //найдены отличия или нет?

    void PrintSnapshot(void); //отладка!!!
    void PrintComparison(void); //отладка!!!
};
