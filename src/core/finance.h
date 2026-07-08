#pragma once

#include <QString>

namespace Rheno::Core {

struct FinanceSimpleResult {
    double futureValue = 0.0;
};

struct FinanceCompoundResult {
    double futureValue = 0.0;
    double totalContributions = 0.0;
    double totalInterest = 0.0;
    double effectiveAnnualRate = 0.0;
};

// Simple compound interest: FV = P * (1 + r)^n
FinanceSimpleResult calculateSimpleCompound(double principal, double ratePercent, double periods);

// Compound interest with regular contributions
FinanceCompoundResult calculateCompoundInterest(
    double principal,
    double annualRatePercent,
    double contribution,
    double years,
    int compoundingPerYear,
    int contributionsPerYear,
    bool contributionsAtStart
);

// Helper functions
int compoundingPerYear(const QString& label);
int contributionsPerYear(const QString& label);
bool parseDouble(const QString& text, double* value);
QString formatMoney(double value);

} // namespace Rheno::Core

