#pragma once

#include <stdexcept>

#include <qfile.h>
#include <qstring.h>

/// @brief 读取整个文件的内容。
/// @throw std::runtime_error
inline QString readFileContent(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Failed to open file: " + filePath.toStdString());
    return QString::fromUtf8(file.readAll());
}
