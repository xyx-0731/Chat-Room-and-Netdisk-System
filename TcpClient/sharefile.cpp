#include "sharefile.h"
#include "tcpclient.h"
#include "opewidget.h"
#pragma execution_character_set("utf-8")

ShareFile::ShareFile(QWidget *parent) : QWidget(parent)
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消选择");

    m_pOkPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");

    m_pSA = new  QScrollArea;
    m_pFriendW = new  QWidget;
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);
    m_pButonGroup = new QButtonGroup(m_pFriendW);
    m_pButonGroup->setExclusive(false);

    QHBoxLayout* pTopHBL = new  QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();	//弹簧


    QHBoxLayout* pDownHBL = new  QHBoxLayout;
    pDownHBL->addWidget(m_pOkPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout* pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);
    this->setFixedSize(300, 400);

    connect(m_pCancelSelectPB, SIGNAL(clicked()), this, SLOT(cancelSelect()));
    connect(m_pSelectAllPB, SIGNAL(clicked()), this, SLOT(selectAll()));
    connect(m_pOkPB, SIGNAL(clicked()), this, SLOT(okShare()));
    connect(m_pCancelPB, SIGNAL(clicked()), this, SLOT(cancelShare()));
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if (nullptr == pFriendList) {
        return;
    }
    QAbstractButton* tmp = nullptr;
    QList<QAbstractButton*> preFriendList = m_pButonGroup->buttons();
    for (int i = 0; i < preFriendList.size(); i++) {
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);	//从布局中移除
        m_pButonGroup->removeButton(tmp);	//从组里移除
        preFriendList.removeOne(tmp);				//从列表移除
        delete tmp;
    }
    tmp = nullptr;
    QCheckBox* pCB = nullptr;
    for (int i = 1; i < pFriendList->count(); i++) {
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButonGroup->buttons();
    for (int i = 0; i < cbList.size(); i++) {
        if (!cbList[i]->isChecked()) {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList = m_pButonGroup->buttons();
    for (int i = 0; i < cbList.size(); i++) {
        if (cbList[i]->isChecked()) {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::okShare()
{
    QString strName = tcpclient::getInstance().loginName();		//发送者
    QString strCurpath = tcpclient::getInstance().curPath();	//当前路径
    //qDebug() << "发送信息： " << strName << strCurpath;
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();	//文件名
    QString strPath = strCurpath + "/" + strShareFileName;		//组合路径
    QList<QAbstractButton*> cbList = m_pButonGroup->buttons();
    int num = 0; //选上的人数
    for (int i = 0; i < cbList.size(); i++) {
        if (cbList[i]->isChecked()) {
            num++;
        }
    }
    PDU* pdu = mkPDU(32 * num + strPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData, "%s %d", strName.toStdString().c_str(), num);	//存放发送者姓名，以及人数
    //qDebug() << "发送信息： " << pdu->caData;
    int j = 0;
    for (int i = 0; i < cbList.size(); i++) {
        if (cbList[i]->isChecked()) {
            memcpy((char*)(pdu->caMsg) + j * 32, cbList[i]->text().toStdString().c_str(), cbList[i]->text().toUtf8().size());	//要分享的人
            //qDebug() << "接收者" << (char*)(pdu->caMsg) + j * 32;
            j++;
        }
    }
    memcpy((char*)pdu->caMsg + num * 32, strPath.toStdString().c_str(), strPath.toUtf8().size());
    //qDebug() << "发送信息： " << strPath;
    //qDebug() << "发送信息： " << (char*)((char*)pdu->caMsg+num * 32) << (char*)((char*)pdu->caMsg);
    tcpclient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void ShareFile::cancelShare()
{
    hide();
}
