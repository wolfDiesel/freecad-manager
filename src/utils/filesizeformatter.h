#ifndef FILESIZEFORMATTER_H
#define FILESIZEFORMATTER_H

#include <QString>

class FileSizeFormatter
{
public:
    static QString format(qint64 size);
};

#endif
