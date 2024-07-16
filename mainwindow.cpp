#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QVariant>
#include <QScrollBar>
#include <QUrl>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Config file.
    configFile = new QSettings("./config.ini", QSettings::IniFormat);

    //Window frameless.
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    //Load QSS file.
    QFile qssFile(":/qss/mainwindow.qss");
    if(qssFile.open(QFile::ReadOnly))
    {
        QString qssString = qssFile.readAll();
        //Set style sheet.
        setStyleSheet(qssString);
        qssFile.close();
    }

    //Language action group.
    languageActionGroup = new QActionGroup(this);
    languageActionGroup->addAction(ui->actionChinese);
    languageActionGroup->addAction(ui->actionEnglish);

    //Chinese false.
    defaultLanguage = QLocale::English;

    //Create about.
    about = new About(this);

    //Translate init.
    languageInit();

    //Install event filter.
    ui->spinBoxRequestId->installEventFilter(this);
    ui->lineEditRequestId->installEventFilter(this);
    ui->plainTextEditParams->installEventFilter(this);
    ui->plainTextEditRequest->installEventFilter(this);
    ui->textBrowserResult->installEventFilter(this);
    ui->textBrowserMessage->installEventFilter(this);
    ui->textBrowserData->installEventFilter(this);
    ui->textBrowserResponse->installEventFilter(this);
    ui->textBrowserError->installEventFilter(this);

    //Hover event.
    ui->plainTextEditParams->setAttribute(Qt::WA_Hover);
    ui->plainTextEditRequest->setAttribute(Qt::WA_Hover);
    ui->textBrowserResult->setAttribute(Qt::WA_Hover);
    ui->textBrowserMessage->setAttribute(Qt::WA_Hover);
    ui->textBrowserData->setAttribute(Qt::WA_Hover);
    ui->textBrowserResponse->setAttribute(Qt::WA_Hover);
    ui->textBrowserError->setAttribute(Qt::WA_Hover);

    //Widget side.
    widgetSide = 5;

    //Widget mouse margin.
    widgetMouseMargin = widgetTopBottomMargin::none;

    //Widget press.
    widgetMousePress = Q_NULLPTR;

    //Widget old height.
    widgetPressHeight = 0;

    //RadioButton not accepted mouse event.
    ui->radioButtonResponseVersion1->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->radioButtonResponseVersion2->setAttribute(Qt::WA_TransparentForMouseEvents);

    //Checkbox not accepted mouse event.
    ui->checkBoxResponseIdNull->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBoxResponseIdInt->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->checkBoxResponseIdString->setAttribute(Qt::WA_TransparentForMouseEvents);

    //Remove pageResultError.
    ui->stackedWidgetResult->removeWidget(ui->pageResultError);

    //Remove status bar.
    setStatusBar(Q_NULLPTR);

    reply = Q_NULLPTR;
    isReplyDone = false;

    //Create manager connect authenticator.
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Translate init.
void MainWindow::languageInit()
{
    QVariant language = configFile->value("Option/Language");

    if(language.isValid())
    {
        if(language == "Chinese")
        {
            translateLanguage(QLocale::Chinese);
            ui->actionChinese->setChecked(true);
        }
        else
        {
            translateLanguage(QLocale::English);
            ui->actionEnglish->setChecked(true);
        }
    }
    else
    {
        if(QLocale::system().language() == QLocale::Chinese)
        {
            translateLanguage(QLocale::Chinese);
            ui->actionChinese->setChecked(true);
        }
        else
        {
            translateLanguage(QLocale::English);
            ui->actionEnglish->setChecked(true);
        }
    }
}

