#pragma once
#include <QWidget>

class QPushButton;
class MainWindow; // 功能 A
class BWindow;    // 功能 B

class StartWindow : public QWidget
{
    Q_OBJECT
public:
    explicit StartWindow(QWidget *parent = nullptr);

    void openA();
    void openB();

private:
    QPushButton *m_btnA = nullptr;
    QPushButton *m_btnB = nullptr;

    MainWindow *m_a = nullptr;
    BWindow *m_b = nullptr;
};
