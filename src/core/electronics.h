#pragma once

#include <QString>

namespace Rheno::Core {

bool parseValue(const QString& text, double* value);
QString formatEngineering(double value, const QString& unit);

struct VoltageDividerResult { bool valid = false; double vout = 0.0; double ratio = 0.0; };
struct LedResistorResult { bool valid = false; QString resistorText; QString powerText; };
struct WheatstoneResult { bool valid = false; QString text; };
struct RcFilterResult { bool valid = false; QString fcText; QString tauText; };
struct LcResonanceResult { bool valid = false; QString f0Text; QString omegaText; };
struct PullUpDownResult { bool valid = false; QString riseText; QString fallText; };

VoltageDividerResult calculateVoltageDivider(double vin, double r1, double r2);
LedResistorResult calculateLedResistor(double vs, double vf, double ifMa);
WheatstoneResult calculateWheatstone(double r1, double r2, double r3, double rxEntered, bool hasRxEntered);
RcFilterResult calculateRcFilter(double rOhm, double cNf);
LcResonanceResult calculateLcResonance(double lUh, double cPf);
PullUpDownResult calculatePullUpDown(double rOhm, double cPf, double vcc, double vth);

} // namespace Rheno::Core

