#include "mainwindow.h"
#include "StartWindow.h"
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>
#include <QResizeEvent>
#include <QtGlobal>

// 這段是為了修正：undefined reference to ASquareButton::ASquareButton(QWidget*)
ASquareButton::ASquareButton(QWidget *parent)
    : QPushButton(parent)
{
}

void ASquareButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    int side = qMin(width(), height());
    setFixedSize(side, side);
}

static QString buildTilesText(const std::array<int , 34> &counts)
{
    QStringList out;

    auto add = [&](const QString &name , int n)
    {
        for (int i = 0; i < n; ++i)
            out << name;
    };

    for (int i = 0; i < 34; ++i)
        add(mj::tileName(i) , counts[i]);

    return out.join(" ");
}

static QString openMeldToText(const mj::OpenMeld &m)
{
    if (m.type == mj::MeldType::Pung)
        return "碰 " + mj::tileName(m.tileIndex);

    if (m.type == mj::MeldType::Kong)
        return "槓 " + mj::tileName(m.tileIndex);

    int s = m.tileIndex;
    return "吃 " + mj::tileName(s) + " " + mj::tileName(s + 1) + " " + mj::tileName(s + 2);
}

static void applyTileStyle(QPushButton *b , int totalCount)
{
    if (!b) return;

    if (totalCount <= 0)
    {
        b->setStyleSheet(
            "QPushButton { background : #E0E0E0; color : #666666; border : 1px solid #BDBDBD; border-radius : 8px; }"
            "QPushButton:pressed { background : #D6D6D6; }"
            );
        return;
    }

    if (totalCount >= 4)
    {
        b->setStyleSheet(
            "QPushButton { background : #F7D6D6; color : #7A1C1C; border : 2px solid #D32F2F; border-radius : 8px; font-weight : 700; }"
            "QPushButton:pressed { background : #F0C3C3; }"
            );
        return;
    }

    b->setStyleSheet(
        "QPushButton { background : #FFFFFF; color : #111111; border : 1px solid #BDBDBD; border-radius : 8px; }"
        "QPushButton:pressed { background : #F2F2F2; }"
        );
}

static void applyFlowerStyle(QPushButton *b , int count)
{
    if (!b) return;

    if (count <= 0)
    {
        b->setStyleSheet(
            "QPushButton { background : #E0E0E0; color : #666666; border : 1px solid #BDBDBD; border-radius : 8px; }"
            "QPushButton:pressed { background : #D6D6D6; }"
            );
        return;
    }

    b->setStyleSheet(
        "QPushButton { background : #FFFFFF; color : #111111; border : 1px solid #BDBDBD; border-radius : 8px; }"
        "QPushButton:pressed { background : #F2F2F2; }"
        );
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 讓視窗不真正關閉，用來快速切換 A / B
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto *tb = addToolBar("功能切換");
    tb->setMovable(false);
    auto *actA = tb->addAction("功能 A");
    auto *actB = tb->addAction("功能 B");
    auto *actHome = tb->addAction("回選單");
    connect(actA, &QAction::triggered, this, &MainWindow::goA);
    connect(actB, &QAction::triggered, this, &MainWindow::goB);
    connect(actHome, &QAction::triggered, this, &MainWindow::goStart);

    m_countsHand.fill(0);
    m_flowers.fill(0);

    buildUi();
    updateAllButtonText();
    refreshScore();
}

int MainWindow::kongCount() const
{
    int k = 0;
    for (const auto &m : m_openMelds)
        if (m.type == mj::MeldType::Kong) k += 1;
    return k;
}

int MainWindow::requiredTotalTiles() const
{
    return 17 + kongCount();
}

std::array<int , 34> MainWindow::countsOpenFromMelds() const
{
    std::array<int , 34> c {};
    c.fill(0);

    for (const auto &m : m_openMelds)
    {
        if (m.type == mj::MeldType::Chow)
        {
            int s = m.tileIndex;
            c[s] += 1;
            c[s + 1] += 1;
            c[s + 2] += 1;
        }
        else if (m.type == mj::MeldType::Pung)
        {
            c[m.tileIndex] += 3;
        }
        else
        {
            c[m.tileIndex] += 4;
        }
    }

    return c;
}

std::array<int , 34> MainWindow::mergedCounts() const
{
    auto open = countsOpenFromMelds();

    std::array<int , 34> out {};
    out.fill(0);

    for (int i = 0; i < 34; ++i)
        out[i] = m_countsHand[i] + open[i];

    return out;
}

