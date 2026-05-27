#include "crchashpage.h"

#include "ui/themecolors.h"

#include <QComboBox>
#include <QCryptographicHash>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

enum class Algorithm {
    Crc8Maxim,
    Crc8J1850,
    Crc16Modbus,
    CrcCcitt,
    Crc32Iso3309,
    Crc32C,
    MurmurHash3,
    XxHash32,
    Md5,
    Sha1,
    Sha256,
    Sha512,
};

struct CrcSpec {
    int width;
    quint64 poly;
    quint64 init;
    quint64 xorOut;
    bool refin;
    bool refout;
};

quint64 reflectBits(quint64 value, int bits) {
    quint64 out = 0;
    for (int i = 0; i < bits; ++i) {
        if (value & (1ULL << i))
            out |= (1ULL << (bits - 1 - i));
    }
    return out;
}

quint64 crcCompute(const QByteArray& data, const CrcSpec& spec) {
    const quint64 topBit = 1ULL << (spec.width - 1);
    const quint64 mask = (spec.width == 64) ? ~0ULL : ((1ULL << spec.width) - 1ULL);

    quint64 crc = spec.init & mask;
    for (unsigned char byte : data) {
        quint64 cur = byte;
        if (spec.refin)
            cur = reflectBits(cur, 8);

        crc ^= (cur << (spec.width - 8));
        for (int i = 0; i < 8; ++i) {
            if (crc & topBit)
                crc = (crc << 1) ^ spec.poly;
            else
                crc <<= 1;
            crc &= mask;
        }
    }

    if (spec.refout)
        crc = reflectBits(crc, spec.width);

    crc ^= spec.xorOut;
    return crc & mask;
}

quint32 rotl32(quint32 v, int r) {
    return (v << r) | (v >> (32 - r));
}

quint32 fmix32(quint32 h) {
    h ^= h >> 16;
    h *= 0x85ebca6bU;
    h ^= h >> 13;
    h *= 0xc2b2ae35U;
    h ^= h >> 16;
    return h;
}

quint32 murmurHash3_x86_32(const QByteArray& data, quint32 seed = 0) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data.constData());
    const int len = data.size();
    const int nblocks = len / 4;

    quint32 h1 = seed;
    constexpr quint32 c1 = 0xcc9e2d51U;
    constexpr quint32 c2 = 0x1b873593U;

    for (int i = 0; i < nblocks; ++i) {
        const int idx = i * 4;
        quint32 k1 = static_cast<quint32>(bytes[idx])
            | (static_cast<quint32>(bytes[idx + 1]) << 8)
            | (static_cast<quint32>(bytes[idx + 2]) << 16)
            | (static_cast<quint32>(bytes[idx + 3]) << 24);

        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1 * 5U + 0xe6546b64U;
    }

    quint32 k1 = 0;
    const unsigned char* tail = bytes + (nblocks * 4);
    switch (len & 3) {
    case 3:
        k1 ^= static_cast<quint32>(tail[2]) << 16;
        [[fallthrough]];
    case 2:
        k1 ^= static_cast<quint32>(tail[1]) << 8;
        [[fallthrough]];
    case 1:
        k1 ^= static_cast<quint32>(tail[0]);
        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;
        h1 ^= k1;
        break;
    default:
        break;
    }

    h1 ^= static_cast<quint32>(len);
    return fmix32(h1);
}

