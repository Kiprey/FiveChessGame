#include "Widget.h"
#include "ui_widget.h"

//画UI界面
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    /****************垂直层面****************/
    //在Heap中建立对象
    TextBrowser = new QTextBrowser();
    TextEdit = new MsgTextEdit();
    Button = new QPushButton();
    ButtonMenu = new QMenu();
    PVEMenu = new QMenu();
    Action1 = new QAction(nullptr);
    Action2 = new QAction(nullptr);
    Action3 = new QAction(nullptr);
    OpenPVEFile = new QAction(nullptr);
    TotalTimeLabel = new QLabel(tr("NONE"));
    TotalTimer = new QTimer();
    RoundTimer = new QTimer();
    RoundTimeLabel = new QLabel(tr("NONE"));
    VBox = new QVBoxLayout();
    BePutChess = new QList<QPoint>;
    networkModule = new NetworkModule();
    OnlineOption = new OnlineOptionWidget(this);

    //添加进垂直UI层，并设置各个空间的垂直比例
    VBox->addWidget(TextBrowser);
    VBox->setStretchFactor(TextBrowser, 4);
    VBox->addWidget(TextEdit);
    VBox->setStretchFactor(TextEdit, 2);
    VBox->addWidget(Button);
    VBox->setStretchFactor(Button, 2);
    VBox->addWidget(TotalTimeLabel);
    VBox->setStretchFactor(TotalTimeLabel, 2);
    VBox->addWidget(RoundTimeLabel);
    VBox->setStretchFactor(RoundTimeLabel, 2);

    /****************水平层面（总层面）****************/

    LabelCanvas = new QLabel();

    HBox = new QHBoxLayout();
    HBox->addLayout(VBox, 2.5);
    HBox->addWidget(LabelCanvas, 7.5);
    setLayout(HBox);
    /*********************初始化*********************/
    //详情请结合widget.h里的注释理解

    PlayingModeStatus = MODE_NONE;
    Player1Status = PLAYER_NONE;
    Player2Status = PLAYER_NONE;
    TurnPlayerStatus = TURNPLAYER_NONE;
    Geometry_X = 800;
    Geometry_Y = 600;
    ChessLines = 15;
    ChooseChessX = 0;
    ChooseChessY = 0;
    ChessCanvasSideLength = (Geometry_X * 3/4 < Geometry_Y? Geometry_X * 3/4 : Geometry_Y);
    ChessLineWidth = ChessCanvasSideLength / ChessLines;
    ChessWidth = ChessLineWidth * 2 / 3;
    ChessCanvasStartX = Geometry_X / 4 + ChessLineWidth / 2;
    ChessCanvasStartY = 0  + ChessLineWidth / 2;
    ChessCanvasEndX = ChessCanvasStartX + ChessCanvasSideLength - ChessLineWidth;
    ChessCanvasEndY = ChessCanvasStartY + ChessCanvasSideLength - ChessLineWidth;
    SmallBlackPointWidth = 6;

    //给存储棋盘数据的变量分配内存并初始化
    ChessPosition = new int * [ChessLines];
    for (int i = 0; i < ChessLines; i++)
    {
        ChessPosition[i] = new int[ChessLines];
        for(int j = 0; j < ChessLines; j++)
            ChessPosition[i][j] = PLAYER_NONE;
    }

 /*********************初始化结束*********************/
    //设置窗口大小
    resize(Geometry_X, Geometry_Y);

    //设置窗口名称
    setWindowTitle(tr("五子棋"));

    //设置禁止最大化按钮
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    //设置禁止拉伸窗口
    setFixedSize(width(), height());

    //设置鼠标位置实时追踪
    setMouseTracking(true);
    LabelCanvas->setMouseTracking(true);

    //设置Label的样式
        // 构造一个字体对象
    QFont Font;
    Font.setFamily("Arial");
    Font.setBold(true);
    Font.setPixelSize(20);
    TotalTimeLabel->setFont(Font);
    RoundTimeLabel->setFont(Font);
    TotalTimeLabel->setAlignment(Qt::AlignCenter);
    RoundTimeLabel->setAlignment(Qt::AlignCenter);
    TotalTimeLabel->setStyleSheet("QLabel{color:#746D65;background:#00FFFF}");
    RoundTimeLabel->setStyleSheet("QLabel{color:#C0C0C0;background:#0022FF}");

    Font.setPixelSize(15);
    TextEdit->setFont(Font);
    //设置MsgTextEdit和QTestBrower不接受富文本
    TextEdit->setAcceptRichText(false);
    TextBrowser->setAcceptRichText(false);

    //设置MsgTextEdit中灰色的占位符信息
    TextEdit->setPlaceholderText(tr("[Enter] 发送信息") + "\n" + tr("#clear 清空文字"));

    //设置TextBrowser里的字体
    Font.setPixelSize(15);
    TextBrowser->setFont(Font);

    //设置按钮以及按钮菜单
    Action1->setText(tr("新游戏"));
    connect(Action1, SIGNAL(triggered(bool)), this, SLOT(OnMode_PVE(void)));
    Action2->setText(tr("局域网联机对战"));
    connect(Action2, SIGNAL(triggered(bool)), this, SLOT(OnChooseOnlineOption(void)));
    Action3->setText(tr("关于"));
    connect(Action3, SIGNAL(triggered(bool)), this, SLOT(About(void)));
    OpenPVEFile->setText(tr("打开数据文件以继续对局"));
    connect(OpenPVEFile, SIGNAL(triggered(bool)), this, SLOT(OnOpenPVEDataFile(void)));

    PVEMenu->addAction(Action1);
    PVEMenu->addAction(OpenPVEFile);
    PVEMenu->setTitle(tr("人机对战"));

    ButtonMenu->addMenu(PVEMenu);
    ButtonMenu->addAction(Action2);
    ButtonMenu->addAction(Action3);
    Button->setMenu(ButtonMenu);
    Button->setText(tr("开始游戏"));

    //连接相关槽函数
    connect(TextEdit, SIGNAL(InToLocalMsg(QString)), this, SLOT(OnToLocalMsg(QString)));
    connect(this, SIGNAL(InToLocalMsg(QString)), this, SLOT(OnToLocalMsg(QString)));
    connect(this, SIGNAL(InToNetworkMsg(QString)), this, SLOT(OnToNetworkMsg(QString)));
    connect(TotalTimer, SIGNAL(timeout(void)), this, SLOT(OnDisplayAndChangeTotalTime(void)));
    connect(RoundTimer, SIGNAL(timeout(void)), this, SLOT(OnChangeRoundTime(void)));

    connect(OnlineOption, SIGNAL(InCancelListenning(void)), networkModule, SLOT(OnCancelListenning(void)));
    connect(OnlineOption, SIGNAL(InServerListenning(void)), networkModule, SLOT(OnServerListenning(void)));
    connect(OnlineOption, SIGNAL(InClientConnect(QString)), networkModule, SLOT(OnClientConnect(QString)));
    connect(networkModule, SIGNAL(InAppendLog(QString)), OnlineOption, SLOT(OnAppendLog(QString)));
    connect(networkModule, SIGNAL(InStartGame(void)), OnlineOption, SLOT(OnHideWidget(void)));
    connect(networkModule, SIGNAL(InStartGame(void)), this, SLOT(OnMode_PVP(void)));
    connect(networkModule, SIGNAL(InFromNetworkMsg(QString)), this, SLOT(OnFromNetworkMsg(QString)));
    connect(networkModule, SIGNAL(InDisconnected(void)), this, SLOT(OnDisconnected(void)));
}

