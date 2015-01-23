#include "tableone.h"
#include "../crypto_crt/crypto.hpp"
#include <iostream>

TableOne::TableOne(QObject*)
{
}

void TableOne::TableOneComp(prepare_table * table)
{
    ecb_prepare_table(table);
}
