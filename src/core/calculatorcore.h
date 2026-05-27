#pragma once

#include <QString>

namespace CalculatorCore {

class CalculatorEngine {
public:
	void setBase(int base);
	void setWordBits(int bits);
	void setBigMode(bool enabled);

	int base() const { return m_base; }
	int wordBits() const { return m_wordBits; }
	bool bigMode() const { return m_bigMode; }

	QString displayText() const;
	QString expressionText() const { return m_expression; }

	void pressDigit(const QString& digit);
	void pressOperator(const QString& op);
	void equals();
	void clearAll();
	void clearEntry();
	void backspace();
	void negate();
	void setPi();
	void applyBitwiseOrFunction(const QString& op);

private:
	long long m_current = 0;
	long long m_accumulator = 0;
	double m_currentDouble = 0.0;
	double m_accumulatorDouble = 0.0;
	QString m_pendingOp;
	QString m_inputString;
	QString m_expression;
	bool m_newInput = true;
	bool m_floatMode = false;
	bool m_bigMode = false;
	int m_base = 10;
	int m_wordBits = 32;
	long long m_memory = 0;

	QString m_bigCurrent = "0";
	QString m_bigAccumulator = "0";
	QString m_bigMemory = "0";
};

long long maskToWidth(long long value, int bits);
QString toBaseString(long long value, int base, int bits);
long long fromBaseString(const QString& text, int base);
QString formatDouble(double value);

long long applyBinary(long long a, long long b, const QString& op);
double applyBinary(double a, double b, const QString& op);

long long applyUnaryInt(long long value, const QString& op, int bits);
double applyUnaryDouble(double value, const QString& op);

} // namespace CalculatorCore

