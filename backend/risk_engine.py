"""
Illustrative prototype thresholds only.
Calibrate the MQ-2 sensor and validate thresholds against your actual hardware.
These rules are not an industrial safety certification or a substitute for local ESP32 safety logic.
"""

GAS_WARNING = 1500
GAS_CRITICAL = 2500
TEMP_WARNING = 55.0
TEMP_CRITICAL = 70.0

def assess_risk(gas_value: float, temperature: float) -> dict:
    reasons = []

    if gas_value >= GAS_CRITICAL:
        reasons.append("Gas/smoke reading crossed the critical demo limit")
    if temperature >= TEMP_CRITICAL:
        reasons.append("Equipment temperature crossed the critical demo limit")

    if reasons:
        return {"level": "CRITICAL", "reason": "; ".join(reasons)}

    if gas_value >= GAS_WARNING:
        reasons.append("Gas/smoke reading is elevated")
    if temperature >= TEMP_WARNING:
        reasons.append("Equipment temperature is elevated")

    if reasons:
        return {"level": "WARNING", "reason": "; ".join(reasons)}

    return {"level": "SAFE", "reason": "Readings are within configured demo limits"}
