#include "Widget.h"
#include "ui_widget.h"

//这三个函数是个比较重要的函数，几乎所有的信息都会被这三个函数处理
//包括，发送到本机的消息，发送到网络的消息，以及显示消息等等
//与这个函数结合很紧密的是头文件MsgEnums.h

//程序内部的消息处理
void Widget::OnToLocalMsg(QString Text)
{
    //将Text切片
    QString EnumString = Text.mid(0, ENUM_SIZE);
    QString TextString = Text.mid(ENUM_SIZE, -1);

    if (EnumString == PLAYERMSG_ENUM)
    {
        if (TextString == tr("#clear"))
            TextBrowser->clear();
        else
        {
            //如果是联机对战
            if (PlayingModeStatus == MODE_PVP)
            {
                //显示文字在TextBrowser
                TextBrowser->append(tr("[我]") + TextString);
                //发送信息给对方
                emit InToNetworkMsg(QString(PLAYERMSG_ENUM) + TextString);
            }
            else
                emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]未进行联机对战，无法发送信息！"));
        }
    }
    else if (EnumString == SYSMSG_ENUM)
    {
        //使字符串以红色显示的CSS代码
        QString RedString = "<font color=red>%1</font>";
        RedString = RedString.arg(TextString);
        TextBrowser->append(RedString);
    }
}

//发送消息到联机玩家
void Widget::OnToNetworkMsg(QString Text)
{
    if (networkModule->NetworkStatus == NetworkModule::NETWORK_SERVER_CONNECT)
        networkModule->ServerMsgSend(Text);
    else if (networkModule->NetworkStatus == NetworkModule::NETWORK_CLIENT)
        networkModule->ClientMsgSend(Text);
}

//从联机玩家那里接受消息
void Widget::OnFromNetworkMsg(QString Text)
{
    //将Text切片
    QString EnumString = Text.mid(0, ENUM_SIZE);
    QString TextString = Text.mid(ENUM_SIZE, -1);

    if(EnumString == PLAYERMSG_ENUM)
        TextBrowser->append(tr("[对方]") + TextString);
    else if (EnumString == YOURCHESSCOLOR_ENUM)
    {
        //这个Case只有客户端在游戏开始时才会执行一次，其他过程不会执行
        if (TextString == "BLACK")
        {
            Player1Status = PLAYER_BLACK;
            Player2Status = PLAYER_WHITE;
            TurnPlayerStatus = Player1Status;
        }
        else if (TextString == "WHITE")
        {
            Player1Status = PLAYER_WHITE;
            Player2Status = PLAYER_BLACK;
            TurnPlayerStatus = Player2Status;
        }
        QString TmpString = tr("[系统提示]开始联机对战！");
        emit InToLocalMsg(QString(SYSMSG_ENUM) + TmpString);
        TmpString = tr("[系统提示]你是") + (Player1Status == PLAYER_BLACK? tr("黑方") : tr("白方"));
        emit InToLocalMsg(QString(SYSMSG_ENUM) + TmpString);

        ResetRoundTimer();
        DisplayRoundTime();
    }
    else if (EnumString == UNDOCHESS_ENUM)
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]对方悔棋了一次"));
        CoreUndoChess();
        update();
    }
    else if (EnumString == CHESSPOSITION_ENUM)
    {
        Player2PutChess(TextString);
    }
    else if (EnumString == GAMEGIVEUP_ENUM)
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]对方认输了，游戏结束！！！")));
        TotalTimeLabel->setText(tr("WINNER:"));
        //接收方肯定是胜利者，故用Player1Status
        RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
        RoundTimeLabel->setText((Player1Status == PLAYER_BLACK? tr("BLACK") : tr("WHITE")));
        AfterPlayGame();
    }
    else if (EnumString == DRAWCHESS_ENUM)
        GetDrawChessMsg(TextString);
    else if (EnumString == GAMEPASS_ENUM)
    {
        ResetRoundTimer();
        ExchangeTurnPlayerStatus();
    }
}

