#include "tcpserver.h"
#include "ui_tcpserver.h"
#pragma execution_character_set("utf-8")


tcpserver::tcpserver(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tcpserver)
{
    ui->setupUi(this);
    loadConfig();
    MyTcpserver::getInstance().listen(QHostAddress(m_strIP),m_usPort);   //监听客户端
}

void tcpserver::loadConfig()
{
    QFile file(":/Client.config");
    if (file.open(QIODevice::ReadOnly)) {   //只读的方式打开
        QByteArray badata = file.readAll();
        QString strdata = badata.toStdString().c_str(); //字节-->string-->char*
        file.close();
        strdata.replace("\r\n", " ");   //将换行换成空格
        QStringList strList = strdata.split(" ");     //用于分割字符串,返回值为一个字符串列表

        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();
        qDebug() << "ip" << m_strIP << " port" << m_usPort; //测试
    }
    else {
        QMessageBox::critical(this, "打开配置文件", "打开配置文件失败");
    }
}


tcpserver::~tcpserver()
{
    delete ui;
}
