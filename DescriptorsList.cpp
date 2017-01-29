//DescriptorsList.cpp
//Author: Bombo
//13.01.2017
//Класс DescriptorsList содержит список всех открытых дескрипторов

#include"DescriptorsList.h"

/*********************************DescriptorQueue*********************************/

DescriptorsList::DescriptorsList()
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    //пустой первый элемент
    pthread_mutex_lock(&mListMutex);
    pdleFirst = NULL;
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::DescriptorsList(SomeDirectory *in_psdRootDirectory)
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_unlock(&mListMutex);
    //создаём список директорий с первым элементом
    pthread_mutex_lock(&mListMutex);
    pdleFirst = new DirListElement(in_psdRootDirectory, NULL);
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::DescriptorsList(FileData *in_pfdData)
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_unlock(&mListMutex);
    //создаём список директорий с первым элементом
    pthread_mutex_lock(&mListMutex);
    pdleFirst = new DirListElement(in_pfdData, NULL, NULL);
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::~DescriptorsList()
{
    //удаляем ссписок директорий
    DirListElement *pdleList, *pdleDel;
    if(pdleFirst == NULL)
	return;
    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst->pdleNext;
    while(pdleList != NULL)
    {
	pdleDel = pdleList;
	pdleList = pdleList->pdleNext;
	delete pdleDel;
    }
    delete pdleFirst;
    pdleFirst = NULL;
    pthread_mutex_unlock(&mListMutex);
}

//перед вызовом метода ОБЯЗАТЕЛЬНО следует блокировать mDescListMutex
//после вызова - освобождать
void DescriptorsList::AddQueueElement(SomeDirectory * const in_psdPtr)
{
    DirListElement *pdleList;

    if(in_psdPtr == NULL)
	return;

    //добавляем элемент списка
    if(pdleFirst == NULL)
    {
	pthread_mutex_lock(&mListMutex);
	//если список пуст - назначаем первый элемент
	pdleFirst = new DirListElement(in_psdPtr, NULL);
	pthread_mutex_unlock(&mListMutex);
	return;
    }

    pthread_mutex_lock(&mListMutex);
    //ищем конец списка
    pdleList = pdleFirst;
    while(pdleList->pdleNext != NULL)
    {
	pdleList = pdleList->pdleNext;
    }

    //добавляем директорию в конец списка
    pdleList->pdleNext = new DirListElement(in_psdPtr, pdleList);
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::SubQueueElement(SomeDirectory const * const in_psdPtr)
{
    DirListElement *pdleList;

    //если список пуст - выходим
    if(pdleFirst == NULL)
	return;

    pthread_mutex_lock(&mListMutex);
    //ищем заданный элемент
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
	if(pdleList->psdDirectory == in_psdPtr)
	{
	    //удаляем найденный элемент из списка
	    if(pdleFirst == pdleList)
	      pdleFirst = pdleList->pdleNext;
	    delete pdleList;
	    pthread_mutex_unlock(&mListMutex);
	    return;
	}
    }
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::SubQueueElement(int in_nDirFd)
{
    int nDirFd;
    DirListElement *pdleList;

    //если список пуст - выходим
    if(pdleFirst == NULL)
	return;

    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
	nDirFd = pdleList->psdDirectory->GetDirFd();
// 	fprintf(stderr, "DescriptorsList::SubQueueElement() : %d, %d\n", (int)nDirFd, (int)in_nDirFd); //отладка!!!
	if(nDirFd == in_nDirFd)
	{
	    //удаляем элемент из очереди
	    if(pdleFirst == pdleList)
	      pdleFirst = pdleList->pdleNext;
	    delete pdleList;
	    pthread_mutex_unlock(&mListMutex);
	    return;
	}
	pdleList = pdleList->pdleNext;
    }
    pthread_mutex_unlock(&mListMutex);
}

//функция для отладки
int DescriptorsList::GetFd(void)
{
    static DirListElement *pdleLast;

    if(pdleLast == NULL)
    {
	pdleLast = pdleFirst;
	return pdleFirst->psdDirectory->GetDirFd();
    }

    pdleLast = pdleLast->pdleNext;

    if(pdleLast == NULL)
	return -1;

    return pdleLast->psdDirectory->GetDirFd();
}

