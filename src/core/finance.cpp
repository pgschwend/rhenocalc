#include "finance.h"
#include "common/format.h"
#include "common/numberparse.h"

#include <QtMath>

namespace Rheno::Core {

int compoundingPerYear(const QString& label) {
    if (label == "Daily") return 365;
    if (label == "Monthly") return 12;
    if (label == "Quarterly") return 4;
    return 1;
}

int contributionsPerYear(const QString& label) {
    if (label == "None") return 0;
    if (label == "Monthly") return 12;
    if (label == "Quarterly") return 4;
    return 1;
}

bool parseDouble(const QString& text, double* value) {
    return tryParseLocalizedDouble(text, value, EmptyNumberPolicy::Zero);
}

QString formatMoney(double value) {
    return formatMoneyLocalized(value, 2);
}

FinanceSimpleResult calculateSimpleCompound(double principal, double ratePercent, double periods) {
    FinanceSimpleResult result;

    // Clamp to non-negative
    principal = qMax(0.0, principal);
    ratePercent = qMax(0.0, ratePercent);
    periods = qMax(0.0, periods);

    result.futureValue = principal * qPow(1.0 + (ratePercent / 100.0), periods);
    return result;
}

FinanceCompoundResult calculateCompoundInterest(
    double principal,
    double annualRatePercent,
    double contribution,
    double years,
    int compoundingPerYear,
    int contributionsPerYear,
    bool contributionsAtStart)
{
    FinanceCompoundResult result;

    // Clamp to non-negative
    principal = qMax(0.0, principal);
    contribution = qMax(0.0, contribution);
    annualRatePercent = qMax(0.0, annualRatePercent);
    years = qMax(0.0, years);

    const int n = compoundingPerYear;
    const int m = contributionsPerYear;

    const double periodRate = (annualRatePercent / 100.0) / static_cast<double>(n);
    const double epsilon = 1e-9;

    // Split the term into whole compounding periods plus a possible fractional
    // leftover period (e.g. years=0.4 with annual compounding is 0 whole periods
    // and a 0.4-period leftover) so short/fractional terms still accrue interest
    // and receive their scheduled contributions, instead of being skipped entirely.
    const double totalPeriodsExact = years * static_cast<double>(n);
    const int wholePeriods = qFloor(totalPeriodsExact + epsilon);
    const double leftoverPeriodLength = qMax(0.0, totalPeriodsExact - wholePeriods);

    double balance = principal;
    double totalContrib = principal;

    const double contribInterval = (m > 0) ? (1.0 / static_cast<double>(m)) : 0.0;
    double nextContribution = (m > 0) ? (contributionsAtStart ? 0.0 : contribInterval) : 1e9;

    // "At start" contributions due before `timeBound` are claimed by the period ending
    // there; one due exactly at `timeBound` belongs to the *next* period's start-window
    // instead (avoids double-counting across the period boundary) -- unless there is no
    // next period, in which case it must still be swept up so it isn't silently dropped.
    const auto applyContributionsExclusive = [&](double timeBound) {
        while (nextContribution < timeBound - epsilon) {
            balance += contribution;
            totalContrib += contribution;
            nextContribution += contribInterval;
        }
    };
    const auto applyContributionsInclusive = [&](double timeBound) {
        while (nextContribution <= timeBound + epsilon) {
            balance += contribution;
            totalContrib += contribution;
            nextContribution += contribInterval;
        }
    };

    for (int i = 1; i <= wholePeriods; ++i) {
        const double periodEnd = static_cast<double>(i) / static_cast<double>(n);
        const bool isFinalSegment = (i == wholePeriods) && (leftoverPeriodLength <= epsilon);

        // Contributions at start of period
        if (m > 0 && contributionsAtStart) {
            if (isFinalSegment) applyContributionsInclusive(periodEnd);
            else applyContributionsExclusive(periodEnd);
        }

        // Apply compound interest for this period
        balance *= (1.0 + periodRate);

        // Contributions at end of period
        if (m > 0 && !contributionsAtStart)
            applyContributionsInclusive(periodEnd);
    }

    if (leftoverPeriodLength > epsilon) {
        const double periodEnd = years;

        if (m > 0 && contributionsAtStart)
            applyContributionsInclusive(periodEnd);

        balance *= qPow(1.0 + periodRate, leftoverPeriodLength);

        if (m > 0 && !contributionsAtStart)
            applyContributionsInclusive(periodEnd);
    }

    result.futureValue = balance;
    result.totalContributions = totalContrib;
    result.totalInterest = balance - totalContrib;
    result.effectiveAnnualRate = qPow(1.0 + periodRate, n) - 1.0;

    return result;
}

} // namespace Rheno::Core

