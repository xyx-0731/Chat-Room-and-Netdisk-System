#include "mytcpsocket.h"
#include "mytcpserver.h"
#pragma execution_character_set("utf-8")

MyTcpSocket::MyTcpSocket()
{
   connect(this, &QIODevice::readyRead, this, &MyTcpSocket::recvMsg);
   connect(this, &QTcpSocket::disconnected, this, &MyTcpSocket::clientoffline);

   qint64 m_iTotal = 0;
   qint64 m_iRecved = 0;
   m_Upload = false;
   m_pTimer = new QTimer;
   connect(m_pTimer, SIGNAL(timeout()), this, SLOT(sendFileToClient()));
}

QString MyTcpSocket::getname()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDesDir)
{
    QDir dir;
    dir.mkdir(strDesDir);
    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();	//获取文件信息

    QString srcTmp;
    QString destTmp;

    for (int i = 0; i < fileInfoList.size(); i++) {
        //qDebug() << fileInfoList[i].fileName();
        if (fileInfoList[i].isFile()) {
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDesDir + '/' + fileInfoList[i].fileName();
            QFile::copy(srcTmp, destTmp);
        }
        else {
            if (QString(".") == fileInfoList[i].fileName()
                || QString("..") == fileInfoList[i].fileName())
            {
                continue;
            }
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDesDir + '/' + fileInfoList[i].fileName();
            copyDir(srcTmp, destTmp);
        }
    }
}

