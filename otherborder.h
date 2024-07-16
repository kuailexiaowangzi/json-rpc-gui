#ifndef OTHERBORDER_H
#define OTHERBORDER_H

#include <QWidget>
#include "position.h"

namespace Ui {
class OtherBorder;
}

class OtherBorder : public QWidget
{
    Q_OBJECT

public:
    explicit OtherBorder(QWidget *parent = nullptr);
    ~OtherBorder();
    void active(const QMouseEvent *mouseMoveEvent, enum Position::positionType windowPosition);
    void active(const QMouseEvent *mouseMoveEvent, enum Position::positionType windowPosition, QRegion screenTopBottom);
    void hide();
    enum Position::positionType borderType() const;

private:
    Ui::OtherBorder *ui;
    QWidget *widgetBorder;
    enum Position::positionType otherBorder;
    int screenLine;
    void show(QRect rect, int duration);

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event);
};

#endif // OTHERBORDER_H
