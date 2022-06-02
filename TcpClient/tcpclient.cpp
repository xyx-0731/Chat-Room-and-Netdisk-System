#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "online.h"
#include "privatechat.h"
#include "friend.h"
#pragma execution_character_set("utf-8")

tcpclient::tcpclient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tcpclient)
{
    ui->setupUi(this);
    resize(600, 400);

    loadConfig();
    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(showConnect()));
    //connect(ui->cancel_pb, &QPushButton::clicked, this, &tcpclient::on_cancel_pb_clicked);
    connect(&m_tcpSocket, &QIODevice::readyRead, this, &tcpclient::recvMsg);
    //链接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);

}

tcpclient::~tcpclient()
{
    delete ui;
}

void tcpclient::loadConfig(){
    QFile file(":/Client.config");
    if (file.open(QIODevice::ReadOnly)) {   //只读的方式打开
        QByteArray badata = file.readAll();
        QString strdata = badata.toStdString().c_str(); //字节-->string-->char*
       // qDebug() << ("测试") << strdata;
        file.close();
        strdata.replace("\r\n", " ");   //将换行换成空格
       // qDebug() << strdata;
        QStringList strList = strdata.split(" ");     //用于分割字符串,返回值为一个字符串列表
        /*for (int i = 0; i < strList.size(); i++) {    //测试
            qDebug() << "-->" << strList[i];
        }*/
        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();
        qDebug() << "ip" << m_strIP << " port" << m_usPort; //测试
    }
    else{
        QMessageBox::critical(this, "打开配置文件", "打开配置文件失败");
    }



}

tcpclient &tcpclient::getInstance()
{
    static tcpclient instance;
    return instance;
}

QTcpSocket &tcpclient::getTcpSocket()
{
    return m_tcpSocket;
}

QString tcpclient::loginName()
{
    return QString(m_strLoginName);
}

QString tcpclient::curPath()
{
    return QString(m_strCurPath);
}

void tcpclient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}

void tcpclient::showConnect()
{
    QMessageBox::information(this, "连接服务器","连接服务器成功");
}