//Translate language.
void MainWindow::translateLanguage(enum QLocale::Language language)
{
    if(language == QLocale::Chinese)
    {
        if(translator.load(":/language/zh_cn.qm"))
        {
           qApp->installTranslator(&translator);
        }
        if(translatorWidgets.load(":/language/widgets.qm"))
        {
            qApp->installTranslator(&translatorWidgets);
        }
        if(translatorNetwork.load("/language/network.qm"))
        {
            qApp->installTranslator(&translatorNetwork);
        }
        if(configFile->value("Option/Language") != "Chinese")
        {
            configFile->setValue("Option/Language", "Chinese");
        }
        defaultLanguage = QLocale::Chinese;
        qDebug() << tr("Language: Chinese");
    }
    else
    {
        qApp->removeTranslator(&translator);
        qApp->removeTranslator(&translatorWidgets);
        if(configFile->value("Option/Language") != "English")
        {
            configFile->setValue("Option/Language", "English");
        }
        defaultLanguage = QLocale::English;
        qDebug() << tr("Language: English");
    }
    ui->retranslateUi(this);

    //About translate.
    about->translate();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    //spinBoxRequestId focus event.
    if(watched == ui->spinBoxRequestId)
    {
        if(event->type() == QEvent::FocusIn)
        {
            ui->checkBoxRequestIdInt->setChecked(true);
            qDebug() << tr("spinBoxRequestId focus event.");
        }
    }

    //lineEditRequestId focus event.
    if(watched == ui->lineEditRequestId)
    {
        if(event->type() == QEvent::FocusIn)
        {
            ui->checkBoxRequestIdString->setChecked(true);
            qDebug() << tr("lineEditRequestId focus event.");
        }
    }

    //plainTextEditRequest mouse event.
    if(watched == ui->plainTextEditParams)
    {
        widgetTopBottomMarginResize<QWidget>(ui->plainTextEditParams, event, widgetTopBottomMargin::bottomMargin, ui->verticalLayoutScrollArea, ui->tabWidgetRequest, ui->stackedWidgetResponse);
    }

    //plainTextEditRequest mouse event.
    if(watched == ui->plainTextEditRequest)
    {
        widgetTopBottomMarginResize<QWidget>(ui->plainTextEditRequest, event, widgetTopBottomMargin::bottomMargin, ui->verticalLayoutScrollArea, ui->tabWidgetRequest, ui->stackedWidgetResponse);
    }

    //textBrowserResult mouse event.
    if(watched == ui->textBrowserResult)
    {
        widgetTopBottomMarginResize<QWidget>(ui->textBrowserResult, event, widgetTopBottomMargin::topMargin, ui->verticalLayoutScrollArea, ui->stackedWidgetResponse, ui->tabWidgetRequest);
    }

    //textBrowserMessage mouse event.
    if(watched == ui->textBrowserMessage)
    {
        widgetSelectMargin(ui->textBrowserMessage, event);

        if(widgetMouseMargin == widgetTopBottomMargin::topMargin)
        {
            widgetTopBottomMarginResize<QWidget>(ui->textBrowserMessage, event, widgetTopBottomMargin::topMargin, ui->verticalLayoutScrollArea, ui->stackedWidgetResponse, ui->tabWidgetRequest);
        }
        if(widgetMouseMargin == widgetTopBottomMargin::bottomMargin)
        {
            widgetTopBottomMarginResize<QBoxLayout>(ui->textBrowserMessage, event, widgetTopBottomMargin::bottomMargin, ui->verticalLayoutResultError, ui->horizontalLayoutMessage, ui->horizontalLayoutData);
        }
    }

    //textBrowserData mouse event.
    if(watched == ui->textBrowserData)
    {
        widgetTopBottomMarginResize<QBoxLayout>(ui->textBrowserData, event, widgetTopBottomMargin::topMargin, ui->verticalLayoutResultError, ui->horizontalLayoutData, ui->horizontalLayoutMessage);
    }

    //textBrowserResponse mouse event.
    if(watched == ui->textBrowserResponse)
    {
        widgetTopBottomMarginResize<QWidget>(ui->textBrowserResponse, event, widgetTopBottomMargin::topMargin, ui->verticalLayoutScrollArea, ui->stackedWidgetResponse, ui->tabWidgetRequest);
    }

    //textBrowserError mouse event.
    if(watched == ui->textBrowserError)
    {
        widgetTopBottomMarginResize<QWidget>(ui->textBrowserError, event, widgetTopBottomMargin::topMargin, ui->verticalLayoutScrollArea, ui->stackedWidgetResponse, ui->tabWidgetRequest);
    }

    return false;
}

