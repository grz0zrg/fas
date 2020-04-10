/***** includes *****/
#include "lfds720_queue_npumm_internal.h"





/****************************************************************************/
void lfds720_queue_npumm_init_valid_on_current_logical_core( struct lfds720_queue_npumm_state *qumms,
                                                             struct lfds720_queue_npumm_element *qumme_dummy,
                                                             void *user_state )
{
  LFDS720_PAL_ASSERT( qumms != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &qumms->enqueue % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &qumms->dequeue % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &qumms->user_state % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( qumme_dummy != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) qumme_dummy->next % LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES == 0 );
  // TRD : user_state can be NULL

  /* TRD : qumme_dummy is a dummy element, needed for init
           the qumms->enqueue and qumms->dequeue counters do not need to be initialized
           but it does no harm to do so, and stops a valgrind complaint
  */

  LFDS720_PRNG_GENERATE( lfds720_misc_globals.ps, qumms->aba_counter );

  qumms->enqueue[LFDS720_MISC_OFFSET]  = LFDS720_MISC_POINTER_TO_OFFSET( qumms, qumme_dummy );
  qumms->enqueue[LFDS720_MISC_COUNTER] = 0;
  qumms->dequeue[LFDS720_MISC_OFFSET]  = LFDS720_MISC_POINTER_TO_OFFSET( qumms, qumme_dummy );
  qumms->dequeue[LFDS720_MISC_COUNTER] = 0;

  qumme_dummy->next[LFDS720_MISC_OFFSET] = 0;
  // TRD : no need here for an atomic add as we have a store barrier and force store below
  qumme_dummy->next[LFDS720_MISC_COUNTER] = qumms->aba_counter++;
  qumme_dummy->key = NULL;
  qumme_dummy->value = NULL;

  qumms->user_state = user_state;

  lfds720_misc_internal_backoff_init( &qumms->dequeue_backoff );
  lfds720_misc_internal_backoff_init( &qumms->enqueue_backoff );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

