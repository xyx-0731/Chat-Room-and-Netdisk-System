#ifndef MYTCPERVER_H
#define MYTCPERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QList>
#include "mytcpsocket.h"

class MyTcpserver : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpserver();
    static MyTcpserver& getInstance(); //单例模式
    void incomingConnection(qintptr socketDescriptor);
    void resend(const char*pername,PDU* pdu); //服务器转发消息（从客户端1到客户端2，用于加好友）

public slots:
    void deleteSocket(MyTcpSocket* mysocket); //用于删除socket的信号槽


private:
    QList<MyTcpSocket*>  m_tcpSocketList; //保存所有客户端的socket

};

#endif // MYTCPERVER_H
