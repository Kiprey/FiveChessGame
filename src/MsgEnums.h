#ifndef ENUMS_H
#define ENUMS_H

/*
    这里是一些字符串的枚举值，代表发送的字符串的功能，反正用文字说明有点乱
    自己结合源代码领悟一下
    尤其结合Widget::OnProcessMsg();
        Widget::OnToNetworkMsg();
        Widget::OnFromNetworkMsg();

    这里的ENM_SIZE只的是下面的PLAYERMSG_ENM等等后面定义的字符串的长度
    至于PLAYERMSG_ENM后面定义的内容是什么，其实是随便定义的，只要不重复就可以了
    只是用来起标识作用

    Example: QString Msg = QString(PLAYERMSG_ENUM) + QString("这是个例子");
*/

//一堆define

#define ENUM_SIZE 2

//PlayerMsg -- 玩家发送的信息
#define PLAYERMSG_ENUM "0#"
//SysMsg -- 程序要打印出来的信息，将以红色显示
#define SYSMSG_ENUM "1#"


//YourChessColor -- 你棋子的颜色，对应的颜色将会接在此枚举后面 ："黑方","白方"
#define YOURCHESSCOLOR_ENUM "2#"
//GamePass -- 跳过当前回合
#define GAMEPASS_ENUM "3#"
//ChessPosition -- 对方落下的棋子的位置,对应的位置将会接在此枚举后面 ： "0511"=>第5行第11列
#define CHESSPOSITION_ENUM "4#"
//GameGiveUp -- 认输
#define GAMEGIVEUP_ENUM "5#"
//UndoChess -- 悔棋
#define UNDOCHESS_ENUM "6#"
//DrawChess -- 和棋
#define DRAWCHESS_ENUM "7#"

#endif // ENUMS_H
