



#include "NartVersion.h"

static int _Major=5;

static int _Minor=0;

static char _Date[7]="140505";

static char _Time[7]="203000";



FIELDDLLSPEC int NartVersionMajor()
{
    return _Major;
}

FIELDDLLSPEC int NartVersionMinor()
{
    return _Minor;
}

FIELDDLLSPEC char* NartVersionDate()
{
    return _Date;
}

FIELDDLLSPEC char* NartVersionTime()
{
    return _Time;
}
