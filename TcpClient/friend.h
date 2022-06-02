#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include<QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include"online.h"
#include <QInputDialog>




class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUsr(PDU* pdu);    //显示所有在线用户
    QString m_strSearchName;
    void updateFriendList(PDU* pdu);    //刷新好友列表
    void updateGrorupMsg(PDU* pdu); //更新群聊消息

    QListWidget* getFriendList();   //获取好友列表信息

signals:

public slots:
    void showOnline();      //展示在线用户
    void searchUsr();       //查找用户
    void frushFriend();     //刷新好友列表
    void delFriend();       //删除好友
    void privateChat();     //私聊
    void groupChat();       //群聊


private:

    QTextEdit* m_pShowMsgTE;  //信息展示框
    QListWidget* m_pFriendListWidget;   //好友展示列表
    QLineEdit* m_pInputMsgLE;       //信息输入框

    QPushButton* m_pDelFriendPB;  //删除好友
    QPushButton* m_pFrishFriendPB;  //刷新好友列表
    QPushButton* m_pshowOnlineUsrPB;    //展示在线好友信息
    QPushButton* m_pSearchUsrPB;        //查找好友
    QPushButton* m_pMsgSendPB;   //信息发送按钮
    QPushButton* m_pPrivateChatPB;  //私聊按钮
    Online* m_pOnline;

};

#endif // FRIEND_H
