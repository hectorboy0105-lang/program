#ifndef MAHJONGSCORER_H
#define MAHJONGSCORER_H

#include <QString>
#include <QStringList>
#include <QVector>

struct TaiItem
{
    QString name;
    int tai = 0;
};

struct TaiInput
{
    QVector<int> counts42;       // 0 ~ 41
    bool hasOpenMelds = false;   // true = 有吃碰明槓 , false = 門清
};

struct TaiOutput
{
    int totalTai = 0;
    QVector<TaiItem> items;
    QString warning;
};

struct DiscardWait
{
    QString discardTile;
    QStringList drawTiles;
};

struct AnalyzeOutput
{
    QString warning;
    QStringList pongTiles;
    QStringList chiOptions;
    QVector<DiscardWait> waitsByDiscard;
};

class MahjongScorer
{
public:
    static TaiOutput scoreA(const TaiInput &in);
    static AnalyzeOutput analyzeB(const QVector<int> &counts34 , const QVector<int> &flowers8);

    static QString tileName42(int idx);
    static QString tileName34(int idx);

private:
    static QVector<int> to34(const QVector<int> &counts42);
    static int flowerCount42(const QVector<int> &counts42);

    static bool isBigThreeDragons(const QVector<int> &c34);
    static bool isBigFourWinds(const QVector<int> &c34);
    static bool isFullFlush(const QVector<int> &c34);
    static bool isHalfFlush(const QVector<int> &c34);
    static bool isAllPungs(const QVector<int> &c34);
    static bool hasAnyPung(const QVector<int> &c34);
    static bool isPingHuSimple(const QVector<int> &c34 , int flowers);

    static bool sameSuitNumberTile(int a34 , int b34);
    static int suitBase(int idx34);
    static int suitRank(int idx34);

    static QStringList drawTilesThatMakeMeldUsingDraw(const QVector<int> &counts34);
};

#endif
