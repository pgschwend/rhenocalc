#include "networkpage.h"

#include "networkcalc.h"
#include "ui/themecolors.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

QLabel* makeValueLabel(QWidget* parent) {
    auto* lbl = new QLabel("-", parent);
    lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return lbl;
}

} // namespace

NetworkPage::NetworkPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyTheme(true);
}

void NetworkPage::setupUI() {
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget(scroll);
    content->setMinimumSize(0, 0);
    content->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    auto* root = new QVBoxLayout(content);
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    m_titleLabel = new QLabel("Network Tools", this);
    root->addWidget(m_titleLabel);

    m_subnetGroup = new QGroupBox("Subnet Calculator", this);
    auto* subnetGrid = new QGridLayout(m_subnetGroup);
    subnetGrid->setHorizontalSpacing(8);
    subnetGrid->setVerticalSpacing(6);

    subnetGrid->addWidget(new QLabel("CIDR input:", this), 0, 0);
    m_cidrEdit = new QLineEdit(this);
    m_cidrEdit->setMinimumWidth(0);
    m_cidrEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_cidrEdit->setPlaceholderText("e.g. 192.168.1.34/24 or 192.168.1.34/255.255.255.0");
    subnetGrid->addWidget(m_cidrEdit, 0, 1, 1, 3);

    subnetGrid->addWidget(new QLabel("IP address:", this), 1, 0);
    m_ipEdit = new QLineEdit(this);
    m_ipEdit->setMinimumWidth(0);
    m_ipEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_ipEdit->setPlaceholderText("e.g. 192.168.1.34");
    subnetGrid->addWidget(m_ipEdit, 1, 1);

    subnetGrid->addWidget(new QLabel("Prefix:", this), 1, 2);
    m_prefixSpin = new QSpinBox(this);
    m_prefixSpin->setRange(0, 32);
    m_prefixSpin->setValue(24);
    m_prefixSpin->setPrefix("/");
    subnetGrid->addWidget(m_prefixSpin, 1, 3);

    subnetGrid->addWidget(new QLabel("Subnet mask:", this), 2, 0);
    m_maskEdit = new QLineEdit(this);
    m_maskEdit->setMinimumWidth(0);
    m_maskEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_maskEdit->setPlaceholderText("optional if CIDR/prefix is set");
    subnetGrid->addWidget(m_maskEdit, 2, 1);

    subnetGrid->addWidget(new QLabel("Desired devices:", this), 2, 2);
    m_devicesSpin = new QSpinBox(this);
    m_devicesSpin->setRange(1, 1000000);
    m_devicesSpin->setValue(254);
    m_devicesSpin->setMinimumWidth(0);
    subnetGrid->addWidget(m_devicesSpin, 2, 3);

    subnetGrid->setColumnStretch(1, 2);
    subnetGrid->setColumnStretch(3, 1);

    m_calcBtn = new QPushButton("Calculate subnet", this);
    m_planBtn = new QPushButton("Plan prefix by devices", this);
    subnetGrid->addWidget(m_calcBtn, 3, 2);
    subnetGrid->addWidget(m_planBtn, 3, 3);

    root->addWidget(m_subnetGroup);

    m_outputGroup = new QGroupBox("Results", this);
    auto* outGrid = new QGridLayout(m_outputGroup);
    outGrid->setHorizontalSpacing(10);
    outGrid->setVerticalSpacing(6);

    outGrid->addWidget(new QLabel("Short notation:", this), 0, 0);
    m_cidrValue = makeValueLabel(this);
    m_cidrValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_cidrValue, 0, 1);

    outGrid->addWidget(new QLabel("Network address:", this), 1, 0);
    m_networkValue = makeValueLabel(this);
    m_networkValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_networkValue, 1, 1);

    outGrid->addWidget(new QLabel("Broadcast:", this), 2, 0);
    m_broadcastValue = makeValueLabel(this);
    m_broadcastValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_broadcastValue, 2, 1);

    outGrid->addWidget(new QLabel("First host:", this), 3, 0);
    m_firstHostValue = makeValueLabel(this);
    m_firstHostValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_firstHostValue, 3, 1);

    outGrid->addWidget(new QLabel("Last host:", this), 4, 0);
    m_lastHostValue = makeValueLabel(this);
    m_lastHostValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_lastHostValue, 4, 1);

    outGrid->addWidget(new QLabel("Wildcard mask:", this), 5, 0);
    m_wildcardValue = makeValueLabel(this);
    m_wildcardValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_wildcardValue, 5, 1);

    outGrid->addWidget(new QLabel("Usable hosts:", this), 6, 0);
    m_hostsValue = makeValueLabel(this);
    m_hostsValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_hostsValue, 6, 1);

    outGrid->addWidget(new QLabel("IP class:", this), 7, 0);
    m_ipClassValue = makeValueLabel(this);
    m_ipClassValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_ipClassValue, 7, 1);

    outGrid->addWidget(new QLabel("Scope:", this), 8, 0);
    m_scopeValue = makeValueLabel(this);
    m_scopeValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    outGrid->addWidget(m_scopeValue, 8, 1);

    outGrid->setColumnStretch(1, 1);

    root->addWidget(m_outputGroup);

    m_toolsGroup = new QGroupBox("IP Number Converter", this);
    auto* toolsLayout = new QHBoxLayout(m_toolsGroup);
    toolsLayout->addWidget(new QLabel("IPv4 as uint32:", this));
    m_uintEdit = new QLineEdit(this);
    m_uintEdit->setMinimumWidth(0);
    m_uintEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_uintEdit->setPlaceholderText("e.g. 3232235777");
    toolsLayout->addWidget(m_uintEdit);
    m_ipToUintBtn = new QPushButton("IP -> uint32", this);
    m_uintToIpBtn = new QPushButton("uint32 -> IP", this);
    toolsLayout->addWidget(m_ipToUintBtn);
    toolsLayout->addWidget(m_uintToIpBtn);
    root->addWidget(m_toolsGroup);
    toolsLayout->setStretch(1, 1);

    m_statusLabel = new QLabel("Enter CIDR/IP and click Calculate.", this);
    m_statusLabel->setWordWrap(true);
    root->addWidget(m_statusLabel);
    root->addStretch();

    scroll->setWidget(content);
    outer->addWidget(scroll);

    connect(m_calcBtn, &QPushButton::clicked, this, &NetworkPage::onCalculateClicked);
    connect(m_planBtn, &QPushButton::clicked, this, &NetworkPage::onPlanHostsClicked);
    connect(m_ipToUintBtn, &QPushButton::clicked, this, &NetworkPage::onIpToUintClicked);
    connect(m_uintToIpBtn, &QPushButton::clicked, this, &NetworkPage::onUintToIpClicked);
}

