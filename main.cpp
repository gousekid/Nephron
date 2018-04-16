#include "mainapplicaton.h"
#include <stdio.h>


#define MAJOR   1
#define MINOR   1
#define PATCH   2

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h>
#include "global.hpp"

#define MY_SEMAPHORE "Nephron_main"

sem_t* gRunning;

bool IsRunning()
{
    bool ret = false;


    gRunning = sem_open(MY_SEMAPHORE, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 1);

    if(gRunning == SEM_FAILED)
    {
        if(errno == EEXIST)
        {
            ret = true;
        }
    }

    return ret;
}

MainApplication *a;
void signal_exit(int sig)
{
    printf("INT SIG received (%d)", sig);
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Receive Exit Signal(%1)").arg(sig));

    a->exit(1);
}


int main(int argc, char *argv[])
{


    sem_close(gRunning);
    sem_unlink(MY_SEMAPHORE);
    if(argc >= 2)
    {
        if(strcmp(argv[1],"-v")==0)
        {
            printf("%d.%d.%d", MAJOR,MINOR,PATCH);
            return 0;
        }
    }

    signal(SIGINT, signal_exit);
    signal(SIGTERM, signal_exit);
    if(IsRunning())
    {
        printf("%s already exists!, i'm guit!\n", MY_SEMAPHORE);
        return 0;
    }

    QCoreApplication::setOrganizationName("NTN");
    QCoreApplication::setOrganizationDomain("ntntek.com");
    QCoreApplication::setApplicationName("Nephron");


    //MainApplication a(argc, argv);

    a = new MainApplication(argc, argv);

    int nRet = a->exec();
    delete a;

    printf("exit code %d", nRet);
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Main Application Exit code(%1)").arg(nRet));

    if (gRunning != NULL)
    {
        sem_close(gRunning);
        sem_unlink(MY_SEMAPHORE);
        gRunning = NULL;
    }

    //dev mode // forced execution
    QProcess process;
    process.execute("/home/superbin/startapp.sh");

    return nRet;
}
