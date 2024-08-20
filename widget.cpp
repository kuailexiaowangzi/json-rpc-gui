#include "widget.h"
#include "ui_widget.h"

#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QScreen>
#include <QPainterPath>
#include <QWindow>
#include <QDebug>

#ifdef Q_OS_WIN
    #include "Windows.h"
#endif

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //Window frameless.
    setWindowFlag(Qt::FramelessWindowHint);
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
    QFile qssFile(":/qss/widget.qss");
    if(qssFile.open(QFile::ReadOnly))
    {
        QString qssString = qssFile.readAll();
        //Set style sheet.
        setStyleSheet(qssString);
        qssFile.close();
    }

    sizeInit();

    windowNormalRect = geometry();

    windowMargin = ui->verticalLayoutWidget->contentsMargins();

    //Install event filter.
    ui->widgetTitleBar->installEventFilter(this);
    window->installEventFilter(this);
    ui->pushButtonTop->installEventFilter(this);
    ui->pushButtonMinimize->installEventFilter(this);
    ui->pushButtonMaximize->installEventFilter(this);
    ui->pushButtonClose->installEventFilter(this);

    //labelTitle not accepted mouse event.
    ui->labelTitle->setAttribute(Qt::WA_TransparentForMouseEvents);

    //Window hover event.
    window->setAttribute(Qt::WA_Hover);

    //Window side.
    windowSide = 5;
    //Screen side.
    screenLine = 10;

    //Init title bar mouse press.
    isTitleBarMousePress = false;

    //Load mainWindow.
    mainWindow = new MainWindow(this);
    ui->verticalLayoutBackground->addWidget(mainWindow);

    //Create otherBorder.
    otherBorder = new OtherBorder(this);

    //Create toolTip.
    toolTip = new ToolTip(this);

    //Init window position.
    windowPosition = Position::positionType::none;

    //Init window press.
    windowPress = Position::positionType::none;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::sizeInit()
{
    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();
    int width = screenRect.width() * 0.8;
    int height = screenRect.height() * 0.8;

    if(width < this->width() || height < this->height())
    {
        int x = screenRect.width() * 0.1;
        int y = screenRect.height() * 0.1;
        setGeometry(x, y, width, height);
    }
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();

    //widgetTitleBar mouse event.
    if(watched == ui->widgetTitleBar)
    {
        QRegion screenCenterRegion = QRegion(screenRect.topLeft().x() + 1, screenRect.topLeft().y() + 1, screenRect.width() - 2, screenRect.height() - 2);

        //widgetTitleBar mouse double click event.
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent *mouseDoubleClickEvent = static_cast<QMouseEvent*>(event);
            if(ui->widgetTitleBar->mapTo(window, mouseDoubleClickEvent->pos()).y() > windowSide)
            {
                //Window maximize.
                on_pushButtonMaximize_clicked(windowPosition != Position::positionType::maximize);
            }
        }

        //widgetTitleBar mouse press positon of relative QWidget.
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mousePressEvent = static_cast<QMouseEvent*>(event);
            if(windowPosition == Position::positionType::maximize)
            {
                isTitleBarMousePress = true;
                widgetMousePressPos = ui->widgetTitleBar->mapTo(this, mousePressEvent->pos());
            }
            else if(ui->widgetTitleBar->mapTo(window, mousePressEvent->pos()).y() > windowSide)
            {
                isTitleBarMousePress = true;
                widgetMousePressPos = ui->widgetTitleBar->mapTo(this, mousePressEvent->pos());
                if(windowPosition == Position::positionType::none)
                {
                    windowNormalRect = geometry();
                }
            }

        }

        //widgetTitleBar mouse move event.
        if(event->type() == QEvent::MouseMove && isTitleBarMousePress && windowPress == Position::positionType::none)
        {
            QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(event);

            if(windowPosition == Position::positionType::maximize || (screenCenterRegion.contains(mouseMoveEvent->globalPos()) && windowPosition != Position::positionType::none))
            {
                //widgetTitleBar mouse drag, revert window normal.
                int titleBarWidth = ui->widgetTitleBar->rect().width();
                int titleBarHeight = ui->widgetTitleBar->rect().height();
                QPoint titleBarGlobalPos = ui->widgetTitleBar->mapToGlobal(ui->widgetTitleBar->pos());
                int titleBarMarginLeft = ui->verticalLayoutWidget->contentsMargins().left();
                int titleBarMarginTop = ui->verticalLayoutWidget->contentsMargins().top();
                showNormal();
                int titleBarNormalWidth = ui->widgetTitleBar->rect().width();
                int titleBarNormalHeight = ui->widgetTitleBar->rect().height();
                int transformPressPosX = qRound(static_cast<double>(titleBarNormalWidth) / static_cast<double>(titleBarWidth) * static_cast<double>(widgetMousePressPos.x()));
                int transformPressPosY = qRound(static_cast<double>(titleBarNormalHeight) / static_cast<double>(titleBarHeight) * static_cast<double>(widgetMousePressPos.y()));
                move(titleBarGlobalPos.x() + widgetMousePressPos.x() - transformPressPosX + titleBarMarginLeft - windowMargin.left(), titleBarGlobalPos.y() + widgetMousePressPos.y() - transformPressPosY + titleBarMarginTop - windowMargin.top());
                widgetMousePressPos = QPoint(transformPressPosX - titleBarMarginLeft + windowMargin.left(), transformPressPosY - titleBarMarginTop + windowMargin.top());
            }
            else
            {
                //widgetTitleBar mouse drag, window move.
                move(mouseMoveEvent->globalPos() - widgetMousePressPos);

                //Screen limit mouse cursor.
                screenLimitWindowMouse();
            }

            //Other border.
            otherBorder->active(mouseMoveEvent, windowPosition);
        }

        //widgetTitleBar mouse release.
        if((event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::Leave) && isTitleBarMousePress)
        {
            isTitleBarMousePress = false;

            //Window resize.
            if(otherBorder->borderType() == Position::positionType::maximize)
            {
                showMaximized();
            }
            else if(otherBorder->borderType() == Position::positionType::left)
            {
                ui->verticalLayoutWidget->setContentsMargins(0, 0, windowMargin.right(), 0);
                setGeometry(screenRect.topLeft().x(), screenRect.topLeft().y(), screenRect.width() / 2, screenRect.height());
                windowPosition = Position::positionType::left;
            }
            else if(otherBorder->borderType() == Position::positionType::right)
            {
                ui->verticalLayoutWidget->setContentsMargins(windowMargin.left(), 0, 0, 0);
                setGeometry(screenRect.topLeft().x() + screenRect.width() / 2, screenRect.topLeft().y(), screenRect.width() / 2, screenRect.height());
                windowPosition = Position::positionType::right;
            }
            else if(otherBorder->borderType() == Position::positionType::topLeft)
            {
                ui->verticalLayoutWidget->setContentsMargins(0, 0, windowMargin.right(), windowMargin.bottom());
                setGeometry(screenRect.topLeft().x(), screenRect.topLeft().y(), screenRect.width() / 2, screenRect.height() / 2);
                windowPosition = Position::positionType::topLeft;
            }
            else if(otherBorder->borderType() == Position::positionType::topRight)
            {
                ui->verticalLayoutWidget->setContentsMargins(windowMargin.left(), 0, 0, windowMargin.bottom());
                setGeometry(screenRect.center().x(), screenRect.topLeft().y(), screenRect.width() / 2, screenRect.height() / 2);
                windowPosition = Position::positionType::topRight;
            }
            else if(otherBorder->borderType() == Position::positionType::bottomLeft)
            {
                ui->verticalLayoutWidget->setContentsMargins(0, windowMargin.top(), windowMargin.right(), 0);
                setGeometry(screenRect.topLeft().x(), screenRect.center().y(), screenRect.width() / 2, screenRect.height() / 2);
                windowPosition = Position::positionType::bottomLeft;
            }
            else if(otherBorder->borderType() == Position::positionType::bottomRight)
            {
                ui->verticalLayoutWidget->setContentsMargins(windowMargin.left(), windowMargin.top(), 0, 0);
                setGeometry(screenRect.center().x(), screenRect.center().y(), screenRect.width() / 2, screenRect.height() / 2);
                windowPosition = Position::positionType::bottomRight;
            }

            //Other border hide.
            if(otherBorder->borderType() != Position::positionType::none)
            {
                otherBorder->hide();
            }

            //Window limit screen top.
            if(window->mapToGlobal(window->pos()).y() < screenRect.y())
            {
                move(x(), screenRect.y() - window->y());
            }
        }
    }

    //Window mouse event.
    if(watched == window)
    {
        //Create window region.
        int windowWidth = window->width();
        int windowHeight = window->height();
        QRegion windowTopMargin = QRegion(windowSide, 0, windowWidth - windowSide * 2, windowSide);
        QRegion windowLeftMargin = QRegion(0, windowSide, windowSide, windowHeight - windowSide * 2);
        QRegion windowRightMargin = QRegion(windowWidth - windowSide, windowSide, windowSide, windowHeight - windowSide * 2);
        QRegion windowBottomMargin = QRegion(windowSide, windowHeight - windowSide, windowWidth - windowSide * 2, windowSide);
        QRegion windowTopLeftCorner = QRegion(0, 0, windowSide, windowSide);
        QRegion windowTopRightCorner = QRegion(windowWidth - windowSide, 0, windowSide, windowSide);
        QRegion windowBottomLeftCorner = QRegion(0, windowHeight - windowSide, windowSide, windowSide);
        QRegion windowBottomRightCorner = QRegion(windowWidth - windowSide, windowHeight - windowSide, windowSide, windowSide);

        //Map to window region.
        QRegion minimizeButtonRect = mapToWidgetRect(this, ui->pushButtonMinimize);
        QRegion maximizeButtonRect = mapToWidgetRect(this, ui->pushButtonMaximize);
        QRegion closeButtonRect = mapToWidgetRect(this, ui->pushButtonClose);

        //Window exclude region.
        QRegion excludeRegion = minimizeButtonRect + maximizeButtonRect + closeButtonRect;

        //Window mouse hover move event.
        if(event->type() == QEvent::HoverMove && windowPosition != Position::positionType::maximize && !isTitleBarMousePress && windowPress == Position::positionType::none)
        {
            QHoverEvent *mouseHoverMoveEvent = static_cast<QHoverEvent*>(event);
            QPoint windowMouseHoverMovePos = mouseHoverMoveEvent->pos();

            //Mouse cursor windowPosition.
            if((windowTopMargin + windowBottomMargin - excludeRegion).contains(windowMouseHoverMovePos))
            {
                window->setCursor(Qt::SizeVerCursor);
            }
            else if((windowLeftMargin + windowRightMargin - excludeRegion).contains(windowMouseHoverMovePos))
            {
                window->setCursor(Qt::SizeHorCursor);
            }
            else if((windowTopLeftCorner + windowBottomRightCorner - excludeRegion).contains(windowMouseHoverMovePos))
            {
                window->setCursor(Qt::SizeFDiagCursor);
            }
            else if((windowTopRightCorner + windowBottomLeftCorner - excludeRegion).contains(windowMouseHoverMovePos))
            {
                window->setCursor(Qt::SizeBDiagCursor);
            }
            else
            {
                window->setCursor(Qt::ArrowCursor);
            }
        }

        //Window mouse double click event.
        if(event->type() == QEvent::MouseButtonDblClick && (windowPosition != Position::positionType::maximize && (windowPosition == Position::positionType::none || windowPosition == Position::positionType::topBottom)))
        {
            QMouseEvent *mouseDoubleClickEvent = static_cast<QMouseEvent*>(event);
            QPoint windowMouseDoubleClickPos = mouseDoubleClickEvent->pos();
            if((windowTopMargin - excludeRegion).contains(windowMouseDoubleClickPos) || (windowBottomMargin - excludeRegion).contains(windowMouseDoubleClickPos))
            {
                if(windowPosition == Position::positionType::topBottom)
                {
                    showNormal();
                }
                else
                {
                    setPositionTopBottom();
                }
            }
        }

        //Window mouse press event.
        if(event->type() == QEvent::MouseButtonPress && windowPosition != Position::positionType::maximize && windowPress == Position::positionType::none && !isTitleBarMousePress)
        {

            QMouseEvent *mousePressEvent = static_cast<QMouseEvent*>(event);
            widgetMousePressPos = mousePressEvent->pos();

            //Window press region.
            if((windowTopMargin - excludeRegion).contains(widgetMousePressPos))
            {
                windowPress = Position::positionType::top;
            }
            else if((windowLeftMargin - excludeRegion).contains(widgetMousePressPos))
            {
                windowPress = Position::positionType::left;
            }
            else if((windowRightMargin - excludeRegion).contains(widgetMousePressPos))
            {
                windowPress = Position::positionType::right;
            }
            else if((windowBottomMargin - excludeRegion).contains(widgetMousePressPos))
            {
                windowPress = Position::positionType::bottom;
            }
            else if((windowTopLeftCorner - excludeRegion).contains(widgetMousePressPos))
            {
                windowPress = Position::positionType::topLeft;
            }
            else if((windowTopRightCorner - excludeRegion).contains(widgetMousePressPos))
            {
                windowPress = Position::positionType::topRight;
            }
            else if((windowBottomLeftCorner - excludeRegion).contains(widgetMousePressPos))
            {
                windowPress = Position::positionType::bottomLeft;
            }
            else if((windowBottomRightCorner - excludeRegion).contains(widgetMousePressPos))
            {
                windowPress = Position::positionType::bottomRight;
            }

            if(windowPosition == Position::positionType::none)
            {
                windowNormalRect = geometry();
            }
        }

        //Window mouse move event.
        if(event->type() == QEvent::MouseMove && windowPosition != Position::positionType::maximize && windowPress != Position::positionType::none)
        {
            QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(event);
            QPoint windowMouseMovePos = mouseMoveEvent->pos();
            QPoint relativeMove = windowMouseMovePos - widgetMousePressPos;
            QRect windowRect = geometry();

            //Window mouse drag resize.
            if(windowPress == Position::positionType::top && cursor().pos().y() >= screenRect.y())
            {
                windowRect = windowResizeTop(relativeMove, windowRect);
                setGeometry(windowRect);
            }
            else if(windowPress == Position::positionType::left  && cursor().pos().x() >= screenRect.x())
            {
                windowRect = windowResizeLeft(relativeMove, windowRect);
                setGeometry(windowRect);
            }
            else if(windowPress == Position::positionType::right && cursor().pos().x() <= screenRect.right())
            {
                windowRect.adjust(0, 0, relativeMove.x(), 0);
                setGeometry(windowRect);
                windowResizeRight(relativeMove, windowMouseMovePos);
            }
            else if(windowPress == Position::positionType::bottom && cursor().pos().y() <= screenRect.bottom())
            {
                windowRect.adjust(0, 0, 0, relativeMove.y());
                setGeometry(windowRect);
                windowResizeBottom(relativeMove, windowMouseMovePos);
            }
            else if(windowPress == Position::positionType::topLeft)
            {
                if(cursor().pos().x() >= screenRect.x())
                {
                    windowRect = windowResizeLeft(relativeMove, windowRect);
                }
                if(cursor().pos().y() >= screenRect.y())
                {
                    windowRect = windowResizeTop(relativeMove, windowRect);
                }
                setGeometry(windowRect);
            }
            else if(windowPress == Position::positionType::topRight)
            {
                if(cursor().pos().y() >= screenRect.y())
                {
                    windowRect = windowResizeTop(relativeMove, windowRect);
                }
                windowRect.adjust(0, 0, relativeMove.x(), 0);
                setGeometry(windowRect);
                if(cursor().pos().x() <= screenRect.right())
                {
                    windowResizeRight(relativeMove, windowMouseMovePos);
                }
            }
            else if(windowPress == Position::positionType::bottomLeft)
            {
                if(cursor().pos().x() >= screenRect.x())
                {
                    windowRect = windowResizeLeft(relativeMove, windowRect);
                }
                windowRect.adjust(0, 0, 0, relativeMove.y());
                setGeometry(windowRect);
                if(cursor().pos().y() <= screenRect.bottom())
                {
                    windowResizeBottom(relativeMove, windowMouseMovePos);
                }
            }
            else if(windowPress == Position::positionType::bottomRight)
            {
                windowRect.adjust(0, 0, relativeMove.x(), relativeMove.y());
                setGeometry(windowRect);
                if(cursor().pos().x() <= screenRect.right())
                {
                    windowResizeRight(relativeMove, windowMouseMovePos);
                }
                if(cursor().pos().y() <= screenRect.bottom())
                {
                    windowResizeBottom(relativeMove, windowMouseMovePos);
                }
            }

            //Screen region.
            QRegion screenTopMargin = QRegion(screenRect.topLeft().x(), screenRect.topLeft().y(), screenRect.width(), 1);
            QRegion screenBottomMargin = QRegion(screenRect.bottomLeft().x(), screenRect.bottomLeft().y(), screenRect.width(), 1);
            QRegion screenTopBottom = screenTopMargin + screenBottomMargin;

            if(windowPress == Position::positionType::top || windowPress == Position::positionType::bottom || windowPress == Position::positionType::topLeft || windowPress == Position::positionType::topRight || windowPress == Position::positionType::bottomLeft || windowPress == Position::positionType::bottomRight)
            {
                //Other border top bottom.
                otherBorder->active(mouseMoveEvent, windowPosition, screenTopBottom);

                //Window resize.
                if(!screenTopBottom.contains(mouseMoveEvent->globalPos()) && windowPosition == Position::positionType::topBottom)
                {
                    windowNormalRect = QRect(x(), y(), width(), windowNormalRect.height());
                    showNormal();
                }
            }


            //Screen limit window mouse cursor.
            screenLimitWindowMouse();
        }

        //Window mouse release event.
        if(event->type() == QEvent::MouseButtonRelease)
        {
            windowPress = Position::positionType::none;
            if(otherBorder->borderType() == Position::positionType::topBottom)
            {
                setPositionTopBottom();
                otherBorder->hide();
            }
        }
    }

    //pushButtonTop mouse event.
    if(watched == ui->pushButtonTop)
    {
        if(ui->pushButtonTop->isChecked())
        {
            toolTip->ready(ui->pushButtonTop, event, tr("Cancel top"));
        }
        else
        {
            toolTip->ready(ui->pushButtonTop, event, tr("Top"));
        }
    }

    //pushButtonMinimize mouse event.
    if(watched == ui->pushButtonMinimize)
    {
        toolTip->ready(ui->pushButtonMinimize, event, tr("Minimize"));
    }

    //pushButtonMaximize mouse event.
    if(watched == ui->pushButtonMaximize)
    {
        if(ui->pushButtonMaximize->isChecked())
        {
            toolTip->ready(ui->pushButtonMaximize, event, tr("Restore"));
        }
        else
        {
            if(event->type() == QEvent::Enter)
            {
                ui->pushButtonMaximize->setStyleSheet("background-color: #1E90FF");
            }
            else if(event->type() == QEvent::Leave)
            {
                ui->pushButtonMaximize->setStyleSheet("background-color: none");
            }
            toolTip->ready(ui->pushButtonMaximize, event, tr("Maximize"));
        }
    }

    //pushButtonClose mouse event.
    if(watched == ui->pushButtonClose)
    {
        toolTip->ready(ui->pushButtonClose, event, tr("Close"));
    }

    return false;
}

