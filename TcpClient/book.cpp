#include "book.h"
#include "tcpclient.h"
#include <QFileDialog>
#include "sharefile.h"
#include "opewidget.h"
#pragma execution_character_set("utf-8")

Book::Book(QWidget *parent) : QWidget(parent)
{
    m_bDownload = false;
    m_pTimer = new QTimer;
    m_strEnterDir.clear();

    m_pBookList = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名文件");
    m_pFrushFilePB = new QPushButton("刷新文件夹");

    QVBoxLayout* pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFrushFilePB);


    m_pUploadPB = new QPushButton("上传文件");
    m_pDownloadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("共享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);  //设置不可选状态

    QVBoxLayout* pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownloadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout* pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookList);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    //信号槽
    connect(m_pCreateDirPB, SIGNAL(clicked()), this, SLOT(createDir()));
    connect(m_pFrushFilePB, SIGNAL(clicked()), this, SLOT(frushFile()));
    connect(m_pDelDirPB, SIGNAL(clicked()), this, SLOT(delDir()));
    connect(m_pRenamePB, SIGNAL(clicked()), this, SLOT(renameFile()));
    connect(m_pBookList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB, SIGNAL(clicked(bool)), this, SLOT(returnPre()));
    connect(m_pUploadPB, SIGNAL(clicked(bool)), this, SLOT(uploadFile()));
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(uploadFileData()));
    connect(m_pDelFilePB, SIGNAL(clicked(bool)), this, SLOT(delRegFile()));
    connect(m_pDownloadPB, SIGNAL(clicked()), this, SLOT(downloadFile()));
    connect(m_pShareFilePB, SIGNAL(clicked()), this, SLOT(shareFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked()), this, SLOT(moveFile()));
    connect(m_pSelectDirPB, SIGNAL(clicked()), this, SLOT(selectDsestDir()));
}

