#include "tcpclient.h"
#include <QApplication>
#pragma execution_character_set("utf-8")


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    //tcpclient w;
    //w.show();
    tcpclient::getInstance().show();

    return a.exec();
}
