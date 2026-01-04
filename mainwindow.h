#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QSplitter>
#include <QMouseEvent>
#include <QGroupBox>
#include <QCloseEvent>

class StartWindow;
#include <QListWidget>
#include <array>
#include <vector>
#include "MahjongCore.h"

class ASquareButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ASquareButton(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
};


class HandTileButton : public ASquareButton
{
    Q_OBJECT
public:
    explicit HandTileButton(int index , QWidget *parent = nullptr)
        : ASquareButton(parent) , m_index(index) {}

signals:
    void leftClicked(int index);
    void rightClicked(int index);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
            emit leftClicked(m_index);
        else if (event->button() == Qt::RightButton)
            emit rightClicked(m_index);
    }

private:
    int m_index;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setStartWindow(StartWindow *sw) { m_startWindow = sw; }

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void goStart();
    void goA();
    void goB();


private:
    QComboBox *m_dealerSeat = nullptr;
    QComboBox *m_selfSeat = nullptr;

    QSpinBox *m_d1 = nullptr;
    QSpinBox *m_d2 = nullptr;
    QSpinBox *m_d3 = nullptr;

    QLabel *m_countInfo = nullptr;
    QLabel *m_openLine = nullptr;
    QLabel *m_handLine = nullptr;

    QTextEdit *m_output = nullptr;

    QPushButton *m_reset = nullptr;
    QSplitter *m_splitter = nullptr;

    std::array<int , 34> m_countsHand {};
    std::array<int , 8>  m_flowers {};

    std::vector<mj::OpenMeld> m_openMelds;

    std::array<HandTileButton* , 34> m_tileBtns {};
    std::array<HandTileButton* , 8>  m_flowerBtns {};

    QComboBox *m_openType = nullptr;
    QComboBox *m_openTile = nullptr;
    QComboBox *m_chowStartTile = nullptr;

    QPushButton *m_addOpenMeld = nullptr;
    QPushButton *m_removeOpenMeld = nullptr;
    QListWidget *m_openList = nullptr;

    StartWindow *m_startWindow = nullptr;

    void buildUi();
    void updateAllButtonText();

    std::array<int , 34> countsOpenFromMelds() const;
    std::array<int , 34> mergedCounts() const;

    int kongCount() const;
    int requiredTotalTiles() const;
    int basicTileTotal() const;

    void addHandTile(int idx);
    void subHandTile(int idx);

    void addFlower(int idx);
    void subFlower(int idx);

    void onOpenTypeChanged();
    void onAddOpenMeld();
    void onRemoveOpenMeld();

    void refreshScore();
    void resetAll();
};
