
#include <QCoreApplication>
#include "classifierapplication.h"


#define MAJOR   2
#define MINOR   0
#define PATCH   0

int main(int argc, char *argv[])
{

    if(argc >= 2)
    {
        if(strcmp(argv[1],"-v")==0)
        {
            printf("%d.%d.%d", MAJOR,MINOR,PATCH);
            return 0;
        }
    }


    ClassifierApplication a(argc, argv);
    return a.exec();
}