int MainWindow::basicTileTotal() const
{
    int s = 0;
    auto total = mergedCounts();
    for (int i = 0; i < 34; ++i) s += total[i];
    return s;
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);

    m_splitter = new QSplitter(Qt::Vertical);
    root->addWidget(m_splitter , 1);

    auto *topPane = new QWidget();
    auto *topRoot = new QVBoxLayout(topPane);

    auto *top = new QHBoxLayout();
    top->addWidget(new QLabel("莊家"));
    m_dealerSeat = new QComboBox();
    m_dealerSeat->addItems({ "東" , "南" , "西" , "北" });
    top->addWidget(m_dealerSeat);

    top->addWidget(new QLabel("自己"));
    m_selfSeat = new QComboBox();
    m_selfSeat->addItems({ "東" , "南" , "西" , "北" });
    top->addWidget(m_selfSeat);

    top->addWidget(new QLabel("骰子"));
    m_d1 = new QSpinBox(); m_d1->setRange(1 , 6);
    m_d2 = new QSpinBox(); m_d2->setRange(1 , 6);
    m_d3 = new QSpinBox(); m_d3->setRange(1 , 6);
    top->addWidget(m_d1);
    top->addWidget(m_d2);
    top->addWidget(m_d3);

    top->addStretch(1);
    m_reset = new QPushButton("清空");
    top->addWidget(m_reset);
    topRoot->addLayout(top);

    m_countInfo = new QLabel();
    topRoot->addWidget(m_countInfo);

    auto *openBox = new QGroupBox("門前（碰 吃 槓 一次加入一組）");
    auto *openLay = new QVBoxLayout(openBox);

    auto *openCtl = new QHBoxLayout();
    m_openType = new QComboBox();
    m_openType->addItems({ "碰" , "吃" , "槓" });
    openCtl->addWidget(new QLabel("類型"));
    openCtl->addWidget(m_openType);

    m_openTile = new QComboBox();
    for (int i = 0; i < 34; ++i)
        m_openTile->addItem(mj::tileName(i) , i);
    openCtl->addWidget(new QLabel("牌"));
    openCtl->addWidget(m_openTile);

    m_chowStartTile = new QComboBox();
    for (int suit = 0; suit < 3; ++suit)
    {
        for (int start = 1; start <= 7; ++start)
        {
            int idx = suit * 9 + (start - 1);
            m_chowStartTile->addItem(mj::tileName(idx) , idx);
        }
    }
    openCtl->addWidget(new QLabel("起始牌"));
    openCtl->addWidget(m_chowStartTile);

    m_addOpenMeld = new QPushButton("加入門前組");
    m_removeOpenMeld = new QPushButton("刪除選取組");
    openCtl->addWidget(m_addOpenMeld);
    openCtl->addWidget(m_removeOpenMeld);

    openCtl->addStretch(1);
    openLay->addLayout(openCtl);

    m_openList = new QListWidget();
    openLay->addWidget(m_openList , 1);

    topRoot->addWidget(openBox);

    m_openLine = new QLabel();
    m_openLine->setWordWrap(true);
    topRoot->addWidget(m_openLine);

    m_handLine = new QLabel();
    m_handLine->setWordWrap(true);
    topRoot->addWidget(m_handLine);

    auto *gridWrap = new QHBoxLayout();

    auto *gridM = new QGridLayout();
    auto *gridP = new QGridLayout();
    auto *gridS = new QGridLayout();
    auto *gridH = new QGridLayout();
    auto *gridF = new QGridLayout();

    auto *left = new QVBoxLayout();
    left->addWidget(new QLabel("萬"));
    left->addLayout(gridM);
    left->addWidget(new QLabel("筒"));
    left->addLayout(gridP);
    left->addWidget(new QLabel("條"));
    left->addLayout(gridS);

    auto *right = new QVBoxLayout();
    right->addWidget(new QLabel("字"));
    right->addLayout(gridH);
    right->addWidget(new QLabel("花"));
    right->addLayout(gridF);

    gridWrap->addLayout(left , 3);
    gridWrap->addLayout(right , 2);
    topRoot->addLayout(gridWrap , 1);

    auto applyBtn = [](HandTileButton *b)
    {
        b->setMinimumSize(48 , 48);
    };

    for (int i = 0; i < 9; ++i)
    {
        auto *bm = new HandTileButton(i);
        applyBtn(bm);
        gridM->addWidget(bm , 0 , i);
        m_tileBtns[i] = bm;
        connect(bm , &HandTileButton::leftClicked , this , &MainWindow::addHandTile);
        connect(bm , &HandTileButton::rightClicked , this , &MainWindow::subHandTile);

        auto *bp = new HandTileButton(9 + i);
        applyBtn(bp);
        gridP->addWidget(bp , 0 , i);
        m_tileBtns[9 + i] = bp;
        connect(bp , &HandTileButton::leftClicked , this , &MainWindow::addHandTile);
        connect(bp , &HandTileButton::rightClicked , this , &MainWindow::subHandTile);

        auto *bs = new HandTileButton(18 + i);
        applyBtn(bs);
        gridS->addWidget(bs , 0 , i);
        m_tileBtns[18 + i] = bs;
        connect(bs , &HandTileButton::leftClicked , this , &MainWindow::addHandTile);
        connect(bs , &HandTileButton::rightClicked , this , &MainWindow::subHandTile);
    }

    for (int i = 27; i <= 33; ++i)
    {
        auto *b = new HandTileButton(i);
        applyBtn(b);
        gridH->addWidget(b , 0 , i - 27);
        m_tileBtns[i] = b;
        connect(b , &HandTileButton::leftClicked , this , &MainWindow::addHandTile);
        connect(b , &HandTileButton::rightClicked , this , &MainWindow::subHandTile);
    }

    for (int i = 0; i < 8; ++i)
    {
        auto *b = new HandTileButton(i);
        applyBtn(b);
        gridF->addWidget(b , i / 4 , i % 4);
        m_flowerBtns[i] = b;
        connect(b , &HandTileButton::leftClicked , this , &MainWindow::addFlower);
        connect(b , &HandTileButton::rightClicked , this , &MainWindow::subFlower);
    }

    m_splitter->addWidget(topPane);

    auto *bottomPane = new QWidget();
    auto *bottomRoot = new QVBoxLayout(bottomPane);
    bottomRoot->addWidget(new QLabel("結果"));
    m_output = new QTextEdit();
    m_output->setReadOnly(true);
    bottomRoot->addWidget(m_output , 1);

    m_splitter->addWidget(bottomPane);
    m_splitter->setStretchFactor(0 , 3);
    m_splitter->setStretchFactor(1 , 2);

    setCentralWidget(central);
    resize(1200 , 820);

    connect(m_reset , &QPushButton::clicked , this , &MainWindow::resetAll);
    connect(m_dealerSeat , &QComboBox::currentTextChanged , this , &MainWindow::refreshScore);
    connect(m_selfSeat , &QComboBox::currentTextChanged , this , &MainWindow::refreshScore);
    connect(m_d1 , qOverload<int>(&QSpinBox::valueChanged) , this , &MainWindow::refreshScore);
    connect(m_d2 , qOverload<int>(&QSpinBox::valueChanged) , this , &MainWindow::refreshScore);
    connect(m_d3 , qOverload<int>(&QSpinBox::valueChanged) , this , &MainWindow::refreshScore);

    connect(m_openType , &QComboBox::currentTextChanged , this , &MainWindow::onOpenTypeChanged);
    connect(m_addOpenMeld , &QPushButton::clicked , this , &MainWindow::onAddOpenMeld);
    connect(m_removeOpenMeld , &QPushButton::clicked , this , &MainWindow::onRemoveOpenMeld);

    onOpenTypeChanged();
}

