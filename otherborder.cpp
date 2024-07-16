#include "otherborder.h"
#include "ui_otherborder.h"

#include <QPainter>
#include <QScreen>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QDebug>

OtherBorder::OtherBorder(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OtherBorder)
{
    ui->setupUi(this);

    //Window frameless.
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    //Define widgetBorder.
    widgetBorder = ui->widgetBackground;

    //widgetBorder not accepted mouse event.
    widgetBorder->setAttribute(Qt::WA_TransparentForMouseEvents);

    //Init other border.
    otherBorder = Position::positionType::none;

    //Screen side.
    screenLine = 10;
}

OtherBorder::~OtherBorder()
{
    delete ui;
}

void OtherBorder::paintEvent(QPaintEvent *event)
{
    //Draw shadow.
    int shadowWidth = 10;
    QPainter shadow(this);
    shadow.setRenderHint(QPainter::Antialiasing);
    QColor shadowColor(0, 0, 0);

    for(int index = 0; index <= shadowWidth; index++)
    {
        shadowColor.setAlpha((index + 1) * 4);
        shadow.setPen(shadowColor);
        shadow.drawRoundedRect(index, index, width() - index * 2, height() - index * 2, 5, 5);
    }
}

void OtherBorder::active(const QMouseEvent *mouseMoveEvent, enum Position::positionType windowPosition)
{
    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();

    //Screen region.
    QRegion screenTopMargin = QRegion(screenRect.topLeft().x() + screenLine, screenRect.topLeft().y(), screenRect.width() - screenLine * 2, 1);
    QRegion screenLeftMargin = QRegion(screenRect.topLeft().x(), screenRect.topLeft().y() + screenLine, 1, screenRect.height() - screenLine * 2);
    QRegion screenRightMargin = QRegion(screenRect.topRight().x(), screenRect.topRight().y() + screenLine, 1, screenRect.height() - screenLine * 2);
    QRegion screenBottomMargin = QRegion(screenRect.bottomLeft().x() + screenLine, screenRect.bottomLeft().y(), screenRect.width() - screenLine * 2, 1);
    QRegion screenTopLeftCorner = QRegion(QRegion(screenRect.topLeft().x(), screenRect.topLeft().y(), screenLine, 1) + QRegion(screenRect.topLeft().x(), screenRect.topLeft().y(), 1, screenLine));
    QRegion screenTopRightCorner = QRegion(QRegion(screenRect.topRight().x() - screenLine, screenRect.topRight().y(), screenLine, 1) + QRegion(screenRect.topRight().x(), screenRect.topRight().y(), 1, screenLine));
    QRegion screenBottomLeftCorner = QRegion(QRegion(screenRect.bottomLeft().x(), screenRect.bottomLeft().y() - screenLine, 1, screenLine) + QRegion(screenRect.bottomLeft().x(), screenRect.bottomLeft().y() - screenLine, 1, screenLine));
    QRegion screenBottomRightCorner = QRegion(QRegion(screenRect.bottomRight().x(), screenRect.bottomRight().y() - screenLine, 1, screenLine) + QRegion(screenRect.bottomRight().x() - screenLine, screenRect.bottomRight().y(), screenLine, 1));
    QRegion screenCenterRegion = QRegion(screenRect.topLeft().x() + 1, screenRect.topLeft().y() + 1, screenRect.width() - 2, screenRect.height() - 2);

    //Other border.
    if(screenTopMargin.contains(mouseMoveEvent->globalPos()) && windowPosition != Position::positionType::maximize && otherBorder != Position::positionType::maximize)
    {
        show(screenRect, 200);
        otherBorder = Position::positionType::maximize;
    }
    else if(screenLeftMargin.contains(mouseMoveEvent->globalPos()) && windowPosition != Position::positionType::maximize && otherBorder != Position::positionType::left)
    {
        show(QRect(screenRect.topLeft().x(), screenRect.topLeft().y(), screenRect.width() / 2, screenRect.height()), 200);
        otherBorder = Position::positionType::left;
    }
    else if(screenRightMargin.contains(mouseMoveEvent->globalPos()) && windowPosition != Position::positionType::maximize && otherBorder != Position::positionType::right)
    {
        show(QRect(screenRect.topLeft().x() + screenRect.width() / 2, screenRect.topLeft().y(), screenRect.width() / 2, screenRect.height()), 200);
        otherBorder = Position::positionType::right;
    }
    else if(screenTopLeftCorner.contains(mouseMoveEvent->globalPos()) && windowPosition != Position::positionType::maximize && otherBorder != Position::positionType::topLeft)
    {
        show(QRect(screenRect.topLeft().x(), screenRect.topLeft().y(), screenRect.width() / 2, screenRect.height() / 2), 200);
        otherBorder = Position::positionType::topLeft;
    }
    else if(screenTopRightCorner.contains(mouseMoveEvent->globalPos()) && windowPosition != Position::positionType::maximize && otherBorder != Position::positionType::topRight)
    {
        show(QRect(screenRect.center().x(), screenRect.topLeft().y(), screenRect.width() / 2, screenRect.height() / 2), 200);
        otherBorder = Position::positionType::topRight;
    }
    else if(screenBottomLeftCorner.contains(mouseMoveEvent->globalPos()) && windowPosition != Position::positionType::maximize && otherBorder != Position::positionType::bottomLeft)
    {
        show(QRect(screenRect.topLeft().x(), screenRect.center().y(), screenRect.width() / 2, screenRect.height() / 2), 200);
        otherBorder = Position::positionType::bottomLeft;
    }
    else if(screenBottomRightCorner.contains(mouseMoveEvent->globalPos()) && windowPosition != Position::positionType::maximize && otherBorder != Position::positionType::bottomRight)
    {
        show(QRect(screenRect.center().x(), screenRect.center().y(), screenRect.width() / 2, screenRect.height() / 2), 200);
        otherBorder = Position::positionType::bottomRight;
    }
    else if((screenCenterRegion + screenBottomMargin).contains(mouseMoveEvent->globalPos()) && otherBorder != Position::positionType::none)
    {
        hide();
    }
}

void OtherBorder::active(const QMouseEvent *mouseMoveEvent, enum Position::positionType windowPosition, QRegion screenTopBottom)
{
    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();

    if(screenTopBottom.contains(mouseMoveEvent->globalPos()) && windowPosition == Position::positionType::none)
    {
        show(QRect(parentWidget()->x(), screenRect.y(), parentWidget()->width(), screenRect.height()), 200);
        otherBorder = Position::positionType::topBottom;
    }
    else if(otherBorder == Position::positionType::topBottom)
    {
        hide();
    }
}

//Show other border.
void OtherBorder::show(QRect rect, int duration)
{
    if(otherBorder == Position::positionType::none)
    {
        setGeometry(parentWidget()->geometry());
        QWidget::show();
    }

    QPropertyAnimation *otherBorderAnimation = new QPropertyAnimation(this, "geometry");
    otherBorderAnimation->setDuration(duration);
    otherBorderAnimation->setEndValue(rect);
    otherBorderAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

//Hide other border.
void OtherBorder::hide()
{
    otherBorder = Position::positionType::none;
    QWidget::hide();
}

enum Position::positionType OtherBorder::borderType() const
{
    return otherBorder;
}
