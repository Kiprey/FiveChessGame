#include "MsgTextEdit.h"

void MsgTextEdit::keyPressEvent(QKeyEvent *e)
{
    //如果按下的是Enter键
    if (e->key() == Qt::Key_Return)
    {
        //默认发送TextEdit里的文字到TextBrowser里
        //得到TextEdit里的所有文字
        QString Text = toPlainText();

        //如果是空行,则取消发送
        if (Text.isEmpty())
            Text = QString(SYSMSG_ENUM) + tr("[系统提示]不允许发送空字符串!");
        else
            Text = QString(PLAYERMSG_ENUM) + Text;

        //清空TextEdit里的文字
        clear();

        emit InToLocalMsg(Text);

    }
    else
        //如果按下的是Enter键的话，不添加enter键至QTextEdit
        QTextEdit::keyPressEvent(e);
}