//Widget select margin.
void MainWindow::widgetSelectMargin(QWidget *widget, QEvent *event)
{
    QRegion topMarginRegion = QRegion(0, 0, widget->width(), widgetSide);
    QRegion bottomMarginRegion = QRegion(0, widget->height() - widgetSide, widget->width(), widgetSide);

    QPoint mousePos;
    if(event->type() == QEvent::HoverMove && widgetMousePress == Q_NULLPTR)
    {
        const QHoverEvent *mouseHoverMoveEvent = static_cast<const QHoverEvent*>(event);
        mousePos = mouseHoverMoveEvent->pos();
    }
    if(event->type() == QEvent::MouseButtonPress && widgetMousePress == Q_NULLPTR)
    {
        const QMouseEvent *mousePressEvent = static_cast<const QMouseEvent*>(event);
        mousePos = mousePressEvent->pos();
    }

    if((event->type() == QEvent::HoverMove || event->type() == QEvent::MouseButtonPress) && widgetMousePress == Q_NULLPTR)
    {
        if(topMarginRegion.contains(mousePos))
        {
            widgetMouseMargin = widgetTopBottomMargin::topMargin;
        }
        if(bottomMarginRegion.contains(mousePos))
        {
            widgetMouseMargin = widgetTopBottomMargin::bottomMargin;
        }
    }
}

//Widget margin resize widget size.
template<typename T>
void MainWindow::widgetTopBottomMarginResize(QWidget *widget, const QEvent *event, enum widgetTopBottomMargin margin, QBoxLayout *ancestor, T *our, T *opposing)
{
    //Create widget region.
    QRegion marginRegion;
    if(margin == widgetTopBottomMargin::topMargin)
    {
        marginRegion = QRegion(0, 0, widget->width(), widgetSide);
    }
    if(margin == widgetTopBottomMargin::bottomMargin)
    {
        marginRegion = QRegion(0, widget->height() - widgetSide, widget->width(), widgetSide);
    }

    //Widget mouse hover move event.
    if(event->type() == QEvent::HoverMove && !ui->scrollArea->verticalScrollBar()->isVisible() && widgetMousePress == Q_NULLPTR)
    {
        const QHoverEvent *mouseHoverMoveEvent = static_cast<const QHoverEvent*>(event);
        QPoint mouseHoverMovePos = mouseHoverMoveEvent->pos();

        //Widget mouse cursor shape.
        if(marginRegion.contains(mouseHoverMovePos))
        {
            widget->setCursor(Qt::SizeVerCursor);
        }
        else
        {
            widget->setCursor(Qt::ArrowCursor);
        }
    }

    //Widget mouse press event.
    if(event->type() == QEvent::MouseButtonPress && !ui->scrollArea->verticalScrollBar()->isVisible() && widgetMousePress == Q_NULLPTR)
    {
        const QMouseEvent *mousePressEvent = static_cast<const QMouseEvent*>(event);
        widgetMousePressPos = mousePressEvent->pos();

        //Widget press top region.
        if(marginRegion.contains(widgetMousePressPos))
        {
            widgetMousePress = widget;
            widgetPressHeight = widget->height();
        }
    }

    //Widget mouse release event.
    if(event->type() == QEvent::MouseButtonRelease)
    {
        widgetMousePress = Q_NULLPTR;
    }

    //Widget mouse move event.
    if(event->type() == QEvent::MouseMove && widgetMousePress == widget)
    {
        //Widget relative move y.
        const QMouseEvent *mouseMoveEvent = static_cast<const QMouseEvent*>(event);
        QPoint mouseMovePos = mouseMoveEvent->pos();
        int relativeMoveY = mouseMovePos.y() - widgetMousePressPos.y();

        //Widget mouse drag resize.
        int limitMove = 100;
        if(margin == widgetTopBottomMargin::topMargin)
        {
            if(relativeMoveY > limitMove)
            {
                relativeMoveY = limitMove;
            }
            else if(relativeMoveY < -limitMove)
            {
                relativeMoveY = -limitMove;
            }
            layoutResize(ancestor, our, opposing, -relativeMoveY);
        }
        if(margin == widgetTopBottomMargin::bottomMargin)
        {
            if(relativeMoveY > limitMove)
            {
                relativeMoveY = limitMove;
            }
            else if(relativeMoveY < -limitMove)
            {
                relativeMoveY = -limitMove;
            }
            layoutResize(ancestor, our, opposing, relativeMoveY);

            //Reset widget mouse press pos y.
            if(widgetPressHeight != widget->height())
            {
                if(widgetMousePressPos.y() + relativeMoveY < widget->pos().y())
                {
                    widgetMousePressPos.setY(widget->pos().y());
                }
                {
                    widgetMousePressPos.setY(widgetMousePressPos.y() + relativeMoveY);
                }
                widgetPressHeight = widget->height();
            }
        }
    }
}

