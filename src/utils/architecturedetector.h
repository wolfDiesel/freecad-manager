#ifndef ARCHITECTUREDETECTOR_H
#define ARCHITECTUREDETECTOR_H

#include <QString>

class ArchitectureDetector
{
public:
    static QString detectArchitecture(const QString &fileName);
};

#endif
