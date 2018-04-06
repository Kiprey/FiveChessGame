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
    //设置机器算法的遍历程度
    //程度越大机器越智能，但同时所消耗的时间也会成指数性增长
    //深度
    int Depth = 6;
    //广度
    int Range = 4;

    //存放分数最高的棋子位置的容器
    QList<QPoint> MaxScorePosition;
    //最高的分数
    int MaxScore = INT_MIN;
    //可下的位置
    QList<QPoint> * AvailablePosition = FindAvailablePosition(ChessPosition);
    //如果没得下了，也就是机器第一个下
    if(AvailablePosition->isEmpty())
        //让机器下在棋盘中央
        AvailablePosition->append(QPoint((int)ChessLines / 2, (int)ChessLines / 2));

    //复制出来的临时棋盘数据(防止在递归中被修改)
    int ** chessPosition = new int* [ChessLines];
    for (int i = 0; i < ChessLines; i++)
        chessPosition[i] = new int [ChessLines];
    //给临时棋盘位置做初始化
    for (int i = 0; i < ChessLines; i++)
        for (int j = 0; j < ChessLines; j++)
            chessPosition[i][j] = ChessPosition[i][j];

    //设置遍历广度
    int RecurrenceRange = 0;
    if (Range > AvailablePosition->count() || Range <= 0)
        RecurrenceRange = AvailablePosition->count();
    else
        RecurrenceRange = Range;

    for(int i = 0; i < RecurrenceRange; i++)
    {
        int TmpScore = Recurrence(Depth, RecurrenceRange,
                                   INT_MIN / 2, INT_MAX / 2, Player2Status,
                                  chessPosition, AvailablePosition->at(i)).y();
        //将分数最大的放入一个容器里
        if (TmpScore > MaxScore)
        {
            MaxScore = TmpScore;
            MaxScorePosition.clear();
            MaxScorePosition.append(AvailablePosition->at(i));
        }
        else if (TmpScore == MaxScore)
            MaxScorePosition.append(AvailablePosition->at(i));
    }
    //尽管在MaxPosition里分数一样大，但也要从中选择一个模拟下棋后分数最大的
    MaxScore = INT_MIN;
    QList<QPoint> MaxEvaluateScorePosition;
    for(int i = 0; i < MaxScorePosition.count(); i++)
    {
        //模拟下棋
        chessPosition[MaxScorePosition[i].x()][MaxScorePosition[i].y()] = Player2Status;
        //得到评估值
        int TmpScore = EvaluateAllPosition(chessPosition);
        //撤销下棋
        chessPosition[MaxScorePosition[i].x()][MaxScorePosition[i].y()] = PLAYER_NONE;

        if (TmpScore > MaxScore)
        {
            MaxScore = TmpScore;
            MaxEvaluateScorePosition.clear();
            MaxEvaluateScorePosition.append(MaxScorePosition[i]);
        }
        else if (TmpScore == MaxScore)
            MaxEvaluateScorePosition.append(MaxScorePosition[i]);
    }

    delete AvailablePosition;
    for (int i = 0; i < ChessLines; i++)
        delete chessPosition[i];
    return MaxEvaluateScorePosition.at(qrand() % MaxEvaluateScorePosition.count());
}