void MyTcpSocket::recvMsg()
{
    if (!m_Upload) {
        qDebug() << this->bytesAvailable();		//获得读取数据的大小
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(uint));	//先获得协议数据单元大小 两个参数：起始位置，read长度
        uint uiMsgLen = uiPDULen - sizeof(PDU);	//消息的长度为弹性结构体的添放数据的大小减去初始状态的大小
        PDU* pdu = mkPDU(uiMsgLen);
        this->read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
        qDebug() << pdu->uiMsgType << (char*)pdu->caMsg;		//读取数据
        PDU* respdu; //用于回复的协议PDU
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST: { //注册请求
            //qDebug() << "运行到此处1";
            char caName[32] = { '\0' };
            char caPwd[32] = { '\0' };
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData + 32, 32);
            //qDebug() << caName << caPwd << pdu->uiMsgType;
            bool ret = OpeDB::getInstance().handleRegist(caName, caPwd); //交给处理函数(去数据库插入)处理注册数据
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND; //消息类型：注册回复
            if (ret) { //注册成功
                strcpy(respdu->caData, REGIST_OK);
                QDir dir;
                qDebug() << "create dir: " << dir.mkdir(QString("./userdir/%1").arg(caName));
            }
            else {
                strcpy(respdu->caData, REGIST_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
            }
        case ENUM_MSG_TYPE_LOGIN_REQUEST: {
            //qDebug() << "运行到此处2";
            char caName[32] = { '\0' };
            char caPwd[32] = { '\0' };
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData + 32, 32);
            m_strName = caName;
            bool ret = OpeDB::getInstance().handleLogin(caName, caPwd);
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if (ret) {
                strcpy(respdu->caData, LOGIN_OK);
            }
            else {
                strcpy(respdu->caData, LOGIN_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_CANCEL_REQUEST:{
            //qDebug() << "运行到此处3";
            char caName[32] = { '\0' };
            char caPwd[32] = { '\0' };
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData + 32, 32);
            m_strName = caName;
            m_strPsd = caPwd;
            bool ret = OpeDB::getInstance().handleCancel(caName, caPwd);
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CANCEL_RESPOND;
            if (ret) {
                strcpy(respdu->caData, CANCEL_OK);
            }
            else {
                strcpy(respdu->caData, CANCEL_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST: {
            //qDebug() << "运行到此处4";
            QStringList ret = OpeDB::getInstance().handleAllOnline();
            uint uiMsgLen = ret.size() * 32; //每个名字32个字节
            PDU* respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for (int i = 0; i < ret.size(); i++) {
                memcpy((char*)respdu->caMsg + i * 32,
                    ret.at(i).toUtf8(),
                    ret.at(i).toUtf8().length());
                //qDebug() << ret.at(i);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST: {
            //qDebug() << "运行到此处5";
            int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if (-1 == ret) {
                strcpy(respdu->caData, SEARCH_USR_NO);
            }
            else if (1 == ret) {
                strcpy(respdu->caData, SEARCH_USR_ONLINE);
                //qDebug() << "运行到此处55";
            }

            else if (0 == ret) {
                strcpy(respdu->caData, SEARCH_USR_OFFLINE);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
            //qDebug() << "运行到此处6";
            char cPername[32] = { '\0' };
            char cName[32] = { '\0' };
            strncpy(cPername, pdu->caData, 32);
            strncpy(cName, pdu->caData + 32, 32);
            int ret = OpeDB::getInstance().handleAddFriend(cPername, cName);
            qDebug()<< ret;
            PDU* respdu = nullptr;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;

            if (-1 == ret) {	//其他错误
                strcpy(respdu->caData, UNKNOW_ERROR);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
            }
            else if (0 == ret) {	//已经是好友
                strcpy(respdu->caData, EXISTED_FRIEND);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
            }
            else if (1 == ret) {	//在线，添加好友
                MyTcpserver::getInstance().resend(cPername, pdu);

            }
            else if (2 == ret) {	//好友不在线
                strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
            }
            else if (3 == ret) {	//不存在
                strcpy(respdu->caData, ADD_FRIEND_NOEXIST);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE: {
            qDebug() << "运行到此处7";
            char cPername[32] = { '\0' };
            char cName[32] = { '\0' };
            strncpy(cPername, pdu->caData, 32);
            strncpy(cName, pdu->caData + 32, 32);
            int ret = OpeDB::getInstance().handleAddSuscess(cPername, cName);
            PDU* respdu = nullptr;
            respdu = mkPDU(0);
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
            if (ret == 1) {
                strcpy(respdu->caData, ADD_SUSSCESS);
            }
            else if (ret == 0) {
                strcpy(respdu->caData, "you can't add youself");
            }
            else {
                strcpy(respdu->caData, ADD_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
            }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
            //qDebug() << "运行到此处8";
            PDU* respdu = nullptr;
            respdu = mkPDU(0);
            strcpy(respdu->caData, ADD_FAILED);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
            }
        case ENUM_MSG_TYPE_FRUSH_FRIEND_REQUEST: {
            //qDebug() << "运行到此处9";
            char caName[32] = { '\0' };
            strncpy(caName, pdu->caData, 32);
            QStringList ret = OpeDB::getInstance().handleFrushFriend(caName);
            //qDebug() << ret[0].data();
            uint uiMsgLen = ret.size() * 32;
            PDU* respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FRIEND_RESPOND;
            for (int i = 0; i < ret.size(); i++) {
                memcpy((char*)respdu->caMsg + i * 32, ret[i].toStdString().c_str(), ret[i].toUtf8().size());
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: {
            //qDebug() << "运行到此处10";
            char caSelfname[32] = { '\0' };
            char caFriendName[32] = { '\0' };
            strncpy(caSelfname, pdu->caData, 32);
            strncpy(caFriendName, pdu->caData + 32, 32);
            bool ret = OpeDB::getInstance().handleDelFriend(caSelfname, caFriendName);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            strcpy(respdu->caData, DEL_FRIEND_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            MyTcpserver::getInstance().resend(caFriendName, pdu); //转发给朋友
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: {
            //qDebug() << "运行到此处11";
            //qDebug() << (char*)pdu->caMsg;
            char caPerName[32] = { '\0' };
            memcpy(caPerName, pdu->caData + 32, 32);
            //qDebug() << caPerName;
            MyTcpserver::getInstance().resend(caPerName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
            //qDebug() << "运行到此处12";
            char caName[32] = { '\0' };
            strncpy(caName, pdu->caData, 32);
            //qDebug() << (char*)(pdu->caMsg);
            QStringList onlineFriend = OpeDB::getInstance().handleFrushFriend(pdu->caData); //获得在线好友
            QString tmp;
            for (int i = 0; i < onlineFriend.size(); i++) {
                 tmp = onlineFriend.at(i);
                 MyTcpserver::getInstance().resend(tmp.toStdString().c_str(), pdu);
                 }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST: {
            //qDebug() << "运行到此处13";
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));
            qDebug() << strCurPath;
            bool ret = dir.exists(QString(strCurPath));
            PDU* respdu = nullptr;
            if (ret) {	//目录存在
                char caNewDir[32] = { '\0' };
                memcpy(caNewDir, pdu->caData + 32, 32);
                //qDebug() << caNewDir;
                QString strNewPath = strCurPath + "/" + caNewDir;
                //qDebug() << strNewPath;
                ret = dir.exists(strNewPath);
                //qDebug() << "--->" << ret;
                if (ret) {	//创建的文件名已存在
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, DIR_NAME_EXISTED);
                }
                else {		//创建文件名不存在
                    dir.mkdir(strNewPath);
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, CREATE_DIR_OK);
                }
            }
            else {	//目录不存在
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData, DIR_NOEXIST);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            pdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_FRUSH_FILE_REQUEST: {
            //qDebug() << "运行到此处14";
            char* pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
            QDir dir(pCurPath);
            QFileInfoList fileInfoList = dir.entryInfoList();
            int iFileCount = fileInfoList.size();
            PDU* respdu = mkPDU(sizeof(FileInfo) * iFileCount);
            respdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FILE_RESPOND;
            FileInfo* pFileInfo = nullptr;
            QString strFileName;

            for (int i = 0; i < iFileCount; i++) {
                /*if ((0 == strcmp(".", strFileName.toStdString().c_str())) ||
                    (0 == strcmp("..", strFileName.toStdString().c_str()))) {
                    continue;
                }*/
                pFileInfo = (FileInfo*)(respdu->caMsg) + i;
                strFileName = fileInfoList[i].fileName();
                //qDebug() << strFileName;
                memcpy(pFileInfo->caFileName, strFileName.toUtf8(), strFileName.toUtf8().size());
                if (fileInfoList[i].isDir()) {
                    pFileInfo->iFileType = 0; //是一个路径
                }
                else if (fileInfoList[i].isFile()) {
                    pFileInfo->iFileType = 1; //是一个常规文件
                }
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            pdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST: {
            //qDebug() << "运行到此处15";
            char caName[32] = { '\0' };
            strcpy(caName, pdu->caData);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            //qDebug() << strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if (fileInfo.isDir()) {
                QDir dir;
                dir.setPath(strPath);
                ret = dir.removeRecursively(); //删除该路径及其中的文件
                //qDebug() << "ret = " << ret;
            }
            else if (fileInfo.isFile()) { //常规文件不删除
                ret = false;
            }
            PDU* respdu = nullptr;
            if (ret) { //删除成功
                respdu = mkPDU(strlen(DEL_DIR_OK) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caMsg, DEL_DIR_OK, strlen(DEL_DIR_OK));
            }
            else {
                respdu = mkPDU(strlen(DEL_DIR_FAILED) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caMsg, DEL_DIR_FAILED, strlen(DEL_DIR_FAILED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            pdu = nullptr;
            delete []pPath;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST: {
            char caOldName[32] = { '\0' };
            char caNewName[32] = { '\0' };
            strncpy(caOldName, pdu->caData, 32);
            strncpy(caNewName, pdu->caData + 32, 32);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);
            //qDebug() << strOldPath;
            //qDebug() << strNewPath;
            QDir dir;
            bool ret = dir.rename(strOldPath, strNewPath);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if (ret) {
                strcpy(respdu->caData, RENAME_FILE_OK);
            }
            else {
                strcpy(respdu->caData, RENAME_FILE_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            pdu = nullptr;
            delete []pPath;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST: {
            char caEnterName[32] = { '\0' };
            strncpy(caEnterName, pdu->caData, 32);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);
            //qDebug() << strPath;
            QFileInfo fileInfo(strPath);
            PDU* respdu = nullptr;
            if (fileInfo.isDir()) {
                //qDebug() << "进入目录";
                QDir dir(strPath);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                //qDebug() << "文件数量" << iFileCount;
                respdu = mkPDU(sizeof(FileInfo) * iFileCount);
                respdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FILE_RESPOND;
                FileInfo* pFileInfo = nullptr;
                QString strFileName;

                for (int i = 0; i < iFileCount; i++) {
                    pFileInfo = (FileInfo*)respdu->caMsg + i;
                    strFileName = fileInfoList[i].fileName();
                    //qDebug() << strFileName;
                    memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.toUtf8().size());
                    if (fileInfoList[i].isDir()) {
                        pFileInfo->iFileType = 0;
                    }
                    else if (fileInfoList[i].isFile()) {
                        pFileInfo->iFileType = 1;
                    }
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                pdu = nullptr;
            }
            else if (fileInfo.isFile()) {
                //qDebug() << "进入文件";
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strncpy(respdu->caData, ENTER_DIR_FAILED, strlen(ENTER_DIR_FAILED)+ 1);
                //qDebug() << "进入失败";
                write((char*)respdu, respdu->uiPDULen);
                //qDebug() << "发送成功";
                free(respdu);
                pdu = nullptr;
            }
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST: {
            char caFileName[32] = { '\0' };
            qint64 fileSize = 0;
            sscanf(pdu->caData, "%s %lld", caFileName, &fileSize);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            //qDebug() << strPath;

            delete[]pPath;
            pPath = nullptr;

            m_file.setFileName(strPath);
            //以只写的方式打开文件，若文件不存在，则会自动创建文件
            if (m_file.open(QIODevice::WriteOnly)) {
                m_Upload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST: {
            char caName[32] = { '\0' };
            strcpy(caName, pdu->caData);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            //qDebug() << strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if (fileInfo.isDir()) {
                ret = false;
            }
            else if (fileInfo.isFile()) {
                QDir dir;
                ret = dir.remove(strPath);
                //qDebug() << "ret = " << ret;
            }
            PDU* respdu = nullptr;
            if (ret) {
                respdu = mkPDU(strlen(DEL_FILE_OK) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caMsg, DEL_FILE_OK, strlen(DEL_FILE_OK));
            }
            else {
                respdu = mkPDU(strlen(DEL_FILE_FAILED) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caMsg, DEL_FILE_FAILED, strlen(DEL_FILE_FAILED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            pdu = nullptr;
            delete []pPath;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST: {
            char caFileName[32] = { '\0' };
            strcpy(caFileName, pdu->caData);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            //qDebug() << strPath;
            QFileInfo fileInfo(strPath);
            qint64 fileSize = fileInfo.size();

            PDU* respdu = mkPDU(0);
            sprintf(respdu->caData, "%s %lld", caFileName, fileSize);
            //qDebug() << respdu->caData;
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;

            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);

            delete[]pPath;
            pPath = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: {
            char caSendName[32] = { '\0' };
            int num = 0;
            sscanf(pdu->caData, "%s %d", caSendName, &num);
            int size = num * 32;
            //qDebug() << "发送文件名:" << (char*)((char*)pdu->caMsg + size);
            //qDebug() << "接收者姓名:" << (char*)(pdu->caMsg);
            PDU* respdu = mkPDU(pdu->uiMsgLen - size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strncpy(respdu->caData, caSendName, 32);	//拷贝发送者姓名
            //qDebug() << "发送姓名:" << respdu->caData << "发送人数:" << num;
            memcpy((char*)(respdu->caMsg), (char*)((char*)pdu->caMsg + size), pdu->uiMsgLen - size);	//拷贝文件路径
            //qDebug() << "发送文件名:" << (char*)(respdu->caMsg)  << size;
            char caRecvName[32] = { '\0' };
            for (int i = 0; i < num; i++) {
                memcpy(caRecvName, (char*)((char*)(pdu->caMsg) + i * 32), 32);
                //qDebug() << "接收者姓名:" << caRecvName;
                MyTcpserver::getInstance().resend(caRecvName, respdu);
            }
            free(respdu);
            respdu = nullptr;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData, SHARE_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND: {
            QString strRecvPath = QString("./userdir/%1").arg(pdu->caData);
            QString strShareFilePath = QString("%1").arg((char*)(pdu->caMsg));
            int index = strShareFilePath.lastIndexOf('/');
            QString strFileName = strShareFilePath.right(strShareFilePath.size() - index - 1);
            strRecvPath = strRecvPath + '/' + strFileName;

            QFileInfo fileInfo(strShareFilePath);
            if (fileInfo.isFile()) {
                QFile::copy(strShareFilePath, strRecvPath);
            }
            else if (fileInfo.isDir()) {
                copyDir(strShareFilePath, strRecvPath);
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST: {
            char caFileName[32] = { '\0' };
            int srcLen = 0;
            int destLen = 0;
            bool ret = sscanf(pdu->caData, "%d %d %s", &srcLen, &destLen, caFileName);
            //qDebug() << "文件名" << caFileName;
            /*qDebug() << "源目录" << (char*)(pdu->caMsg);
            qDebug() << "目标目录" << (char*)((char*)(pdu->caMsg) + srcLen + 1);*/
            char* pSrcPath = new char[srcLen + 1];
            char* pDestPath = new char[destLen + 1 + 32];
            memset(pSrcPath, '\0', srcLen + 1);  //清空
            memset(pDestPath, '\0', destLen + 1 + 32);

            memcpy(pSrcPath, pdu->caMsg, srcLen);
            memcpy(pDestPath, (char*)((char*)(pdu->caMsg) + srcLen + 1), destLen);
            //qDebug() << "源目录" << pSrcPath;
            //qDebug() << "目标目录" << pDestPath;

            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            QFileInfo fileInfo(pDestPath);
            if (fileInfo.isDir()) {
                strcat(pDestPath,"/");
                strcat(pDestPath, caFileName);
                //qDebug() << "当前" << pDestPath;
                //strcat(pSrcPath, "/");
                bool ret = QFile::rename(pSrcPath, pDestPath);
                if (ret) {
                    strcpy(respdu->caData, MOVE_FILE_OK);
                    //qDebug() << "成功";
                }
                else {
                    //qDebug() << "失败";
                    strcpy(respdu->caData, COMMON1_ERR);
                }
            }
            else if(fileInfo.isFile()){
                strcpy(respdu->caData, MOVE_FILE_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu = nullptr;
    }
    else{
        PDU* respdu = nullptr;
        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();
        //qDebug() << "正在上传文件";
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        if (m_iTotal == m_iRecved) {
            m_file.close();
            m_Upload = false;
            strcpy(respdu->caData, UPLOAD_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
        }
        else if (m_iTotal < m_iRecved) {
            m_file.close();
            m_Upload = false;
            strcpy(respdu->caData, UPLOAD_FILE_FAILED);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = nullptr;
    }
}

void MyTcpSocket::clientoffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this); //发送信号,注销信号

}

void MyTcpSocket::sendFileToClient()
{
    m_pTimer->stop();
    char* pData = new char[4096];
    qint64 ret = 0;
    while (true) {
        qDebug() << "正在读文件" << m_file.fileName();
        ret = m_file.read(pData, 4096);
        if (ret > 0 && ret <= 4096) {
            write(pData, ret);
        }
        else if (0 == ret) {
            m_file.close();
            qDebug() << "发送文件内容给客户端成功";
            break;
        }
        else if (ret < 0) {
            qDebug() << "发送文件内容给客户端失败";
            m_file.close();
            break;
        }
    }
    delete[]pData;
    pData = nullptr;

}