//Window resize top.
QRect Widget::windowResizeTop(QPoint relativeMove, QRect windowRect)
{
    if(height() - relativeMove.y() < minimumHeight())
    {
        relativeMove.setY(height() - minimumHeight());
    }
    windowRect.adjust(0, relativeMove.y(), 0, 0);
    return windowRect;
}

//Window resize left.
QRect Widget::windowResizeLeft(QPoint relativeMove, QRect windowRect)
{
    if(width() - relativeMove.x() < minimumWidth())
    {
        relativeMove.setX(width() - minimumWidth());
    }
    windowRect.adjust(relativeMove.x(), 0, 0, 0);
    return windowRect;
}

//Window resize right.
void Widget::windowResizeRight(QPoint relativeMove, QPoint windowMouseMovePos)
{
    if(width() + relativeMove.x() >= minimumWidth())
    {
        widgetMousePressPos.setX(windowMouseMovePos.x());
    }
    else
    {
        widgetMousePressPos.setX(window->width());
    }
}

//Window resize bottom.
void Widget::windowResizeBottom(QPoint relativeMove, QPoint windowMouseMovePos)
{
    if(height() + relativeMove.y() >= minimumHeight())
    {
        widgetMousePressPos.setY(windowMouseMovePos.y());
    }
    else
    {
        widgetMousePressPos.setY(window->height());
    }
}