//Layout resize.
template<typename T>
void MainWindow::layoutResize(QBoxLayout *ancestor, T *our, T *opposing, int relativeMoveY)
{
    int count = ancestor->count();
    int ourIndex = ancestor->indexOf(our);
    int opposingIndex = ancestor->indexOf(opposing);
    QBoxLayout::Direction direction = ancestor->direction();

    for(int index = 0; index < count; ++index)
    {
        if(ourIndex == index)
        {
            layoutItemResize(ancestor, index, relativeMoveY);
        }
        else if(opposingIndex == index)
        {
            layoutItemResize(ancestor, index, -relativeMoveY);
        }
        else if(direction == QBoxLayout::LeftToRight || direction == QBoxLayout::RightToLeft)
        {
            layoutItemResize(ancestor, index, relativeMoveY);
        }
    }
}

//Layout item resize.
void MainWindow::layoutItemResize(QBoxLayout *ancestor, int index, int relativeMoveY)
{
    int stretch = ancestor->itemAt(index)->geometry().height() + relativeMoveY;
    ancestor->setStretch(index, stretch);
}

bool MainWindow::isJsonString(QString name, QString string)
{
    QString capitalName = name[0].toUpper() + name.mid(1);
    if(isEmptyStringError(name, string))
    {
        return false;
    }
    else if((string.startsWith("[") && string.endsWith("]")) || (string.startsWith("{") && string.endsWith("}")))
    {
        QJsonParseError error;
        QJsonDocument json = QJsonDocument::fromJson(string.toUtf8(), &error);
        if(error.error == QJsonParseError::NoError)
        {
            ui->stackedWidgetResponse->setCurrentWidget(ui->pageResponse);
            if(json.isArray())
            {
                if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestWidget)
                {
                    QJsonArray array = json.array();
                    requestJson[name] = array;
                    qDebug() << name + ": " + string;
                }
                if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestJson)
                {
                    requestDocument = json;
                }
                qDebug() << capitalName + tr(" is array.");
                return true;
            }
            if(json.isObject())
            {
                if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestWidget)
                {
                    QJsonObject object = json.object();
                    requestJson[name] = object;
                    qDebug() << name + ": " + string;
                }
                if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestJson)
                {
                    requestDocument = json;
                }
                qDebug() << capitalName + tr(" is object.");
                return true;
            }
        }
        else
        {
            ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
            outputError(name + tr(" parse error: ") + error.errorString());
            outputError(name + ": " + string);
            ui->pushButtonSend->setEnabled(true);
            return false;
        }
    }
    else
    {
        ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
        outputError(tr("Error, ") + name + tr(" type only supports array and object."));
        outputError(name + ": " + string);
        ui->pushButtonSend->setEnabled(true);
        return false;
    }
}

//Empty string error.
bool MainWindow::isEmptyStringError(QString name, QString string)
{
    if(string.isEmpty())
    {
        ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
        outputError(tr("Please enter ") + name + tr("."));
        ui->pushButtonSend->setEnabled(true);
        return true;
    }
    return false;
}

//Output error.
void MainWindow::outputError(QString error)
{
    ui->textBrowserError->append(error);
    qDebug() << error;
}

//Menu action quit.
void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

//Menu action clear.
void MainWindow::on_actionClear_triggered()
{
    on_pushButtonClear_clicked();
}

//Chinese select.
void MainWindow::on_actionChinese_triggered()
{
    if(defaultLanguage != QLocale::Chinese)
    {
        translateLanguage(QLocale::Chinese);
    }
}

//English select.
void MainWindow::on_actionEnglish_triggered()
{
    if(defaultLanguage != QLocale::English)
    {
        translateLanguage(QLocale::English);
    }
}

//About popup.
void MainWindow::on_actionAbout_triggered()
{
    about->show();
}

//checkBoxRequestIdNull radio.
void MainWindow::on_checkBoxRequestIdNull_toggled(bool checked)
{
    if(checked)
    {
        ui->checkBoxRequestIdInt->setChecked(false);
        ui->checkBoxRequestIdString->setChecked(false);
        qDebug() << "Check box request ID NULL.";
    }
}

//checkBoxRequestIdInt radio.
void MainWindow::on_checkBoxRequestIdInt_toggled(bool checked)
{
    if(checked)
    {
        ui->checkBoxRequestIdNull->setChecked(false);
        ui->checkBoxRequestIdString->setChecked(false);
        qDebug() << "Check box request ID int.";
    }
}

