#include "privatechat.h"
#include "ui_privatechat.h"
#pragma execution_character_set("utf-8")

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;

}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName;
    m_strLoginName = tcpclient::getInstance().loginName();

}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if (nullptr == pdu) {
        return;
    }
    char caSendName[32] = { '\0' };
    memcpy(caSendName, pdu->caData, 32);
    qDebug() << pdu->caMsg;
    QString strMsg = QString("[%1]: %2").arg(caSendName).arg((char*)(pdu->caMsg));
    ui->showMsg_te->append(strMsg);
    ui->showMsg_te->append("");

}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text();
    ui->inputMsg_le->clear();
    if (!strMsg.isEmpty()) {
        PDU* pdu = mkPDU(strMsg.toUtf8().size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, m_strLoginName.toStdString().c_str(), m_strLoginName.toUtf8().size());
        memcpy(pdu->caData + 32, m_strChatName.toStdString().c_str(), m_strChatName.toUtf8().size());
        memcpy((char*)pdu->caMsg, strMsg.toUtf8(), strMsg.toUtf8().size());

        tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        ui->showMsg_te->append(QString("[%1]: %2").arg("本人").arg(strMsg));
        ui->showMsg_te->append("");
        ui->inputMsg_le->clear();
        QMessageBox::information(this, "send","send susscess");
        free(pdu);
        pdu = nullptr;
    }
    else {
        QMessageBox::warning(this, "私聊", "发送的聊天信息不能为空！");
    }

}
