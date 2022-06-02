#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include "mytcpserver.h"
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

namespace Ui {
class tcpserver;
}

class tcpserver : public QWidget
{
    Q_OBJECT

public:
    explicit tcpserver(QWidget *parent = nullptr);
    void loadConfig();  //加载配置文件
    ~tcpserver();

private:
    Ui::tcpserver *ui;
    QString m_strIP;    //ip信息
    quint16 m_usPort;   //quint 代表无符号的短整型

};

#endif // TCPSERVER_H
