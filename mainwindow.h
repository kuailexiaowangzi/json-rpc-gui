#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "about.h"

#include <QMainWindow>
#include <QSettings>
#include <QActionGroup>
#include <QTranslator>
#include <QBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    About *about;
    QSettings *configFile;
    QActionGroup *languageActionGroup;
    QTranslator translator;
    QTranslator translatorWidgets;
    QTranslator translatorNetwork;
    enum QLocale::Language defaultLanguage;
    int widgetSide;
    QPoint widgetMousePressPos;
    int widgetPressHeight;
    QWidget* widgetMousePress;
    enum class widgetTopBottomMargin
    {
        none,
        topMargin,
        bottomMargin
    };
    enum widgetTopBottomMargin widgetMouseMargin;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    bool isReplyDone;
    QJsonDocument requestDocument;
    QJsonObject requestJson;

    void languageInit();
    void translateLanguage(enum QLocale::Language language);
    void widgetSelectMargin(QWidget *widget, QEvent *event);
    template<typename T>
    void widgetTopBottomMarginResize(QWidget *widget, const QEvent *event, enum widgetTopBottomMargin margin, QBoxLayout *ancestor, T *our, T *opposing);
    template<typename T>
    void layoutResize(QBoxLayout *ancestor, T *our, T *opposing, int relativeMoveY);
    void layoutItemResize(QBoxLayout *ancestor, int index, int relativeMoveY);
    bool isJsonString(QString name, QString string);
    bool isEmptyStringError(QString name, QString string);
    void outputError(QString error);

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void on_actionQuit_triggered();
    void on_actionClear_triggered();
    void on_actionChinese_triggered();
    void on_actionEnglish_triggered();
    void on_actionAbout_triggered();
    void on_checkBoxRequestIdNull_toggled(bool checked);
    void on_checkBoxRequestIdInt_toggled(bool checked);
    void on_checkBoxRequestIdString_toggled(bool checked);
    void on_pushButtonSend_clicked();
    void on_pushButtonStop_clicked();
    void on_pushButtonClear_clicked();
    void finished();
    void readyRead();
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void redirected(const QUrl &url);
    void errorOccurred(QNetworkReply::NetworkError code);
    void sslErrors(const QList<QSslError> &errors);
};

#endif // MAINWINDOW_H
