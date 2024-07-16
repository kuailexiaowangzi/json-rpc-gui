#include "tooltip.h"
#include "ui_tooltip.h"

#include <QFile>
#include <QScreen>
#include <QDebug>

ToolTip::ToolTip(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ToolTip)
{
    ui->setupUi(this);

    //Window frameless.
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);

    QFile qssFile(":/qss/tooltip.qss");
    if(qssFile.open(QFile::ReadOnly))
    {
        QString qssString = qssFile.readAll();
        setStyleSheet(qssString);
        qssFile.close();
    }

    oldWidget = Q_NULLPTR;
    timer = new QTimer(this);
    wait = 1000;

    //toolTipOffset offset.
    toolTipOffset = QPoint(0, 20);

    connect(this, SIGNAL(timerStart(int)), timer, SLOT(start(int)));
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));
    connect(this, SIGNAL(timerStop()), timer, SLOT(stop()));
}

ToolTip::~ToolTip()
{
    delete ui;
}

void ToolTip::ready(const QWidget *widget, const QEvent *event, QString text)
{
    if(event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonDblClick || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove)
    {
        cancel();
    }
    else if(widget != oldWidget && (event->type() == QEvent::Enter || event->type() == QEvent::MouseMove))
    {
        oldWidget = widget;
        ui->labelToolTip->setText(text);
        emit timerStart(wait);
    }
}

void ToolTip::timerTimeOut()
{
    emit timerStop();

    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();

    //toolTip pos.
    QPoint toolTipPos = cursor().pos() + toolTipOffset;
    if(toolTipPos.x() < screenRect.x())
    {
        toolTipPos.setX(screenRect.x());
    }
    else if(toolTipPos.y() < screenRect.y())
    {
        toolTipPos.setY(screenRect.y());
    }
    else if(toolTipPos.x() + sizeHint().width() > screenRect.right())
    {
        toolTipPos.setX(screenRect.right() - sizeHint().width());
    }
    else if(toolTipPos.y() + sizeHint().height() > screenRect.bottom())
    {
        toolTipPos.setY(screenRect.bottom() - sizeHint().height());
    }

    move(toolTipPos);
    adjustSize();

    //toolTip show.
    show();

    qDebug() << "ToolTip: " << ui->labelToolTip->text();
}

void ToolTip::cancel()
{
    if(timer->isActive())
    {
        emit timerStop();
    }
    hide();
    oldWidget = Q_NULLPTR;
}