Widget::~Widget()
{
    //一堆delete，没什么好看的
    delete BePutChess;
    delete networkModule;
    delete TextBrowser;
    delete TextEdit;
    delete Button;
    delete ButtonMenu;
    delete PVEMenu;
    delete TotalTimeLabel;
    delete TotalTimer;
    delete RoundTimeLabel;
    delete RoundTimer;
    delete VBox;
    delete HBox;
    delete LabelCanvas;
    delete Action1;
    delete Action2;
    delete Action3;
    delete OpenPVEFile;

    for (int i = 0; i < ChessLines; i++)
        delete ChessPosition[i];
    delete[] ChessPosition;

    delete ui;
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter * Painter = new QPainter(this);

    //反走样
    Painter->setRenderHint(QPainter::Antialiasing, true);

    /*********************开始画棋盘*********************/

    //设置画刷为棋盘的颜色，用于填充棋盘
    Painter->setBrush(QBrush(qRgb(249, 214, 91)));
    Painter->drawRect(ChessCanvasStartX - ChessLineWidth / 2, ChessCanvasStartY - ChessLineWidth / 2, ChessCanvasSideLength + 2 * ChessLineWidth / 2, ChessCanvasSideLength + 2 * ChessLineWidth / 2);

    //设置画笔为黑色
    Painter->setPen(QPen(Qt::black, 1));

    //画棋盘的行列
    for (int line = 0; line < ChessLines; line++)
    {
        Painter->drawLine(ChessCanvasStartX, ChessCanvasStartY + ChessLineWidth * line, ChessCanvasEndX, ChessCanvasStartY + ChessLineWidth * line);//行
        Painter->drawLine(ChessCanvasStartX + ChessLineWidth * line, ChessCanvasStartY, ChessCanvasStartX + ChessLineWidth * line, ChessCanvasEndY);//列
    }

    //设置画刷为黑色，用于画小黑点
    Painter->setBrush(QBrush(Qt::black));

    //画小黑点
    // 1)左上角第一个点
    SmallBlackPointX = ChessCanvasStartX + ChessLineWidth * ChessLines / 5;
    SmallBlackPointY = ChessCanvasStartY + ChessLineWidth * ChessLines / 5;
    Painter->drawRect(SmallBlackPointX - SmallBlackPointWidth / 2, SmallBlackPointY - SmallBlackPointWidth / 2, SmallBlackPointWidth, SmallBlackPointWidth);

    // 2)右上角第二个点
    SmallBlackPointX = ChessCanvasEndX - ChessLineWidth * ChessLines / 5;
    SmallBlackPointY = ChessCanvasStartY + ChessLineWidth * ChessLines / 5;
    Painter->drawRect(SmallBlackPointX - SmallBlackPointWidth / 2, SmallBlackPointY - SmallBlackPointWidth / 2, SmallBlackPointWidth, SmallBlackPointWidth);

    // 3)正中间第三个点
    SmallBlackPointX = (ChessCanvasStartX + ChessCanvasEndX) / 2;
    SmallBlackPointY = (ChessCanvasStartY + ChessCanvasEndY) / 2;
    Painter->drawRect(SmallBlackPointX - SmallBlackPointWidth / 2, SmallBlackPointY - SmallBlackPointWidth / 2, SmallBlackPointWidth, SmallBlackPointWidth);

    // 4)左下角第四个点
    SmallBlackPointX = ChessCanvasStartX + ChessLineWidth * ChessLines / 5;
    SmallBlackPointY = ChessCanvasEndY - ChessLineWidth * ChessLines / 5;
    Painter->drawRect(SmallBlackPointX - SmallBlackPointWidth / 2, SmallBlackPointY - SmallBlackPointWidth / 2, SmallBlackPointWidth, SmallBlackPointWidth);

    // 2)右下角第五个点
    SmallBlackPointX = ChessCanvasEndX - ChessLineWidth * ChessLines / 5;
    SmallBlackPointY = ChessCanvasEndY - ChessLineWidth * ChessLines / 5;
    Painter->drawRect(SmallBlackPointX - SmallBlackPointWidth / 2, SmallBlackPointY - SmallBlackPointWidth / 2, SmallBlackPointWidth, SmallBlackPointWidth);

    /***********结束画棋盘***********/

    //声明颜色
    QColor PlayerColor;

    //画已经落下的棋子
    for (int i = 0; i < ChessLines; i++)
    {
        for (int j = 0; j < ChessLines; j++)
        {
            if (ChessPosition[i][j] != PLAYER_NONE)
            {
                int PaintTmpY = j * ChessLineWidth + ChessCanvasStartY;
                int PaintTmpX = i * ChessLineWidth + ChessCanvasStartX;

                QRadialGradient Gradient(PaintTmpX - ChessWidth / 5, PaintTmpY - ChessWidth / 5, ChessWidth * 3/5, PaintTmpX - ChessWidth / 5, PaintTmpY - ChessWidth / 5);

                //设置画立体棋子的必要颜色
                if (ChessPosition[i][j] == PLAYER_BLACK)
                {
                    Gradient.setColorAt(0, QColor::fromRgb(200, 200, 200));
                    Gradient.setColorAt(0.8, QColor::fromRgb(32, 32, 32));
                    Gradient.setColorAt(1, QColor::fromRgb(0, 0, 0));
                }
                else
                {
                    Gradient.setColorAt(0, QColor::fromRgb(255, 255, 255));
                    Gradient.setColorAt(0.8, QColor::fromRgb(210, 210, 210));
                    Gradient.setColorAt(1, QColor::fromRgb(180, 180, 180));
                }

                QBrush TmpBrush(Gradient);
                Painter->setPen(Qt::NoPen);
                Painter->setBrush(TmpBrush);
                Painter->drawEllipse(QPointF(PaintTmpX, PaintTmpY), ChessWidth / 2, ChessWidth / 2);
            }
        }
    }
    //突出整盘游戏中的最后一颗棋子
    if (!BePutChess->isEmpty())
    {
        PlayerColor = Qt::red;
        Painter->setPen(QPen(QBrush(PlayerColor), 2, Qt::DashLine));
        Painter->setBrush(Qt::NoBrush);

        //这个4/3无特殊含义， 就只是感觉好看而已
        int LastChooseChessWidth = ChessWidth * 4/3;
        int LastChooseChessX = BePutChess->last().x();
        int LastChooseChessY = BePutChess->last().y();
        Painter->drawRect(LastChooseChessX - LastChooseChessWidth / 2, LastChooseChessY - LastChooseChessWidth / 2, LastChooseChessWidth, LastChooseChessWidth);
    }
    //如果选中的落子点没有被设置为空，则画选中的棋子
    if (TurnPlayerStatus == Player1Status && ChooseChessX != 0 && ChooseChessY != 0)
    {
        PlayerColor = (Player1Status == PLAYER_BLACK? Qt::black : Qt::white);
        Painter->setPen(QPen(QBrush(PlayerColor), 2, Qt::DashDotLine));
        Painter->setBrush(Qt::NoBrush);
        Painter->drawEllipse(QPointF(ChooseChessX, ChooseChessY), ChessWidth / 2, ChessWidth / 2);
    }
    delete Painter;
}

