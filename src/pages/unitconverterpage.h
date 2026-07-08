#pragma once
#include "core/unitconvertercore.h"
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>

class UnitConverterPage : public QWidget {
    Q_OBJECT
public:
    explicit UnitConverterPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

private slots:
    void onCategoryChanged(int index);
    void onFromValueChanged();
    void onFromUnitChanged();
    void onToUnitChanged();

private:
    void setupUI();
    void populate(int category);
    void convert();
    void updateQuickReference(int category);

    QComboBox* m_categoryCombo;
    QLineEdit* m_fromEdit;
    QComboBox* m_fromUnit;
    QComboBox* m_toUnit;
    QLabel*    m_resultLabel;
    QLabel*    m_formulaLabel;
    QLabel*    m_titleLabel;
    QLabel*    m_catLabel;
    QGroupBox* m_convGroup;
    QGroupBox* m_refGroup;

    QList<QList<Rheno::Core::UnitDef>> m_categories;
    QStringList m_categoryNames;
};
