#pragma once

#include <QWidget>

class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

class NetworkPage : public QWidget {
    Q_OBJECT

public:
    explicit NetworkPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

private slots:
    void onCalculateClicked();
    void onPlanHostsClicked();
    void onIpToUintClicked();
    void onUintToIpClicked();

private:
    void setupUI();
    void calculateAndRender();
    void setStatus(const QString& text, bool isError);

    QLabel* m_titleLabel = nullptr;
    QGroupBox* m_subnetGroup = nullptr;
    QGroupBox* m_outputGroup = nullptr;
    QGroupBox* m_toolsGroup = nullptr;

    QLineEdit* m_cidrEdit = nullptr;
    QLineEdit* m_ipEdit = nullptr;
    QSpinBox* m_prefixSpin = nullptr;
    QLineEdit* m_maskEdit = nullptr;
    QSpinBox* m_devicesSpin = nullptr;
    QPushButton* m_calcBtn = nullptr;
    QPushButton* m_planBtn = nullptr;

    QLabel* m_networkValue = nullptr;
    QLabel* m_broadcastValue = nullptr;
    QLabel* m_firstHostValue = nullptr;
    QLabel* m_lastHostValue = nullptr;
    QLabel* m_wildcardValue = nullptr;
    QLabel* m_cidrValue = nullptr;
    QLabel* m_hostsValue = nullptr;
    QLabel* m_ipClassValue = nullptr;
    QLabel* m_scopeValue = nullptr;
    QLabel* m_statusLabel = nullptr;

    QLineEdit* m_uintEdit = nullptr;
    QLineEdit* m_converterIpEdit = nullptr;
    QPushButton* m_ipToUintBtn = nullptr;
    QPushButton* m_uintToIpBtn = nullptr;
};

