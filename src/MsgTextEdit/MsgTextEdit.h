#ifndef MSGTEXTEDIT_H
#define MSGTEXTEDIT_H

//此为重写过后的QTextEdit，重写了keyPressEvent()

#include <QWidget>
#include <QString>
#include <QTextEdit>
#include <QKeyEvent>

#include "MsgEnums.h"

class MsgTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MsgTextEdit(QWidget *parent = Q_NULLPTR)
        : QTextEdit(parent){}
    ~MsgTextEdit(){}
private:
    void keyPressEvent(QKeyEvent *e);

signals:
    void InProcessMsg(QString);

};

#endif // MSGTEXTEDIT_H
