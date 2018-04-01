#include "Widget.h"
#include "ui_widget.h"

//玩家2下棋
//注意，函数形参chessPosition与类里的ChessPosition不一样
void Widget::Player2PutChess(QString chessPosition)
{
    //这个Position在这里是个例外
    //它不是棋子的坐标，而是棋子位置的行列数
    //例如(1,4)=>第一行第四列
    QPoint Position;
    if (PlayingModeStatus == MODE_PVE)
        Position = ComputerPutChess();
    else if (PlayingModeStatus == MODE_PVP)
    {
        Position.setX(chessPosition.mid(0, 2).toInt());
        Position.setY(chessPosition.mid(2, 2).toInt());
    }

    //下棋
    ChessPosition[Position.x()][Position.y()] = Player2Status;
    //添加进下过的棋子
    int LastChooseChessX = Position.x() * ChessLineWidth + ChessCanvasStartX;
    int LastChooseChessY = Position.y() * ChessLineWidth + ChessCanvasStartY;
    BePutChess->append(QPoint(LastChooseChessX, LastChooseChessY));
    update();

    //判断是否胜利
    OnCheckWin(true);

}

//主动按下悔棋键时调用的核心指令，或者收到悔棋事件时调用
void Widget::CoreUndoChess(void)
{
    //删除两颗棋子
    for (int i = 0; i < 2; i++)
    {
        int LastChooseChessX = BePutChess->last().x();
        int LastChooseChessY = BePutChess->last().y();
        int TempRow = (LastChooseChessX - ChessCanvasStartX) / ChessLineWidth;
        int TempColumn = (LastChooseChessY - ChessCanvasStartY) / ChessLineWidth;
        ChessPosition[TempRow][TempColumn] = PLAYER_NONE;
        BePutChess->removeLast();
    }
}

//接收到和局申请并处理
void Widget::GetDrawChessMsg(QString TextString)
{
    if (TextString == "Try")
    {
        QMessageBox MsgBox(this);
        MsgBox.setWindowTitle(tr("新申请"));
        MsgBox.setText(tr("对方向你发起和棋申请，是否同意？"));
        MsgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::NoToAll);
        MsgBox.setButtonText(QMessageBox::Yes, tr("同意"));
        MsgBox.setButtonText(QMessageBox::No, tr("不同意"));
        MsgBox.setButtonText(QMessageBox::NoToAll, tr("不同意此次与以后的申请"));
        MsgBox.setDefaultButton(QMessageBox::Yes);
        MsgBox.setIcon(QMessageBox::Question);
        switch(MsgBox.exec())
        {
        case QMessageBox::Yes:
            emit InToNetworkMsg(QString(DRAWCHESS_ENUM) + "Yes");
            emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！和局！")));
            TotalTimeLabel->setText(tr("WINNER:"));
            RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
            RoundTimeLabel->setText(tr("NONE"));
            AfterPlayGame();
            break;
        case QMessageBox::No:
            emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]您拒绝了对方的和棋请求")));
            emit InToNetworkMsg(QString(DRAWCHESS_ENUM) + "No");
            break;
        case QMessageBox::NoToAll:
            emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]您拒绝了对方的所有和棋请求")));
            emit InToNetworkMsg(QString(DRAWCHESS_ENUM) + "NoToAll");
            break;
        }
    }
    else if (TextString == "Yes")
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]对方同意了您的和棋请求！"));
        emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！和局！")));
        TotalTimeLabel->setText(tr("WINNER:"));
        RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
        RoundTimeLabel->setText(tr("NONE"));
        AfterPlayGame();
    }
    else if (TextString == "No")
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]对方拒绝了您的和棋请求！"));
    }
    else if (TextString == "NoToAll")
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]对方拒绝了您的") + "<font color=red>"
                          + tr("所有") + "</font>" + tr("和棋请求！"));
        emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]您将被禁止发送和局请求")));
        Action2->setDisabled(true);
    }
}

