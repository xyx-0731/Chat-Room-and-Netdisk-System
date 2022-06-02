#include "mytcpserver.h"
#pragma execution_character_set("utf-8")

MyTcpserver::MyTcpserver()
{

}

MyTcpserver &MyTcpserver::getInstance()
{
    static MyTcpserver instance;
    //qDebug() << "创建成功";
    return instance;

}

void MyTcpserver::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected";
    MyTcpSocket* pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, &MyTcpSocket::offline, this, &MyTcpserver::deleteSocket); //有offline信号就删除socket


}

void MyTcpserver::resend(const char *pername, PDU *pdu)
{
    if (nullptr == pername || nullptr == pdu) {
        return;
    }
    QString strname = pername;
    for (int i = 0; i < m_tcpSocketList.size(); i++) {
        if (strname == m_tcpSocketList.at(i)->getname()) {
            m_tcpSocketList.at(i)->write((char*)pdu, pdu->uiPDULen);
            break;
        }
    }

}

void MyTcpserver::deleteSocket(MyTcpSocket *mysocket)
{
    QList<MyTcpSocket*>::iterator it = m_tcpSocketList.begin();
    for (; it != m_tcpSocketList.end(); it++) {
        if (mysocket == *it) {
            m_tcpSocketList.erase(it);
            *it = nullptr;
            break;
        }
    }
    qDebug() << "删除成功";
    for (int i = 0; i < m_tcpSocketList.size(); i++) {
        qDebug() << m_tcpSocketList.at(i)->getname();
    }

}

