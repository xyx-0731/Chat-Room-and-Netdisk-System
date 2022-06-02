#include "friend.h"
#include "tcpclient.h"
#include "privatechat.h"
#pragma execution_character_set("utf-8")

Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pShowMsgTE = new QTextEdit;
    m_pFriendListWidget =new QListWidget;
    m_pInputMsgLE = new QLineEdit;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFrishFriendPB = new QPushButton("刷新好友");
    m_pshowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户");
    m_pMsgSendPB = new QPushButton("信息发送");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout* prightPBVBL = new QVBoxLayout;
    prightPBVBL->addWidget(m_pDelFriendPB);
    prightPBVBL->addWidget(m_pFrishFriendPB);
    prightPBVBL->addWidget(m_pshowOnlineUsrPB);
    prightPBVBL->addWidget(m_pSearchUsrPB);
    prightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout* pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(prightPBVBL);

    QHBoxLayout* pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;

    QVBoxLayout* pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();
    setLayout(pMain);

    //信号槽
    connect(m_pshowOnlineUsrPB, SIGNAL(clicked()), this, SLOT(showOnline()));
    connect(m_pSearchUsrPB, SIGNAL(clicked()), this, SLOT(searchUsr()));
    connect(m_pFrishFriendPB, SIGNAL(clicked()), this, SLOT(frushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked()), this, SLOT(delFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked()), this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked()), this, SLOT(groupChat()));
}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if (nullptr == pdu) {
        return;
    }
    m_pOnline->showUsr(pdu);

}

void Friend::updateFriendList(PDU *pdu)
{
    if (nullptr == pdu) {
        return;
    }
    m_pFriendListWidget->clear();
    uint uiSize = pdu->uiMsgLen / 32;
    char caName[32] = { '\0' };
    m_pFriendListWidget->addItem("在线好友");
    for (uint i = 0; i < uiSize; i++) {
        memcpy(caName, (char*)pdu->caMsg + i * 32, 32);
        m_pFriendListWidget->addItem(caName);
    }

}

void Friend::updateGrorupMsg(PDU *pdu)
{
    QString strMsg = QString("[%1]: %2 ").arg(pdu->caData).arg((char*)(pdu->caMsg));
    m_pShowMsgTE->append(strMsg);

}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()
{
    if (m_pOnline->isHidden()) {
        m_pOnline->show();
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;

    }
    else {
        //m_pOnline->ui->online_lw->clear();
        m_pOnline->hide();
    }

}

void Friend::searchUsr()
{
    m_strSearchName = QInputDialog::getText(this,"搜索","用户名");

    //name.toUtf8();
    if (!m_strSearchName.isEmpty()) {
        qDebug() << m_strSearchName;
        PDU* pdu = mkPDU(0);
        strncpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.toUtf8().length());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    return;

}

void Friend::frushFriend()
{
    QString strName = tcpclient::getInstance().loginName();
    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, strName.toStdString().c_str(),strName.toUtf8().length());
    tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;

}

void Friend::delFriend()
{
    //qDebug() << "delFriend运行";
    if (nullptr != m_pFriendListWidget->currentItem()) {
        QString strFriendName = m_pFriendListWidget->currentItem()->text();
        qDebug() << strFriendName;
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        QString strSelfName = tcpclient::getInstance().loginName();
        memcpy(pdu->caData, strSelfName.toStdString().c_str(), strSelfName.toUtf8().size());
        memcpy(pdu->caData+32, strFriendName.toStdString().c_str(), strFriendName.toUtf8().size());
        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }

}

void Friend::privateChat()
{
    if (nullptr != m_pFriendListWidget->currentItem()) {
        QString strFriendName = m_pFriendListWidget->currentItem()->text();
        PrivateChat::getInstance().setChatName(strFriendName);
        if (PrivateChat::getInstance().isHidden()) {
            PrivateChat::getInstance().setWindowTitle(strFriendName);
            PrivateChat::getInstance().show();

        }
    }
    else {
        QMessageBox::warning(this, "私聊", "请选择私聊的对象");
    }

}

void Friend::groupChat()
{
    QString strMsg = m_pInputMsgLE->text();
    if (!strMsg.isEmpty()) {
        PDU* pdu = mkPDU(strMsg.toUtf8().size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strname = tcpclient::getInstance().loginName();
        memcpy(pdu->caData, strname.toUtf8(), strname.toUtf8().size());
        memcpy((char*)pdu->caMsg, strMsg.toUtf8(), strMsg.toUtf8().size());
        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        m_pShowMsgTE->append(QString("[%1]: %2").arg("本人").arg(strMsg));
        //m_pShowMsgTE->append("");
        m_pInputMsgLE->clear();
    }
    else {
        QMessageBox::warning(this, "群聊", "信息不能为空");
    }

}



