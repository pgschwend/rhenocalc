#include "finance.h"

#include <QLocale>
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
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        *value = 0.0;
        return true;
    }

    bool ok = false;
    double parsed = QLocale::system().toDouble(trimmed, &ok);
    if (!ok) {
        QString normalized = trimmed;
        parsed = normalized.replace(',', '.').toDouble(&ok);
    }
    if (!ok) return false;
    *value = parsed;
    return true;
}

QString formatMoney(double value) {
    return QLocale::system().toString(value, 'f', 2);
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
    const int totalPeriods = static_cast<int>(qRound(years * n));

    double balance = principal;
    double totalContrib = principal;

    const double contribInterval = (m > 0) ? (1.0 / static_cast<double>(m)) : 0.0;
    double nextContribution = (m > 0) ? (contributionsAtStart ? 0.0 : contribInterval) : 1e9;
    const double epsilon = 1e-9;

    for (int i = 1; i <= totalPeriods; ++i) {
        const double periodStart = (static_cast<double>(i - 1)) / static_cast<double>(n);
        const double periodEnd = (static_cast<double>(i)) / static_cast<double>(n);

        // Contributions at start of period
        if (m > 0 && contributionsAtStart) {
            while (nextContribution <= periodStart + epsilon) {
                balance += contribution;
                totalContrib += contribution;
                nextContribution += contribInterval;
            }
        }

        // Apply compound interest for this period
        balance *= (1.0 + periodRate);

        // Contributions at end of period
        if (m > 0 && !contributionsAtStart) {
            while (nextContribution <= periodEnd + epsilon) {
                balance += contribution;
                totalContrib += contribution;
                nextContribution += contribInterval;
            }
        }
    }

    result.futureValue = balance;
    result.totalContributions = totalContrib;
    result.totalInterest = balance - totalContrib;
    result.effectiveAnnualRate = qPow(1.0 + periodRate, n) - 1.0;

    return result;
}

} // namespace Rheno::Core

