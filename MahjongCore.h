#pragma once
#include <QString>
#include <array>
#include <vector>

namespace mj
{
enum class Wind
{
    East = 0 ,
    South = 1 ,
    West = 2 ,
    North = 3 ,
    Invalid = 99
};

enum class MeldType
{
    Chow ,
    Pung ,
    Kong
};

struct OpenMeld
{
    MeldType type = MeldType::Chow;
    int tileIndex = -1;
};

struct ScoreBreakdown
{
    int totalTai = 0;
    QString detail;
    bool huValid = false;
    QString error;

    Wind dealerSeat = Wind::Invalid;
    Wind selfSeat = Wind::Invalid;
    Wind roundWind = Wind::Invalid;
};

QString tileName(int tileIndex);
QString flowerName(int flowerIndex);

Wind parseWind(const QString &text);
QString windName(Wind w);

Wind calcOpenDoorSeat(Wind dealerSeat , int d1 , int d2 , int d3);

Wind calcRoundWindFromDice(Wind dealerSeat , int d1 , int d2 , int d3);

int calcZhengHuaTai(Wind roundWind , const std::array<int , 8> &flowers , QString *detail = nullptr);

ScoreBreakdown scoreTaiwanMahjong(
    const std::array<int , 34> &handCounts ,
    const std::vector<OpenMeld> &openMelds ,
    const std::array<int , 8> &flowers ,
    Wind dealerSeat ,
    Wind selfSeat ,
    int d1 ,
    int d2 ,
    int d3
    );
}