//玩家1选择下棋点
void Widget::mouseMoveEvent(QMouseEvent * mouse)
{
    //如果没轮到自己，这个函数是没用的
    if (TurnPlayerStatus != Player1Status)
        return;

    //得到鼠标当前位置
    int x = mouse->x();
    int y = mouse->y();

    //如果超出界限，则取消接下来的一系列操作,并消去选中的棋子
    if (x > ChessCanvasEndX || x < ChessCanvasStartX || y > ChessCanvasEndY || y < ChessCanvasStartY)
    {
        ChooseChessX = 0;
        ChooseChessY = 0;
        update();
        return;
    }
    //得到当前选择的点的周围四个点的XY坐标
    //在一除一乘中，除是为了取整，乘是为了得到坐标
    int LeftUp_X = (int)((x - ChessCanvasStartX) / ChessLineWidth) * ChessLineWidth + ChessCanvasStartX;
    int LeftUp_Y = (int)((y - ChessCanvasStartY) / ChessLineWidth) * ChessLineWidth + ChessCanvasStartY;

    int RightDown_X = LeftUp_X + ChessLineWidth;
    int RightDown_Y = LeftUp_Y + ChessLineWidth;

    //开始判断鼠标指向的位置离哪个点近
    if (x - LeftUp_X > RightDown_X - x)
        ChooseChessX = RightDown_X;
    else
        ChooseChessX = LeftUp_X;

    if (y - LeftUp_Y > RightDown_Y - y)
        ChooseChessY = RightDown_Y;
    else
        ChooseChessY = LeftUp_Y;

    //最后选中的落子点的行列
    int TempRow = (ChooseChessX - ChessCanvasStartX) / ChessLineWidth;
    int TempColumn = (ChooseChessY - ChessCanvasStartY) / ChessLineWidth;

    //如果选中的地方已经落子
    if (ChessPosition[TempRow][TempColumn] != PLAYER_NONE)
    {
        //将选中的落子点设置为无（0）
        ChooseChessX = 0;
        ChooseChessY = 0;
    }
    update();
}

