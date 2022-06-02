#include "opedb.h"
#pragma execution_character_set("utf-8")


OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE"); //添加使用SQLITE3数据库
    m_db.setHostName("localhost");
    m_db.setDatabaseName("E://cloud//TcpServer//cloud.db");
    m_db.open();
}

OpeDB::~OpeDB()
{
    m_db.close();
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    if (m_db.open()) {
        QSqlQuery query(m_db);
        query.exec("select * from usrInfo");
        while (query.next()) {
            //qDebug() << query.value(0).toString().toUtf8().data();
            QString data = QString("%1, %2, %3, %4").arg(query.value(0).toString()).arg(query.value(1).toString())
                .arg(query.value(2).toString()).arg(query.value(3).toString());
            qDebug() << data;
        }
    }
    else {
        QMessageBox::critical(NULL, "打开数据库", "失败");
    }
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if (name == nullptr || pwd == nullptr) {
        qDebug() << "name or pwd is null";
        return false;
    }
    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\' , \'%2\')").arg(name).arg(pwd).toStdString().c_str();
    QSqlQuery query(m_db);
    bool ret = query.exec(data);
    //query.exec("set @i = 0");
    //query.exec("update usrinfo set id = (@i := @i + 1)");	//对数据进行排序
    return ret;
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if (name == nullptr || pwd == nullptr) {
        qDebug() << "name or pwd is null";
        return false;
    }
    QString data = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0 ").arg(name).arg(pwd).toStdString().c_str();
    QSqlQuery query;
    query.exec(data);
    if (query.next()) {
        QString data = QString("update usrInfo set online = 1 where name = \'%1\' and pwd = \'%2\' ").arg(name).arg(pwd).toStdString().c_str();
        QSqlQuery query;
        query.exec(data);
        return true;
    }
    else {
        return false;
    }
}

void OpeDB::handleOffline(const char *name)
{
    if (name == nullptr ) {
        qDebug() << "name  is null";
        return;
    }
    //qDebug() << name;
    QString data = QString("update usrInfo set online=0 where name = \'%1\'").arg(name);
    //qDebug() << data;
    QSqlQuery query;
    if (query.exec(data)) {
        qDebug() << "修改成功";
    }
}

bool OpeDB::handleCancel(const char *name, const char *pwd)
{
    if (name == nullptr || pwd == nullptr) {
        qDebug() << "name or pwd is null";
        return false;
    }
    QString data = QString("delete from usrInfo where name = \'%1\' and pwd = \'%2\'").arg(name).arg(pwd).toStdString().c_str();
    QSqlQuery query;
    bool ret = query.exec(data);

    data = QString
    ("DELETE FROM friend WHERE id in (SELECT id FROM usrInfo WHERE NAME = '\%1\');")
        .arg(name);
    //qDebug() << data;
    query.exec(data);

    data = QString
    ("DELETE FROM friend WHERE friendId in (SELECT id FROM usrInfo WHERE NAME = '\%2');")
        .arg(name);
    //qDebug() << data;
    query.exec(data);

    return ret;
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from usrInfo where online = 1");
    //qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    while (query.next()) {
        result.append(query.value(0).toString());
        //qDebug() << "找到数据";
    }
    return QStringList(result);
}