void Book::updateFileList(const PDU *pdu)
{
    if (nullptr == pdu) {
        return;
    }
    QListWidgetItem* pItTmp = nullptr;
    int row = m_pBookList->count();
    while(m_pBookList->count() > 0) {
        pItTmp = m_pBookList->item(row-1);
        m_pBookList->removeItemWidget(pItTmp);
        delete pItTmp;
        row--;
    }
    FileInfo* pFileInfo = nullptr;
    int iCount = pdu->uiMsgLen / sizeof(FileInfo);
    for (int i = 0; i < iCount; i++) {

        pFileInfo = (FileInfo*)(pdu->caMsg) + i;
        //qDebug() << pFileInfo->caFileName << pFileInfo->iFileType;
        QListWidgetItem* pItem = new QListWidgetItem;
        if (0 == pFileInfo->iFileType) {
            pItem->setIcon(QIcon(QPixmap(":/map/dir.png")));
        }
        else if (1 == pFileInfo->iFileType) {
            pItem->setIcon(QIcon(QPixmap(":/map/reg.jpg")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pBookList->addItem(pItem);
    }

}

QString Book::enterDir()
{
    return QString(m_strEnterDir);
}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

void Book::setDownloadStatus(bool status)
{
    m_bDownload = status;
}

bool Book::getDownloadStatus()
{
    return  m_bDownload;
}

QString Book::getSaveFilePath()
{
    return QString(m_strSaveFilePath);
}

QString Book::getShareFileName()
{
    return QString(m_strShareFilName);
}

void Book::createDir()
{
    QString strNewDir =  QInputDialog::getText(this, "新建文件夹", "新文件夹名");
    //qDebug() << strNewDir;
    if (!strNewDir.isEmpty()) {
        if (strNewDir.toUtf8().size() > 32) {
            QMessageBox::warning(this, "新建文件夹", "新文件夹名过长");
        }
        else {
            QString strName = tcpclient::getInstance().loginName();
            QString strCurPath = tcpclient::getInstance().curPath();
            PDU* pdu = mkPDU(strCurPath.toUtf8().size() + 1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            memcpy((char*)pdu->caData, strName.toStdString().c_str(), strName.toUtf8().size());
            memcpy((char*)pdu->caData + 32, strNewDir.toStdString().c_str(), strNewDir.toUtf8().size());
            memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());
            //qDebug() << (char*)pdu->caMsg;
            tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = nullptr;
        }

    }
    else {
        QMessageBox::warning(this, "新建文件夹", "新文件夹名不能为空");
    }

}

void Book::frushFile()
{
    QString strCurPath = tcpclient::getInstance().curPath();
    PDU* pdu = mkPDU(strCurPath.toUtf8().size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMsg), strCurPath.toUtf8(), strCurPath.toUtf8().size());
    //qDebug() << (char*)pdu->caMsg;
    tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Book::delDir()
{
    QString strCurPath = tcpclient::getInstance().curPath();
    QListWidgetItem*pItem =  m_pBookList->currentItem();
    if (nullptr == pItem) {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
    }
    else {
        QString strdelName = pItem->text();
        PDU* pdu = mkPDU(strCurPath.toUtf8().size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData, strdelName.toStdString().c_str(), strdelName.toUtf8().size());
        memcpy((char*)pdu->caMsg, strCurPath.toUtf8(), strCurPath.toUtf8().size());
        //qDebug() << strCurPath;
        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Book::renameFile()
{
    QString strCurPath = tcpclient::getInstance().curPath();
    QListWidgetItem* pItem = m_pBookList->currentItem();
    if (nullptr == pItem) {
        QMessageBox::warning(this, "重命名文件", "请选择要重命名的文件");
    }
    else {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this, "重命名文件", "请输入新的文件名");
        if (!strNewName.isEmpty()) {
            if (strNewName.toUtf8().size() > 32) {
                QMessageBox::warning(this, "新建文件", "新文件名过长");
            }
            else {
                PDU* pdu = mkPDU(strCurPath.toUtf8().size() + 1);
                pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
                strncpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.toUtf8().size());
                strncpy(pdu->caData+32, strNewName.toStdString().c_str(), strNewName.toUtf8().size());
                memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());
                qDebug() << strCurPath;
                tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
                free(pdu);
                pdu = nullptr;
            }
        }
        else {
            QMessageBox::warning(this, "重命名文件", "新文件名不能为空");
        }
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    m_strEnterDir = strDirName;
    QString strCurPath = tcpclient::getInstance().curPath();
    PDU* pdu = mkPDU(strCurPath.toUtf8().size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData, strDirName.toStdString().c_str(), strDirName.toUtf8().size());
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());
    //qDebug() << strCurPath;
    tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Book::returnPre()
{
    QString strCurPath = tcpclient::getInstance().curPath();
    QString strRootPath =  "./userdir/" + tcpclient::getInstance().loginName();
    //qDebug() << "当前目录：" << strCurPath;
    //qDebug() << "根目录" << strRootPath;
    if (strCurPath == strRootPath) {
        QMessageBox::warning(this, "返回", "返回失败：当前目录为根目录");
    }else{
        int index =  strCurPath.lastIndexOf("/"); //从后往前找第一个斜杠
        strCurPath.remove(index, strCurPath.size() - index);
        tcpclient::getInstance().setCurPath(strCurPath);
        //qDebug() << "return path:" <<strCurPath;
        clearEnterDir();
        frushFile();    //刷新文件
    }

}

void Book::uploadFile()
{
    QString strCurPath = tcpclient::getInstance().curPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    //qDebug() << m_strUploadFilePath;
    if (!m_strUploadFilePath.isEmpty()) {
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - index - 1);
        //qDebug() << strFileName;
        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();      //获得文件大小
        PDU* pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), file.size());

        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
        m_pTimer->start(1000); //设置定时器防止粘包
    }
    else {
        QMessageBox::warning(this, "上传文件", "上传文件不能为空");
    }
}

void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }
    char* pbuffer = new char[4096]; //一次读4096较快
    qint64 ret = 0;
    while (true) {
        ret = file.read(pbuffer, 4096);
        if (ret > 0 && ret <= 4096) {
            tcpclient::getInstance().getTcpSocket().write(pbuffer, ret);
        }
        else if (ret == 0) {
            break;
        }
        else {
            QMessageBox::warning(this, "上传文件", "上传文件失败:读取文件出错!");
            break;
        }
    }
    file.close();
    delete[]pbuffer;
    pbuffer = nullptr;
}

