/***** includes *****/
#include "lfds720_misc_internal.h"





/****************************************************************************/
void lfds720_misc_internal_backoff_init( struct lfds720_misc_backoff_state *bs )
{
  LFDS720_PAL_ASSERT( bs != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &bs->lock % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );

  bs->lock = LFDS720_MISC_FLAG_LOWERED;
  bs->backoff_iteration_frequency_counters[0] = 0;
  bs->backoff_iteration_frequency_counters[1] = 0;
  bs->metric = 1;
  bs->total_operations = 0;

  return;
}