void NetworkPage::setStatus(const QString& text, bool isError) {
    if (isError)
        m_statusLabel->setText(QString("Error: %1").arg(text));
    else
        m_statusLabel->setText(text);
}

void NetworkPage::calculateAndRender() {
    quint32 ip = 0;
    int prefix = m_prefixSpin->value();
    QString err;

    const QString cidrText = m_cidrEdit->text().trimmed();
    if (!cidrText.isEmpty()) {
        if (!NetworkCalc::parseCidr(cidrText, ip, prefix, &err)) {
            setStatus(err, true);
            return;
        }
        m_ipEdit->setText(NetworkCalc::toIpv4(ip));
        m_prefixSpin->setValue(prefix);
        m_maskEdit->setText(NetworkCalc::toIpv4(NetworkCalc::prefixToMask(prefix)));
    } else {
        if (!NetworkCalc::parseIpv4(m_ipEdit->text(), ip, &err)) {
            setStatus(err, true);
            return;
        }

        const QString maskText = m_maskEdit->text().trimmed();
        if (!maskText.isEmpty()) {
            quint32 mask = 0;
            if (!NetworkCalc::parseIpv4(maskText, mask, &err) || !NetworkCalc::maskToPrefix(mask, prefix, &err)) {
                setStatus(err, true);
                return;
            }
            m_prefixSpin->setValue(prefix);
        }
    }

    const NetworkCalc::SubnetResult r = NetworkCalc::calculateSubnet(ip, prefix);
    if (!r.valid) {
        setStatus(r.error, true);
        return;
    }

    m_maskEdit->setText(NetworkCalc::toIpv4(r.mask));
    m_cidrValue->setText(QString("%1/%2").arg(NetworkCalc::toIpv4(r.ip)).arg(r.prefix));
    m_networkValue->setText(NetworkCalc::toIpv4(r.network));
    m_broadcastValue->setText(NetworkCalc::toIpv4(r.broadcast));
    m_firstHostValue->setText(NetworkCalc::toIpv4(r.firstHost));
    m_lastHostValue->setText(NetworkCalc::toIpv4(r.lastHost));
    m_wildcardValue->setText(NetworkCalc::toIpv4(r.wildcard));
    m_hostsValue->setText(QString::number(r.usableHosts));

    const NetworkCalc::IpInfo info = NetworkCalc::classifyIp(r.ip);
    m_ipClassValue->setText(info.ipClass);
    m_scopeValue->setText(info.scope);

    const quint64 need = static_cast<quint64>(m_devicesSpin->value());
    if (r.usableHosts >= need) {
        setStatus(QString("Subnet supports %1 devices (need %2).")
                      .arg(r.usableHosts)
                      .arg(need), false);
    } else {
        setStatus(QString("Subnet supports %1 devices but you need %2. Use planning button.")
                      .arg(r.usableHosts)
                      .arg(need), false);
    }
}

