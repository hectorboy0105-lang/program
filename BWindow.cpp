#include "BWindow.h"

#include "StartWindow.h"
#include "mainwindow.h"      // 功能 A
#include "TileButton.h"
#include "MahjongScorer.h"

#include <QToolBar>
#include <QAction>
#include <QCloseEvent>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QFont>

BWindow::BWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("功能 B：可碰 / 可吃 / 捨牌進張提示");
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto *tb = addToolBar("功能切換");
    tb->setMovable(false);

    auto *actA = tb->addAction("功能 A");
    auto *actB = tb->addAction("功能 B");
    auto *actHome = tb->addAction("回選單");

    connect(actA, &QAction::triggered, this, &BWindow::goA);
    connect(actB, &QAction::triggered, this, &BWindow::goB);
    connect(actHome, &QAction::triggered, this, &BWindow::goStart);

    auto *root = new QWidget(this);
    setCentralWidget(root);

    auto *outer = new QVBoxLayout(root);

    auto *main = new QHBoxLayout();
    outer->addLayout(main , 1);

    auto *left = new QWidget(root);
    auto *lv = new QVBoxLayout(left);

    lv->addWidget(buildTileGroup(left) , 1);
    lv->addWidget(buildPanelB(left));
    main->addWidget(left , 2);

    auto *right = new QWidget(root);
    auto *rv = new QVBoxLayout(right);

    rv->addWidget(new QLabel("輸出" , right));

    txtOutput = new QTextEdit(right);
    txtOutput->setReadOnly(true);
    rv->addWidget(txtOutput , 1);

    main->addWidget(right , 3);

    refreshCountLabel();

    txtOutput->setPlainText(
        "目前模式  B\n"
        "請選 16 張非花牌  花牌另選但不計入 16\n"
        "按下執行 B 會列出可碰 可吃 與每一張捨牌對應的進張清單"
        );
}

void BWindow::setStartWindow(StartWindow *sw)
{
    m_startWindow = sw;
}

QWidget *BWindow::buildTileGroup(QWidget *parent)
{
    auto *gb = new QGroupBox("選牌區  左鍵加一張  右鍵減一張" , parent);
    auto *grid = new QGridLayout(gb);

    tileButtons.clear();
    tileButtons.reserve(42);

    auto addHeader = [&](const QString &t , int &r)
    {
        auto *lab = new QLabel(t , gb);
        QFont f = lab->font();
        f.setBold(true);
        lab->setFont(f);
        grid->addWidget(lab , r , 0 , 1 , 6);
        r += 1;
    };

    auto addRange = [&](int from , int to , int cols , int maxCount , int &r)
    {
        int c = 0;
        for (int i = from; i <= to; ++i)
        {
            auto *btn = new TileButton(i , MahjongScorer::tileName42(i) , maxCount , gb);
            tileButtons.push_back(btn);

            connect(btn , &TileButton::countChanged , this , [this](int , int) { refreshCountLabel(); });

            int col = (c % cols);
            int rr = r + (c / cols);
            grid->addWidget(btn , rr , col);
            c += 1;
        }
        r += (((to - from + 1) + cols - 1) / cols);
    };

    int r = 0;
    addHeader("萬子" , r); addRange(0 , 8 , 6 , 4 , r);
    addHeader("筒子" , r); addRange(9 , 17 , 6 , 4 , r);
    addHeader("條子" , r); addRange(18 , 26 , 6 , 4 , r);
    addHeader("風牌與三元牌" , r); addRange(27 , 33 , 6 , 4 , r);
    addHeader("花牌" , r); addRange(34 , 41 , 6 , 1 , r);

    auto *row = new QHBoxLayout();

    auto *btnClear = new QPushButton("清空牌面" , gb);
    btnClear->setMinimumHeight(34);

    labCount = new QLabel("" , gb);

    row->addWidget(btnClear);
    row->addWidget(labCount);
    row->addStretch(1);

    grid->addLayout(row , r , 0 , 1 , 6);

    connect(btnClear , &QPushButton::clicked , this , &BWindow::clearAllTiles);

    return gb;
}

QWidget *BWindow::buildPanelB(QWidget *parent)
{
    auto *panel = new QWidget(parent);
    auto *pv = new QVBoxLayout(panel);

    auto *gb = new QGroupBox("功能 B 說明" , panel);
    auto *gbv = new QVBoxLayout(gb);

    gbv->addWidget(new QLabel(
        "非花牌需剛好 16 張  花牌不算在 16 張內\n"
        "輸出會列出可碰  可吃  以及每一張捨牌對應的進張清單"
        , gb));

    pv->addWidget(gb);

    btnRunB = new QPushButton("執行 B  分析吃碰與進張" , panel);
    btnRunB->setMinimumHeight(44);
    pv->addWidget(btnRunB);

    connect(btnRunB , &QPushButton::clicked , this , &BWindow::runB);

    pv->addStretch(1);
    return panel;
}

void BWindow::refreshCountLabel()
{
    int totalNoFlower = 0;
    int totalFlower = 0;

    for (int i = 0; i < tileButtons.size(); ++i)
    {
        int c = tileButtons[i]->count();
        if (i >= 34) totalFlower += c;
        else totalNoFlower += c;
    }

    if (labCount)
        labCount->setText(QString("非花張數 = %1  花牌張數 = %2").arg(totalNoFlower).arg(totalFlower));
}

void BWindow::clearAllTiles()
{
    for (auto *b : tileButtons) b->setCount(0);
    refreshCountLabel();
}

void BWindow::runB()
{
    QVector<int> c34;
    QVector<int> f8;
    c34.resize(34);
    f8.resize(8);

    for (int i = 0; i < 34; ++i) c34[i] = tileButtons[i]->count();
    for (int i = 0; i < 8; ++i) f8[i] = tileButtons[34 + i]->count();

    AnalyzeOutput out = MahjongScorer::analyzeB(c34 , f8);

    int handCount = 0;
    for (int v : c34) handCount += v;

    int flowerCount = 0;
    for (int v : f8) flowerCount += v;

    QString text;
    text += "功能 B 結果\n\n";
    text += QString("非花手牌張數  :  %1\n").arg(handCount);
    text += QString("花牌張數  :  %1\n\n").arg(flowerCount);

    if (!out.warning.isEmpty())
    {
        text += "提醒\n";
        text += out.warning;
        text += "\n\n";
    }

    text += "可碰\n";
    for (const auto &s : out.pongTiles) text += s + "\n";
    text += "\n";

    text += "可吃\n";
    for (const auto &s : out.chiOptions) text += s + "\n";
    text += "\n";

    text += "捨牌進張\n";
    for (const auto &w : out.waitsByDiscard)
    {
        text += QString("捨  %1  :  進張  =  ").arg(w.discardTile);
        text += w.drawTiles.join(" , ");
        text += "\n";
    }

    if (txtOutput) txtOutput->setPlainText(text);
}

void BWindow::goStart()
{
    if (!m_startWindow) return;
    hide();
    m_startWindow->show();
    m_startWindow->raise();
    m_startWindow->activateWindow();
}

void BWindow::goA()
{
    if (!m_startWindow) return;
    hide();
    m_startWindow->openA();
}

void BWindow::goB()
{
    // 已在功能 B
}

void BWindow::closeEvent(QCloseEvent *e)
{
    if (m_startWindow)
    {
        hide();
        m_startWindow->show();
        m_startWindow->raise();
        m_startWindow->activateWindow();
        e->ignore();
        return;
    }
    e->accept();
}
