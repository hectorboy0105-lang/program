#include "MahjongCore.h"

namespace mj
{
QString tileName(int idx)
{
    if (idx >= 0 && idx <= 8)  return QString::number(idx % 9 + 1) + "萬";
    if (idx >= 9 && idx <= 17) return QString::number(idx % 9 + 1) + "筒";
    if (idx >= 18 && idx <= 26) return QString::number(idx % 9 + 1) + "條";

    switch (idx)
    {
    case 27: return "東";
    case 28: return "南";
    case 29: return "西";
    case 30: return "北";
    case 31: return "中";
    case 32: return "發";
    case 33: return "白";
    default: return "？";
    }
}

QString flowerName(int idx0)
{
    static const char *names[8] =
        {
            "春" , "夏" , "秋" , "冬" ,
            "梅" , "蘭" , "竹" , "菊"
        };

    if (idx0 < 0 || idx0 >= 8) return "？";
    return names[idx0];
}

Wind parseWind(const QString &text)
{
    if (text == "東" || text == "E") return Wind::East;
    if (text == "南" || text == "S") return Wind::South;
    if (text == "西" || text == "W") return Wind::West;
    if (text == "北" || text == "N") return Wind::North;
    return Wind::Invalid;
}

QString windName(Wind w)
{
    switch (w)
    {
    case Wind::East:  return "東";
    case Wind::South: return "南";
    case Wind::West:  return "西";
    case Wind::North: return "北";
    default: return "未知";
    }
}

Wind calcOpenDoorSeat(Wind dealerSeat , int d1 , int d2 , int d3)
{
    if (dealerSeat == Wind::Invalid) return Wind::Invalid;
    int sum = d1 + d2 + d3;
    int offset = (sum - 1) % 4;
    return static_cast<Wind>((static_cast<int>(dealerSeat) + offset) % 4);
}

Wind calcRoundWindFromDice(Wind dealerSeat , int d1 , int d2 , int d3)
{
    if (dealerSeat == Wind::Invalid) return Wind::Invalid;
    int sum = d1 + d2 + d3;
    return static_cast<Wind>((static_cast<int>(dealerSeat) + (sum % 4)) % 4);
}

int calcWindTai(
    Wind roundWind,
    Wind selfWind,
    const std::array<int, 34> &counts,
    QString *detail
    )
{
    int roundIdx = 27 + static_cast<int>(roundWind);
    int selfIdx  = 27 + static_cast<int>(selfWind);

    int roundTai = (roundWind != Wind::Invalid && counts[roundIdx] >= 3) ? 1 : 0;
    int selfTai  = (selfWind  != Wind::Invalid && counts[selfIdx]  >= 3) ? 1 : 0;

    int tai = roundTai + selfTai;

    if (detail)
    {
        QString s;
        s += "風牌 : " + QString::number(tai) + " 台\n";
        s += "本風（圈風） : " + QString::number(roundTai) + " 台\n";
        s += "圈風（自風） : " + QString::number(selfTai) + " 台";
        *detail = s;
    }

    return tai;
}

int calcFlowerTai(
    Wind selfWind,
    const std::array<int, 8> &flowers,
    QString *detail
    )
{
    int a = -1;
    int b = -1;

    // 規則：東 春梅 南 夏蘭 西 秋菊 北 冬竹
    if (selfWind == Wind::East)  { a = 0; b = 4; }
    if (selfWind == Wind::South) { a = 1; b = 5; }
    if (selfWind == Wind::West)  { a = 2; b = 7; }
    if (selfWind == Wind::North) { a = 3; b = 6; }

    int tai = 0;
    if (a >= 0) tai += flowers[a];
    if (b >= 0) tai += flowers[b];

    if (detail)
        *detail = "正花 : " + QString::number(tai) + " 台";

    return tai;
}

struct MeldInternal
{
    enum Type { Pung , Chow } type;
    int tileIndex;
};

static bool isHonor(int i) { return i >= 27; }
static int suitOf(int i) { return (i >= 0 && i < 27) ? (i / 9) : -1; }

static int sumTiles(const std::array<int , 34> &c)
{
    int s = 0;
    for (int i = 0; i < 34; ++i) s += c[i];
    return s;
}

static int findFirst(const std::array<int , 34> &c)
{
    for (int i = 0; i < 34; ++i)
        if (c[i] > 0) return i;
    return -1;
}

static bool takePung(std::array<int , 34> &c , int i)
{
    if (c[i] >= 3) { c[i] -= 3; return true; }
    return false;
}

static bool takeChow(std::array<int , 34> &c , int i)
{
    if (i >= 27) return false;
    int pos = i % 9;
    if (pos > 6) return false;

    if (c[i] > 0 && c[i + 1] > 0 && c[i + 2] > 0)
    {
        c[i] -= 1; c[i + 1] -= 1; c[i + 2] -= 1;
        return true;
    }
    return false;
}

static bool dfsMeldN(std::array<int , 34> &c ,
                     std::vector<MeldInternal> &melds ,
                     int needMelds)
{
    if ((int)melds.size() == needMelds)
        return findFirst(c) < 0;

    int i = findFirst(c);
    if (i < 0) return false;

    if (takePung(c , i))
    {
        melds.push_back(MeldInternal{ MeldInternal::Pung , i });
        if (dfsMeldN(c , melds , needMelds)) return true;
        melds.pop_back();
        c[i] += 3;
    }

    if (takeChow(c , i))
    {
        melds.push_back(MeldInternal{ MeldInternal::Chow , i });
        if (dfsMeldN(c , melds , needMelds)) return true;
        melds.pop_back();
        c[i] += 1; c[i + 1] += 1; c[i + 2] += 1;
    }

    return false;
}

static bool applyOpenMelds(std::array<int , 34> &c ,
                           const std::vector<OpenMeld> &openMelds)
{
    for (const auto &m : openMelds)
    {
        if (m.type == MeldType::Chow)
        {
            int s = m.tileIndex;
            if (s < 0 || s >= 27) return false;
            if ((s % 9) > 6) return false;

            if (c[s] <= 0 || c[s + 1] <= 0 || c[s + 2] <= 0) return false;
            c[s] -= 1; c[s + 1] -= 1; c[s + 2] -= 1;
        }
        else if (m.type == MeldType::Pung)
        {
            int i = m.tileIndex;
            if (i < 0 || i >= 34) return false;
            if (c[i] < 3) return false;
            c[i] -= 3;
        }
        else
        {
            int i = m.tileIndex;
            if (i < 0 || i >= 34) return false;
            if (c[i] < 4) return false;
            c[i] -= 4;
        }
    }
    return true;
}

static bool isHuWithOpenMelds(const std::array<int , 34> &counts ,
                              const std::vector<OpenMeld> &openMelds ,
                              std::vector<MeldInternal> &outAllMelds ,
                              int &outPairIndex)
{
    std::array<int , 34> remain = counts;

    if (!applyOpenMelds(remain , openMelds))
        return false;

    int needMelds = 5 - (int)openMelds.size();
    if (needMelds < 0) return false;

    int remainTiles = sumTiles(remain);
    if (remainTiles != 2 + 3 * needMelds)
        return false;

    for (int pairIdx = 0; pairIdx < 34; ++pairIdx)
    {
        if (remain[pairIdx] < 2) continue;

        auto c = remain;
        c[pairIdx] -= 2;

        std::vector<MeldInternal> meldsRemain;
        if (dfsMeldN(c , meldsRemain , needMelds))
        {
            std::vector<MeldInternal> all;
            all.reserve(openMelds.size() + meldsRemain.size());

            for (const auto &om : openMelds)
            {
                if (om.type == MeldType::Chow)
                    all.push_back(MeldInternal{ MeldInternal::Chow , om.tileIndex });
                else
                    all.push_back(MeldInternal{ MeldInternal::Pung , om.tileIndex });
            }

            for (const auto &m : meldsRemain)
                all.push_back(m);

            outPairIndex = pairIdx;
            outAllMelds = all;
            return true;
        }
    }

    return false;
}

static bool isAllPungs(const std::vector<MeldInternal> &melds)
{
    for (const auto &m : melds)
        if (m.type != MeldInternal::Pung) return false;
    return true;
}

static int countPungLike(const std::array<int , 34> &c , int idx)
{
    return (c[idx] >= 3) ? 1 : 0;
}

static bool isBigFourWinds(const std::array<int , 34> &c)
{
    return c[27] >= 3 && c[28] >= 3 && c[29] >= 3 && c[30] >= 3;
}

static bool isSmallFourWinds(const std::array<int , 34> &c , int pairIdx)
{
    int windPungs =
        countPungLike(c , 27) +
        countPungLike(c , 28) +
        countPungLike(c , 29) +
        countPungLike(c , 30);

    bool pairIsWind = (pairIdx >= 27 && pairIdx <= 30 && c[pairIdx] >= 2);
    return windPungs == 3 && pairIsWind;
}

static bool isBigThreeDragons(const std::array<int , 34> &c)
{
    return c[31] >= 3 && c[32] >= 3 && c[33] >= 3;
}

static bool isSmallThreeDragons(const std::array<int , 34> &c , int pairIdx)
{
    int dragonPungs =
        countPungLike(c , 31) +
        countPungLike(c , 32) +
        countPungLike(c , 33);

    bool pairIsDragon = (pairIdx >= 31 && pairIdx <= 33 && c[pairIdx] >= 2);
    return dragonPungs == 2 && pairIsDragon;
}

static int countDragonPungs(const std::array<int , 34> &c)
{
    return
        countPungLike(c , 31) +
        countPungLike(c , 32) +
        countPungLike(c , 33);
}

static bool isHunYiSe(const std::array<int , 34> &c)
{
    int suit = -1;
    bool hasHonor = false;

    for (int i = 0; i < 34; ++i)
    {
        if (c[i] == 0) continue;

        if (isHonor(i))
        {
            hasHonor = true;
            continue;
        }

        int s = suitOf(i);
        if (suit < 0) suit = s;
        else if (suit != s) return false;
    }

    return hasHonor && suit >= 0;
}

static bool isQingYiSe(const std::array<int , 34> &c)
{
    int suit = -1;

    for (int i = 0; i < 34; ++i)
    {
        if (c[i] == 0) continue;
        if (isHonor(i)) return false;

        int s = suitOf(i);
        if (suit < 0) suit = s;
        else if (suit != s) return false;
    }

    return suit >= 0;
}

static bool isZiYiSe(const std::array<int , 34> &c)
{
    for (int i = 0; i < 27; ++i)
        if (c[i] > 0) return false;
    return true;
}

static bool isPingHuNoHonorNoFlower_AllChows(const std::vector<MeldInternal> &melds ,
                                             int pairIdx ,
                                             const std::array<int , 34> &counts ,
                                             const std::array<int , 8> &flowers)
{
    if (pairIdx < 0) return false;
    if (isHonor(pairIdx)) return false;

    int flowerTotal = 0;
    for (int i = 0; i < 8; ++i) flowerTotal += flowers[i];
    if (flowerTotal != 0) return false;

    for (int i = 27; i < 34; ++i)
        if (counts[i] > 0) return false;

    for (const auto &m : melds)
        if (m.type != MeldInternal::Chow) return false;

    return true;
}

ScoreBreakdown scoreTaiwanMahjong(
    const std::array<int , 34> &handCounts ,
    const std::vector<OpenMeld> &openMelds ,
    const std::array<int , 8> &flowers ,
    Wind dealerSeat ,
    Wind selfSeat ,
    int d1 ,
    int d2 ,
    int d3
    )
{
    ScoreBreakdown out;
    out.dealerSeat = dealerSeat;
    out.selfSeat = selfSeat;
    out.roundWind = calcRoundWindFromDice(dealerSeat , d1 , d2 , d3);

    QString d;
    int tai = 0;

    auto add = [&](const QString &name , int v)
    {
        if (v <= 0) return;
        tai += v;
        if (!d.isEmpty()) d += "\n";
        d += name + " : " + QString::number(v) + " 台";
    };

    std::vector<MeldInternal> melds;
    int pairIdx = -1;

    if (!isHuWithOpenMelds(handCounts , openMelds , melds , pairIdx))
    {
        out.huValid = false;
        out.error = "此牌組無法胡牌（門前組與手牌不相容）";
        out.detail = out.error;
        out.totalTai = 0;
        return out;
    }

    out.huValid = true;

    if (dealerSeat != Wind::Invalid && selfSeat != Wind::Invalid && dealerSeat == selfSeat)
        add("莊家" , 1);

    // 風牌台：本風（圈風） + 圈風（自風），最多 2 台
    {
        QString windDetail;
        int windTai = calcWindTai(out.roundWind , selfSeat , handCounts , &windDetail);
        add("風牌" , windTai);
    }

    // 正花台：依自風對應（東春梅 南夏蘭 西秋菊 北冬竹）
    {
        QString flowerDetail;
        int flowerTai = calcFlowerTai(selfSeat , flowers , &flowerDetail);
        add("正花" , flowerTai);
    }

    int bestWind = 0;
    QString bestWindName;
    if (isBigFourWinds(handCounts)) { bestWind = 16; bestWindName = "大四喜"; }
    else if (isSmallFourWinds(handCounts , pairIdx)) { bestWind = 8; bestWindName = "小四喜"; }
    if (bestWind > 0) add(bestWindName , bestWind);

    int bestDragon = 0;
    QString bestDragonName;
    if (isBigThreeDragons(handCounts)) { bestDragon = 8; bestDragonName = "大三元"; }
    else if (isSmallThreeDragons(handCounts , pairIdx)) { bestDragon = 4; bestDragonName = "小三元"; }
    if (bestDragon > 0) add(bestDragonName , bestDragon);

    int bestSuit = 0;
    QString bestSuitName;
    if (isZiYiSe(handCounts)) { bestSuit = 8; bestSuitName = "字一色"; }
    else if (isQingYiSe(handCounts)) { bestSuit = 8; bestSuitName = "清一色"; }
    else if (isHunYiSe(handCounts)) { bestSuit = 4; bestSuitName = "混一色"; }
    if (bestSuit > 0) add(bestSuitName , bestSuit);

    if (isAllPungs(melds))
        add("碰碰胡" , 4);

    if (isPingHuNoHonorNoFlower_AllChows(melds , pairIdx , handCounts , flowers))
        add("平胡" , 2);

    int dragonPungs = countDragonPungs(handCounts);
    if (dragonPungs > 0)
        add("三元刻" , dragonPungs);

    if (!d.isEmpty()) d += "\n";
    d += "合計 : " + QString::number(tai) + " 台";

    out.totalTai = tai;
    out.detail = d;
    return out;
}

}
