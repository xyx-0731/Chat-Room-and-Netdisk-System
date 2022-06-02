#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include "ui_tcpclient.h"
#include<QFile>
#include<QDebug>
#include<QMessageBox>
#include<QTcpSocket>
#include<QHostAddress>
#include "protocol.h"
#include"opewidget.h"




namespace Ui {
class tcpclient;
}

class tcpclient : public QWidget
{
    Q_OBJECT

public:
    explicit tcpclient(QWidget *parent = nullptr);
    ~tcpclient();
    void loadConfig();  //加载配置文件
    static tcpclient& getInstance();
    QTcpSocket& getTcpSocket();
    QString loginName();
    QString curPath();  //当前目录
    void setCurPath(QString strCurPath);    //设置当前目录

public slots:
    void showConnect();
    void recvMsg();



private slots:
    //void on_send_pb_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:

    Ui::tcpclient *ui;
    QString m_strIP;    //ip信息
    quint16 m_usPort;   //quint 代表无符号的短整型

    //连接服务器,和服务器进行数据交互
    QTcpSocket m_tcpSocket; //创建一个qt的tcp套接字
    QString m_strLoginName; //登录的用户名
    QString m_strPerName;   //添加好友的用户名
    QString m_strCurPath;   //当前所在目录
    QFile m_file;

};

#endif // TCPCLIENT_H