quint32 xxHash32(const QByteArray& data, quint32 seed = 0) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(data.constData());
    int len = data.size();
    const unsigned char* end = p + len;

    constexpr quint32 p1 = 2654435761U;
    constexpr quint32 p2 = 2246822519U;
    constexpr quint32 p3 = 3266489917U;
    constexpr quint32 p4 = 668265263U;
    constexpr quint32 p5 = 374761393U;

    auto read32 = [](const unsigned char* ptr) {
        return static_cast<quint32>(ptr[0])
            | (static_cast<quint32>(ptr[1]) << 8)
            | (static_cast<quint32>(ptr[2]) << 16)
            | (static_cast<quint32>(ptr[3]) << 24);
    };

    quint32 h32 = 0;
    if (len >= 16) {
        quint32 v1 = seed + p1 + p2;
        quint32 v2 = seed + p2;
        quint32 v3 = seed;
        quint32 v4 = seed - p1;

        const unsigned char* limit = end - 16;
        do {
            v1 += read32(p) * p2;
            v1 = rotl32(v1, 13);
            v1 *= p1;
            p += 4;

            v2 += read32(p) * p2;
            v2 = rotl32(v2, 13);
            v2 *= p1;
            p += 4;

            v3 += read32(p) * p2;
            v3 = rotl32(v3, 13);
            v3 *= p1;
            p += 4;

            v4 += read32(p) * p2;
            v4 = rotl32(v4, 13);
            v4 *= p1;
            p += 4;
        } while (p <= limit);

        h32 = rotl32(v1, 1) + rotl32(v2, 7) + rotl32(v3, 12) + rotl32(v4, 18);
    } else {
        h32 = seed + p5;
    }

    h32 += static_cast<quint32>(len);

    while (p + 4 <= end) {
        h32 += read32(p) * p3;
        h32 = rotl32(h32, 17) * p4;
        p += 4;
    }

    while (p < end) {
        h32 += (*p) * p5;
        h32 = rotl32(h32, 11) * p1;
        ++p;
    }

    h32 ^= h32 >> 15;
    h32 *= p2;
    h32 ^= h32 >> 13;
    h32 *= p3;
    h32 ^= h32 >> 16;

    return h32;
}

QString toHex(quint64 value, int widthHex) {
    return QString("0x%1").arg(QString::number(value, 16).toUpper().rightJustified(widthHex, '0'));
}

QString crcHex(const QByteArray& data, const CrcSpec& spec) {
    const int digits = (spec.width + 3) / 4;
    return toHex(crcCompute(data, spec), digits);
}

QString hashHex(const QByteArray& data, QCryptographicHash::Algorithm alg) {
    return QCryptographicHash::hash(data, alg).toHex().toUpper();
}

} // namespace

CrcHashPage::CrcHashPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyTheme(true);
    recalculate();
}

void CrcHashPage::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    m_titleLabel = new QLabel("CRC and Hash Tools", this);
    root->addWidget(m_titleLabel);

    m_inputGroup = new QGroupBox("Input", this);
    auto* inGrid = new QGridLayout(m_inputGroup);
    inGrid->setHorizontalSpacing(8);
    inGrid->setVerticalSpacing(6);

    m_algoLabel = new QLabel("Algorithm:", this);
    m_algoCombo = new QComboBox(this);
    m_algoCombo->addItem("CRC-8 MAXIM", static_cast<int>(Algorithm::Crc8Maxim));
    m_algoCombo->addItem("CRC-8 J1850", static_cast<int>(Algorithm::Crc8J1850));
    m_algoCombo->addItem("CRC-16 Modbus", static_cast<int>(Algorithm::Crc16Modbus));
    m_algoCombo->addItem("CRC-CCITT", static_cast<int>(Algorithm::CrcCcitt));
    m_algoCombo->addItem("CRC-32 ISO 3309", static_cast<int>(Algorithm::Crc32Iso3309));
    m_algoCombo->addItem("CRC-32C (Castagnoli)", static_cast<int>(Algorithm::Crc32C));
    m_algoCombo->addItem("MurmurHash3 (x86_32)", static_cast<int>(Algorithm::MurmurHash3));
    m_algoCombo->addItem("xxHash32", static_cast<int>(Algorithm::XxHash32));
    m_algoCombo->addItem("MD5", static_cast<int>(Algorithm::Md5));
    m_algoCombo->addItem("SHA-1", static_cast<int>(Algorithm::Sha1));
    m_algoCombo->addItem("SHA-256", static_cast<int>(Algorithm::Sha256));
    m_algoCombo->addItem("SHA-512", static_cast<int>(Algorithm::Sha512));

    inGrid->addWidget(m_algoLabel, 0, 0);
    inGrid->addWidget(m_algoCombo, 0, 1);

    auto* inputLabel = new QLabel("Input (UTF-8 text):", this);
    m_inputEdit = new QPlainTextEdit(this);
    m_inputEdit->setPlaceholderText("Enter text or payload here...");
    m_inputEdit->setMinimumHeight(140);

    inGrid->addWidget(inputLabel, 1, 0, 1, 2);
    inGrid->addWidget(m_inputEdit, 2, 0, 1, 2);

    root->addWidget(m_inputGroup);

    m_outputGroup = new QGroupBox("Output", this);
    auto* outGrid = new QGridLayout(m_outputGroup);
    outGrid->setHorizontalSpacing(8);
    outGrid->setVerticalSpacing(6);

    auto* outLabel = new QLabel("Checksum / Hash:", this);
    m_outputEdit = new QLineEdit(this);
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setPlaceholderText("Result appears here");

    m_copyBtn = new QPushButton("Copy", this);

    outGrid->addWidget(outLabel, 0, 0);
    outGrid->addWidget(m_outputEdit, 0, 1);
    outGrid->addWidget(m_copyBtn, 0, 2);
    outGrid->setColumnStretch(1, 1);

    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setWordWrap(true);
    outGrid->addWidget(m_statusLabel, 1, 0, 1, 3);

    root->addWidget(m_outputGroup);
    root->addStretch();

    connect(m_algoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CrcHashPage::recalculate);
    connect(m_inputEdit, &QPlainTextEdit::textChanged, this, &CrcHashPage::recalculate);
    connect(m_copyBtn, &QPushButton::clicked, this, &CrcHashPage::copyResult);
}

