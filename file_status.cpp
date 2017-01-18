//File Status by Bombo
//10.01.2017

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<dirent.h> //opendir(), readdir()
#include<stdlib.h>
#include<pthread.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/select.h>

#include"RootMonitor.h"

void *fd_queue_thread(void *arg);

//временная замена очереди
int gIsChanged;

//мьютекс, сообщающий о присутствии дескрипторов в очереди на обработку
//можно будет попытаться перенести этот мьютекс в класс RootMonitor
pthread_mutex_t handler_thread_mutex = PTHREAD_MUTEX_INITIALIZER;

//мьютекс, блокирующий добавление/удаление объектов в/из очереди
//в последствии можно будет перенести этот мьютекс в класс очереди
pthread_mutex_t queue_thread_mutex = PTHREAD_MUTEX_INITIALIZER;

//обработчик сигнала об изменении в некотором файле
void sig_handler(int nsig, siginfo_t *siginfo, void *context)
{
    pthread_t thread;
    pthread_attr_t attr;
    int *pFd;

    switch(nsig)
    {
	case SIGIO:
	    {
		//gIsChanged = 3;
	    }
	    break;
	case SIGUSR1:
	    {
		//в обработчике не должно быть функций вывода на экран!!!
		//fprintf(stderr, "SIGUSR1\n"); //отладка!!!
	        //fprintf(stderr, "siginfo->si_code=%d\n", siginfo->si_code); //отладка!!!
	        //fprintf(stderr, "siginfo->si_band=%ld\n", siginfo->si_band); //отладка!!!
	        //fprintf(stderr, "siginfo->si_fd=%d\n", siginfo->si_fd); //отладка!!!

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
}

//поток постановки дескриптора в очередь на обработку
//поскольку таких потоков может породиться много за короткое время,
//сама обработка происходит в другом потоке
void *fd_queue_thread(void *arg)
{
    int nFd;

    nFd = *((int *) arg);

    //а вот тут должно быть непосредственно добавление дескриптора в очередь
    //с поиском среди объектов класса SomeDirectory
    //...

    gIsChanged = nFd; //временно

    delete (int *)arg; //освобождаем ранее выделенную для параметра память

    //сообщаем о новом сообщении обработчику
    pthread_mutex_unlock(&handler_thread_mutex);

    pthread_exit(NULL);
}

//поток обработки очереди дескрипторов
void *file_thread(void *arg)
{
    int fd;

    fprintf(stderr, "start thread\n"); //отладка!!!

    for(;;)
    {
	if(gIsChanged > 0)
	{
	    fd = gIsChanged;
//	    pthread_mutex_lock(&handler_thread_mutex);
	    gIsChanged = 0;
//	    pthread_mutex_unlock(&handler_thread_mutex);

	    //после обработки сигнала обработчик сбрасывается на тот, что был по умолчанию
	    //поэтому обновляем обработчик сигнала для дескриптора
	    //вешаем сигнал на дескриптор
	    if(fcntl(fd, F_SETSIG, SIGUSR1) < 0)
	    {
		close(fd);
		fprintf(stderr, "Can not init signal for fd\n");
		continue;
	    }
	    //устанавливаем типы оповещений
	    if(fcntl(fd, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_RENAME/*|DN_ACCESS*/) < 0)
	    {
		close(fd);
		fprintf(stderr, "Can not set types for the signal\n");
		continue;
	    }

	    fprintf(stderr, "Some operation with file: fd=%d\n", fd);
	}
	//+требуется добавить семафор/мьютекс вместо ожидания
//        usleep(100000);

        pthread_mutex_lock(&handler_thread_mutex); //проверка наличая изменений в файлах

        pthread_mutex_lock(&queue_thread_mutex); //очередь обрабатываемых дескрипторов
        //fprintf(stderr, "mutex is locked\n"); //отладка!!!
        //тут должно быть удаление дескриптора из очереди
        //...
        pthread_mutex_unlock(&queue_thread_mutex); //освобождение очереди дескрипторов
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int i;
    int fd[1024];
    DIR *dir[1024];
    pid_t pid;
    char filename[256];
    char buff[2048];
    struct stat st;
    sigset_t set;
    struct sigaction signal_data;
    union sigval path;
    pthread_t thread;
    pthread_attr_t attr;
    RootMonitor *rmProject;
    struct dirent *dir_val;

    char szRoot[] = "./test";
    char szRootUpper[] = "../versions";
    rmProject = new RootMonitor(szRoot);
    stat(szRoot, &st);
    fprintf(stderr, "inode=%ld, mode=%d, DIR=%d\n", st.st_ino, st.st_mode & S_IFDIR, S_IFDIR);
    rmProject->SetRootPath(szRootUpper);
    if(stat(szRootUpper, &st) >= 0)
        fprintf(stderr, "inode=%ld, mode=%d, DIR=%d\n", st.st_ino, st.st_mode & S_IFDIR, S_IFDIR);
    else
	perror("stat():");
    delete rmProject;

    pthread_mutex_unlock(&queue_thread_mutex); //освобождение (запуск) обработчика очереди дескрипторов

    //проверяем количество аргументов
    if(argc <= 1)
    {
	fprintf(stderr, "USAGE: file_status <filename 1> ... <filename N>\n");
	return -1;
    }

    memset(fd, -1, sizeof(fd)/sizeof(fd[0]));

    for(i = 0; i < argc-1 && i < sizeof(fd)/sizeof(fd[0]); ++i)
    {
	//обнуляем имя файла
	memset(filename, 0, sizeof(filename));
	//копируем имя файла
	strncpy(filename, argv[i+1], sizeof(filename));

	//пытаемся открыть файл
	fd[i] = open(filename, O_RDONLY);
	//при ошибке - выходим
	if(fd[i] < 0)
	{
	    fprintf(stderr, "Can not open file %s\n", filename);
	    perror("open():");
	    continue;
	}

        dir[i] = fdopendir(fd[i]);
        memset(buff, 0, sizeof(buff));
        dir_val = readdir(dir[i]);
        if(dir_val != NULL)
            fprintf(stderr, "d_name=%s, d_ino=%d, d_off=%ld\n", dir_val->d_name, (int)dir_val->d_ino, dir_val->d_off);
        dir_val = readdir(dir[i]);
        if(dir_val != NULL)
            fprintf(stderr, "d_name=%s, d_ino=%d, d_off=%ld\n", dir_val->d_name, (int)dir_val->d_ino, dir_val->d_off);
        while(dir_val != NULL)
	{
	    dir_val = readdir(dir[i]);
	    if(dir_val != NULL)
		fprintf(stderr, "d_name=%s, d_ino=%d, d_off=%ld\n", dir_val->d_name, (int)dir_val->d_ino, dir_val->d_off);
        }
        //closedir(dir[i]);

	//обнуляем описание сигнала
        memset(&signal_data, 0, sizeof(signal_data));
	//назначаем обработчик сигнала
        signal_data.sa_sigaction = &sig_handler;
        //сопровождаем сигнал дополнительной информацией
        signal_data.sa_flags = SA_SIGINFO;
	//обнуляем маску блокируемых сигналов
        sigemptyset(&set);
	//инициализируем маску
        signal_data.sa_mask = set;

	if(sigaction(SIGUSR1, &signal_data, NULL) < 0)
        {
	    close(fd[i]);
	    fprintf(stderr, "Can not activate signal\n");
	    continue;
	}

	//вешаем сигнал на дескриптор
	if(fcntl(fd[i], F_SETSIG, SIGUSR1) < 0)
	{
	    close(fd[i]);
	    fprintf(stderr, "Can not init signal for fd\n");
	    perror("fcntl():");
	    continue;
	}

	//устанавливаем типы оповещений
	if(fcntl(fd[i], F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_RENAME) < 0)
	{
	    close(fd[i]);
	    fprintf(stderr, "Can not set types for the signal\n");
	    continue;
	}
    }

    //запускаем поток обработки сигнала
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 102400);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, file_thread, NULL);
    pthread_attr_destroy(&attr);

    for(;;)
    {
	usleep(100000);
    }

    //получаем данные о файле
    fstat(fd[0], &st);

    //обнуляем буфер
    memset(buff, 0, sizeof(buff));
    //считываем файл в буфер
    read(fd[0], buff, st.st_size);

    //выводим содержимое буфера
    fprintf(stderr, "%s\n", buff);

    //закрываем файл
    for(i = 0; i < sizeof(fd)/sizeof(fd[0]); ++i)
    {
	if(fd[i] > 0)
	    close(fd[i]);
    }

    return 0;
}
