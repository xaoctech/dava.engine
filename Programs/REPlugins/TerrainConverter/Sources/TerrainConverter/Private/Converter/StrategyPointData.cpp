#include "StrategyPointData.h"

namespace SStrategyPointData
{
using namespace DAVA;

const String BASE_ID = "baseID";
const String BONUS_CAPTURE = "bonusCapture";
const String PENALTY_LOSS = "penaltyLoss";
const String INCOME_VICTORY_POINTS = "incomeVictoryPoints";
const String SPAWN_VICTORY_POINTS_TIME = "spawnVictoryPointsTime";
const String CAPTURE_POINTS = "capturePoints";
const String POINTS_PER_SECOND = "pointsPerSecond";
const String MAX_POINTS_PER_SECOND = "maxPointsPerSecond";
const String RADIUS = "radius";

String AsPattern(const String& value)
{
    return "%(" + value + ")";
}
}

StrategyPointData StrategyPointData::Create(DAVA::KeyedArchive* customProperties)
{
    using namespace SStrategyPointData;

    StrategyPointData data;
    data.baseID = customProperties->GetInt32(BASE_ID, StrategyPointData::INVALID_BASE_ID);
    data.bonus—apture = customProperties->GetInt32(BONUS_CAPTURE, data.bonus—apture);
    data.penaltyLoss = customProperties->GetInt32(PENALTY_LOSS, data.penaltyLoss);
    data.incomeVictoryPoints = customProperties->GetInt32(INCOME_VICTORY_POINTS, data.incomeVictoryPoints);
    data.spawnVictoryPointsTime = customProperties->GetFloat(SPAWN_VICTORY_POINTS_TIME, data.spawnVictoryPointsTime);
    data.capturePoints = customProperties->GetInt32(CAPTURE_POINTS, data.capturePoints);
    data.pointsPerSecond = customProperties->GetFloat(POINTS_PER_SECOND, data.pointsPerSecond);
    data.maxPointsPerSecond = customProperties->GetFloat(MAX_POINTS_PER_SECOND, data.maxPointsPerSecond);
    data.radius = customProperties->GetFloat(RADIUS, data.radius);

    return data;
}

void StrategyPointData::FillPattern(DAVA::String& pattern)
{
    using namespace SStrategyPointData;

    StringReplace(pattern, AsPattern(BASE_ID), Format("%d", baseID));
    StringReplace(pattern, AsPattern(BONUS_CAPTURE), Format("%d", bonus—apture));
    StringReplace(pattern, AsPattern(PENALTY_LOSS), Format("%d", penaltyLoss));
    StringReplace(pattern, AsPattern(INCOME_VICTORY_POINTS), Format("%d", incomeVictoryPoints));
    StringReplace(pattern, AsPattern(SPAWN_VICTORY_POINTS_TIME), Format("%f", spawnVictoryPointsTime));
    StringReplace(pattern, AsPattern(CAPTURE_POINTS), Format("%d", capturePoints));
    StringReplace(pattern, AsPattern(POINTS_PER_SECOND), Format("%f", pointsPerSecond));
    StringReplace(pattern, AsPattern(MAX_POINTS_PER_SECOND), Format("%f", maxPointsPerSecond));
    StringReplace(pattern, AsPattern(RADIUS), Format("%f", radius));
}
