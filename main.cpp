//File Status by Bombo
//10.01.2017

//потоки:
//1 - поток обработки ОЧЕРЕДИ дескрипторов, созданной из обработчика сигнала, стартует с этим приложением
//2 - потоки добавления дескрипторов в очередь на обработку, запускаемые из обработчика сигнала
//3 - поток обработки СПИСКА открытых директорий (дескрипторов), стартует вместе с этим приложением

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<dirent.h> //opendir(), readdir()
#include<stdlib.h>
#include<pthread.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/select.h>
#include<sys/resource.h>

#include"RootMonitor.h"

#define THREAD_SLEEP_TIME 0

RootMonitor *rmProject;

void *fd_queue_thread(void *arg);

//обработчик сигнала об изменении в некотором файле
void sig_handler(int nsig, siginfo_t *siginfo, void *context)
{
    pthread_t thread;
    pthread_attr_t attr;
    int *pFd;

//    fprintf(stderr, "старт обработчика сигнала\n"); //отладка!!!
    switch(nsig)
    {
	case SIGUSR1:
	    {
		//готовим параметр для передачи в поток
		pFd = new int(siginfo->si_fd); //выделяем память для параметра
		//создаём поток постановки дескриптора на обработку
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, 102400);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&thread, &attr, fd_queue_thread, (void *)pFd);
		pthread_attr_destroy(&attr);
	    }
	    break;
    }
//    fprintf(stderr, "выход из обработчика сигнала\n"); //отладка!!!
}

//поток постановки дескриптора в очередь на обработку
//поскольку таких потоков может породиться много за короткое время,
//сама обработка происходит в другом потоке
void *fd_queue_thread(void *arg)
{
    int nFd;

//    fprintf(stderr, "старт fd_queue_thread\n"); //отладка!!!
    nFd = *((int *) arg);
    //а вот тут должно быть непосредственно добавление дескриптора в очередь
    //с поиском среди объектов класса SomeDirectory

    delete (int *)arg; //освобождаем ранее выделенную для параметра память

    if(RootMonitor::pdqQueue != NULL)
	RootMonitor::pdqQueue->AddDescriptor(nFd);
    else
    {
	fprintf(stderr, "очередь ещё не инициализирована!\n");
	pthread_exit(NULL);
    }

    //сообщаем о новом сигнале обработчику
    //запуск потока обработки очереди происходит автоматически при добавлении дескриптора в очередь (?)
    //поэтому освобождать мьютекс в этом месте уже не требуется
    //pthread_mutex_unlock(&handler_thread_mutex);
    pthread_mutex_unlock(&(RootMonitor::mDescThreadMutex));
//    fprintf(stderr, "выход из fd_queue_thread\n\n"); //отладка!!!
    pthread_exit(NULL);
}

//поток обработки очереди дескрипторов
void *file_thread(void *arg)
{
    int nFd;
    char *pPath;
    SomeDirectory *psdDir;

//    fprintf(stderr, "start file_thread\n"); //отладка!!!

    for(;;)
    {
	//чуть притормозим
	if(THREAD_SLEEP_TIME)
	  usleep(THREAD_SLEEP_TIME);
	pthread_mutex_lock(&(RootMonitor::mDescThreadMutex));
//	fprintf(stderr, "старт обработчика очереди дескрипторов\n"); //отладка!!!
	pthread_mutex_lock(&(RootMonitor::mDescQueueMutex)); //очередь обрабатываемых дескрипторов
//	fprintf(stderr, "заняли очередь\n"); //отладка!!!
        //удаление дескриптора из очереди дескрипторов
        if(RootMonitor::pdqQueue != NULL)
        {
	    nFd = RootMonitor::pdqQueue->GetDescriptor();
	    while(nFd != -1)
	    {
//		fprintf(stderr, "nFd=%d, обработчик\n", nFd); //отладка!!!
		//сравнение слепков директорий
		psdDir = RootMonitor::pdlList->GetDirectory(nFd);
		if(psdDir != NULL)
		    psdDir->CompareSnapshots();
		else
		    fprintf(stderr, "fd_queue_thread() : No such directory!\n");

		if(nFd >= 0)
		{
		    //после обработки сигнала обработчик сбрасывается на тот, что был по умолчанию
		    //поэтому здесь обновляем обработчик сигнала для дескриптора
		    //вешаем сигнал на дескриптор
		    //возможно, имеет смысл убрать это обновление в метод класса DecriptorsQueue или DescriptorsList
		    if(fcntl(nFd, F_SETSIG, SIGUSR1) < 0)
		    {
			perror("fcntl");
			close(nFd);
			fprintf(stderr, "Can not init signal for fd\n");
			continue;
		    }
		    //устанавливаем типы оповещений
		    if(fcntl(nFd, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_RENAME/*|DN_ACCESS*/) < 0)
		    {
			perror("fcntl");
			close(nFd);
			fprintf(stderr, "Can not set types for the signal\n");
			continue;
		    }
		    //pPath = psdDir->GetFullPath();
		    //fprintf(stderr, "There are some changes in \"%s\" directory!\n", (pPath==NULL)?"?":pPath);
		    //if(pPath != NULL)
			//delete [] pPath;
		}
		nFd = RootMonitor::pdqQueue->GetDescriptor();
	    }
	}
	pthread_mutex_unlock(&(RootMonitor::mDescQueueMutex)); //освобождение очереди дескрипторов
//	fprintf(stderr, "конец обработки file_thread\n"); //отладка!!!
    }
    pthread_exit(NULL);
}

