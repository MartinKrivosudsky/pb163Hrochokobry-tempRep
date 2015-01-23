#ifndef TABLEONE_H
#define TABLEONE_H

#include <QThread>
#include <string>
#include <QString>

struct prepare_table;

class TableOne : public QThread
{ Q_OBJECT

public:
    TableOne(QObject*);

public slots:
    void TableOneComp(prepare_table * table);
};

#endif // TABLEONE_H
