#include "StartWindow.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "mainwindow.h"
#include "BWindow.h"

StartWindow::StartWindow(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel("請選擇功能", this);
    title->setAlignment(Qt::AlignCenter);

    m_btnA = new QPushButton("功能 A : 計算胡牌台數", this);
    m_btnB = new QPushButton("功能 B : 可碰 可吃 捨牌後進張提示", this);

    m_btnA->setMinimumHeight(44);
    m_btnB->setMinimumHeight(44);

    layout->addWidget(title);
    layout->addWidget(m_btnA);
    layout->addWidget(m_btnB);

    setLayout(layout);
    setWindowTitle("麻將功能選擇");
    resize(360, 200);

    connect(m_btnA, &QPushButton::clicked, this, &StartWindow::openA);
    connect(m_btnB, &QPushButton::clicked, this, &StartWindow::openB);
}

void StartWindow::openA()
{
    if (!m_a)
    {
        m_a = new MainWindow;
        m_a->setStartWindow(this);
    }
    hide();
    m_a->show();
    m_a->raise();
    m_a->activateWindow();
}

void StartWindow::openB()
{
    if (!m_b)
    {
        m_b = new BWindow;
        m_b->setStartWindow(this);
    }
    hide();
    m_b->show();
    m_b->raise();
    m_b->activateWindow();
}