void MainWindow::onOpenTypeChanged()
{
    bool isChow = (m_openType->currentText() == "吃");
    m_openTile->setEnabled(!isChow);
    m_chowStartTile->setEnabled(isChow);
}

void MainWindow::onAddOpenMeld()
{
    mj::OpenMeld m;

    QString t = m_openType->currentText();
    if (t == "碰") m.type = mj::MeldType::Pung;
    else if (t == "槓") m.type = mj::MeldType::Kong;
    else m.type = mj::MeldType::Chow;

    if (m.type == mj::MeldType::Chow)
        m.tileIndex = m_chowStartTile->currentData().toInt();
    else
        m.tileIndex = m_openTile->currentData().toInt();

    auto total = mergedCounts();

    auto canAddTile = [&](int idx , int add) -> bool
    {
        return (total[idx] + add) <= 4;
    };

    if (m.type == mj::MeldType::Chow)
    {
        int s = m.tileIndex;
        if (s < 0 || s >= 27) return;
        int pos = s % 9;
        if (pos > 6) return;

        if (!canAddTile(s , 1) || !canAddTile(s + 1 , 1) || !canAddTile(s + 2 , 1))
            return;
    }
    else if (m.type == mj::MeldType::Pung)
    {
        if (!canAddTile(m.tileIndex , 3)) return;
    }
    else
    {
        if (!canAddTile(m.tileIndex , 4)) return;
    }

    m_openMelds.push_back(m);
    m_openList->addItem(openMeldToText(m));

    updateAllButtonText();
    refreshScore();
}