//Window top button.
void Widget::on_pushButtonTop_clicked(bool checked)
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    if(checked)
    {
        #ifdef Q_OS_WIN
            SetWindowPos((HWND)winId(), HWND_TOPMOST, x(), y(), width(), height(), SWP_SHOWWINDOW);
        #endif

        #ifndef Q_OS_WIN
            windowHandle()->setFlag(Qt::WindowStaysOnTopHint);
        #endif

        ui->pushButtonTop->setChecked(true);
    }
    else
    {
        #ifdef Q_OS_WIN
            SetWindowPos((HWND)winId(), HWND_NOTOPMOST, x(), y(), width(), height(), SWP_SHOWWINDOW);
        #endif

        #ifndef Q_OS_WIN
            windowHandle()->setFlag(Qt::WindowStaysOnTopHint, false);
        #endif

        ui->pushButtonTop->setChecked(false);
    }
    setAttribute(Qt::WA_TranslucentBackground);
}

//Window minimize button.
void Widget::on_pushButtonMinimize_clicked()
{
    showMinimized();
}

//Window maximize or normal button.
void Widget::on_pushButtonMaximize_clicked(bool checked)
{
    if(checked)
    {
        //Window normal geometry.
        if(windowPosition != Position::positionType::maximize && windowPosition == Position::positionType::none)
        {
            windowNormalRect = geometry();
        }

        showMaximized();
    }
    else
    {
        showNormal();
    }
}