//checkBoxRequestIdString radio.
void MainWindow::on_checkBoxRequestIdString_toggled(bool checked)
{
    if(checked)
    {
        ui->checkBoxRequestIdNull->setChecked(false);
        ui->checkBoxRequestIdInt->setChecked(false);
        qDebug() << "Check box request ID string.";
    }
}

void MainWindow::on_pushButtonSend_clicked()
{
    qDebug() << tr("Click send.");
    ui->pushButtonSend->setEnabled(false);
    isReplyDone = false;
    on_pushButtonClear_clicked();
    QString url = ui->lineEditUrl->text().trimmed();
    if(isEmptyStringError("url", url))
    {
        return;
    }
    else
    {
        ui->lineEditUrl->setText(url);
    }
    QUrl requestUrl(url);
    QString requestScheme = requestUrl.scheme();
    if(requestScheme.isEmpty())
    {
        url = "http://" + url;
        ui->lineEditUrl->setText(url);
        requestUrl.setUrl(url);
    }
    if(requestUrl.isValid())
    {
        qDebug() << "URL: " << requestUrl.toString();
    }
    else
    {
        qDebug() << tr("URL invalid.");
        ui->pushButtonSend->setEnabled(true);
        return;
    }

    QString userName = ui->lineEditUserName->text().trimmed();
    ui->lineEditUserName->setText(userName);
    if (!userName.isEmpty())
    {
        requestUrl.setUserName(userName);
        qDebug() << "UserName: " << requestUrl.userName();
    }

    QString passWord = ui->lineEditPassWord->text().trimmed();
    ui->lineEditPassWord->setText(passWord);
    if (!passWord.isEmpty())
    {
        requestUrl.setPassword(passWord);
        qDebug() << "PassWord: " << requestUrl.password().fill('*');
    }

    if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestWidget)
    {

        if(ui->radioButtonRequestVersion2->isChecked())
        {
            requestJson["jsonrpc"] = QString("2.0");
            qDebug() << QString("jsonrpc") << ": " << QString("2.0");
        }
        else
        {
            qDebug() << QString("jsonrpc") << ": " << QString("1.0");
        }

        QString method = ui->lineEditMethod->text().trimmed();
        if(isEmptyStringError("method", method))
        {
            return;
        }
        else
        {
            ui->lineEditMethod->setText(method);
            requestJson["method"] = method;
            qDebug() << QString("method") << ": " << method;
        }

        QString params = ui->plainTextEditParams->toPlainText().trimmed();
        if(!isJsonString("params", params))
        {
            return;
        }

        if(ui->checkBoxRequestIdNull->isChecked())
        {
            requestJson["id"] = QJsonValue();
            qDebug() << QString("id") << ": " << QString("NULL");
        }
        if(ui->checkBoxRequestIdInt->isChecked())
        {
            int requestIdInt = ui->spinBoxRequestId->value();
            requestJson["id"] = requestIdInt;
            qDebug() << QString("id") << ": " << requestIdInt;
        }
        if(ui->checkBoxRequestIdString->isChecked())
        {
            QString requestIdString = ui->lineEditRequestId->text().trimmed();
            ui->lineEditRequestId->setText(requestIdString);
            if(requestIdString.isEmpty())
            {
                ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
                outputError(tr("Please enter ID."));
                ui->pushButtonSend->setEnabled(true);
                return;
            }
            else
            {
                QString requestIdString = ui->lineEditRequestId->text().trimmed();
                ui->lineEditRequestId->setText(requestIdString);
                requestJson["id"] = requestIdString;
                qDebug() << QString("id") << ": " << QString(requestIdString);
            }
        }
    }
    else if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestJson)
    {
        QString json = ui->plainTextEditRequest->toPlainText().trimmed();
        if(!isJsonString("json", json))
        {
            return;
        }
    }

    if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestWidget)
    {
        requestDocument.setObject(requestJson);
    }
    QByteArray requestData = requestDocument.toJson();
    if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestWidget)
    {
        ui->plainTextEditRequest->setPlainText(requestData);
    }
    qDebug() << tr("Request data: ") << requestData;

    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    reply = manager->post(request, requestData);

    connect(reply, SIGNAL(finished()), this, SLOT(finished()), Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
    connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
    connect(reply, SIGNAL(redirected(QUrl)), this, SLOT(redirected(QUrl)), Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(errorOccurred(QNetworkReply::NetworkError)), Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)), Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
}