int OpeDB::handleSearchUsr(const char *name)
{
    if (nullptr == name) {
        return -1;
    }
    QString data = QString("select online from usrInfo where name = \'%1\'").arg(name);
    //qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    //qDebug() << query.value(0);
    if (query.next()) {
        int ret = query.value(0).toInt();
        if (1 == ret) {
            return 1;
        }
        else if (0 == ret){
            return 0;
        }
    }
    else {
        return -1;
    }
    return 0;

}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if (nullptr == pername || nullptr == name) {
        return -1; //异常
    }
    //qDebug() << pername << name;
    QString data = QString("SELECT * FROM friend WHERE (id = (SELECT id FROM usrInfo WHERE NAME = '\%1\') AND friendId = (SELECT id FROM usrInfo WHERE NAME = '\%2\')) OR (friendId = (SELECT id FROM usrInfo WHERE NAME = '\%3\') AND id = (SELECT id FROM usrInfo WHERE NAME = '%4'));").arg(pername).arg(name).arg(pername).arg(name);
    //qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if (query.next()) {
        return 0;		//双方已经是好友才会有查询结果
    }
    else {
        data = QString("select online from usrInfo where name = \'%1\'").arg(pername);
        //qDebug() << data;
        QSqlQuery query;
        query.exec(data);
        //qDebug() << query.value(0);
        if (query.next()) {
            int ret = query.value(0).toInt();
            if (1 == ret) {
                return 1;	//在线

            }
            else if (0 == ret) {
                return 2;	//不在线
            }
        }
        else {
            return 3;	//不存在
        }
    }
}

int OpeDB::handleAddSuscess(const char *pername, const char *name)
{
    if (nullptr == pername || nullptr == name) {
        qDebug() << "此处错误";
        return -1;
    }
    QSqlQuery query;
    QString data1 = QString("SELECT id FROM usrInfo WHERE NAME = '\%1\'").arg(pername);
    //qDebug() << data1;
    query.exec(data1);
    query.next();
    int ret1 = query.value(0).toInt();

    QString data2 = QString("SELECT id FROM usrInfo WHERE NAME = '\%1'").arg(name);
    //qDebug() << data2;
    query.exec(data2);
    query.next();
    int ret2 = query.value(0).toInt();
    //qDebug() << ret1 << ret2;
    /*QString data3 = QString("INSERT INTO friend(id, friendId) VALUES('\%1\','\%2\')")
        .arg(ret1).arg(ret2);*/
    if (ret1 == ret2) {
        return 0;
    }
    QString data3 = QString("INSERT INTO friend(id, friendId) VALUES('\%1\','\%2\');")
        .arg(ret1).arg(ret2);
    //qDebug() << data3;
    if (query.exec(data3)) {
        return 1;
    }
    else {
        return 2;
    }

}

QStringList OpeDB::handleFrushFriend(const char *name)
{
    QStringList strFriendList;
        strFriendList.clear();
        if (nullptr == name) {
            qDebug() << "好友列表为空";
            return strFriendList;
        }
        QString data = QString
        ("SELECT NAME FROM usrInfo WHERE id in (SELECT id FROM friend WHERE friendId = (SELECT id FROM usrInfo WHERE NAME = \'%1\')) AND online = 1;")
            .arg(name);
        qDebug() << data;
        QSqlQuery query;
        query.exec(data);
        while (query.next()) {
            strFriendList.append(query.value(0).toString());
            //qDebug() << query.value(0).toString();
        }

        //query.clear();
        data = QString
        ("SELECT NAME FROM usrInfo WHERE id in (SELECT friendId FROM friend WHERE id = (SELECT id FROM usrInfo WHERE NAME = \'%1\')) AND online = 1;")
             .arg(name);
        qDebug() << data;
        query.exec(data);
        while (query.next()) {
              strFriendList.append(query.value(0).toString());
              //qDebug() << query.value(0).toString();
        }

        return QStringList(strFriendList);

}

bool OpeDB::handleDelFriend(const char *name, const char *friendname)
{
    if (nullptr == name || nullptr == friendname) {
        qDebug() << " 为空";
        return false;
    }
    QString data = QString
    ("DELETE FROM friend WHERE id = (SELECT id FROM usrInfo WHERE NAME = '\%1\') AND friendId = (SELECT id FROM usrInfo WHERE NAME = '\%2');")
        .arg(name).arg(friendname);
    //qDebug() << data;
    QSqlQuery query;
    query.exec(data);

    data = QString
    ("DELETE FROM friend WHERE id = (SELECT id FROM usrInfo WHERE NAME = '\%1\') AND friendId = (SELECT id FROM usrInfo WHERE NAME = '\%2');")
        .arg(friendname).arg(name);
    //qDebug() << data;
    query.exec(data);
    return true;

}