//对某个点的评分函数
//参数中chessPosition为备份的棋盘数据， Point为将要评估的点
int Widget::EvaluatePositionScore(int** chessPosition, QPoint Point)
{
    int TmpScore = 0;

    //待评分的点的行列坐标
    int X = Point.x();
    int Y = Point.y();

    //！！！评分比例！！！
    //注意：设置这些比率的首要条件是2 * ChessUpLevelRate > DeadChessRate
    // 2 * DeadChessRate < 1
    //活棋和死棋直接的比例
    float DeadChessRate = 0.4;
    //主棋组设置的权值和副棋组设置的比例
    float SecondaryGroupChessRate = 0.5;
    //每种活棋之间的比例
    float ChessUpLevelRate = 0.25;
    //机器优先比例
    int AIFirstRate = 5;

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
                            if (chessPosition[tmpX][tmpY] == PLAYER_NONE)
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
                            else if (chessPosition[tmpX][tmpY] == MyStatus)
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
                                    TmpScore += LiveOneChessScore * SecondaryGroupChessRate;
                                else
                                    TmpScore += DeadOneChessScore * SecondaryGroupChessRate;
                                break;
                            case 2:
                                if (SecondaryGroupEndIsLive)
                                    TmpScore += LiveTwoChessScore * SecondaryGroupChessRate;
                                else
                                    TmpScore += DeadTwoChessScore * SecondaryGroupChessRate;
                                break;
                            case 3:
                                if (SecondaryGroupEndIsLive)
                                    TmpScore += LiveThreeChessScore * SecondaryGroupChessRate;
                                else
                                    TmpScore += DeadThreeChessScore * SecondaryGroupChessRate;
                                break;
                            //四子及以上的
                            default:
                                TmpScore += AllFourChessScore * SecondaryGroupChessRate;
                                break;
                            }
                            //如果现在正在为AI着想，那肯定要加多一点权值，机不为己天诛地灭！
                            if(MyStatus == Player2Status)
                                TmpScore += AIFirstRate * SecondaryGroupChessCount * SecondaryGroupChessRate;

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
                            TmpScore += LiveOneChessScore;
                        else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                            TmpScore += DeadOneChessScore;
                        break;
                    case 2:
                        if (MajorGroupStartIsLive && MajorGroupEndIsLive)
                            TmpScore += LiveTwoChessScore;
                        else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                            TmpScore += DeadTwoChessScore;
                        break;
                    case 3:
                        if (MajorGroupStartIsLive && MajorGroupEndIsLive)
                            TmpScore += LiveThreeChessScore;
                        else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                            TmpScore += DeadThreeChessScore;
                        break;
                        //四字及以上的
                    default:
                        //如果是机器已经连了四颗棋子
                        if (MyStatus == Player2Status)
                            //设置更高优先级
                            TmpScore += AllFourChessScore / ChessUpLevelRate;
                        else
                            TmpScore += AllFourChessScore;
                        break;
                    }
                    //如果现在正在为AI着想，那肯定要加多一点权值，机不为己天诛地灭！
                    if(MyStatus == Player2Status)
                        TmpScore += AIFirstRate * MajorGroupChessCount;
                }
            }
        }
    }
    return TmpScore;
}