//当断开连接时，信号会被接收
void Widget::OnDisconnected(void)
{
    if (TurnPlayerStatus != TURNPLAYER_NONE)
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏连接断开，游戏结束！")));
        TotalTimeLabel->setText(tr("WINNER:"));
        RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
        RoundTimeLabel->setText(tr("NONE"));
        AfterPlayGame();
    }
}

//点击： 开始游戏->关于 时会弹出的关于信息
void Widget::About(void)
{
    QMessageBox MsgBox(this);
    MsgBox.setWindowTitle(tr("关于"));
    MsgBox.setText(tr("作者： Hipre") + "\n" +
                   tr("邮箱： Hipre@qq.com") + "\n" +
                   tr("PS：如果有BUG或其他更好的想法，欢迎致信至邮箱！"));
    MsgBox.setStandardButtons(QMessageBox::Ok);
    MsgBox.setButtonText(QMessageBox::Ok, tr("确定"));
    MsgBox.setIcon(QMessageBox::Information);
    MsgBox.exec();
}

//在分出胜负后，开始新对局前，所要做的相关收尾动作
void Widget::AfterPlayGame(void)
{
    Player1Status = PLAYER_NONE;
    Player2Status = PLAYER_NONE;
    //TurnPlayerStatus的设置应放在if前面，不然取消监听时会触发OnDisconnected();
    TurnPlayerStatus = TURNPLAYER_NONE;
    if (PlayingModeStatus == MODE_PVP)
        networkModule->OnCancelListenning();
    PlayingModeStatus = MODE_NONE;

    TotalTimer->stop();
    RoundTimer->stop();

    Button->setText(tr("开始游戏"));
    Action1->setText(tr("新游戏"));
    Action2->setText(tr("局域网联机对战"));
    Action3->setText(tr("关于"));
    Action1->setEnabled(true);
    Action2->setEnabled(true);
    disconnect(Action1, SIGNAL(triggered(bool)), this, SLOT(OnUndoChess(void)));
    disconnect(Action2, SIGNAL(triggered(bool)), this, SLOT(OnSavePVEDataFile(void)));
    disconnect(Action3, SIGNAL(triggered(bool)), this, SLOT(OnCheckWin(bool)));
    connect(Action1, SIGNAL(triggered(bool)), this, SLOT(OnMode_PVE(void)));
    connect(Action2, SIGNAL(triggered(bool)), this, SLOT(OnChooseOnlineOption(void)));
    connect(Action3, SIGNAL(triggered(bool)), this, SLOT(About(void)));

    ButtonMenu->clear();
    PVEMenu->addAction(Action1);
    PVEMenu->addAction(OpenPVEFile);
    PVEMenu->setTitle(tr("人机对战"));

    ButtonMenu->addMenu(PVEMenu);
    ButtonMenu->addAction(Action2);
    ButtonMenu->addAction(Action3);
    Button->setMenu(ButtonMenu);
}

