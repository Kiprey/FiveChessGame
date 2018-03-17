#ifndef NETWORKMODULE_H
#define NETWORKMODULE_H

//这就是一个网络模块，包括监听，连接，查看内网IP等等的功能
//PS:本来要搞观战模块的，但突然发现这程序只是练手的，估计没几人会用，
    //所以搞好一半的观战模块被我全删了

#include <QDebug>

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QByteArray>
#include <QHostAddress>
#include <QNetworkInterface>

#include "MsgEnums.h"

class NetworkModule : public QObject
{
    Q_OBJECT

public:
    explicit NetworkModule();
    ~NetworkModule();

    //返回QString类型的内网IP
    static QString InsideIPAddress(void);

    //当前网络状态
    enum NetworkState{NETWORK_NONE, NETWORK_SERVER_LISTEN, NETWORK_SERVER_CONNECT, NETWORK_CLIENT};
    int NetworkStatus;

    //这2个函数会被Widget里直接调用
    //服务器发送数据到特定套接字
    void ServerMsgSend(QString Msg);
    //客户端发送数据到服务器
    void ClientMsgSend(QString Msg);

public slots:
    //此3个槽函数会被ChlidWidgt里的信号所接收
    //服务器开始监听
    void OnServerListenning(void);
    //服务器断开所有连接，取消监听
    void OnCancelListenning(void);
    //客户端尝试连接
    void OnClientConnect(QString IP);

private:
    QTcpServer * Server;
    //玩家的套接字
    QTcpSocket * PlayerSocket;

    QTcpSocket * Client;
    //存数据的容器
    QByteArray * ByteArray;
signals:
    //此信号会连接OnlineOptionWidget里的OnHideWidget()，把OnlineOptionWidget隐藏，并开始游戏
    void InStartGame(void);
    //这个是对OnlineOptionWidget的信号
    void InAppendLog(QString Log);
    //这个是对Widget里的信号
    void InProcessMsg(QString Log);
    //同Widget,当断开连接时会触发
    void InDisconnected(void);
private slots:

    //以下的槽函数则会被内部调用
    //当有新连接连接至服务器时
    void OnNewConnectServer(void);
    //有连接与服务器断开连接（或者服务器自己断开连接）
    void OnServerDisconnected(void);

    //服务器接收数据
    void OnServerMsgReceive(void);

    //当客户端连接成功时
    void OnNewConnectClient(void);
    //当客户端与服务器的连接断开
    void OnClientDisconnected(void);

    //客户端从服务器接收数据
    void OnClientMsgReceive(void);

};

#endif // NETWORKMODULE_H
