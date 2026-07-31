#include "electronics.h"
#include "common/format.h"
#include "common/numberparse.h"

#include <cmath>

namespace Rheno::Core {

bool parseValue(const QString& text, double* value) {
    return tryParseLocalizedDouble(text, value, EmptyNumberPolicy::Invalid);
}

QString formatEngineering(double value, const QString& unit) {
    return Rheno::Core::formatEngineeringValue(value, unit);
}

VoltageDividerResult calculateVoltageDivider(double vin, double r1, double r2) {
    VoltageDividerResult out;
    if (r1 <= 0 || r2 <= 0) return out;
    out.vout = vin * r2 / (r1 + r2);
    out.ratio = r2 / (r1 + r2);
    out.valid = true;
    return out;
}

LedResistorResult calculateLedResistor(double vs, double vf, double ifMa) {
    LedResistorResult out;
    const double ifA = ifMa / 1000.0;
    if (ifA <= 0 || vs <= vf) return out;
    const double r = (vs - vf) / ifA;
    const double p = (vs - vf) * ifA;
    out.resistorText = formatEngineering(r, "Ω");
    out.powerText = formatEngineering(p, "W");
    out.valid = true;
    return out;
}

WheatstoneResult calculateWheatstone(double r1, double r2, double r3, double rxEntered, bool hasRxEntered) {
    WheatstoneResult out;
    if (r1 <= 0) return out;
    const double rxCalc = r2 * r3 / r1;
    if (hasRxEntered && rxEntered > 0) {
        const double diff = std::abs(rxEntered - rxCalc) / rxCalc * 100.0;
        if (diff < 0.01) out.text = "Balanced! Rx = " + formatEngineering(rxCalc, "Ω");
        else out.text = QString("Unbalanced (%1%). Need Rx = %2").arg(QString::number(diff, 'f', 2)).arg(formatEngineering(rxCalc, "Ω"));
    } else {
        out.text = "Rx = " + formatEngineering(rxCalc, "Ω");
    }
    out.valid = true;
    return out;
}

RcFilterResult calculateRcFilter(double rOhm, double cNf) {
    RcFilterResult out;
    const double c = cNf * 1e-9;
    if (rOhm <= 0 || c <= 0) return out;
    const double tau = rOhm * c;
    const double fc = 1.0 / (2.0 * M_PI * tau);
    out.fcText = formatEngineering(fc, "Hz");
    out.tauText = formatEngineering(tau, "s");
    out.valid = true;
    return out;
}

LcResonanceResult calculateLcResonance(double lUh, double cPf) {
    LcResonanceResult out;
    const double l = lUh * 1e-6;
    const double c = cPf * 1e-12;
    if (l <= 0 || c <= 0) return out;
    const double omega0 = 1.0 / std::sqrt(l * c);
    const double f0 = omega0 / (2.0 * M_PI);
    out.f0Text = formatEngineering(f0, "Hz");
    out.omegaText = formatEngineering(omega0, "rad/s");
    out.valid = true;
    return out;
}

PullUpDownResult calculatePullUpDown(double rOhm, double cPf, double vcc, double vth) {
    PullUpDownResult out;
    const double c = cPf * 1e-12;
    if (rOhm <= 0 || c <= 0 || vcc <= 0 || vth <= 0 || vth >= vcc) return out;
    const double tau = rOhm * c;
    const double tRise = -tau * std::log(1.0 - vth / vcc);
    const double tFall = -tau * std::log(vth / vcc);
    out.riseText = formatEngineering(tRise, "s");
    out.fallText = formatEngineering(tFall, "s");
    out.valid = true;
    return out;
}

} // namespace Rheno::Core

