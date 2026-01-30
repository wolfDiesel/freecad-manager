#ifndef VERSIONEXTRACTOR_H
#define VERSIONEXTRACTOR_H

#include <QString>

class VersionExtractor
{
public:
    static QString extractVersionFromFileName(const QString &fileName);
};

#endif