//这个函数是只由RoundTimer调用的，程序不显式调用
void Widget::OnChangeRoundTime(void)
{
    RoundTimeCount -= 1;
    DisplayRoundTime();
}

//这个函数是刷新回合时间在标签上的显示
void Widget::DisplayRoundTime(void)
{
/*
在联机对战中
Client的定时器如果时间到了，那么什么也不做，必须等待Server发送过来的GamePass消息
        才能轮换棋方（在这一步里，轮换棋方的相关操作在OnFromNetworkMsg()里）
Server的定时器如果时间到了，则会发送相应的信息，让Client结束本回合，并轮换棋方
*/
    //如果此时是玩家1的回合
    if (TurnPlayerStatus == Player1Status)
    {
        //如果时间快结束了，则颜色变红并闪烁
        if (RoundTimeCount > 0 && RoundTimeCount <= 10)
        {
            if (RoundTimeCount % 2 == 0)
                RoundTimeLabel->setStyleSheet("QLabel{color:#0022FF;background:#FF0000}");
            else
                RoundTimeLabel->setStyleSheet("QLabel{color:#FF0000;background:#0022FF}");
        }
        else if (RoundTimeCount > 10)
            RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
        //如果在人机模式中超时，则pass到下一个玩家里去
        else if (PlayingModeStatus == MODE_PVE && RoundTimeCount <= 0)
        {
            //重置选中的棋子，把选择棋子的痕迹抹去
            ChooseChessX = 0;
            ChooseChessY = 0;
            update();

            ResetRoundTimer();
            ExchangeTurnPlayerStatus();
        }
    }
    //如果是玩家2的回合
    else if (TurnPlayerStatus == Player2Status)
    {
        RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#808080}");
        //在人机模式下，根本就不需要考虑人机超时
        //60s够它算N遍的了
    }

    //在联机模式中，定时器的超时事件
    //作为服务器的福利，自己的定时器在整场比赛中为主要的
    if (PlayingModeStatus == MODE_PVP && RoundTimeCount <= 0 &&
            networkModule->NetworkStatus == NetworkModule::NETWORK_SERVER_CONNECT)
    {
        ResetRoundTimer();
        ExchangeTurnPlayerStatus();
        emit InToNetworkMsg(QString(GAMEPASS_ENUM));
    }

    RoundTimeLabel->setText(QString("%1\n %2s").arg(tr("RoundTime")).arg(RoundTimeCount));
}

//这个函数既由TotalTimer调用，又在BeforePlayGame()中显式调用
void Widget::OnDisplayAndChangeTotalTime(void)
{
    TotalTimeLabel->setText(QString("%1\n %2m %3s").arg("TotalTime").arg(TotalTimeCount / 60).arg(TotalTimeCount % 60));
    TotalTimeCount += 1;

}

//这个函数在点击：开始游戏->联机对战时会弹出来
//主要是由玩家选择是否创建对局和连接对局
void Widget::OnChooseOnlineOption(void)
{
    OnlineOption->show();
}