//玩家1下棋
void Widget::mouseReleaseEvent(QMouseEvent *)
{
    if (TurnPlayerStatus != Player1Status || ChooseChessX == 0 || ChooseChessY == 0)
        return;
    else
    {
        //其余绘图在paintEvent()中操作

        //设置棋子的位置在ChessPosition上
        //TmpRow 行数， TmpColumn 列数
        int TmpRow = (ChooseChessX - ChessCanvasStartX) / ChessLineWidth;
        int TmpColumn = (ChooseChessY - ChessCanvasStartY) / ChessLineWidth;

        //在此空落子点上更改落子状态
        ChessPosition[TmpRow][TmpColumn] = Player1Status;

        //之所以不用再判断落子点是否是空，是因为已经在mouseMoveEvent()确定好了

        //添加进下过的棋子
        BePutChess->append(QPoint(ChooseChessX, ChooseChessY));

        //重置选中的棋子
        ChooseChessX = 0;
        ChooseChessY = 0;

        //给联机玩家发送信息
        if (PlayingModeStatus == MODE_PVP)
        {
            //发送下的棋子的位置
            //将int转化为QString
            QString Row = QString::number(TmpRow, 10);
            if (Row.length() < 2)
                Row = '0' + Row; //使数字变成两位数，如：02 12

            QString Column = QString::number(TmpColumn, 10);
            if (Column.length() < 2)
                Column = '0' + Column; //使数字变成两位数，如：02 12
            emit InToNetworkMsg(QString(CHESSPOSITION_ENUM) + Row + Column);
        }
        update();
        OnCheckWin(true);
    }
}

