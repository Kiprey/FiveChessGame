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
            emit InProcessMsg(QString(TOONLINE_ENUM) + QString(DRAWCHESS_ENUM) + "Yes");
            emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！和局！")));
            TotalTimeLabel->setText(tr("WINNER:"));
            RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
            RoundTimeLabel->setText(tr("NONE"));
            AfterPlayGame();
            break;
        case QMessageBox::No:
            emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]您拒绝了对方的和棋请求")));
            emit InProcessMsg(QString(TOONLINE_ENUM) + QString(DRAWCHESS_ENUM) + "No");
            break;
        case QMessageBox::NoToAll:
            emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]您拒绝了对方的所有和棋请求")));
            emit InProcessMsg(QString(TOONLINE_ENUM) + QString(DRAWCHESS_ENUM) + "NoToAll");
            break;
        }
    }
    else if (TextString == "Yes")
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + tr("[系统提示]对方同意了您的和棋请求！"));
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！和局！")));
        TotalTimeLabel->setText(tr("WINNER:"));
        RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
        RoundTimeLabel->setText(tr("NONE"));
        AfterPlayGame();
    }
    else if (TextString == "No")
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + tr("[系统提示]对方拒绝了您的和棋请求！"));
    }
    else if (TextString == "NoToAll")
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + tr("[系统提示]对方拒绝了您的") + "<font color=red>"
                          + tr("所有") + "</font>" + tr("和棋请求！"));
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]您将被禁止发送和局请求")));
        Action2->setDisabled(true);
    }
}

//当断开连接时，信号会被接收
void Widget::OnDisconnected(void)
{
    if (TurnPlayerStatus != TURNPLAYER_NONE)
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]网络连接断开，游戏结束！")));
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
    {
        networkModule->OnCancelListenning();
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + tr("[系统提示]已断开游戏连接"));
    }
    PlayingModeStatus = MODE_NONE;

    TotalTimer->stop();
    RoundTimer->stop();

    Button->setText(tr("开始游戏"));
    Action1->setText(tr("人机对战"));
    Action2->setText(tr("局域网联机对战"));
    Action3->setText(tr("关于"));
    Action1->setEnabled(true);
    Action2->setEnabled(true);
    disconnect(Action1, SIGNAL(triggered(bool)), this, SLOT(OnUndoChess(void)));
    disconnect(Action2, SIGNAL(triggered(bool)), this, SLOT(OnDrawChess(void)));
    disconnect(Action3, SIGNAL(triggered(bool)), this, SLOT(OnCheckWin(bool)));
    connect(Action1, SIGNAL(triggered(bool)), this, SLOT(OnMode_PVE(void)));
    connect(Action2, SIGNAL(triggered(bool)), this, SLOT(OnChooseOnlineOption(void)));
    connect(Action3, SIGNAL(triggered(bool)), this, SLOT(About(void)));
}

//点击悔棋或者每次下完棋子后检查是否胜利
void Widget::OnCheckWin(bool Status)
{
/*
    Status 的意思是状态，这里只的是调用OnCheckWin的方式
    True表示的是每次下棋后的正常调用(正常调用)
    False指的是使用这个参数的人认输了(不正常调用)

    6_6之所以弄了这个bool是想直接和那个按钮的信号所发出来的False对应（好吧，实际上发出来的是0，对应False）
*/
    if (Status)
    {
        int WinCount = 0;//成列的棋数
        int tmpI;
        int tmpJ;

        //检查所下最后一颗棋子有没有连成五子
        //理论上这两个if是必须成立的，除非发生错误
        //这两个if只是起保险作用
        if (!BePutChess->isEmpty())
        {
            int TmpRow = (BePutChess->last().x() - ChessCanvasStartX) / ChessLineWidth;
            int TmpColumn = (BePutChess->last().y() - ChessCanvasStartY) / ChessLineWidth;
            if (ChessPosition[TmpRow][TmpColumn] == TurnPlayerStatus)
            {
                //遍历下的最后一颗棋子的八个方向
                for (int a = -1; a <= 1; a++)
                {
                    for (int b = -1; b <= 1; b++)
                    {
                        //排除遍历到中心方向的可能
                        //一旦遍历到此中心方向，结局就是死循环
                        if(a == 0 && b == 0)
                            continue;

                        tmpI = TmpRow;
                        tmpJ = TmpColumn;
                        WinCount = 0;
                        while (ChessPosition[tmpI][tmpJ] == TurnPlayerStatus)
                        {
                            //向此方向继续遍历
                            tmpI += a;
                            tmpJ += b;
                            //连成的棋子数+1
                            WinCount++;
                            //如果越界，退出
                            if (tmpI < 0 || tmpJ < 0 || tmpI >= ChessLines || tmpJ >= ChessLines)
                                break;
                        }
                        if (WinCount >= 5)
                            break;
                    }
                    if (WinCount >= 5)
                        break;
                }
            }
        }
        //两个特殊情况，胜利或者棋盘满了
        if (WinCount >= 5 || BePutChess->count() >= ChessLines * ChessLines)
        {
            //如果有人胜利
            if (WinCount >= 5)
            {
                emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！！！")));
                TotalTimeLabel->setText(tr("WINNER:"));
                RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
                RoundTimeLabel->setText((TurnPlayerStatus == PLAYER_BLACK? tr("BLACK") : tr("WHITE")));
            }
            //如果棋盘刚好满了，并且还没人胜利
            else
            {
                emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！和局！")));
                TotalTimeLabel->setText(tr("WINNER:"));
                RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
                RoundTimeLabel->setText(tr("NONE"));
            }

            //只要进入这个if,肯定都是要结束游戏
            AfterPlayGame();

            //break;
        }
        else//否则
        {
            ExchangeTurnPlayerStatus();
            ResetRoundTimer();
        }
    }
    else if (!Status)
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]游戏结束！！")));
        TotalTimeLabel->setText(tr("WINNER:"));
        RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");
        //这里反转了一下
        RoundTimeLabel->setText((Player2Status == PLAYER_BLACK? tr("BLACK") : tr("WHITE")));
        if(PlayingModeStatus == MODE_PVP)
            emit InProcessMsg(QString(TOONLINE_ENUM) + QString(GAMEGIVEUP_ENUM));
        AfterPlayGame();
    }
}

//主动按下悔棋键
void Widget::OnUndoChess(void)
{
    if (TurnPlayerStatus != Player1Status)
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + tr("[系统提示]只能在轮到你时悔棋"));
        return;
    }
    else if (BePutChess->length() < 2)
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + tr("[系统提示]您没下过棋子，无法悔棋"));
        return;
    }

    //设置只能毁一次棋
    Action1->setDisabled(true);
    emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + tr("[系统提示]你使用了唯一的悔棋机会"));
    CoreUndoChess();
    if (PlayingModeStatus == MODE_PVP)
        emit InProcessMsg(QString(TOONLINE_ENUM) + QString(UNDOCHESS_ENUM));

    update();
}

//主动按下和棋键
void Widget::OnDrawChess(void)
{
    //如果是人机模式，机器是注定不会同意的
    if(PlayingModeStatus == MODE_PVE)
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + QString(tr("[系统提示]对方拒绝了您的和棋请求！")));
    }
    else if (PlayingModeStatus == MODE_PVP)
    {
        emit InProcessMsg(QString(TOLOCAL_ENUM) + QString(SYSMSG_ENUM) + tr("[系统提示]已发送和局申请"));
        emit InProcessMsg(QString(TOONLINE_ENUM) + QString(DRAWCHESS_ENUM) + "Try");
    }
}