//对整个棋盘的评分函数
int Widget::EvaluateAllPosition(int **chessPosition)
{
    int AIScore = 0;
    int PlayerScore = 0;

    int **TmpChessPosition = new int*[ChessLines];
    for (int i = 0; i < ChessLines; i++)
        TmpChessPosition[i] = new int[ChessLines];

    //遍历两个回合，分别遍历AI方和玩家方
    for(int RoundIndex = 0; RoundIndex < 2; RoundIndex++)
    {
        int MyStatus;
        int TmpScore = 0;
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
                for(int i = 0; i < ChessLines; i++)
                    for(int j = 0; j < ChessLines; j++)
                        TmpChessPosition[i][j] = chessPosition[i][j];

                //遍历这个方向的棋子
                for (int i = 0; i < ChessLines; i++)
                {
                    for (int j = 0; j < ChessLines; j++)
                    {
                        if (TmpChessPosition[i][j] == MyStatus)
                        {
                            //这两个for循环配套这个if的目的
                            //是为了只遍历八个方向中的4个方向
                            //剩下4个方向在for循环里的具体操作中会遍历到
                            if(ColumnPerAdd == 0 && (RowPerAdd == 0 || RowPerAdd == 1))
                                continue;

                            int X = i;
                            int Y = j;

                            //主棋组的棋子数
                            int MajorGroupChessCount = 1;
                            //主棋组头的状态
                            bool MajorGroupStartIsLive = false;
                            //主棋组尾的状态
                            bool MajorGroupEndIsLive = false;

                            //初始化一系列的棋子权值
                            int LiveOneChessScore = 1000;
                            int DeadOneChessScore = 100;

                            int LiveTwoChessScore = 10000;
                            int DeadTwoChessScore = 1000;

                            int LiveThreeChessScore = 100000;
                            int DeadThreeChessScore = 10000;

                            int LiveFourChessScore = 1000000;
                            int DeadFourChessScore = 100000;

                            int MoreThanFourScore = 10000000;

                            //在一个方向里反向遍历另外一个方向
                            //Direct == -1时的方向是Direct == 1的反方向
                            for (int Direct = -1; Direct <= 1; Direct++)
                            {
                                if (Direct != 0)
                                {
                                    //某个方向遍历到的头
                                    bool OneDirectIsLive = false;

                                    //在一个方向中遍历旗子
                                    for (int tmpX = X + Direct * RowPerAdd, tmpY = Y + Direct * ColumnPerAdd;
                                         tmpX >= 0 && tmpX < ChessLines && tmpY >= 0 && tmpY < ChessLines;
                                         tmpX += Direct * RowPerAdd, tmpY += Direct * ColumnPerAdd)
                                    {
                                        if (TmpChessPosition[tmpX][tmpY] == PLAYER_NONE)
                                        {
                                            OneDirectIsLive = true;
                                            break;
                                        }
                                        else if (TmpChessPosition[tmpX][tmpY] == MyStatus)
                                        {
                                            MajorGroupChessCount++;
                                            TmpChessPosition[tmpX][tmpY] = PLAYER_NONE;

                                        }
                                        //如果遇到的是敌人的棋子
                                        else
                                            //那么就退出此方向的遍历
                                            //之所以不用设置SecondaryGroupStartIsLive等等等，是因为初始值就已经是false了
                                            break;
                                    }
                                    if (Direct == -1)
                                        MajorGroupStartIsLive = OneDirectIsLive;
                                    else
                                        MajorGroupEndIsLive = OneDirectIsLive;

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
                                        TmpScore += LiveOneChessScore;
                                    else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                                        TmpScore += DeadOneChessScore;
                                    break;
                                case 2:
                                    if (MajorGroupStartIsLive && MajorGroupEndIsLive)
                                        TmpScore += LiveTwoChessScore;
                                    else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                                        TmpScore += DeadTwoChessScore;
                                    break;
                                case 3:
                                    if (MajorGroupStartIsLive && MajorGroupEndIsLive)
                                        TmpScore += LiveThreeChessScore;
                                    else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                                        TmpScore += DeadThreeChessScore;
                                    break;
                                case 4:
                                    if (MajorGroupStartIsLive && MajorGroupEndIsLive)
                                        TmpScore += LiveFourChessScore;
                                    else if (MajorGroupStartIsLive || MajorGroupEndIsLive)
                                        TmpScore += DeadFourChessScore;
                                    break;
                                    //四子以上的
                                default:
                                    TmpScore += MoreThanFourScore;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (AIScore == 0)
            AIScore = TmpScore;
        else
            PlayerScore = TmpScore;
    }

    for (int i = 0; i < ChessLines; i++)
        delete TmpChessPosition[i];
    delete[] TmpChessPosition;

    return AIScore - PlayerScore;
}

//找寻可下棋的位置，并使用评分函数对其排列
//分数大的排在前面，分数小的排在后面
//注意返回值是一个指针，这就意味着没有释放内存！！！
QList<QPoint> * Widget::FindAvailablePosition(int** chessPosition)
{
    //寻找可下的位置
    QList<QPoint> * AvailablePosition = new QList<QPoint>;
    for (int i = 0; i < ChessLines; i++)
    {
        for (int j = 0; j < ChessLines; j++)
        {
            //如果找到空位置
            if (chessPosition[i][j] == PLAYER_NONE)
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
                            if (chessPosition[tmpI][tmpJ] != PLAYER_NONE)
                            {
                                //那么此空位就被视为有效空位
                                if (AvailablePosition->isEmpty())
                                    AvailablePosition->append(QPoint(i, j));
                                else
                                {
                                    for (int k = 0; k < AvailablePosition->count(); k++)
                                    {
                                        int Score = EvaluatePositionScore(chessPosition, QPoint(i, j));
                                        int IndexScore = EvaluatePositionScore(chessPosition, AvailablePosition->at(k));
                                        if (IndexScore <= Score)
                                        {
                                            AvailablePosition->insert(k, QPoint(i, j));
                                            break;
                                        }
                                    }
                                }
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
    return AvailablePosition;
}

//向下递归调用的函数
//Depth递归调用的深度， Range递归的广度， TmpTurnPlayerStatus为模拟的下棋方
//Alpha 考虑最小值中的最大值
//Beta 考虑最大值中的最小值
//Point 为这个回合会模拟下棋的位置
//返回的为QPoint(Alpha, Beta)
//Alpha-Beta算法原理来自 https://blog.csdn.net/tangchenyi/article/details/22925957
//                     https://www.jianshu.com/p/8376efe0782d
QPoint Widget::Recurrence(int Depth, int Range, int Alpha, int Beta,
                          int TmpTurnPlayerStatus, int** chessPosition, QPoint Point)
{
    //存储返回的AlphaBeta值
    QPoint AlphaBeta;
    //模拟下棋
    chessPosition[Point.x()][Point.y()] = TmpTurnPlayerStatus;

    //检查模拟下棋后是否连成五子
    if (CoreCheckWin(Point, chessPosition))
    {
        if (TmpTurnPlayerStatus == Player2Status)
            Beta = INT_MAX / 2 + Depth;
        else
            Alpha = INT_MIN / 2 - Depth;
    }
    else
    {
        //如果超出限制
        if (Depth <= 1)
        {
            int Score = 0;
            Score = EvaluateAllPosition(chessPosition);
            if (TmpTurnPlayerStatus == Player2Status && Score < Beta)
                Beta = Score;
            else if (TmpTurnPlayerStatus == Player1Status && Score > Alpha)
                Alpha = Score;
        }
        //如果没有超出限制
        else
        {
            //找寻模拟下棋后的有效空位
            QList<QPoint> * AvailablePosition = FindAvailablePosition(chessPosition);

            //设置遍历广度
            int RecurrenceRange = 0;
            if (Range > AvailablePosition->count() || Range <= 0)
                RecurrenceRange = AvailablePosition->count();
            else
                RecurrenceRange = Range;

            //遍历空位置
            for (int i = 0; i < RecurrenceRange; i++)
            {
                //模拟下棋并继续递归
                if(TmpTurnPlayerStatus == Player1Status)
                {
                    AlphaBeta = Recurrence(Depth - 1, Range, Alpha, Beta,
                                           Player2Status, chessPosition,
                                           AvailablePosition->at(i));
                    if (Alpha < AlphaBeta.y())
                        Alpha = AlphaBeta.y();
                }
                else
                {
                    AlphaBeta = Recurrence(Depth - 1, Range, Alpha, Beta,
                                           Player1Status, chessPosition,
                                           AvailablePosition->at(i));
                    if (Beta > AlphaBeta.x())
                        Beta = AlphaBeta.x();
                }

                if(Alpha > Beta)
                    //停止遍历，因为继续遍历也没有用处了
                    //剪枝
                    break;
            }
            delete AvailablePosition;
        }
    }
    //撤销模拟下的棋
    chessPosition[Point.x()][Point.y()] = PLAYER_NONE;
    return QPoint(Alpha, Beta);
}
