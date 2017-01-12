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

int gIsChanged;

//обработчик сигнала
void sig_handler(int nsig, siginfo_t *siginfo, void *context)
{
    struct signal_path_data *arg;

    switch(nsig)
    {
	case SIGIO:
	    {
		gIsChanged = 3;
	    }
	    break;
	case SIGUSR1:
	    {
		fprintf(stderr, "SIGUSR1\n"); //отладка!!!
	        fprintf(stderr, "siginfo->si_fd=%d\n", siginfo->si_fd); //отладка!!!

		//+нужен запуск потока
		//+требуется замена на список/очередь
		gIsChanged = siginfo->si_fd;
	    }
	    break;
    }
}

//поток обработки сообщения
void *file_thread(void *arg)
{
    int fd;

    fprintf(stderr, "start thread\n"); //отладка!!!

    for(;;)
    {
	if(gIsChanged > 0)
	{
	    fd = gIsChanged;
	    gIsChanged = 0;

	    //вешаем сигнал на дескриптор
	    if(fcntl(fd, F_SETSIG, SIGUSR1) < 0)
	    {
		close(fd);
		fprintf(stderr, "Can not init signal for fd\n");
		continue;
	    }
	    //устанавливаем типы оповещений
	    if(fcntl(fd, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_RENAME) < 0)
	    {
		close(fd);
		fprintf(stderr, "Can not set types for the signal\n");
		continue;
	    }
	    
	    fprintf(stderr, "Some operation with file\n");
	}
	//+требуется добавить блокировку вместо ожидания
        usleep(100000);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int i;
    int fd[1024];
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
	    continue;
	}

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
    
    //получаем данные а файле
    fstat(fd[0], &st);

    //обнууляем буфер
    memset(buff, 0, sizeof(buff));
    //считываем файл в буфер
    read(fd[0], buff, st.st_size);
    
    //выводим содуржимое буфера
    fprintf(stderr, "%s\n", buff);
    
    //закрываем файл
    for(i = 0; i < sizeof(fd)/sizeof(fd[0]); ++i)
    {
	if(fd[i] > 0)
    	    close(fd[i]);
    }
    
    return 0;
}
