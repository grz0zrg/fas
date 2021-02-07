#include <stdlib.h>

typedef struct sp_auxdata {
	size_t size;
	void *ptr;
} sp_auxdata;

#include "soundpipe.h"
#include "bar.h"
#include "clip.h"
#include "comb.h"
#include "conv.h"
#include "dcblock.h"
#include "dist.h"
#include "drip.h"
#include "fold.h"
#include "fofilt.h"
#include "lpf18.h"
#include "mode.h"
#include "moogladder.h"
#include "panst.h"
#include "pareq.h"
#include "pdhalf.h"
#include "revsc.h"
#include "streson.h"
#include "tbvcf.h"
#include "waveset.h"

int sp_auxdata_alloc(sp_auxdata *aux, size_t size);
int sp_auxdata_free(sp_auxdata *aux);