//在游戏开始后，第一个玩家下棋前所必须要做的一些相关准备
void Widget::BeforePlayGame(void)
{
    //初始化棋盘数据
    for(int i = 0; i < ChessLines; i++)
        for (int k = 0; k < ChessLines; k++)
            ChessPosition[i][k] = PLAYER_NONE;

    //清空下过棋子的历史记录
    BePutChess->clear();
    update();//刷新棋盘

    //设置总时间开始计时
    TotalTimer->start(1000);

    //初始化回合时间
    TotalTimeCount = 0;
    RoundTimeCount = 60;
    OnDisplayAndChangeTotalTime();
    DisplayRoundTime();

    //设置按钮菜单的相关Action
    Action1->setText(tr("唯一的悔棋机会"));
    Action2->setText(tr("和棋"));
    Action3->setText(tr("认输"));
    Button->setText(tr("游戏选项"));
    ButtonMenu->addAction(Action3);

    disconnect(Action1, SIGNAL(triggered(bool)), this, SLOT(OnMode_PVE(void)));
    disconnect(Action2, SIGNAL(triggered(bool)), this, SLOT(OnChooseOnlineOption(void)));
    disconnect(Action3, SIGNAL(triggered(bool)), this, SLOT(About(void)));
    connect(Action1, SIGNAL(triggered(bool)), this, SLOT(OnUndoChess(void)));
    connect(Action2, SIGNAL(triggered(bool)), this, SLOT(OnDrawChess(void)));
    connect(Action3, SIGNAL(triggered(bool)), this, SLOT(OnCheckWin(bool)));

    if (PlayingModeStatus == MODE_PVE ||
            (PlayingModeStatus == MODE_PVP &&
             networkModule->NetworkStatus == NetworkModule::NETWORK_SERVER_CONNECT))
    {
        //这边，之所以加了networkModule->NetworkStatus == NetworkModule::NETWORK_SERVER_CONNECT
        //这条判断，是因为我将决定哪方黑哪方白的的权限给了服务器一方，服务器将在连接上客户端后自动选择黑白方

        //初始化随机数种子
        qsrand(time(nullptr));
        //设置玩家的棋子颜色
        if (qrand() % 2 == 0)
        {
            Player1Status = PLAYER_WHITE;
            Player2Status = PLAYER_BLACK;
        }
        else
        {
            Player1Status = PLAYER_BLACK;
            Player2Status = PLAYER_WHITE;
        }
        TurnPlayerStatus = (Player1Status == PLAYER_BLACK? Player1Status : Player2Status);

        QString TmpString;

        //如果是人机模式
        if (PlayingModeStatus == MODE_PVE)
        {
            TmpString = tr("[系统提示]开始人机对战V1.0！");
            emit InToLocalMsg(QString(SYSMSG_ENUM) + TmpString);
            TmpString = tr("[系统提示]你是") + (Player1Status == PLAYER_BLACK? tr("黑方") : tr("白方"));
            emit InToLocalMsg(QString(SYSMSG_ENUM) + TmpString);
            ResetRoundTimer();
            DisplayRoundTime();
            if (Player2Status == TurnPlayerStatus)
                Player2PutChess(nullptr);
        }
        //如果是联机模式中的服务器端玩家
        else
        {
            TmpString = tr("[系统提示]开始联机对战！");
            emit InToLocalMsg(QString(SYSMSG_ENUM) + TmpString);
            TmpString = tr("[系统提示]你是") + (Player1Status == PLAYER_BLACK? tr("黑方") : tr("白方"));
            emit InToLocalMsg(QString(SYSMSG_ENUM) + TmpString);
            //通知联机玩家它的棋子颜色
            TmpString = (Player2Status == PLAYER_BLACK? "BLACK" : "WHITE");
            emit InToNetworkMsg(QString(YOURCHESSCOLOR_ENUM) + TmpString);

            ResetRoundTimer();
            DisplayRoundTime();
        }
    }
}

//当联机对战游戏开始，即ChooseOnlineOptionWidget关闭后
//会被调用，设置此时的模式为PVP
void Widget::OnMode_PVP(void)
{
    PlayingModeStatus = MODE_PVP;
    BeforePlayGame();

}

//当人机对战开始时会调用
void Widget::OnMode_PVE(void)
{
    PlayingModeStatus = MODE_PVE;
    BeforePlayGame();
}

//更改轮到的棋方，比如轮到的玩家由玩家1转变为玩家2
void Widget::ExchangeTurnPlayerStatus(void)
{
    if (TurnPlayerStatus == Player1Status)
        TurnPlayerStatus = Player2Status;
    else if (TurnPlayerStatus == Player2Status)
        TurnPlayerStatus = Player1Status;
    //刷新回合时间显示
    DisplayRoundTime();

    //当轮换完棋方后，轮到的为Player2则调用机器算法下棋
    if (TurnPlayerStatus == Player2Status && PlayingModeStatus == MODE_PVE)
        Player2PutChess(nullptr);
}

//重置回合时间
void Widget::ResetRoundTimer(void)
{
    //重置回合时间为60s
    RoundTimeCount = 60;
    //开始新的周期，需要停止以前的周期
    RoundTimer->stop();
    RoundTimer->start(1000);
}