void NetworkPage::onCalculateClicked() {
    calculateAndRender();
}

void NetworkPage::onPlanHostsClicked() {
    QString err;
    int prefix = 24;
    const quint64 devices = static_cast<quint64>(m_devicesSpin->value());
    if (!NetworkCalc::minimalPrefixForHosts(devices, prefix, &err)) {
        setStatus(err, true);
        return;
    }

    m_prefixSpin->setValue(prefix);
    m_maskEdit->setText(NetworkCalc::toIpv4(NetworkCalc::prefixToMask(prefix)));
    setStatus(QString("Planned prefix /%1 for %2 devices.").arg(prefix).arg(devices), false);

    if (!m_ipEdit->text().trimmed().isEmpty() || !m_cidrEdit->text().trimmed().isEmpty())
        calculateAndRender();
}

void NetworkPage::onIpToUintClicked() {
    quint32 ip = 0;
    QString err;
    if (!NetworkCalc::parseIpv4(m_ipEdit->text(), ip, &err)) {
        setStatus(err, true);
        return;
    }

    m_uintEdit->setText(QString::number(static_cast<qulonglong>(ip)));
    setStatus("Converted IP to uint32.", false);
}

void NetworkPage::onUintToIpClicked() {
    bool ok = false;
    const qulonglong value = m_uintEdit->text().trimmed().toULongLong(&ok);
    if (!ok || value > 0xFFFFFFFFull) {
        setStatus("uint32 value must be between 0 and 4294967295.", true);
        return;
    }

    const quint32 ip = static_cast<quint32>(value);
    m_ipEdit->setText(NetworkCalc::toIpv4(ip));
    setStatus("Converted uint32 to IP.", false);
}

void NetworkPage::applyTheme(bool dark) {
    const QString grpS = ThemeColors::unitGroupStyle(dark);
    const QString fldS = ThemeColors::unitFieldStyle(dark);
    const QString resS = ThemeColors::unitResultStyle(dark);
    const QString ttlS = ThemeColors::unitTitleStyle(dark);
    const QString frmS = ThemeColors::unitFormulaStyle(dark);

    m_titleLabel->setStyleSheet(ttlS);
    m_subnetGroup->setStyleSheet(grpS);
    m_outputGroup->setStyleSheet(grpS);
    m_toolsGroup->setStyleSheet(grpS);

    m_cidrEdit->setStyleSheet(fldS);
    m_ipEdit->setStyleSheet(fldS);
    m_maskEdit->setStyleSheet(fldS);
    m_uintEdit->setStyleSheet(fldS);

    m_networkValue->setStyleSheet(resS);
    m_broadcastValue->setStyleSheet(resS);
    m_firstHostValue->setStyleSheet(resS);
    m_lastHostValue->setStyleSheet(resS);
    m_wildcardValue->setStyleSheet(resS);
    m_cidrValue->setStyleSheet(resS);
    m_hostsValue->setStyleSheet(resS);
    m_ipClassValue->setStyleSheet(resS);
    m_scopeValue->setStyleSheet(resS);
    m_statusLabel->setStyleSheet(frmS);
}

