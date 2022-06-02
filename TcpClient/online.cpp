#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"
#pragma execution_character_set("utf-8")

Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu)
{
    if (nullptr == pdu ) {
        return;
    }
    ui->online_lw->clear();
    uint uiSize = pdu->uiMsgLen / 32;	//个数
    char caTmp[32];
    for (uint i = 0; i < uiSize; i++){
        memcpy(caTmp, (char*)pdu->caMsg + i * 32, 32);
        //QString m = caTmp;
        //m.toLocal8Bit().data();
        ui->online_lw->addItem(caTmp);
    }

}

void Online::on_addFriend_pb_clicked()
{
    QListWidgetItem*pItem =  ui->online_lw->currentItem();
    strPerUsrName =  pItem->text();
    QString strLoginUsrName = tcpclient::getInstance().loginName();
    if (strLoginUsrName == strPerUsrName) {
        QMessageBox::information(this, "好友申请", "you can't add yourself");
        return;
    }
    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData, strPerUsrName.toStdString().c_str(), strPerUsrName.toUtf8().size());
    memcpy(pdu->caData + 32, strLoginUsrName.toStdString().c_str(), strLoginUsrName.toUtf8().size());
    tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
    QMessageBox::information(this, "发送申请", "已发送好友申请");

}
