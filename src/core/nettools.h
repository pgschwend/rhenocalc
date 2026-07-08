#pragma once

#include <QList>
#include <QString>

namespace Rheno::Core {

struct StyledLine {
	QString text;
	QString color;
};

QList<int> parsePorts(const QString& portsExpression);
QString serviceNameForPort(int port);
StyledLine classifyPingLine(const QString& line);
StyledLine formatTraceHopLine(int hop, int rtt, const QString& address);

} // namespace Rheno::Core


