#include "NetworkModule.h"

NetworkModule::NetworkModule()
{
    NetworkStatus = NETWORK_NONE;
    PlayerSocket = nullptr;

    Server = new QTcpServer();
    Client = new QTcpSocket();
    ByteArray = new QByteArray();

    //Server
    connect(Server, SIGNAL(newConnection()), this, SLOT(OnNewConnectServer()));

    //Client
    connect(Client, SIGNAL(connected()), this, SLOT(OnNewConnectClient()));
    //当断开连接时发出的信号
    connect(Client, SIGNAL(disconnected()), this, SLOT(OnClientDisconnected()));
    connect(Client, SIGNAL(readyRead()), this, SLOT(OnClientMsgReceive()));
}

NetworkModule::~NetworkModule()
{
    delete Client;
    delete Server;
    delete ByteArray;
}

void NetworkModule::OnServerListenning(void)
{
    if (NetworkStatus != NETWORK_NONE)
    {
        if (NetworkStatus == NETWORK_CLIENT)
            emit InAppendLog(SYSMSG_ENUM + tr("您正在连接对局中，无法创建对局"));
        else
            emit InAppendLog(SYSMSG_ENUM + tr("您已经创建过对局，重复操作无效！！！"));
        return;
    }
    else
        NetworkStatus = NETWORK_SERVER_LISTEN;

    //创建监听失败的重试次数
    int FailedCount;
    for (FailedCount = 0; FailedCount < 3; FailedCount++)
    {
        if(Server->listen(QHostAddress::Any, 12345))
            break;
        else
            emit InAppendLog(SYSMSG_ENUM + tr("创建对局失败！！！正在重试...") + QString("%1/3").arg(FailedCount + 1));
    }
    //如果失败次数小于3次
    if (FailedCount < 3)
        emit InAppendLog(PLAYERMSG_ENUM + tr("对局创建成功，等待玩家连接..."));
    else
    {
        emit InAppendLog(SYSMSG_ENUM + tr("创建对局失败！！！请稍后重试"));
        NetworkStatus = NETWORK_NONE;
    }
}

void NetworkModule::OnNewConnectServer(void)
{
    NetworkStatus = NETWORK_SERVER_CONNECT;

    QTcpSocket * NewSocket;

    NewSocket = Server->nextPendingConnection();//根据当前新连接创建一个QTepSocket
    connect(NewSocket, SIGNAL(readyRead()), this, SLOT(OnServerMsgReceive()));
    connect(NewSocket, SIGNAL(disconnected()), this, SLOT(OnServerDisconnected()));

    PlayerSocket = NewSocket;
    //暂停接受任何连接
    Server->pauseAccepting();
    emit InAppendLog(PLAYERMSG_ENUM + tr("玩家已连接成功，准备开始对局..."));
    emit InStartGame();
}

void NetworkModule::OnServerDisconnected(void)
{
    emit InDisconnected();

    PlayerSocket->deleteLater();
    PlayerSocket = nullptr;

    if(NetworkStatus == NETWORK_SERVER_CONNECT)
    {
        NetworkStatus = NETWORK_SERVER_LISTEN;
        Server->resumeAccepting();
    }
}

void NetworkModule::OnServerMsgReceive(void)
{
    ByteArray->clear();

    //替换接收到的信息的第一个枚举
    QString Msg = QString(ByteArray->append(PlayerSocket->readAll()));
    QString TextString = Msg.mid(ENUM_SIZE, -1);
    emit InProcessMsg(QString(ONLINETOLOCAL_ENUM) + TextString);
}

void NetworkModule::ServerMsgSend(QString Msg)
{
    ByteArray->clear();
    ByteArray->append(Msg);

    PlayerSocket->write(*ByteArray);
}

void NetworkModule::OnClientConnect(QString IP)
{
    if (NetworkStatus != NETWORK_NONE)
    {
        if (NetworkStatus == NETWORK_CLIENT)
            emit InAppendLog(SYSMSG_ENUM + tr("您已经发起连接，重复操作无效"));
        else
            emit InAppendLog(SYSMSG_ENUM + tr("您已创建对局，等待玩家连接。如需连接玩家对局，请取消创建对局"));
        return;
    }
    else
        NetworkStatus = NETWORK_CLIENT;

    emit InAppendLog(PLAYERMSG_ENUM + QString(tr("尝试连接%1...")).arg(IP));
    Client->connectToHost(IP, 12345);

    if (!Client->waitForConnected(1000))
    {
        int i = Client->error();
        switch (i)
        {
        case QAbstractSocket::ConnectionRefusedError:
            emit InAppendLog(SYSMSG_ENUM + tr("错误码:%1 连接被拒绝，请检查IP地址是否错误").arg(i));
            break;
        case QAbstractSocket::RemoteHostClosedError:
            emit InAppendLog(SYSMSG_ENUM + tr("错误码:%1 远程主机强迫关闭了一个现有的连接，请重新连接").arg(i));
            break;
        case QAbstractSocket::HostNotFoundError:
            emit InAppendLog(SYSMSG_ENUM + tr("错误码:%1 未找到主机，请检查IP地址是否错误").arg(i));
            break;
        case QAbstractSocket::SocketTimeoutError:
            emit InAppendLog(SYSMSG_ENUM + tr("错误码:%1 连接超时，请检查IP地址是否错误").arg(i));
            break;
        default:
            emit InAppendLog(SYSMSG_ENUM + tr("错误码:%1 网络模块出错").arg(i));
            break;
        }
        Client->close();
        NetworkStatus = NETWORK_NONE;
    }
}

void NetworkModule::OnNewConnectClient(void)
{
    emit InAppendLog(PLAYERMSG_ENUM + tr("对局连接成功，正在准备对局..."));
    emit InStartGame();
}

void NetworkModule::OnClientDisconnected(void)
{
    emit InDisconnected();
    NetworkStatus = NETWORK_NONE;
}

void NetworkModule::ClientMsgSend(QString Msg)
{
    //只发送数据
    ByteArray->clear();
    ByteArray->append(Msg);

    Client->write(*ByteArray);
}

void NetworkModule::OnClientMsgReceive(void)
{
    //接受数据
    //1.服务器主动发送的数据
    //2.客户端发送到服务器的数据，服务器回显回来证明连接正常

    ByteArray->clear();
     //替换接收到的信息的第一个枚举
    QString Msg = QString(ByteArray->append(Client->readAll()));
    QString TextString = Msg.mid(ENUM_SIZE, -1);
    emit InProcessMsg(QString(ONLINETOLOCAL_ENUM) + TextString);
}

void NetworkModule::OnCancelListenning(void)
{
    if (NetworkStatus == NETWORK_CLIENT)
    {
        Client->disconnectFromHost();
    }
    else if (NetworkStatus == NETWORK_SERVER_CONNECT)
    {
        PlayerSocket->disconnectFromHost();
    }
    Server->close();
    NetworkStatus = NETWORK_NONE;
}

QString NetworkModule::InsideIPAddress(void)
{
    QString IP;
    foreach(QHostAddress Address, QNetworkInterface::allAddresses())
    {
        //此if判断只寻找当前的内网IPv4地址
        if(Address.protocol() == QAbstractSocket::IPv4Protocol &&
                !Address.toString().contains("127.0.0.1"))
            IP = Address.toString();
    }
    return IP;
}