void Book::delRegFile()
{
    QString strCurPath = tcpclient::getInstance().curPath();
    QListWidgetItem* pItem = m_pBookList->currentItem();
    if (nullptr == pItem) {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
    }
    else {
        QString strdelName = pItem->text();
        PDU* pdu = mkPDU(strCurPath.toUtf8().size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData, strdelName.toStdString().c_str(), strdelName.toUtf8().size());
        memcpy((char*)pdu->caMsg, strCurPath.toUtf8(), strCurPath.toUtf8().size());
        qDebug() << strCurPath;
        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
        }
}

void Book::downloadFile()
{
    QListWidgetItem* pItem = m_pBookList->currentItem();
    if (nullptr == pItem) {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
    }
    else {
        QString strSaveFilePath = QFileDialog::getSaveFileName(); //弹出窗口，选择路径
        if (strSaveFilePath.isEmpty()) {
            QMessageBox::warning(this, "下载文件", "请指定要保存的位置");
            m_strSaveFilePath.clear();
            return;
        }
        else {
            m_strSaveFilePath = strSaveFilePath;
            //m_bDownload = true;
        }
        //qDebug() << "将要下载的路径" << strSaveFilePath;

        QString strCurPath = tcpclient::getInstance().curPath();
        PDU* pdu = mkPDU(strCurPath.toUtf8().size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString strFileName = pItem->text();
        strncpy(pdu->caData, strFileName.toStdString().c_str(), 32);
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());
        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    }
}

void Book::shareFile()
{
    QListWidgetItem* pItem = m_pBookList->currentItem();
    if (nullptr == pItem) {
        QMessageBox::warning(this, "共享文件", "请选择要共享的文件");
        return;
    }
    else {
        m_strShareFilName = pItem->text();
    }
    Friend* pFriend = OpeWidget::getInstance().getFriend();
    QListWidget* pFriendList = pFriend->getFriendList();
    /*for (int i = 1; i < pFriendList->count(); i++) {
        qDebug() << pFriendList->item(i)->text();
    }*/
    ShareFile::getInstance().updateFriend(pFriendList);
    if (ShareFile::getInstance().isHidden()) {
        ShareFile::getInstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookList->currentItem();
    if (nullptr != pCurItem) {
        m_strMoveFileName = pCurItem->text();
        QString strCurPath = tcpclient::getInstance().curPath();
        m_strMoveFilePath = strCurPath + '/' + m_strMoveFileName;
        //qDebug() << "移动文件" << m_strMoveFileName;
        m_pSelectDirPB->setEnabled(true);
    }
    else {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    }
}

void Book::selectDsestDir()
{
    QListWidgetItem* pCurItem = m_pBookList->currentItem();
    if (nullptr != pCurItem) {
        QString strDestDir = pCurItem->text();
        QString strCurPath = tcpclient::getInstance().curPath();
        m_strDestDir = strCurPath + '/' + strDestDir;

        int srcLen = m_strMoveFilePath.toUtf8().size();
        int destLen = m_strDestDir.toUtf8().size();
        PDU* pdu = mkPDU(srcLen + destLen + 2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        bool ret = sprintf(pdu->caData, "%d %d %s", srcLen, destLen, m_strMoveFileName.toStdString().c_str());
        //qDebug() << pdu->caData << m_strMoveFileName;
        //qDebug() << ret;
        //qDebug() << "源目录" << m_strMoveFilePath;
        memcpy((char*)(pdu->caMsg), m_strMoveFilePath.toStdString().c_str(), srcLen);
        memcpy((char*)((char*)(pdu->caMsg) + srcLen + 1), m_strDestDir.toStdString().c_str(), destLen);
        //qDebug() << "源目录" << (char*)(pdu->caMsg);
        //qDebug() << "目标目录" << (char*)((char*)(pdu->caMsg) + srcLen + 1);
        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    }
    m_pSelectDirPB->setEnabled(false);

}