void MainWindow::on_pushButtonStop_clicked()
{
    qDebug() << tr("Click stop.");
    if(reply && !isReplyDone)
    {
        reply->close();
        reply->deleteLater();
        isReplyDone = false;
        ui->pushButtonSend->setEnabled(true);
    }
}

void MainWindow::on_pushButtonClear_clicked()
{
    qDebug() << tr("Click clear.");
    manager->clearAccessCache();
    ui->stackedWidgetResponse->setCurrentWidget(ui->pageResponse);
    ui->stackedWidgetResult->setCurrentWidget(ui->pageResult);
    if(ui->tabWidgetRequest->currentWidget() == ui->tabRequestWidget)
    {
        ui->plainTextEditRequest->clear();
    }
    ui->radioButtonResponseVersion1->setChecked(false);
    ui->radioButtonResponseVersion1->setChecked(false);
    ui->textBrowserResult->clear();
    ui->lineEditCode->clear();
    ui->textBrowserMessage->clear();
    ui->textBrowserData->clear();
    ui->textBrowserResponse->clear();
    ui->textBrowserError->clear();
    ui->checkBoxResponseIdNull->setChecked(false);
    ui->checkBoxResponseIdInt->setChecked(false);
    ui->spinBoxResponseId->setValue(0);
    ui->checkBoxResponseIdString->setChecked(false);
    ui->lineEditResponseId->clear();
    if(ui->stackedWidgetResponse->indexOf(ui->pageResultError) != -1)
    {
        ui->stackedWidgetResult->removeWidget(ui->pageResultError);
    }
}

void MainWindow::finished()
{
    if(reply->error() == QNetworkReply::NoError)
    {
        ui->stackedWidgetResponse->setCurrentWidget(ui->pageResponse);
        qDebug() << tr("Reply finished.");
    }
    else
    {
        ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
        outputError(tr("Reply error."));
    }
    reply->deleteLater();
    isReplyDone = true;
    ui->pushButtonSend->setEnabled(true);
}

