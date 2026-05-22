#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

class UnitConverterPage : public QWidget {
    Q_OBJECT
public:
    explicit UnitConverterPage(QWidget* parent = nullptr);

private slots:
    void onCategoryChanged(int index);
    void onFromValueChanged();
    void onFromUnitChanged();
    void onToUnitChanged();

private:
    void setupUI();
    void populate(int category);
    void convert();

    QComboBox* m_categoryCombo;
    QLineEdit* m_fromEdit;
    QComboBox* m_fromUnit;
    QComboBox* m_toUnit;
    QLabel*    m_resultLabel;
    QLabel*    m_formulaLabel;

    struct UnitDef { QString name; double toBase; }; // toBase: multiply to get SI base unit
    QList<QList<UnitDef>> m_categories;
    QStringList m_categoryNames;
};

