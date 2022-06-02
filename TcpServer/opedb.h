#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QStringList>

class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    ~OpeDB();
    static OpeDB& getInstance();
    void init();//数据库连接初始化

    bool handleRegist(const char*name, const char* pwd);    //处理注册信息
    bool handleLogin(const char* name, const char* pwd);    //处理登录信息
    void handleOffline(const char*name);        //处理下线信息
    bool handleCancel(const char*name, const char* pwd);    //处理注销信息
    QStringList handleAllOnline();      //处理所有在线用户信息
    int handleSearchUsr(const char* name);  //查找在线用户
    int handleAddFriend(const char* pername, const char* name); //处理添加好友的信息
    int handleAddSuscess(const char* pername, const char* name);    //好友添加成功
    QStringList handleFrushFriend(const char* name);    //处理刷新好友
    bool handleDelFriend(const char* name, const char* friendname); //处理删除好友


signals:

public slots:

private:
    QSqlDatabase m_db;  //连接数据库

};

#endif // OPEDB_H
