#include "architecturedetector.h"

QString ArchitectureDetector::detectArchitecture(const QString &fileName)
{
    if (fileName.contains("x86_64", Qt::CaseInsensitive)) {
        return "x86_64";
    } else if (fileName.contains("aarch64", Qt::CaseInsensitive) || 
              fileName.contains("arm64", Qt::CaseInsensitive)) {
        return "aarch64";
    } else if (fileName.contains("arm", Qt::CaseInsensitive)) {
        return "arm";
    } else {
        return "unknown";
    }
}
