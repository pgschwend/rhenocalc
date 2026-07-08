#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;

class FinancePage : public QWidget {
    Q_OBJECT

public:
    explicit FinancePage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

private:
    void setupUI();
    void recalc();

    QLabel* m_titleLabel = nullptr;
    QLabel* m_resultTitleLabel = nullptr;

    QLineEdit* m_simplePrincipalEdit = nullptr;
    QLineEdit* m_simpleRateEdit = nullptr;
    QLineEdit* m_simplePeriodsEdit = nullptr;
    QLabel* m_simpleResultLabel = nullptr;

    QLineEdit* m_principalEdit = nullptr;
    QLineEdit* m_contribEdit = nullptr;
    QLineEdit* m_rateEdit = nullptr;
    QLineEdit* m_yearsEdit = nullptr;
    QComboBox* m_compoundCombo = nullptr;
    QComboBox* m_contribFreqCombo = nullptr;
    QCheckBox* m_contribBeginCheck = nullptr;

    QLabel* m_futureValueLabel = nullptr;
    QLabel* m_totalContribLabel = nullptr;
    QLabel* m_totalInterestLabel = nullptr;
    QLabel* m_effectiveRateLabel = nullptr;

    bool m_isDark = true;
};