//点击悔棋或者每次下完棋子后检查是否胜利
void Widget::OnCheckWin(bool Status)
{
/*
    Status 的意思是状态，这里只的是调用OnCheckWin的方式
    true表示的是每次下棋后的正常调用(正常调用)
    false指的是使用这个参数的人认输了(不正常调用)

    6_6之所以弄了这个bool是想直接和那个按钮的信号所发出来的false对应（好吧，实际上发出来的是0，对应false）
*/
    if (Status)
    {
        //连成的棋子数
        bool Win = false;

        //检查所下最后一颗棋子有没有连成五子
        if (!BePutChess->isEmpty())
        {
            int LastChessRow = (BePutChess->last().x() - ChessCanvasStartX) / ChessLineWidth;
            int LastChessColumn = (BePutChess->last().y() - ChessCanvasStartY) / ChessLineWidth;
            //为了保险而加的if
            if (ChessPosition[LastChessRow][LastChessColumn] == TurnPlayerStatus)
                Win = CoreCheckWin(QPoint(LastChessRow, LastChessColumn));
        }
        //两个特殊情况，胜利或者棋盘满了
        if (Win || BePutChess->count() >= ChessLines * ChessLines)
        {
            //如果有人胜利
            if (Win)
            {
                emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！！！")));
                TotalTimeLabel->setText(tr("WINNER:"));
                RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
                RoundTimeLabel->setText((TurnPlayerStatus == PLAYER_BLACK? tr("BLACK") : tr("WHITE")));
            }
            //如果棋盘刚好满了，并且还没人胜利
            else
            {
                emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！和局！")));
                TotalTimeLabel->setText(tr("WINNER:"));
                RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
                RoundTimeLabel->setText(tr("NONE"));

                //if(PlayingModeStatus == MODE_PVE)
                    //OnMode_PVE();
            }
            //只要进入这个if,肯定都是要结束游戏
            AfterPlayGame();
        }
        else//否则
        {
            ResetRoundTimer();
            ExchangeTurnPlayerStatus();
        }
    }
    else if (!Status)
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！！")));
        TotalTimeLabel->setText(tr("WINNER:"));
        RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
        //这里反转了一下
        RoundTimeLabel->setText((Player2Status == PLAYER_BLACK? tr("BLACK") : tr("WHITE")));
        if(PlayingModeStatus == MODE_PVP)
            emit InToNetworkMsg(QString(GAMEGIVEUP_ENUM));
        AfterPlayGame();
    }
}

//检查五子的核心
//返回是否连成五子
bool Widget::CoreCheckWin(QPoint Point)
{
    int WinCount = 0;
    int LastChessRow = Point.x();
    int LastChessColumn = Point.y();

    //遍历下的最后一颗棋子的4个方向
    for (int a = -1; a <= 1; a++)
    {
        for (int b = -1; b <= 0; b++)
        {
            //这个if判断与上面的两个嵌套for循环搭配
            //使得只能遍历4个方向
            //再不懂就画图自己看
            if(b == 0 && (a == 0 || a == 1))
                continue;

            //之所以设置为1而不是为0
            //是因为这个1代表的是最初的棋子，也就是下的最后一颗棋子
            WinCount = 1;

            //分别向两个方向遍历
            int TmpRow = 0;
            int TmpColumn = 0;

            for (int Direct = -1; Direct <= 1; Direct++)
            {
                if(Direct != 0)
                {
                    //设置当前遍历的位置为最后一颗下的棋子的行列
                    TmpRow = LastChessRow;
                    TmpColumn = LastChessColumn;

                    while(true)
                    {
                        //向遍历方向推进一个
                        TmpRow += a * Direct;
                        TmpColumn += b * Direct;

                        //如果越界，退出
                        if (TmpRow < 0 || TmpColumn < 0 || TmpRow >= ChessLines || TmpColumn >= ChessLines)
                            break;
                        //如果为自己的棋子，则棋数+1
                        if (ChessPosition[TmpRow][TmpColumn] == TurnPlayerStatus)
                            WinCount++;
                        //否则，退出此循环
                        else
                            break;
                    }
               }
            }
            if (WinCount >= 5)
                return true;
        }
    }
    return false;

}

//主动按下悔棋键
void Widget::OnUndoChess(void)
{
    if (TurnPlayerStatus != Player1Status)
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]只能在轮到你时悔棋"));
        return;
    }
    else if (BePutChess->length() < 2)
    {
        emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]您没下过棋子，无法悔棋"));
        return;
    }

    //设置只能毁一次棋
    Action1->setDisabled(true);
    emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]你使用了唯一的悔棋机会"));
    CoreUndoChess();
    if (PlayingModeStatus == MODE_PVP)
        emit InToNetworkMsg(QString(UNDOCHESS_ENUM));

    update();
}

//PVP中主动按下和棋键
void Widget::OnDrawChess(void)
{
    emit InToLocalMsg(QString(SYSMSG_ENUM) + tr("[系统提示]已发送和局申请"));
    emit InToNetworkMsg(QString(DRAWCHESS_ENUM) + "Try");
}

