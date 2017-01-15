//DirSnapshot.h
//Author: Bombo
//12.01.2017
//Класс DirSnapshot является "слепком" директории

#pragma once

#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

enum {IS_NOTAFILE = 0,IS_DIRECTORY,IS_FILE,IS_LINK}; //виды файлов

struct FileData;

//элемент списка файлов, находящихся в отслеживаемой директории
//обязательно должен хранить всю структуру stat для данного файла
//и его хэш
struct FileData
{
    char *pName; //имя файла
    int nType; //тип файла (каталог, обычный, ссылка)
    struct stat stData; //данные файла
    char szHash[32]; //хэш

    struct FileData *pfdNext;
    struct FileData *pfdPrev;

    FileData();
    FileData(char const * const in_pName, struct FileData * const in_pfdNext, struct FileData * const in_pfdPrev);
    ~FileData();

private:
    void SetFileData(char const * const in_pName); //задать имя файла
};

// "слепок" директории (двунаправленный список всех файлов данной директории)
class DirSnapshot
{
    FileData *pfdFirst; //первый файл в списке
public:
    DirSnapshot();
    DirSnapshot(char const * const in_pName);
    ~DirSnapshot();

    void AddFile(char const * const in_pName); //добавить файл в список
    void SubFile(char const * const in_pName); //удалить файл из списка
};
