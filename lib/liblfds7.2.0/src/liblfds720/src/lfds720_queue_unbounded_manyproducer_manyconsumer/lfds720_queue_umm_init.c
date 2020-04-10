/***** includes *****/
#include "lfds720_queue_umm_internal.h"





/****************************************************************************/
void lfds720_queue_umm_init_valid_on_current_logical_core( struct lfds720_queue_umm_state *qummhps,
                                                                  struct lfds720_queue_umm_element *qummhpe_dummy,
                                                                  struct lfds720_smrhp_state *smrhps,
                                                                  void *user_state )
{
  LFDS720_PAL_ASSERT( qummhps != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &qummhps->enqueue % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &qummhps->dequeue % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &qummhps->user_state % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( qummhpe_dummy != NULL );
  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) qummhpe_dummy->next % LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES == 0 );
  // TRD : user_state can be NULL

  /* TRD : qummhpe_dummy is a dummy element, needed for init
           the qummhps->enqueue and qummhps->dequeue counters do not need to be initialized
           but it does no harm to do so, and stops a valgrind complaint
  */

  qummhps->enqueue = qummhpe_dummy;
  qummhps->dequeue = qummhpe_dummy;

  qummhps->smrhps = smrhps;

  qummhpe_dummy->next = NULL;
  qummhpe_dummy->key = NULL;
  qummhpe_dummy->value = NULL;

  qummhps->user_state = user_state;

  lfds720_misc_internal_backoff_init( &qummhps->dequeue_backoff );
  lfds720_misc_internal_backoff_init( &qummhps->enqueue_backoff );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

