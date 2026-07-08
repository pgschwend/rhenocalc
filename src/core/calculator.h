#pragma once

#include "calc/calcparser.h"
#include <QStringList>

namespace Rheno::Core {

class CalculatorEngine {
public:
	void setBase(int base);
	void setWordBits(int bits);
	void setBigMode(bool enabled);
	bool isClearState() const;

	int base() const { return m_base; }
	int wordBits() const { return m_wordBits; }
	bool bigMode() const { return m_bigMode; }

	QString displayText() const;
	QString expressionText() const { return m_expression; }

	void pressDigit(const QString& digit);
	void pressLeftParen();
	void pressRightParen();
	void pressOperator(const QString& op);
	void equals();
	void clearAll();
	void clearAllAndMemory();
	void clearEntry();
	void backspace();
	void negate();
	void setPi();
	void setEuler();
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

	BigDecimal m_bigCurrent{0};
	BigDecimal m_bigAccumulator{0};
	BigDecimal m_bigMemory{0};
	QStringList m_infixTokens;
	int m_openParens = 0;

	QString currentOperandToken() const;
	QString formatTokenForExpression(const QString& token) const;
	QString expressionFromTokens(const QStringList& tokens) const;
	void syncExpressionOperand();
	void resetExpressionBuilder();
};

} // namespace Rheno::Core