void MainWindow::onRemoveOpenMeld()
{
    int row = m_openList->currentRow();
    if (row < 0 || row >= (int)m_openMelds.size()) return;

    m_openMelds.erase(m_openMelds.begin() + row);
    delete m_openList->takeItem(row);

    updateAllButtonText();
    refreshScore();
}

void MainWindow::updateAllButtonText()
{
    auto total = mergedCounts();

    for (int i = 0; i < 34; ++i)
    {
        if (!m_tileBtns[i]) continue;

        m_tileBtns[i]->setText(mj::tileName(i) + "\n×" + QString::number(m_countsHand[i]));
        applyTileStyle(m_tileBtns[i] , total[i]);
    }

    for (int i = 0; i < 8; ++i)
    {
        if (!m_flowerBtns[i]) continue;

        m_flowerBtns[i]->setText(mj::flowerName(i) + "\n×" + QString::number(m_flowers[i]));
        applyFlowerStyle(m_flowerBtns[i] , m_flowers[i]);
    }

    int totalTiles = basicTileTotal();
    int need = requiredTotalTiles();
    m_countInfo->setText("基本牌總張數（手牌 + 門前，不含花） : " + QString::number(totalTiles) + " / " + QString::number(need));

    QStringList openParts;
    for (const auto &m : m_openMelds)
        openParts << openMeldToText(m);

    m_openLine->setText("門前（組）：" + openParts.join(" | "));
    m_handLine->setText("手牌：" + buildTilesText(m_countsHand));
}

void MainWindow::addHandTile(int idx)
{
    if (idx < 0 || idx >= 34) return;

    int need = requiredTotalTiles();
    if (basicTileTotal() >= need) return;

    auto total = mergedCounts();
    if (total[idx] >= 4) return;

    m_countsHand[idx] += 1;
    updateAllButtonText();
    refreshScore();
}

void MainWindow::subHandTile(int idx)
{
    if (idx < 0 || idx >= 34) return;
    if (m_countsHand[idx] <= 0) return;

    m_countsHand[idx] -= 1;
    updateAllButtonText();
    refreshScore();
}

void MainWindow::addFlower(int idx)
{
    if (idx < 0 || idx >= 8) return;
    if (m_flowers[idx] >= 1) return;

    m_flowers[idx] += 1;
    updateAllButtonText();
    refreshScore();
}

void MainWindow::subFlower(int idx)
{
    if (idx < 0 || idx >= 8) return;
    if (m_flowers[idx] <= 0) return;

    m_flowers[idx] -= 1;
    updateAllButtonText();
    refreshScore();
}

void MainWindow::resetAll()
{
    m_countsHand.fill(0);
    m_flowers.fill(0);
    m_openMelds.clear();
    m_openList->clear();

    updateAllButtonText();
    refreshScore();
}

void MainWindow::refreshScore()
{
    QString detail;

    mj::Wind dealer = mj::parseWind(m_dealerSeat->currentText());
    mj::Wind selfSeat = mj::parseWind(m_selfSeat->currentText());

    int totalTiles = basicTileTotal();
    int need = requiredTotalTiles();

    if (totalTiles != need)
    {
        detail =
            "請讓 手牌 + 門前 的基本牌總數等於 " + QString::number(need) +
            "\n目前 : " + QString::number(totalTiles) + " / " + QString::number(need);
        m_output->setPlainText(detail);
        return;
    }

    auto total = mergedCounts();

    auto result = mj::scoreTaiwanMahjong(
        total ,
        m_openMelds ,
        m_flowers ,
        dealer ,
        selfSeat ,
        m_d1->value() ,
        m_d2->value() ,
        m_d3->value()
        );

    m_output->setPlainText(result.detail);
}

void MainWindow::goStart()
{
    if (!m_startWindow) return;
    hide();
    m_startWindow->show();
    m_startWindow->raise();
    m_startWindow->activateWindow();
}

void MainWindow::goA()
{
    // 已在功能 A
}

void MainWindow::goB()
{
    if (!m_startWindow) return;
    hide();
    m_startWindow->openB();
}

void MainWindow::closeEvent(QCloseEvent *e)
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
