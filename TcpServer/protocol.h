#ifndef PROTOCOL_H
#define PROTOCOL_H
#include<stdlib.h>
#include<string.h>

typedef unsigned int uint;

#define REGIST_OK "regist ok"  //注册成功
#define REGIST_FAILED "regist failed:name exited"  //注册失败

#define LOGIN_OK "login ok"  //登录成功
#define LOGIN_FAILED "login failed:name, pwd or relogin"  //登录失败

#define CANCEL_OK "cancel ok"  //注销成功
#define CANCEL_FAILED "cancel failed:name, pwd or recancel"  //注销失败

#define SEARCH_USR_NO "不存在该用户！"
#define SEARCH_USR_ONLINE "查找到该用户在线！"
#define SEARCH_USR_OFFLINE "查找到该用户不在线！"

#define UNKNOW_ERROR "unknow error"
#define EXISTED_FRIEND "friend existed"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NOEXIST "usr not existed"

#define ADD_SUSSCESS "add susscess"
#define ADD_FAILED  "add failed"

#define DEL_FRIEND_OK "delete friend ok"

#define DIR_NOEXIST "dir not exist"
#define DIR_NAME_EXISTED "dir existed"
#define CREATE_DIR_OK "create dir ok"

#define DEL_DIR_OK "delete dir ok"
#define DEL_DIR_FAILED "delete dir failed: is reguler file"

#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILED "rename file failed"

#define ENTER_DIR_FAILED "enter dir failed: is reguler file"

#define UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAILED "upload file filed"

#define DEL_FILE_OK "delete file ok"
#define DEL_FILE_FAILED "delete file failed: is  dir"

#define SHARE_FILE_OK "share file ok"

#define MOVE_FILE_OK "move file ok"
#define MOVE_FILE_FAILED "move file failed: it's a file"
#define COMMON1_ERR "operate failed: system is busy"


enum ENUM_MSG_TYPE {
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,	//注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND,	//注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST,	//登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,	//登录回复

    ENUM_MSG_TYPE_CANCEL_REQUEST,	//注销请求
    ENUM_MSG_TYPE_CANCEL_RESPOND,	//注销回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,	//在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,	//在线用户回复

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,	//搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,	//搜索用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,	//添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,	//添加好友回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGGREE,	//同意好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,	//拒绝好友回复

    ENUM_MSG_TYPE_FRUSH_FRIEND_REQUEST,	//刷新好友请求
    ENUM_MSG_TYPE_FRUSH_FRIEND_RESPOND,	//刷新好友回复

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,	//删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,	//删除好友回复

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,	//私聊好友请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,	//私聊好友回复

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,	//群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,	//群聊回复

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,	//创建文件夹的请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,	//创建文件夹的回复

    ENUM_MSG_TYPE_FRUSH_FILE_REQUEST,	//刷新文件的请求
    ENUM_MSG_TYPE_FRUSH_FILE_RESPOND,	//刷新文件的回复

    ENUM_MSG_TYPE_DEL_DIR_REQUEST,	//删除目录的请求
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,	//删除目录的回复

    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,	//重命名文件的请求
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,	//重命名文件回复

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,	//进入目录的请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,	//进入目录的回复

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,	//上传文件的请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,	//上传文件回复

    ENUM_MSG_TYPE_DEL_FILE_REQUEST,	//删除常规文件的请求
    ENUM_MSG_TYPE_DEL_FILE_RESPOND,	//删除常规文件回复

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,	//下载文件的请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,	//下载文件回复

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,	//共享文件的请求
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,	//共享文件回复
    ENUM_MSG_TYPE_SHARE_FILE_NOTE,		//共享文件的通知
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND, //共享文件通知回复

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,	//移动文件的请求
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,	//移动文件回复

    ENUM_MSG_TYPE_MAX = 0x00ffffff,
};

typedef struct FileInfo {
    char caFileName[32];	//文件名
    int iFileType;			//文件类型

}FileInfo;


typedef struct PDU
{
    uint uiPDULen;	//总的协议数据单元大小
    uint uiMsgType;	//消息类型
    char caData[64];	//文件名
    uint uiMsgLen;	//实际消息长度
    int caMsg[];	//实际消息
}PDU;

PDU* mkPDU(uint uiMsgLen); //用于产生结构体,传入uiMsgLen为实际消息长度

#endif // PROTOCOL_H