void tcpclient::recvMsg()
{
    if (!(OpeWidget::getInstance().getBook()->getDownloadStatus())) {
        qDebug() << m_tcpSocket.bytesAvailable();		//获得读取数据的大小
        uint uiPDULen = 0;
        m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));	//先获得协议数据单元大小 两个参数：起始位置，read长度
        uint uiMsgLen = uiPDULen - sizeof(PDU);	//消息的长度为弹性结构体的添放数据的大小减去初始状态的大小
        PDU* pdu = mkPDU(uiMsgLen);
        m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
        qDebug() << pdu->uiMsgType << (char*)pdu->caMsg;		//读取数据
        PDU* respdu; //用于回复的协议PDU
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_RESPOND: { //注册回复请求
            if (0 == strcmp(pdu->caData, REGIST_OK)) {

                   QMessageBox::information(this, "注册", REGIST_OK);

               }
                else if (0 == strcmp(pdu->caData, REGIST_FAILED)) {
                   QMessageBox::information(this, "注册", REGIST_FAILED);
               }

            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND: {
            if (0 == strcmp(pdu->caData, LOGIN_OK)) {
                m_strCurPath = QString("./userdir/%1").arg(m_strLoginName);
                QMessageBox::information(this, "登录", LOGIN_OK);
                OpeWidget::getInstance().show();
                //Friend::getInstance().frishFriend();
                this->hide();
            }
            else if (0 == strcmp(pdu->caData, LOGIN_FAILED)) {
                QMessageBox::warning(this, "登录", LOGIN_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_CANCEL_RESPOND: {
            if (0 == strcmp(pdu->caData, CANCEL_OK)) {
                m_strCurPath = QString("./userdir/%1").arg(m_strLoginName);
                QMessageBox::information(this, "注销", CANCEL_OK);
            }
            else if (0 == strcmp(pdu->caData, CANCEL_FAILED)) {
                QMessageBox::warning(this, "注销", CANCEL_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND: {
            OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND: {
            if (0 == strcmp(SEARCH_USR_NO, pdu->caData)) {
                QMessageBox::information(this, "搜索", QString("%1:不存在")
                    .arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            else if (0 == strcmp(SEARCH_USR_ONLINE, pdu->caData)) {
                QMessageBox::information(this, "搜索", QString("%1:在线")
                    .arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            else if (0 == strcmp(SEARCH_USR_OFFLINE, pdu->caData)) {
                QMessageBox::information(this, "搜索", QString("%1:不在线")
                    .arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
            char caPername[32] = { '\0' };
            strncpy(caPername, pdu->caData, 32);

            char caName[32] = { '\0' };
            strncpy(caName, pdu->caData + 32, 32);
            int ret = QMessageBox::information(this, "添加好友",
                QString("%1 want to add you as friend ?").arg(caName),
                QMessageBox::Yes, QMessageBox::No);
            PDU* respdu = mkPDU(0);
            memcpy(respdu->caData, pdu->caData, 32);
            memcpy(respdu->caData + 32, pdu->caData + 32, 32);
            if (QMessageBox::Yes == ret) {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
            }
            else {
                qDebug() << "拒绝好友";
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            }
            m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: {
            QMessageBox::information(this, "添加好友", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE: {
            qDebug() << pdu->caData;

            QMessageBox::information(this, "添加好友", pdu->caData);

            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
            char caPerName[32] = { '\0' };
            memcpy(caPerName, pdu->caData, 32);
            QMessageBox::information(this, "拒绝添加好友", QString(caPerName));
            break;
        }
        case ENUM_MSG_TYPE_FRUSH_FRIEND_RESPOND: {
             OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
             break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: {
             char caName[32] = { '\0' };
             memcpy(caName, pdu->caData, 32);
             QMessageBox::information(this, "删除好友", QString("%1已将你删除").arg(caName));
             break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND: {
             QMessageBox::information(this, "删除好友", "删除成功");
             break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: {
            char caSendName[32] = { '\0' };
            memcpy(caSendName, pdu->caData, 32);
            QString strSendName = caSendName;
            if (PrivateChat::getInstance().isHidden()) {
                PrivateChat::getInstance().setWindowTitle(strSendName);
                PrivateChat::getInstance().show();
            }
            PrivateChat::getInstance().setChatName(strSendName);
            PrivateChat::getInstance().setWindowTitle(strSendName);
            PrivateChat::getInstance().updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
            OpeWidget::getInstance().getFriend()->updateGrorupMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND: {
            QMessageBox::information(this, "创建文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FRUSH_FILE_RESPOND: {
            OpeWidget::getInstance().getBook()->updateFileList(pdu);
            QString strEnterDir = OpeWidget::getInstance().getBook()->enterDir();
            if (!strEnterDir.isEmpty()) {
                 m_strCurPath = m_strCurPath + "/" + strEnterDir;
                 //qDebug() << "enter dir:" << m_strCurPath;
            }
            break;
         }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND: {
            QMessageBox::information(this, "删除文件夹", (char*)pdu->caMsg);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND: {
            QMessageBox::information(this, "重命名文件", (char*)pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND: {
            OpeWidget::getInstance().getBook()->clearEnterDir();
            QMessageBox::information(this, "进入文件夹", (char*)pdu->caData);
            /*int index = m_strCurPath.lastIndexOf('/');
            QString strTem = m_strCurPath.right(m_strCurPath.size() - index - 1);*/
            QString strEnterDir = OpeWidget::getInstance().getBook()->enterDir();
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND: {
             QMessageBox::information(this, "上传文件", pdu->caData);
             break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_RESPOND: {
             QMessageBox::information(this, "删除文件", (char*)pdu->caMsg);
             break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND: {
            char caFileName[32] = { '\0' };
            //qDebug() << pdu->caData;
            bool ret = sscanf(pdu->caData, "%s %lld", caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
            //qDebug() << caFileName << "接受文件大小" << OpeWidget::getInstance().getBook()->m_iTotal;
            OpeWidget::getInstance().getBook()->m_iRecved = 0;
            if (strlen(caFileName) > 0 && (OpeWidget::getInstance().getBook()->m_iTotal) > 0) {
                OpeWidget::getInstance().getBook()->setDownloadStatus(true);
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                //qDebug() << m_file.fileName();
                if (!m_file.open(QIODevice::WriteOnly)) {
                    QMessageBox::warning(this, "下载文件", "无法获取下载路径");
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: {
             QMessageBox::information(this, "共享文件", pdu->caData);
             break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE: {
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, (char*)(pdu->caMsg), pdu->uiMsgLen);
            //qDebug() << "文件原名" << (char*)(pdu->caMsg);
            char* pos = strrchr(pPath, '/');    //此时*pos包含/
            //qDebug() << "分享文件名" << pos;
            if (nullptr != pos) {
                pos++;  //偏移一下
                QString strNote = QString("%1 分享文件：%2, 是否接收？").arg(pdu->caData).arg(QString(pos));
                int ret = QMessageBox::question(this, "共享文件", strNote);
                if (QMessageBox::Yes == ret) {
                    PDU* respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
                    QString strName = tcpclient::getInstance().loginName();
                    strcpy(respdu->caData, strName.toUtf8());
                    m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
                    free(respdu);
                    pdu = nullptr;
                }
                break;
            }
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND: {
             QMessageBox::information(this, "移动文件", pdu->caData);
             break;
        }
        default:
            break;
        }
        free(pdu);
        pdu = nullptr;
    }
    else {
        qDebug() << "正在接受文件";
        QByteArray buffer =  m_tcpSocket.readAll();
        m_file.write(buffer);
        Book* pBook = OpeWidget::getInstance().getBook();
        pBook->m_iRecved += buffer.size();
        qDebug() << "buffer: " <<buffer.size();
        qDebug() << "total:  " << pBook->m_iTotal;
        qDebug() << "curr:   " << pBook->m_iRecved;
        if (pBook->m_iTotal == pBook->m_iRecved) {
            m_file.close();
            pBook->m_iRecved = 0;
            pBook->m_iTotal = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this, "下载文件", "成功");
        }
        else if (pBook->m_iTotal < pBook->m_iRecved) {
            m_file.close();
            pBook->m_iRecved = 0;
            pBook->m_iTotal = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }
    }
}

#if 0   //测试
void tcpclient::on_send_pb_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if (!strMsg.isEmpty()) {
        PDU* pdu = mkPDU(strMsg.size() + 1);
        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());
        pdu->uiMsgType = 8888;
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);        //发送数据,参1：数据，参2：数据长度
        free(pdu);
        pdu = nullptr;
    }
    else {
        QMessageBox::warning(this,"信息发送", "发送信息不能为空");
    }
}
#endif

void tcpclient::on_login_pb_clicked()
{
    QString strname = ui->name_le->text();
    QString strpwd = ui->pwd_le->text();
    if (!strname.isEmpty() && !strpwd.isEmpty()) {
        m_strLoginName = strname;
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strname.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strpwd.toStdString().c_str(), 32);
        bool ret = m_tcpSocket.write((char*)pdu, pdu->uiPDULen);

        /*if (ret) {
            QMessageBox::warning(this, "连接服务器", "无法连接,正在重连...");
            m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
        }*/
        free(pdu);
        pdu = nullptr;
    }
    else {
        QMessageBox::warning(this, "登录", "登录失败:用户名或者密码为空！");
    }

}

void tcpclient::on_regist_pb_clicked()
{

    QString strname = ui->name_le->text();
    qDebug() << strname;
    QString strpwd = ui->pwd_le->text();
    if (!strname.isEmpty() && !strpwd.isEmpty()) {
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strname.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strpwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else {
        QMessageBox::critical(this, "注册", "注册失败:用户名或者密码为空！");
    }

}

void tcpclient::on_cancel_pb_clicked()
{
    QString strname = ui->name_le->text();
    qDebug() << strname;
    QString strpwd = ui->pwd_le->text();
    if (!strname.isEmpty() && !strpwd.isEmpty()) {
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_CANCEL_REQUEST;
        strncpy(pdu->caData, strname.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strpwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else {
        QMessageBox::critical(this, "注销", "注销失败:用户名或者密码为空！");
    }

}
