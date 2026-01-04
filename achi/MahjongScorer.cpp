#include "MahjongScorer.h"
#include <algorithm>

static void addItem(QVector<TaiItem> &items , const QString &name , int tai)
{
    if (tai <= 0) return;
    TaiItem it;
    it.name = name;
    it.tai = tai;
    items.push_back(it);
}

QString MahjongScorer::tileName42(int idx)
{
    if (idx >= 0 && idx <= 8) return QString("%1萬").arg(idx + 1);
    if (idx >= 9 && idx <= 17) return QString("%1筒").arg(idx - 9 + 1);
    if (idx >= 18 && idx <= 26) return QString("%1條").arg(idx - 18 + 1);

    static QStringList honors = { "東" , "南" , "西" , "北" , "中" , "發" , "白" };
    if (idx >= 27 && idx <= 33) return honors[idx - 27];

    static QStringList flowers = { "春" , "夏" , "秋" , "冬" , "梅" , "蘭" , "竹" , "菊" };
    if (idx >= 34 && idx <= 41) return flowers[idx - 34];

    return "未知";
}

QString MahjongScorer::tileName34(int idx)
{
    return tileName42(idx);
}

QVector<int> MahjongScorer::to34(const QVector<int> &counts42)
{
    QVector<int> c34;
    c34.resize(34);
    for (int i = 0; i < 34; ++i) c34[i] = counts42[i];
    return c34;
}

int MahjongScorer::flowerCount42(const QVector<int> &counts42)
{
    int f = 0;
    for (int i = 34; i < 42; ++i) f += counts42[i];
    return f;
}

bool MahjongScorer::isBigThreeDragons(const QVector<int> &c34)
{
    return c34[31] >= 3 && c34[32] >= 3 && c34[33] >= 3;
}

bool MahjongScorer::isBigFourWinds(const QVector<int> &c34)
{
    return c34[27] >= 3 && c34[28] >= 3 && c34[29] >= 3 && c34[30] >= 3;
}

bool MahjongScorer::isFullFlush(const QVector<int> &c34)
{
    bool man = false , pin = false , sou = false;

    for (int i = 0; i < 9; ++i) if (c34[i] > 0) man = true;
    for (int i = 9; i < 18; ++i) if (c34[i] > 0) pin = true;
    for (int i = 18; i < 27; ++i) if (c34[i] > 0) sou = true;

    for (int i = 27; i < 34; ++i) if (c34[i] > 0) return false;

    int suits = (man ? 1 : 0) + (pin ? 1 : 0) + (sou ? 1 : 0);
    return suits == 1;
}

bool MahjongScorer::isHalfFlush(const QVector<int> &c34)
{
    bool man = false , pin = false , sou = false , honor = false;

    for (int i = 0; i < 9; ++i) if (c34[i] > 0) man = true;
    for (int i = 9; i < 18; ++i) if (c34[i] > 0) pin = true;
    for (int i = 18; i < 27; ++i) if (c34[i] > 0) sou = true;
    for (int i = 27; i < 34; ++i) if (c34[i] > 0) honor = true;

    int suits = (man ? 1 : 0) + (pin ? 1 : 0) + (sou ? 1 : 0);
    return suits == 1 && honor;
}

bool MahjongScorer::isAllPungs(const QVector<int> &c34)
{
    for (int p = 0; p < 34; ++p)
    {
        if (c34[p] >= 2)
        {
            QVector<int> t = c34;
            t[p] -= 2;

            bool ok = true;
            for (int i = 0; i < 34; ++i)
            {
                if (t[i] % 3 != 0)
                {
                    ok = false;
                    break;
                }
            }
            if (ok) return true;
        }
    }
    return false;
}

bool MahjongScorer::hasAnyPung(const QVector<int> &c34)
{
    for (int i = 0; i < 34; ++i) if (c34[i] >= 3) return true;
    return false;
}

bool MahjongScorer::isPingHuSimple(const QVector<int> &c34 , int flowers)
{
    if (flowers > 0) return false;
    for (int i = 27; i < 34; ++i) if (c34[i] > 0) return false;
    if (isAllPungs(c34)) return false;
    return true;
}

TaiOutput MahjongScorer::scoreA(const TaiInput &in)
{
    TaiOutput out;

    if (in.counts42.size() != 42)
    {
        out.warning = "A 功能輸入錯誤 : counts42 長度必須是 42";
        return out;
    }

    QVector<int> c34 = to34(in.counts42);
    int flowers = flowerCount42(in.counts42);

    if (isBigFourWinds(c34)) addItem(out.items , "大四喜" , 16);
    if (isBigThreeDragons(c34)) addItem(out.items , "大三元" , 8);

    if (isFullFlush(c34)) addItem(out.items , "清一色" , 8);
    else if (isHalfFlush(c34)) addItem(out.items , "混一色" , 4);

    if (isAllPungs(c34)) addItem(out.items , "碰碰胡" , 4);

    if (!in.hasOpenMelds && isPingHuSimple(c34 , flowers))
        addItem(out.items , "平胡(門清無字無花)" , 2);

    if (!in.hasOpenMelds && hasAnyPung(c34))
        addItem(out.items , "暗刻(門清簡化判斷)" , 1);

    int total = 0;
    for (const auto &it : out.items) total += it.tai;
    out.totalTai = total;

    return out;
}

