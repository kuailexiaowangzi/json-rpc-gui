#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "mainwindow.h"
#include "otherborder.h"
#include "tooltip.h"
#include "position.h"

#include <QPoint>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    QWidget *window;
    MainWindow *mainWindow;
    OtherBorder *otherBorder;
    ToolTip *toolTip;
    QMargins windowMargin;
    QRect windowNormalRect;
    enum Position::positionType windowPosition;
    bool isTitleBarMousePress;
    QPoint widgetMousePressPos;
    int windowSide;
    int screenLine;
    enum Position::positionType windowPress;

    void sizeInit();
    void screenLimitWindowMouse();
    void setPositionTopBottom();
    QRect mapToWidgetRect(const QWidget *target, QWidget *source);
    QRect windowResizeTop(QPoint relativeMove, QRect windowRect);
    QRect windowResizeLeft(QPoint relativeMove, QRect windowRect);
    void windowResizeRight(QPoint relativeMove, QPoint windowMouseMovePos);
    void windowResizeBottom(QPoint relativeMove, QPoint windowMouseMovePos);
    void toolTipHide();

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event);

    void showMaximized();
    void showNormal();

private slots:
    void on_pushButtonTop_clicked(bool checked);
    void on_pushButtonMinimize_clicked();
    void on_pushButtonMaximize_clicked(bool checked);
    void on_pushButtonClose_clicked();
};
#endif // WIDGET_H
