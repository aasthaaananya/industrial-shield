
#pragma once

#include "sensor_data.h"

enum RiskLevel {
    SAFE,
    WARNING,
    CRITICAL
};

RiskLevel calculateRisk(const SensorData& data) {
    if (data.temperature <= -100) {
        return WARNING;
    }

    if (data.temperature >= 70 || data.gasLevel >= 2500) {
        return CRITICAL;
    }

    if (data.temperature >= 55 || data.gasLevel >= 1500) {
        return WARNING;
    }

    return SAFE;
}

const char* getRiskLabel(RiskLevel level) {
    switch (level) {
        case SAFE: return "SAFE";
        case WARNING: return "WARNING";
        case CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}
