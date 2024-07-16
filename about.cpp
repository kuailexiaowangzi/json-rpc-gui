#include "about.h"
#include "ui_about.h"

#include <QFile>
#include <QTranslator>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QDesktopServices>
#include <QDebug>

About::About(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);

    //Window frameless.
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    //Define window;
    window = ui->widgetBackground;

    //Window shadow.
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(64, 64, 64));
    shadow->setBlurRadius(10);
    window->setGraphicsEffect(shadow);

    //Load QSS file.
    QFile qssFile(":/qss/about.qss");
    if(qssFile.open(QFile::ReadOnly))
    {
        QString qssString = qssFile.readAll();
        //Set style sheet.
        setStyleSheet(qssString);
        qssFile.close();
    }

    //Install event filter.
    ui->widgetTitleBar->installEventFilter(this);

    //labelTitle not accepted mouse event.
    ui->labelTitle->setAttribute(Qt::WA_TransparentForMouseEvents);

    //Window mouse press.
    isTitleBarMousePress = false;

    //Create connect url.
    connect(ui->textBrowserContent, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(openUrl(const QUrl&)));
}

About::~About()
{
    delete ui;
}

void About::translate()
{
    ui->retranslateUi(this);
}

bool About::eventFilter(QObject *watched, QEvent *event)
{
    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();

    //widgetTitleBar mouse event.
    if(watched == ui->widgetTitleBar)
    {
        //widgetTitleBar mouse press positon of relative QWidget.
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mousePressEvent = static_cast<QMouseEvent*>(event);
            isTitleBarMousePress = true;
            titleBarPressPos = ui->widgetTitleBar->mapTo(this, mousePressEvent->pos());
        }

        //widgetTitleBar mouse move event.
        if(event->type() == QEvent::MouseMove && isTitleBarMousePress)
        {
            QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(event);
            //widgetTitleBar mouse drag, window move.
            move(mouseMoveEvent->globalPos() - titleBarPressPos);

            //Screen limit window mouse.
            QCursor screenCursor;
            if(cursor().pos().x() < screenRect.x())
            {
                screenCursor.setPos(screenRect.x(), cursor().pos().y());
                setCursor(screenCursor);
            }
            else if(cursor().pos().x() > screenRect.right())
            {
                screenCursor.setPos(screenRect.right(), cursor().pos().y());
                setCursor(screenCursor);
            }
            if(cursor().pos().y() < screenRect.y())
            {
                screenCursor.setPos(cursor().pos().x(), screenRect.y());
                setCursor(screenCursor);
            }
            else if(cursor().pos().y() > screenRect.bottom())
            {
                screenCursor.setPos(cursor().pos().x(), screenRect.bottom());
                setCursor(screenCursor);
            }
        }

        //widgetTitleBar mouse release.
        if(event->type() == QEvent::MouseButtonRelease && isTitleBarMousePress)
        {
            isTitleBarMousePress = false;

            //Window limit screen top.
            if(window->mapToGlobal(window->pos()).y() < screenRect.y())
            {
                move(x(), screenRect.y() - window->y());
            }
        }
    }

    return false;
}

//Windows close.
void About::on_pushButtonClose_clicked()
{
    close();
}

//Window OK.
void About::on_pushButtonOk_clicked()
{
    close();
}

//Open url.
void About::openUrl(const QUrl &url)
{
    if(QDesktopServices::openUrl(url))
    {
        qDebug() << tr("Success open URL: ");
    }
    else
    {
        qDebug() << tr("Fail open URL: ");
    }

    qDebug() << url.toString();
}

