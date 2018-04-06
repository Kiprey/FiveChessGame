#include "Widget.h"
#include "ui_widget.h"

/*
    注意，在本源代码里的注释中
    所说的活棋，死棋
    指的是两边都没被阻挡的棋组，和有一边被阻挡的棋组
*/

//机器下棋的算法
//返回值QPoint类型
//在这里返回的不是棋子的坐标，而是棋子位置的行列数
//例如(0, 0)=>第一行第一列
QPoint Widget::ComputerPutChess(void)
{

    //注意：设置这些比率的首要条件是2 * ChessUpLevelRate > DeadChessRate
    // 2 * DeadChessRate < 1
    //活棋和死棋直接的比例
    float DeadChessRate = 0.4;
    //主棋组设置的权值和副棋组设置的比例
    float SecondaryGroupChessRate = 0.5;
    //每种活棋之间的比例
    float ChessUpLevelRate = 0.25;
    //考虑AI优先的额外权值
    float AIFirstRate = 5;


    //存储棋盘位置权值的变量
    QList<QList<int>> PositionScore;
    //初始化棋盘上每个位置的权值
    for (int i = 0; i < ChessLines; i++)
    {
        QList<int> TmpPositionScore;
        for (int j = 0; j < ChessLines; j++)
            TmpPositionScore.append(0);
        PositionScore.append(TmpPositionScore);
    }

    //寻找可下的位置
    QStack<QPoint> AvailablePosition;
    for (int i = 0; i < ChessLines; i++)
    {
        for (int j = 0; j < ChessLines; j++)
        {
            //如果找到空位置
            if (ChessPosition[i][j] == PLAYER_NONE)
            {
                //如果有效空位添加完成，则遍历下一个空位置
                int ExitSign = false;
                //遍历它周围的空位置
                //搜索离自身范围2格内，有没有棋子
                for (int RowPerAdd = -2; RowPerAdd <= 2; RowPerAdd++)
                {
                    for (int ColumnPerAdd = -2; ColumnPerAdd <= 2; ColumnPerAdd++)
                    {
                        //自己打开画图软件画画看，推测一下为什么要排除ColumnPerAdd * RowPerAdd == +2 和 -2两种情况
                        if (!((RowPerAdd == 0 && ColumnPerAdd == 0) ||
                              (ColumnPerAdd * RowPerAdd == -2 || ColumnPerAdd * RowPerAdd == 2)))
                        {
                            int tmpI = i + RowPerAdd;
                            int tmpJ = j + ColumnPerAdd;
                            if (tmpI < 0 || tmpJ < 0 || tmpI >= ChessLines || tmpJ >= ChessLines)
                                continue;
                            //如果有棋子
                            if (ChessPosition[tmpI][tmpJ] != PLAYER_NONE)
                            {
                                //那么此空位就被视为有效空位
                                AvailablePosition.push(QPoint(i, j));
                                PositionScore[i][j] += 5;
                                ExitSign = true;
                                break;
                            }
                        }
                    }
                    if (ExitSign)
                        break;
                }
            }
        }
    }

    //如果可下棋子的位置是空的，也就是机器第一个下棋
    if (AvailablePosition.isEmpty())
        //给棋盘中央设置权值
        PositionScore[(int)(ChessLines / 2)][(int)(ChessLines / 2)] += 5;
    else
    {
        //如果可下棋子的位置不是空的
        //在设置权值后，将会删除有效空位在QList占的位置
        while (!AvailablePosition.isEmpty())
        {
            //将有效空位取出来
            QPoint Position = AvailablePosition.pop();
            int X = Position.x();
            int Y = Position.y();

            //遍历两个回合，分别遍历AI方和玩家方
            for(int RoundIndex = 0; RoundIndex < 2; RoundIndex++)
            {
                int MyStatus;
                if (RoundIndex == 0)
                    //首先为AI考虑
                    MyStatus = Player2Status;
                else
                    //设身处地的为玩家着想，因为敌人最想下的位置就是AI所要考虑的
                    MyStatus = Player1Status;

                //限制了一半的方向
                //因为要在一个位置上判断两个方向
                for (int RowPerAdd = -1; RowPerAdd <= 1; RowPerAdd++)
                {
                    for (int ColumnPerAdd = -1; ColumnPerAdd <= 0; ColumnPerAdd++)
                    {
                        //这两个for循环配套这个if的目的
                        //是为了只遍历八个方向中的4个方向
                        //剩下4个方向在for循环里的具体操作中会遍历到
                        if(ColumnPerAdd == 0 && (RowPerAdd == 0 || RowPerAdd == 1))
                            continue;

                        //主棋组的棋子数
                        int MajorGroupChessCount = 0;
                        //主棋组头的状态
                        bool MajorGroupStartIsLive = false;
                        //主棋组尾的状态
                        bool MajorGroupEndIsLive = false;

                        //初始化一系列的棋子权值
                        int LiveOneChessScore = 1000;
                        int DeadOneChessScore = LiveOneChessScore * DeadChessRate;

                        int LiveTwoChessScore = LiveOneChessScore / ChessUpLevelRate;
                        int DeadTwoChessScore = LiveTwoChessScore * DeadChessRate;

                        int LiveThreeChessScore = LiveTwoChessScore / ChessUpLevelRate;
                        int DeadThreeChessScore = LiveThreeChessScore * DeadChessRate;

                        //无论遇见的是活的还是死的四子，只要下了就准赢，所以不考虑四子的状态
                        int AllFourChessScore = LiveThreeChessScore / ChessUpLevelRate;

                        //在一个方向里反向遍历另外一个方向
                        //Direct == -1时的方向是Direct == 1的反方向
                        for (int Direct = -1; Direct <= 1; Direct++)
                        {
                            if (Direct != 0)
                            {
                                //副棋组的棋数
                                int SecondaryGroupChessCount = 0;
                                //副棋组的头的状态
                                bool SecondaryGroupStartIsLive = false;
                                //副棋组的尾的状态
                                bool SecondaryGroupEndIsLive = false;

                                //在一个方向中遍历旗子
                                for (int tmpX = X + Direct * RowPerAdd, tmpY = Y + Direct * ColumnPerAdd;
                                     tmpX >= 0 && tmpX < ChessLines && tmpY >= 0 && tmpY < ChessLines;
                                     tmpX += Direct * RowPerAdd, tmpY += Direct * ColumnPerAdd)
                                {
                                    if (ChessPosition[tmpX][tmpY] == PLAYER_NONE)
                                    {
                                        if (!SecondaryGroupStartIsLive)
                                            SecondaryGroupStartIsLive = true;
                                        else
                                        {
                                            SecondaryGroupEndIsLive = true;
                                            //终止遍历
                                            break;
                                        }
                                    }
                                    else if (ChessPosition[tmpX][tmpY] == MyStatus)
                                    {
                                        if(!SecondaryGroupStartIsLive)
                                            MajorGroupChessCount++;
                                        else
                                            SecondaryGroupChessCount++;
                                    }
                                    //如果遇到的是敌人的棋子
                                    else
                                        //那么就退出此方向的遍历
                                        //之所以不用设置SecondaryGroupStartIsLive等等等，是因为初始值就已经是false了
                                        break;
                                }
                                //设置分数
                                //如果当前方向的第一个位置是活的，就表明有副棋组的存在
                                if (SecondaryGroupStartIsLive && SecondaryGroupChessCount > 0)
                                {
                                    switch(SecondaryGroupChessCount)
                                    {
                                    case 1:
                                        if (SecondaryGroupEndIsLive)
                                            PositionScore[X][Y] += LiveOneChessScore * SecondaryGroupChessRate;
                                        else
                                            PositionScore[X][Y] += DeadOneChessScore * SecondaryGroupChessRate;
                                        break;
                                    case 2:
                                        if (SecondaryGroupEndIsLive)
                                            PositionScore[X][Y] += LiveTwoChessScore * SecondaryGroupChessRate;
                                        else
                                            PositionScore[X][Y] += DeadTwoChessScore * SecondaryGroupChessRate;
                                        break;
                                    case 3:
                                        if (SecondaryGroupEndIsLive)
                                            PositionScore[X][Y] += LiveThreeChessScore * SecondaryGroupChessRate;
                                        else
                                            PositionScore[X][Y] += DeadThreeChessScore * SecondaryGroupChessRate;
                                        break;
                                    //四子及以上的
                                    default:
                                        PositionScore[X][Y] += AllFourChessScore * SecondaryGroupChessRate;
                                        break;
                                    }

                                    //如果现在正在为AI着想，那肯定要加多一点权值，机不为己天诛地灭！
                                    if(MyStatus == Player2Status)
                                        PositionScore[X][Y] += AIFirstRate * SecondaryGroupChessCount * SecondaryGroupChessRate;

                                }
                                //映射副棋组的头的状态到主棋组的头或尾的状态
                                if(Direct == -1)
                                    MajorGroupStartIsLive = SecondaryGroupStartIsLive;
                                else
                                    MajorGroupEndIsLive = SecondaryGroupStartIsLive;
                            }
                        }
                        //当两个方向都遍历过后
                        //如果主棋组有棋子
                        if (MajorGroupChessCount > 0)
                        {
                            switch(MajorGroupChessCount)
                            {
                            case 1:
                                if (MajorGroupStartIsLive && MajorGroupEndIsLive)
                                    PositionScore[X][Y] += LiveOneChessScore;
                                else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                                    PositionScore[X][Y] += DeadOneChessScore;
                                break;
                            case 2:
                                if (MajorGroupStartIsLive && MajorGroupEndIsLive)
                                    PositionScore[X][Y] += LiveTwoChessScore;
                                else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                                    PositionScore[X][Y] += DeadTwoChessScore;
                                break;
                            case 3:
                                if (MajorGroupStartIsLive && MajorGroupEndIsLive)
                                    PositionScore[X][Y] += LiveThreeChessScore;
                                else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                                    PositionScore[X][Y] += DeadThreeChessScore;
                                break;
                            //四字及以上的
                            default:
                                //如果是机器已经连了四颗棋子
                                if (MyStatus == Player2Status)
                                    //设置更高优先级
                                    PositionScore[X][Y] += AllFourChessScore / ChessUpLevelRate;
                                else
                                    PositionScore[X][Y] += AllFourChessScore;
                                break;
                            }
                            //如果现在正在为AI着想，那肯定要加多一点权值，机不为己天诛地灭！
                            if(MyStatus == Player2Status)
                                PositionScore[X][Y] += AIFirstRate * MajorGroupChessCount;
                        }
                    }
                }
            }
        }
    }
    //查找哪个位置的权值最大
    QList<QPoint> MaxPosition;
    int MaxScore = 0;
    for(int i = 0; i < ChessLines; i++)
    {
        for (int j = 0; j < ChessLines; j++)
        {
            //这里的if中的判断条件，设置了一个为2的浮动区间
            //是避免因float转int丢失精度造成的权值减小1到2，导致判断出错
            if (MaxScore < PositionScore[i][j] - 2)
            {
                MaxScore = PositionScore[i][j];
                MaxPosition.clear();
                MaxPosition.append(QPoint(i, j));
            }
            else if (MaxScore > PositionScore[i][j] - 2 && MaxScore < PositionScore[i][j] + 2)
                MaxPosition.append(QPoint(i, j));
        }
    }

    //下棋
    return MaxPosition.at(qrand() % MaxPosition.count());
}

