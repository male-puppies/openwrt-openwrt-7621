


#include <stdio.h>
#include "Field.h"
#include "Qc98xxDevice.h"
#include "Qc98xxField.h"
#include "peregrine_reg_map_ART_template.h"
#include "peregrine_2p0_reg_map_ART_template.h"
#include "swift_1p0_reg_map_ART_template.h"

void Qc98xxFieldSelect()
{
	// Swift doesn't support otp.
	if (Qc98xxIsVersion3())
	{
	    FieldSelect(_F_swift_1p0,sizeof(_F_swift_1p0)/sizeof(_F_swift_1p0[0]));
		return;
	}
    
    if (Qc98xxIsVersion1())
    {
	    FieldSelect(_F,sizeof(_F)/sizeof(_F[0]));
    }
    else
    {
	    FieldSelect(_F_peregrine_2p0,sizeof(_F_peregrine_2p0)/sizeof(_F_peregrine_2p0[0]));
    }
}


