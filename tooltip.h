#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class ToolTip;
}

class ToolTip : public QWidget
{
    Q_OBJECT

public:
    explicit ToolTip(QWidget *parent = nullptr);
    ~ToolTip();

    void ready(const QWidget *widget, const QEvent *event, QString text);
    void cancel();

private:
    Ui::ToolTip *ui;

    const QWidget *oldWidget;
    QTimer *timer;
    int wait;
    QPoint toolTipOffset;

signals:
    void timerStart(int msec);
    void timerStop();

private slots:
    void timerTimeOut();
};

#endif // TOOLTIP_H
