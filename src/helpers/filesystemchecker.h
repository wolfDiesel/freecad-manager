#ifndef FILESYSTEMCHECKER_H
#define FILESYSTEMCHECKER_H

#include <QString>

class FileSystemChecker
{
public:
    static bool isFileDownloaded(const QString &fileName, const QString &workingDir);

private:
    static bool checkByVersion(const QString &fileName, const QString &workingDir);
};

#endif