int MahjongScorer::suitBase(int idx34)
{
    if (idx34 >= 0 && idx34 <= 8) return 0;
    if (idx34 >= 9 && idx34 <= 17) return 9;
    if (idx34 >= 18 && idx34 <= 26) return 18;
    return -1;
}

int MahjongScorer::suitRank(int idx34)
{
    int base = suitBase(idx34);
    if (base < 0) return -1;
    return idx34 - base;
}

bool MahjongScorer::sameSuitNumberTile(int a34 , int b34)
{
    int ba = suitBase(a34);
    int bb = suitBase(b34);
    if (ba < 0 || bb < 0) return false;
    return ba == bb;
}

QStringList MahjongScorer::drawTilesThatMakeMeldUsingDraw(const QVector<int> &counts34)
{
    QStringList res;

    for (int draw = 0; draw < 34; ++draw)
    {
        if (counts34[draw] >= 4) continue;

        bool ok = false;

        // 刻子 : 原本至少 2 張 , 進一張變 3 張
        if (counts34[draw] >= 2) ok = true;

        // 順子 : 必須同花色數牌 , 並且能用 draw 組成 x x x
        int base = suitBase(draw);
        int r = suitRank(draw);

        if (!ok && base >= 0)
        {
            // (draw - 2 , draw - 1 , draw)
            if (r >= 2)
            {
                if (counts34[draw - 1] > 0 && counts34[draw - 2] > 0) ok = true;
            }
            // (draw - 1 , draw , draw + 1)
            if (!ok && r >= 1 && r <= 7)
            {
                if (counts34[draw - 1] > 0 && counts34[draw + 1] > 0) ok = true;
            }
            // (draw , draw + 1 , draw + 2)
            if (!ok && r <= 6)
            {
                if (counts34[draw + 1] > 0 && counts34[draw + 2] > 0) ok = true;
            }
        }

        if (ok) res.push_back(tileName34(draw));
    }

    return res;
}

AnalyzeOutput MahjongScorer::analyzeB(const QVector<int> &counts34 , const QVector<int> &flowers8)
{
    AnalyzeOutput out;

    if (counts34.size() != 34)
    {
        out.warning = "B 功能輸入錯誤 : counts34 長度必須是 34";
        return out;
    }
    if (flowers8.size() != 8)
    {
        out.warning = "B 功能輸入錯誤 : flowers8 長度必須是 8";
        return out;
    }

    int handCount = 0;
    for (int v : counts34) handCount += v;

    if (handCount != 16)
    {
        out.warning = QString("提醒 : 目前非花手牌張數 = %1 , 需為 16 張").arg(handCount);
    }

    // 可碰 : 手牌同種 >= 2
    for (int i = 0; i < 34; ++i)
    {
        if (counts34[i] >= 2) out.pongTiles.push_back(tileName34(i));
    }
    if (out.pongTiles.isEmpty()) out.pongTiles.push_back("目前沒有可碰");

    // 可吃 : 若對家打出 y , 你手上能用哪兩張組順子 (用文字列出)
    for (int y = 0; y <= 26; ++y)
    {
        int base = suitBase(y);
        int r = suitRank(y);
        if (base < 0) continue;

        if (r <= 6 && counts34[y + 1] > 0 && counts34[y + 2] > 0)
        {
            out.chiOptions.push_back(QString("打出 %1 , 你可吃 %2 %3 %4")
                .arg(tileName34(y))
                .arg(tileName34(y))
                .arg(tileName34(y + 1))
                .arg(tileName34(y + 2)));
        }
        if (r >= 1 && r <= 7 && counts34[y - 1] > 0 && counts34[y + 1] > 0)
        {
            out.chiOptions.push_back(QString("打出 %1 , 你可吃 %2 %3 %4")
                .arg(tileName34(y))
                .arg(tileName34(y - 1))
                .arg(tileName34(y))
                .arg(tileName34(y + 1)));
        }
        if (r >= 2 && counts34[y - 2] > 0 && counts34[y - 1] > 0)
        {
            out.chiOptions.push_back(QString("打出 %1 , 你可吃 %2 %3 %4")
                .arg(tileName34(y))
                .arg(tileName34(y - 2))
                .arg(tileName34(y - 1))
                .arg(tileName34(y)));
        }
    }
    if (out.chiOptions.isEmpty()) out.chiOptions.push_back("目前沒有可吃");

    // 每一張捨牌後的進張清單 : 進張定義為 加一張能立刻形成刻子或順子(且必須用到那張進來的牌)
    for (int d = 0; d < 34; ++d)
    {
        if (counts34[d] <= 0) continue;

        QVector<int> t = counts34;
        t[d] -= 1;

        DiscardWait item;
        item.discardTile = tileName34(d);
        item.drawTiles = drawTilesThatMakeMeldUsingDraw(t);

        if (item.drawTiles.isEmpty()) item.drawTiles.push_back("目前沒有明顯進張");

        out.waitsByDiscard.push_back(item);
    }

    // 讓輸出穩定 : 依捨牌名稱排序(可自行改成依進張數量排序)
    std::sort(out.waitsByDiscard.begin() , out.waitsByDiscard.end() ,
        [](const DiscardWait &a , const DiscardWait &b) { return a.discardTile < b.discardTile; });

    return out;
}