//Windows close button.
void Widget::on_pushButtonClose_clicked()
{
    close();
}

//Window maximized remove margin.
void Widget::showMaximized()
{
    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();
    ui->verticalLayoutWidget->setContentsMargins(0, 0, 0, 0);
    setGeometry(screenRect);
    QWidget::showMaximized();
    windowPosition = Position::positionType::maximize;
    ui->pushButtonMaximize->setChecked(true);
    ui->pushButtonMaximize->setStyleSheet("background-color: #1E90FF");
    otherBorder->hide();
    toolTipHide();
}

//Window normal add margin.
void Widget::showNormal()
{
    ui->verticalLayoutWidget->setContentsMargins(windowMargin);
    QWidget::showNormal();
    setGeometry(windowNormalRect);
    windowPosition = Position::positionType::none;
    ui->pushButtonMaximize->setChecked(false);
    ui->pushButtonMaximize->setStyleSheet("background-color: none");
    toolTipHide();
}

//Screen limit window mouse.
void Widget::screenLimitWindowMouse()
{
    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();
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

//Set windowPosition top bottom.
void Widget::setPositionTopBottom()
{
    //Current screen available rect.
    QRect screenRect = screen()->availableGeometry();
    ui->verticalLayoutWidget->setContentsMargins(windowMargin.left(), 0, windowMargin.right(), 0);
    setGeometry(x(), screenRect.y(), width(), screenRect.height());
    windowPosition = Position::positionType::topBottom;
}

//Map to window rect.
QRect Widget::mapToWidgetRect(const QWidget *target, QWidget *source)
{
    QPoint topLeftPos = source->mapTo(target, QPoint(0, 0));
    QRect rect = QRect(topLeftPos, source->size());
    return rect;
}

//toolTip hide.
void Widget::toolTipHide()
{
    if(toolTip->isVisible())
    {
        toolTip->cancel();
    }
}

