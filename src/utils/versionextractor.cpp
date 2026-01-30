#include "versionextractor.h"
#include <QRegularExpression>

QString VersionExtractor::extractVersionFromFileName(const QString &fileName)
{
    // Паттерн для стабильных версий: FreeCAD_1.0.2
    QRegularExpression stableRx("FreeCAD[_\\-](\\d+\\.\\d+\\.\\d+)");
    QRegularExpressionMatch stableMatch = stableRx.match(fileName);
    if (stableMatch.hasMatch()) {
        return stableMatch.captured(1);
    }
    
    // Паттерн для weekly: FreeCAD_weekly-2026.01.14
    QRegularExpression weeklyRx("FreeCAD[_\\-]weekly[_\\-](\\d{4}\\.\\d{2}\\.\\d{2})");
    QRegularExpressionMatch weeklyMatch = weeklyRx.match(fileName);
    if (weeklyMatch.hasMatch()) {
        return "weekly-" + weeklyMatch.captured(1);
    }
    
    // Паттерн для других форматов
    QRegularExpression genericRx("FreeCAD[_\\-]([\\d\\.]+)");
    QRegularExpressionMatch genericMatch = genericRx.match(fileName);
    if (genericMatch.hasMatch()) {
        return genericMatch.captured(1);
    }
    
    return QString();
}
