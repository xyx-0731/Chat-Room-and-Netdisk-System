#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include "protocol.h"
#include "opedb.h"
#include <QFile>
#include <QTimer>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getname();
    void copyDir(QString strSrcDir, QString strDesDir); //拷贝文件夹

signals:
    void offline(MyTcpSocket*mysocket);

public slots:
    void recvMsg();
    void clientoffline();   //客户端下线
    void sendFileToClient();    //发送数据给客户端

private:
    QString m_strName; // 记录用户登录名
    QString m_strPsd; // 记录用户登录密码
    QFile m_file;
    qint64 m_iTotal;    //文件总大小
    qint64 m_iRecved;   //文件接收大小
    bool m_Upload;    //判断是否处于上传文件的状态

    QTimer* m_pTimer;
};

#endif // MYTCPSOCKET_H
