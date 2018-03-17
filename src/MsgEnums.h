#ifndef ENUMS_H
#define ENUMS_H

//这里是一些字符串的枚举值，代表发送的字符串的功能，反正用文字说明有点乱
//自己结合源代码领悟一下，尤其结合Widget::OnProcessMsg();
//一堆define

//这里的ENM_SIZE只的是下面的PLAYERMSG_ENM等等后面定义的字符串的长度
//至于PLAYERMSG_ENM后面定义的内容是什么，其实是随便定义的，只要不重复就可以了
//只是用来起标识作用

//Example: QString Msg = QString(TOLOCAL_ENUM) + QString(PLAYERMSG_ENUM) + QString("这是个例子");

#define ENUM_SIZE 3

//分级，第一级别，此消息是本机的还是联机玩家的

//ToLocal -- 本机的
#define TOLOCAL_ENUM "01#"
//ToOnline -- 发送至联机玩家的
#define TOONLINE_ENUM "02#"
//OnlineToLocal -- 从联机玩家而来，将作用在本地的
#define ONLINETOLOCAL_ENUM "03#"

/* ************************************** */

//第二级别
//PlayerMsg -- 玩家发送的信息
#define PLAYERMSG_ENUM "10#"
//SysMsg -- 程序要打印出来的信息，将以红色显示
#define SYSMSG_ENUM "11#"


//YourChessColor -- 你棋子的颜色，对应的颜色将会接在此枚举后面 ："黑方","白方"
#define YOURCHESSCOLOR_ENUM "12#"
//GamePass -- 跳过当前回合
#define GAMEPASS_ENUM "13#"
//ChessPosition -- 对方落下的棋子的位置,对应的位置将会接在此枚举后面 ： "0511"=>第5行第11列
#define CHESSPOSITION_ENUM "14#"
//GameGiveUp -- 认输
#define GAMEGIVEUP_ENUM "15#"
//UndoChess -- 悔棋
#define UNDOCHESS_ENUM "16#"
//DrawChess -- 和棋
#define DRAWCHESS_ENUM "17#"

#endif // ENUMS_H