void MainWindow::readyRead()
{
    QByteArray responseData = reply->readAll();
    QJsonParseError jsonError;
    QJsonDocument responseDocument = QJsonDocument::fromJson(responseData, &jsonError);
    responseData = responseDocument.toJson();
    qDebug() << tr("Reply data: ") << responseData;

    reply->deleteLater();
    isReplyDone = true;

    if(responseDocument.isNull())
    {
        ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
        outputError(tr("Response JSON is NULL."));
        ui->textBrowserError->append(responseData);
        return;
    }

    if(responseDocument.isEmpty())
    {
        ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
        outputError(tr("Response JSON is empty."));
        ui->textBrowserError->append(responseData);
        return;
    }

    if(jsonError.error == QJsonParseError::NoError)
    {
        ui->stackedWidgetResponse->setCurrentWidget(ui->pageResponse);
        ui->textBrowserResponse->append(responseData);
        qDebug() << tr("Response JSON not parse error.");
    }
    else
    {
        ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
        outputError(tr("Response JSON parse error: ") + jsonError.errorString());
        ui->textBrowserError->append(responseData);
        return;
    }

    if(responseDocument.isArray())
    {
        qDebug() << tr("Response JSON is Array.");
        return;
    }

    if(responseDocument.isObject())
    {
        QJsonObject responseJson = responseDocument.object();
        qDebug() << tr("Response JSON is object.");

        if(responseJson.contains("jsonrpc"))
        {
            QString requestVersion = responseJson["jsonrpc"].toString().trimmed();
            if(requestVersion == "1.0")
            {
                ui->radioButtonResponseVersion1->setChecked(true);
            }
            if(requestVersion == "2.0")
            {
                ui->radioButtonResponseVersion2->setChecked(true);
            }
            qDebug() << QString("jsonrpc") << ": " << requestVersion;
        }

        if(responseJson.contains("result"))
        {
            ui->stackedWidgetResponse->setCurrentWidget(ui->pageResponse);
            ui->stackedWidgetResult->setCurrentWidget(ui->pageResult);
            QString result = responseJson["result"].toString().trimmed();
            ui->textBrowserResult->append(result);
            qDebug() << QString("result") << ": " << result;
        }

        if(responseJson.contains("error"))
        {
            ui->stackedWidgetResponse->setCurrentWidget(ui->pageResponse);
            if(ui->stackedWidgetResponse->indexOf(ui->pageResultError) == -1)
            {
                ui->stackedWidgetResult->addWidget(ui->pageResultError);
            }
            ui->stackedWidgetResult->setCurrentWidget(ui->pageResultError);
            QJsonValue errorValue = responseJson["error"];
            if(errorValue.isObject())
            {
                QJsonObject errorJson = errorValue.toObject();

                if(errorJson.contains("code"))
                {
                    if(errorJson["code"].isNull())
                    {
                        qDebug() << tr("Code absent.");
                    }
                    else
                    {
                        int code = errorJson["code"].toInt();
                        ui->lineEditCode->setText(QString::number(code));
                        qDebug() << QString("code") << ": " << code;
                    }
                }

                if(errorJson.contains("message"))
                {
                    if(errorJson["message"].isNull())
                    {
                        qDebug() << "Message absent.";
                    }
                    if(errorJson["message"].isString())
                    {
                        QString message = errorJson["message"].toString();
                        ui->textBrowserMessage->setText(message);
                        qDebug() << QString("message") << ": " << message;
                    }
                }
                else
                {
                    qDebug() << tr("Message absent.");
                }

                if(errorJson.contains("data"))
                {
                    if(errorJson["data"].isNull())
                    {
                        qDebug() << tr("Data absent.");
                    }
                    if(errorJson["data"].isString())
                    {
                        QString data = errorJson["data"].toString();
                        ui->textBrowserData->setText(data);
                        qDebug() << QString("Data") << ": " << data;
                    }
                }
                else
                {
                    qDebug() << tr("Data absent.");
                }
            }
        }

        if(responseJson.contains("id"))
        {
            if(responseJson["id"].isNull())
            {
                ui->checkBoxResponseIdNull->setChecked(true);
                qDebug() << QString("id") << ": " << QString("NULL");
            }
            else if(responseJson["id"].isString())
            {
                ui->checkBoxResponseIdString->setChecked(true);
                QString responseIdString = responseJson["id"].toString().trimmed();
                ui->lineEditResponseId->setText(responseIdString);
                qDebug() << QString("id") << ": " << responseIdString;
            }
            else
            {
                QVariant responseIdVariant = responseJson["id"].toVariant();
                bool isInt;
                int responseIdInt = responseIdVariant.toInt(&isInt);
                if(isInt)
                {
                    ui->checkBoxResponseIdInt->setChecked(true);
                    ui->spinBoxResponseId->setValue(responseIdInt);
                    qDebug() << QString("id") << ": " << responseIdInt;
                }
            }
        }
        else
        {
            qDebug() << tr("ID absent.");
        }
    }
    ui->pushButtonSend->setEnabled(true);
}

void MainWindow::authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
    if(authenticator->isNull())
    {
        outputError(tr("Please enter username and password."));
    }
    else if(authenticator->user().isEmpty())
    {
        outputError(tr("Please enter username."));
    }
    else if(authenticator->password().isEmpty())
    {
        outputError(tr("Please enter password."));
    }
    else
    {
        outputError(tr("Authentication error."));
    }
    reply->deleteLater();
    isReplyDone = true;
    ui->pushButtonSend->setEnabled(true);
}

void MainWindow::redirected(const QUrl &url)
{
    ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
    ui->textBrowserError->append("URL redirected: " + url.toString());
    qDebug() << tr("URL redirected:") << url.toString();
    reply->deleteLater();
    isReplyDone = true;
    ui->pushButtonSend->setEnabled(true);
}

void MainWindow::errorOccurred(QNetworkReply::NetworkError code)
{
    ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
    outputError(tr("Error ") + QString::number(code) + ": " + reply->errorString());
    reply->deleteLater();
    isReplyDone = true;
    ui->pushButtonSend->setEnabled(true);
}

void MainWindow::sslErrors(const QList<QSslError> &errors)
{
    ui->stackedWidgetResponse->setCurrentWidget(ui->pageError);
    int count = errors.count();
    for(int i = 0; i < count; i++)
    {
        outputError(tr("SSL error: ") + errors[i].errorString());
    }
    reply->deleteLater();
    isReplyDone = true;
    ui->pushButtonSend->setEnabled(true);
}