//вывести список директорий
void DescriptorsList::PrintList(void)
{
    int nDirFd;
    DirListElement *pdleList;
    FileData *pfdData;

    //если список пуст - выходим
    if(pdleFirst == NULL)
    {
	fprintf(stderr, "\nDescriptorsList::PrintList() : DirectoryList is empty!\n\n");
	return;
    }

    fprintf(stderr, "\nDescriptorsList::PrintList() : DirectoryList: начало списка.\n");
    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
	pfdData = pdleList->psdDirectory->GetFileData();
	if(pfdData != NULL)
	{
	  nDirFd = pdleList->psdDirectory->GetDirFd();
	  fprintf(stderr, "DescriptorsList::PrintList() : DirectoryList: файл %s, fd=%d, inode=%d.\n", pdleList->psdDirectory->GetDirName(), pdleList->psdDirectory->GetDirFd(), (int)pfdData->stData.st_ino);
	}
	pdleList = pdleList->pdleNext;
    }
    fprintf(stderr, "DescriptorsList::PrintList() : DirectoryList: конец списка.\n\n");
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::UpdateList(void)
{
    DirListElement *pdleList;
    struct sigaction signal_data;
    sigset_t set;
    char *pPath = NULL;
    int nDirFd;
    FileData *pfdData;

    //надо исключить повторное обновление тех директорий, доступ к которым запрещён (!)
    //...

//     fprintf(stderr, "DescriptorsList::UpdateList() : start\n"); //отладка!!!
    if(pdleFirst == NULL)
    {
	fprintf(stderr, "DescriptorsList::UpdateList(): the list is empty!\n"); //отладка!!!
	return;
    }
//    pthread_mutex_lock(&mListMutex); (?)
    //ищем новые директории и обновляем их
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
// 	fprintf(stderr, "DescriptorsList::UpdateList(), inode, name : %d, \"%s\"\n", (int)(pdleList->psdDirectory->GetFileData())->stData.st_ino, pdleList->psdDirectory->GetDirName()); //отладка!!!

	if(pdleList->psdDirectory == NULL)
	{
	  fprintf(stderr, "DescriptorsList::UpdateList() Update: NULL!\n"); //отладка!!!
	  continue;
	}
// 	pPath = pdleList->psdDirectory->GetDirName(); //отладка!!!
// 	fprintf(stderr, "DescriptorsList::UpdateList() 1: %s\n", (pPath==NULL)?"NULL!!!":pPath); //отладка!!!
// 	pPath = NULL; //отладка!!!
	//pdleList->psdDirectory->PrintSnapshot(); //отладка!!!
	pfdData = pdleList->psdDirectory->GetFileData();
// 	fprintf(stderr, "DescriptorsList::UpdateList() 2: pName=\"%s\"\n", (pfdData==NULL)?"NULL!!!":pfdData->pName); //отладка!!!
	//создаём слепки для новых директорий в списке
	if(pdleList->psdDirectory != NULL && pdleList->psdDirectory->IsSnapshotNeeded() && pfdData != NULL)
	{
	    pdleList->psdDirectory->MakeSnapshot(true);
	    //проверяем, открыта уже директория или ещё нет
	    nDirFd = pfdData->nDirFd;
// 	    if(pfdData->nDirFd >= 0) //отладка!!! (?)
// 	      close(pfdData->nDirFd); //отладка!!! (?)
	    if(nDirFd == -1) //отладка!!!
	    {
	      pPath = pdleList->psdDirectory->GetFullPath();
	      if(pPath != NULL)
	      {
		//учесть при инкапсуляции (!)
// 		fprintf(stderr, "DescriptorsList::UpdateList(), check : \"%s\"\n", pPath); //отладка!!!
		pfdData->nDirFd = open(pPath, O_RDONLY);
		delete [] pPath;
	      }
	    }
	    nDirFd = pfdData->nDirFd;
	    if(nDirFd >= 0)
	    {
		//назначаем обработчик сигнала для дескриптора
		if(fcntl(nDirFd, F_SETSIG, SIGUSR1) != -1)
		{
		    if(fcntl(nDirFd, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_RENAME/*|DN_ACCESS*/) == -1)
		    {
			struct stat st; //отладка!!!
			fstat(nDirFd, &st); //отладка!!!
			pPath = pdleList->psdDirectory->GetFullPath(); //отладка!!!
			fprintf(stderr, "DescriptorsList::UpdateList() : \"%s\", fd=%d, st.st_ino=%d\n", pPath, nDirFd, (int)st.st_ino); //отладка!!!
			if(pPath != NULL) //отладка!!!
			  delete [] pPath; //отладка!!!

			perror("DescriptorsList::UpdateList() ???невозможно назначить обработку для дескриптора");
		    }
		}
		else
		{
		    perror("DescriptorsList::UpdateList(), fcntl");
		    close(nDirFd);
		    //учесть после инкапсуляции (!)
		    pfdData->nDirFd = -1;
		}
	    }
	}
	else
	{
// 	  if(pdleList->psdDirectory->IsSnapshotNeeded())
// 	    fprintf(stderr, "DescriptorsList::UpdateList() 3: no name or file data!\n");
	}
	pdleList = pdleList->pdleNext;
    }
