#ifndef WIDGET_H
#define WIDGET_H

//此为游戏的核心，所有关于游戏的操作几乎全在类Widget里

#include <QWidget>
#include <QDebug>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QMessageBox>
#include <QList>
#include <QStack>
#include <QPoint>

#include <ctime>    //初始化随机数用
#include "MsgEnums.h"
#include "MsgTextEdit/MsgTextEdit.h"
#include "OnlineOptionWidget/OnlineOptionWidget.h"
#include "NetworkModule/NetworkModule.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    //处理从程序内部来的消息
    void OnToLocalMsg(QString);
    //发送消息到联机玩家
    void OnToNetworkMsg(QString);
    //处理从联机玩家来的消息
    void OnFromNetworkMsg(QString);
    //回合时间的更改
    void OnChangeRoundTime(void);
    //总时间的显示与更改
    void OnDisplayAndChangeTotalTime(void);
    //双人联机对战
    void OnMode_PVP(void);
    //人机对战
    void OnMode_PVE(void);
    //检查是否胜利，参数详情在Widget.cpp里
    void OnCheckWin(bool);
    //悔棋（点击后的操作）
    void OnUndoChess(void);
    //和棋（点击后的操作）
    void OnDrawChess(void);
    //弹出子窗口，进行网络连接，并在连接成功后开始游戏
    void OnChooseOnlineOption(void);
    //在联机对战中，突然断开连接
    void OnDisconnected(void);
    //关于程序
    void About(void);

signals:
    void InToLocalMsg(QString);
    void InToNetworkMsg(QString);

private:
    /***********棋盘上的函数与棋子***********/
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    //机器下棋
    QPoint ComputerPutChess(void);
    //在选择任何模式后，都要执行的语句，开始游戏
    void BeforePlayGame(void);
    //重置回合时间
    void ResetRoundTimer(void);
    //更改轮换方
    void ExchangeTurnPlayerStatus(void);
    //悔棋的核心操作
    void CoreUndoChess(void);
    //游戏结束后的操作
    void AfterPlayGame(void);
    //玩家2下棋
    void Player2PutChess(QString);
    //联机对战中，得到和棋信息并开始相关步骤
    void GetDrawChessMsg(QString);
    //显示回合时间
    void DisplayRoundTime(void);

    //网络模块，详情见NetworkModule.h
    NetworkModule * networkModule;
    //子窗口，详情见OnlineOptionWidget.h
    OnlineOptionWidget * OnlineOption;

    int TotalTimeCount;                //总时间计数
    QTimer * TotalTimer;             //总时间计时器
    int RoundTimeCount;              //回合时间计数
    QTimer * RoundTimer;             //回合时间的计时器

    //当前玩耍的模式以及其枚举
    enum PlayedModeState {MODE_NONE, MODE_PVE, MODE_PVP};
    //PVE人机， PVP联机
    PlayedModeState PlayingModeStatus;

    //玩家状态
        //之所以又搞了TURNPLAY_NONE，是因为，当TurnPlayerStatus = PLAYER_NONE
        //Player1Status = PLAYER_NONE， 则 TurnPlayerStatus == Player1Status，
        //就会引发错误
    enum PlayerState {TURNPLAYER_NONE, PLAYER_NONE, PLAYER_WHITE, PLAYER_BLACK, PLAYER_WATCHING};

    //玩家这盘游戏的棋方（黑方白方）
    PlayerState Player1Status;
    PlayerState Player2Status;
    //轮到的棋方
    PlayerState TurnPlayerStatus;

    //棋盘上落子点的状态(棋盘上下棋的地方下的是黑，白，还是空）
    QList<QList<PlayerState>> ChessPosition;

    //选中的棋子的XY
    int ChooseChessX;
    int ChooseChessY;

    //下过的棋子的XY坐标都会在这里
    QList<QPoint> * BePutChess;
    /***************************以下貌似没有那么重要*************************/

    Ui::Widget *ui;

    /****************垂直层面****************/
    QTextBrowser * TextBrowser;  //消息输出框
    MsgTextEdit * TextEdit;       //消息输入框

    QPushButton * Button ;     //总按钮
    QMenu * ButtonMenu;        //按钮菜单
    QAction *Action1, *Action2, *Action3; //按钮菜单中的动作

    QLabel * TotalTimeLabel;         //输出时间的标签
    QLabel * RoundTimeLabel;         //输出回合时间的标签

    QVBoxLayout * VBox;         //垂直层面  上面的控件都要拖进去
    /****************水平层面（总层面）****************/
    QHBoxLayout * HBox;         //水平层面

    //QRgb BackColor;             //棋盘颜色
    QLabel * LabelCanvas;       //棋盘将会画在这个画布上面

     /*************以下为Widget::paintEvent()专用***************/
    //默认窗口大小
    int Geometry_X;
    int Geometry_Y;

    //棋盘行数，默认值15
    int ChessLines;

    //棋盘画布边长    //注意！！！棋盘画布与棋盘不一样， 棋盘画布比棋盘多了上下两条空边
    int ChessCanvasSideLength;

    //棋盘边距
    int ChessLineWidth;

    //棋子宽度
    int ChessWidth;

    //棋盘的起始与终止XY坐标
        //起始XY坐标都会加上半个来显示棋子
    int ChessCanvasStartX;
    int ChessCanvasStartY;
        //终止XY坐标在计算时会加上棋盘画布边长，再减去一个空行的距离，原因是因为棋盘画布边长比棋盘边长多了一个空行的距离
    int ChessCanvasEndX;
    int ChessCanvasEndY;

    //正规棋盘上的小黑点
    //小黑点的宽度,6最佳
    int SmallBlackPointWidth;
    int SmallBlackPointX;
    int SmallBlackPointY;

};

#endif // WIDGET_H
