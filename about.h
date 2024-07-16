#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QLocale>
#include <QPoint>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();
    void translate();

private:
    Ui::About *ui;
    QWidget *window;
    bool isTitleBarMousePress;
    QPoint titleBarPressPos;
    enum QLocale::Language language;

    QRect mapToWindowRect(const QWidget *widget);

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event);
private slots:
    void on_pushButtonClose_clicked();
    void on_pushButtonOk_clicked();
    void openUrl(const QUrl &url);
};

#endif // ABOUT_H