void CrcHashPage::recalculate() {
    const QByteArray data = m_inputEdit->toPlainText().toUtf8();
    const auto alg = static_cast<Algorithm>(m_algoCombo->currentData().toInt());

    QString result;
    switch (alg) {
    case Algorithm::Crc8Maxim:
        result = crcHex(data, {8, 0x31, 0x00, 0x00, true, true});
        break;
    case Algorithm::Crc8J1850:
        result = crcHex(data, {8, 0x1D, 0xFF, 0xFF, false, false});
        break;
    case Algorithm::Crc16Modbus:
        result = crcHex(data, {16, 0x8005, 0xFFFF, 0x0000, true, true});
        break;
    case Algorithm::CrcCcitt:
        result = crcHex(data, {16, 0x1021, 0xFFFF, 0x0000, false, false});
        break;
    case Algorithm::Crc32Iso3309:
        result = crcHex(data, {32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true});
        break;
    case Algorithm::Crc32C:
        result = crcHex(data, {32, 0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, true, true});
        break;
    case Algorithm::MurmurHash3:
        result = toHex(murmurHash3_x86_32(data, 0), 8);
        break;
    case Algorithm::XxHash32:
        result = toHex(xxHash32(data, 0), 8);
        break;
    case Algorithm::Md5:
        result = hashHex(data, QCryptographicHash::Md5);
        break;
    case Algorithm::Sha1:
        result = hashHex(data, QCryptographicHash::Sha1);
        break;
    case Algorithm::Sha256:
        result = hashHex(data, QCryptographicHash::Sha256);
        break;
    case Algorithm::Sha512:
        result = hashHex(data, QCryptographicHash::Sha512);
        break;
    }

    m_outputEdit->setText(result);
    m_statusLabel->setText(QString("%1 bytes processed").arg(data.size()));
}

void CrcHashPage::copyResult() {
    m_outputEdit->selectAll();
    m_outputEdit->copy();
    m_outputEdit->deselect();
    m_statusLabel->setText("Result copied to clipboard");
}

void CrcHashPage::applyTheme(bool dark) {
    const QString grpS = ThemeColors::unitGroupStyle(dark);
    const QString fldS = ThemeColors::unitFieldStyle(dark);
    const QString resS = ThemeColors::unitResultStyle(dark);
    const QString ttlS = ThemeColors::unitTitleStyle(dark);
    const QString frmS = ThemeColors::unitFormulaStyle(dark);

    m_titleLabel->setStyleSheet(ttlS);
    m_inputGroup->setStyleSheet(grpS);
    m_outputGroup->setStyleSheet(grpS);

    m_inputEdit->setStyleSheet(fldS);
    m_outputEdit->setStyleSheet(resS);
    m_statusLabel->setStyleSheet(frmS);
}

