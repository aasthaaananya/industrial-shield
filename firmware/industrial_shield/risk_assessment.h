
#pragma once

#include "sensor_data.h"

enum RiskLevel {
    SAFE,
    WARNING,
    DANGER,
    CRITICAL
};

RiskLevel calculateRisk(const SensorData& data) {

    if (data.temperature >= 70 || data.gasLevel >= 800) {
        return CRITICAL;
    }

    if (data.temperature >= 60 || data.gasLevel >= 600) {
        return DANGER;
    }

    if (data.temperature >= 45 || data.gasLevel >= 400) {
        return WARNING;
    }

    return SAFE;
}

const char* getRiskLabel(RiskLevel level) {

    switch (level) {
        case SAFE: return "SAFE";
        case WARNING: return "WARNING";
        case DANGER: return "DANGER";
        case CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}
