/***** includes *****/
#include "lfds720_ringbuffer_n_internal.h"





/****************************************************************************/
void lfds720_ringbuffer_n_init_valid_on_current_logical_core( struct lfds720_ringbuffer_n_state *rs,
                                                              struct lfds720_ringbuffer_n_element *re_array_inc_dummy,
                                                              lfds720_pal_uint_t number_elements_inc_dummy,
                                                              void *user_state )
{
  lfds720_pal_uint_t
    loop;

  LFDS720_PAL_ASSERT( rs != NULL );
  LFDS720_PAL_ASSERT( re_array_inc_dummy != NULL );
  LFDS720_PAL_ASSERT( number_elements_inc_dummy >= 2 );
  // TRD : user_state can be NULL

  rs->user_state = user_state;

  re_array_inc_dummy[0].qumme_use = &re_array_inc_dummy[0].qumme;

  lfds720_freelist_n_init_valid_on_current_logical_core( &rs->fs, rs );
  lfds720_queue_numm_init_valid_on_current_logical_core( &rs->qumms, &re_array_inc_dummy[0].qumme, rs );

  for( loop = 1 ; loop < number_elements_inc_dummy ; loop++ )
  {
    re_array_inc_dummy[loop].qumme_use = &re_array_inc_dummy[loop].qumme;
    LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT( re_array_inc_dummy[loop].fe, &re_array_inc_dummy[loop] );
    lfds720_freelist_n_threadsafe_push( &rs->fs, NULL, &re_array_inc_dummy[loop].fe );
  }

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

