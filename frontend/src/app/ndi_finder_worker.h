#pragma once

#include <qobject.h>
#include <qlist.h>
#include <qpair.h>
#include <qstring.h>

class NdiFindWorker : public QObject
{
    Q_OBJECT

public:
    explicit NdiFindWorker(QObject* parent = nullptr);

    void doFind();

signals:
    void finished(const QList<QPair<QString, QString>>& sources, bool success);
};