//    pthread_mutex_unlock(&mListMutex); (?)
}

SomeDirectory *DescriptorsList::GetDirectory(int in_nFd)
{
    DirListElement *pdleDir;

    if(pdleFirst == NULL)
	return NULL;

    pdleDir = pdleFirst;
    while(pdleDir != NULL)
    {
	if(pdleDir->psdDirectory->GetDirFd() == in_nFd)
	    return pdleDir->psdDirectory;
	pdleDir = pdleDir->pdleNext;
    }

    return NULL;
}

/*********************************DirListElement*********************************/

DirListElement::DirListElement()
{
    psdDirectory = NULL;
    pdleNext = NULL;
    pdlePrev = NULL;
}

//!!!Возможно, надо сделать в RootMonitor ссылку на SomeDirectory вместо наследования
//!!!для избежания создания ещё одной копии описания директории в этом конструкторе
//!!!т.к. создаётся ещё она копия слепка одной и той же директории
DirListElement::DirListElement(SomeDirectory *in_psdDirectory, DirListElement * const in_pdlePrev)
{
    psdDirectory = in_psdDirectory;
    pdlePrev = in_pdlePrev;

//     fprintf(stderr, "DirListElement::DirListElement() : inode=%d\n", (int)(psdDirectory->GetFileData())->stData.st_ino); //отладка!!!
    //исключаем выпадение части элементов списка
    if(in_pdlePrev != NULL)
    {
        pdleNext = in_pdlePrev->pdleNext;
	//если за предшествующим элементом есть последующий, меняем у последующего предыдущий на this
        if(in_pdlePrev->pdleNext != NULL)
	    in_pdlePrev->pdleNext->pdlePrev = this;
	in_pdlePrev->pdleNext = this;
    }
    else
    {
	pdleNext = NULL;
    }
}

DirListElement::DirListElement(FileData *in_pfdData, SomeDirectory * const in_psdParent, DirListElement * const in_pdlePrev)
{
    //осторожно! Родительский каталок не ищется! Надо делать! Краш!!!
    psdDirectory = new SomeDirectory(in_pfdData, in_psdParent, true); //снимок обязан присутствовать в элементе очереди
    pdlePrev = in_pdlePrev;
    //исключаем выпадение части элементов списка
    if(in_pdlePrev != NULL)
    {
        pdleNext = in_pdlePrev->pdleNext;
	//если за предшествующим элементом есть последующий, меняем у последующего предыдущий на this
        if(in_pdlePrev->pdleNext != NULL)
	    in_pdlePrev->pdleNext->pdlePrev = this;
	in_pdlePrev->pdleNext = this;
    }
    else
    {
	pdleNext = NULL;
    }
}

DirListElement::~DirListElement()
{
//     SomeDirectory *psdList; //отладка!!!
//     psdList = psdDirectory->GetParent(); //отладка!!!
//     if(psdList != NULL) //отладка!!!
//     {
//       fprintf(stderr, "DirListElement::~DirListElement() : printing snapshot for \"%s\":\n", psdList->GetDirName()); //отладка!!!
//       psdList->PrintSnapshot(); //отладка!!!
//     }
    //полностью удаляем директорию
    //FileData удаляется из своего слепка автоматически
    delete psdDirectory;
    //обновляем очередь
    if(pdlePrev != NULL)
      pdlePrev->pdleNext = pdleNext;
    if(pdleNext != NULL)
      pdleNext->pdlePrev = pdlePrev;
}
