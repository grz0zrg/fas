/***** includes *****/
#include "lfds720_prng_internal.h"





/****************************************************************************/
void lfds720_prng_init_valid_on_current_logical_core( struct lfds720_prng_state *ps, lfds720_pal_uint_t seed )
{
  LFDS720_PAL_ASSERT( ps != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &ps->entropy % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  // TRD : seed can be any value in its range (unlike for the mixing function)

  LFDS720_PRNG_ST_MIXING_FUNCTION( seed );

  ps->entropy = seed;

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}





/****************************************************************************/
void lfds720_prng_st_init( struct lfds720_prng_st_state *psts, lfds720_pal_uint_t seed )
{
  LFDS720_PAL_ASSERT( psts != NULL );
  LFDS720_PAL_ASSERT( seed != 0 );

  LFDS720_PRNG_ST_MIXING_FUNCTION( seed );

  psts->entropy = seed;

  return;
}

