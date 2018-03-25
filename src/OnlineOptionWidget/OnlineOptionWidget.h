#ifndef ONLINEOPTIONWIDGET_H
#define ONLINEOPTIONWIDGET_H

//这个子widget为在主界面点击联机对战时，选择创建对局和加入对局的一个窗口

#include <QWidget>
#include <QTextBrowser>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>

#include "MsgEnums.h"
#include "NetworkModule/NetworkModule.h"


class OnlineOptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OnlineOptionWidget(QWidget *parent = 0);
    ~OnlineOptionWidget();
private:
    QPushButton * ServerButton;
    QPushButton * ClientButton;

    QLabel * ServerIP;
    QLineEdit * ClientConnectIP;
    QTextBrowser * LogOutput;

    QVBoxLayout * ServerVBox;
    QVBoxLayout * ClientVBox;
    QVBoxLayout * GroupVBox;

    QHBoxLayout * WidgetHBox;

    QGroupBox * ServerGroup;
    QGroupBox * ClientGroup;
signals:
    void InAppendLog(QString log);

    //此3个信号会被NetworkModule里的槽函数所接收
    //服务器开始监听
    void InServerListenning(void);
    //服务器断开所有连接，取消监听
    void InCancelListenning(void);
    //客户端尝试连接
    void InClientConnect(QString);

private slots:
    //点击”创建“按钮后的槽函数
    void OnCreateOnlineGame(void);
    //点击”连接“按钮后的槽函数
    void OnConnectOnlineGame(void);
    //点击”取消创建“按钮后的槽函数
    void OnCancelCreateOnlineGame(void);
    //向输出窗口输出数据
    void OnAppendLog(QString log);
    //当游戏开始后，对此窗口的一些操作
    void OnHideWidget(void);
};

#endif // ONLINEOPTIONWIDGET_H
