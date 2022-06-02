#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include<QPushButton>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QButtonGroup>
#include<QScrollArea>    //浏览区域
#include<QCheckBox>
#include<QListWidget>


class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);
    static ShareFile& getInstance();
    void updateFriend(QListWidget*pFriendList);    //更新好友列表

signals:


public slots:
    void selectAll();       //全选
    void cancelSelect();    //取消选择

    void okShare();      //确定
    void cancelShare();  //返回


private:
    QPushButton* m_pSelectAllPB;    //全选按钮
    QPushButton* m_pCancelSelectPB; //取消选择

    QPushButton* m_pOkPB;           //确定
    QPushButton* m_pCancelPB;       //返回

    QScrollArea* m_pSA;             //展示区域
    QWidget* m_pFriendW;            //放在展示区域
    QVBoxLayout* m_pFriendWVBL;     //
    QButtonGroup* m_pButonGroup;    //管理好友

};

#endif // SHAREFILE_H
