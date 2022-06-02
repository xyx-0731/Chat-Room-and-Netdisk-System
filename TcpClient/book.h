#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include<QListWidget>
#include<QPushButton>
#include<QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include<QMessageBox>
#include"protocol.h"
#include<QInputDialog>
#include<QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU* pdu);    //更新文件列表
    QString enterDir();     //进入文件
    void clearEnterDir();  //清空文件名
    void setDownloadStatus(bool status);    //下载
    bool getDownloadStatus();   //获取状态
    QString getSaveFilePath();  //获取下载路径
    QString getShareFileName(); //获取分享文件
    QTimer* m_pTimer;
    qint64 m_iTotal;        //总文件大小
    qint64 m_iRecved;   //已收到大小;


signals:

public slots:
    void createDir();    //创建目录
    void frushFile();    //刷新(查看)文件
    void delDir();   //删除文件夹
    void renameFile();   //重命名文件
    void enterDir(const QModelIndex& index);     //进入文件夹
    void returnPre();        //返回上一级
    void uploadFile();       //上传文件(信息)
    void uploadFileData();    //上传文件(信息数据)
    void delRegFile();       //删除常规文件
    void downloadFile();     //下载文件

    void shareFile();   //分享文件
    void moveFile();     //移动文件
    void selectDsestDir();   //选择移动文件的目标目录

private:
    QListWidget* m_pBookList;
    QPushButton* m_pReturnPB; //返回
    QPushButton* m_pCreateDirPB; //创建文件夹
    QPushButton* m_pDelDirPB;  //删除文件夹
    QPushButton* m_pRenamePB;  //重命名
    QPushButton* m_pFrushFilePB; //刷新文件夹
    QPushButton* m_pUploadPB;  //上传文件
    QPushButton* m_pDownloadPB; //下载文件
    QPushButton* m_pDelFilePB; //删除文件
    QPushButton* m_pShareFilePB; //分享文件
    QPushButton* m_pMoveFilePB; //移动文件
    QPushButton* m_pSelectDirPB;  //选择文件夹

    QString m_strEnterDir;
    QString m_strUploadFilePath;
    QString m_strSaveFilePath;
    bool m_bDownload;   //是否处于下载状态

    QString m_strShareFilName; //分享文件
    QString m_strMoveFileName;  //移动文件名
    QString m_strMoveFilePath;  //移动文件路径
    QString m_strDestDir;       //设置目标路径
};

#endif // BOOK_H