//поток обработки списка найденных директорий
void *directory_thread(void *arg)
{
    for(;;)
    {
	pthread_mutex_lock(&(RootMonitor::mDirThreadMutex));
// 	fprintf(stderr, "старт потока обработки списка директорий\n"); //отладка!!!
	//обработка списка директорий
	//(поиск новых директорий в списке, открытие, создание слепка)
	RootMonitor::pdlList->UpdateList();
//  	RootMonitor::pdlList->PrintList();
	if(THREAD_SLEEP_TIME)
	  usleep(THREAD_SLEEP_TIME);
    }
    pthread_exit(NULL);
}

void *send_json_thread(void *)
{
  for(;;)
  {
    pthread_mutex_lock(&(RootMonitor::mSendJSONThreadMutex));
    if(rmProject != NULL)
      rmProject->SendChangesToServer();
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int i;
    pid_t pid;
    char filename[256];
    char buff[2048];
    struct stat st;
    sigset_t set;
    struct sigaction signal_data;
    union sigval path;
    pthread_t thread;
    pthread_attr_t attr;
    struct dirent *dir_val;
    rlimit r;

    //увеличиваем лимит открытых файлов для текущего процесса
    r.rlim_cur = 8192;
    r.rlim_max = 8192;
    if(setrlimit(RLIMIT_NOFILE, &r) < 0)
      perror("main(), setrlimit error:");
    getrlimit(RLIMIT_NOFILE, &r);
    fprintf(stderr, "main() : nofile limits: rlim_cur=%ld, rlim_max=%ld\n", r.rlim_cur, r.rlim_max);

    pthread_mutex_unlock(&(RootMonitor::mDescListMutex));
    pthread_mutex_unlock(&(RootMonitor::mDescQueueMutex));
    pthread_mutex_lock(&(RootMonitor::mSendJSONThreadMutex));

    //проверяем количество аргументов
    if(argc <= 2)
    {
	fprintf(stderr, "USAGE: %s <path to directory 1> ... <path to directory N> <server URL>\n", argv[0]);
	return -1;
    }

    //обнуляем описание сигнала
    memset(&signal_data, 0, sizeof(signal_data));
    //назначаем обработчик сигнала
    signal_data.sa_sigaction = &sig_handler;
    //сопровождаем сигнал дополнительной информацией
    signal_data.sa_flags = SA_SIGINFO;
    //обнуляем маску блокируемых сигналов
    sigemptyset(&set);
    //добавляем блокировку обработки SIGUSR1
    //sigaddset(&set, SIGUSR1);
    //инициализируем маску
    signal_data.sa_mask = set;
    if(sigaction(SIGUSR1, &signal_data, NULL) < 0)
	fprintf(stderr, "sigaction(): can not activate signal\n");

    for(i = 0; i < argc-2; ++i)
    {
	//обнуляем имя файла
	memset(filename, 0, sizeof(filename));
	//копируем имя файла
	strncpy(filename, argv[i+1], sizeof(filename));
	if(filename[0] != '/')
	{
	  fprintf(stderr, "main() wrong path: A path to project directory must be full! (\"%s\")\n", filename);
	  continue;
	}

	//пытаемся открыть файл
	rmProject = new RootMonitor(filename);
	stat(filename, &st);
	fprintf(stderr, "Project path: \"%s\", inode=%ld, mode=%d, DIR=%d\n", filename, st.st_ino, st.st_mode & S_IFDIR, S_IFDIR);
	break;
    }

    fprintf(stderr, "Server URL: %s\n", argv[argc-1]);
    rmProject->SetServerURL(argv[argc-1]);

    //запускаем поток обработки сигнала
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 102400);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, file_thread, NULL);
    pthread_attr_destroy(&attr);

    //запускаем поток обработки списка найденных директорий
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 102400);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, directory_thread, NULL);
    pthread_attr_destroy(&attr);

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 102400);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, send_json_thread, NULL);
    pthread_attr_destroy(&attr);

//      pthread_mutex_unlock(&(RootMonitor::mDirThreadMutex));
//     RootMonitor::pdlList->PrintList();

    usleep(2000000); //отладка!!!
    char *list; //отладка!!!
    list = rmProject->GetJSON(rmProject->GetLastSessionNumber()); //отладка!!!
    fprintf(stderr, "%s\n", (list==NULL)?"NULL":list); //отладка!!!
    if(list != NULL) //отладка!!!
      delete [] list; //отладка!!!

    rmProject->SendChangesToServer(); //отладка!!!

    for(;;)
    {
	usleep(70000000);
    }

    return 0;
}