//关闭时弹出的窗口
void Widget::closeEvent(QCloseEvent * CloseEvent)
{
    if (PlayingModeStatus == MODE_PVP)
    {
        QMessageBox MsgBox(this);
        MsgBox.setWindowTitle(tr("确定离开？"));
        MsgBox.setText(tr("您当前正在进行联机对战，离开将视为认输\n是否离开？"));
        MsgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        MsgBox.setButtonText(QMessageBox::Yes, tr("是"));
        MsgBox.setButtonText(QMessageBox::No, tr("否"));
        MsgBox.setDefaultButton(QMessageBox::No);
        MsgBox.setIcon(QMessageBox::Question);
        switch(MsgBox.exec())
        {
        case QMessageBox::Yes:
            //认输
            OnCheckWin(false);
            CloseEvent->accept();
            break;
        case QMessageBox::No:
            CloseEvent->ignore();
            break;
        }
    }
    else if (PlayingModeStatus == MODE_PVE)
    {
        QMessageBox MsgBox(this);
        MsgBox.setWindowTitle(tr("确定离开？"));
        MsgBox.setText(tr("您当前进行的人机对战尚未分出胜负\n是否保存当前对局再离开？或者不保存就离开？"));
        MsgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Ignore);
        MsgBox.setButtonText(QMessageBox::Save, tr("保存对局后再离开"));
        MsgBox.setButtonText(QMessageBox::Discard, tr("不保存直接离开"));
        MsgBox.setButtonText(QMessageBox::Ignore, tr("返回游戏"));
        MsgBox.setDefaultButton(QMessageBox::Ignore);
        MsgBox.setIcon(QMessageBox::Question);
        switch(MsgBox.exec())
        {
        case QMessageBox::Save:
            if (OnSavePVEDataFile())
                CloseEvent->accept();
            else
                CloseEvent->ignore();
            break;
        case QMessageBox::Discard:
            CloseEvent->accept();
            break;
        case QMessageBox::Ignore:
            CloseEvent->ignore();
            break;
        }
    }
}
