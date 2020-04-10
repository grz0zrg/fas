/***** includes *****/
#include "lfds720_ringbuffer_n_internal.h"





/****************************************************************************/
void lfds720_ringbuffer_n_write( struct lfds720_ringbuffer_n_state *rs,
                               void *key,
                               void *value,
                               enum lfds720_misc_flag *overwrite_occurred_flag,
                               void **overwritten_key,
                               void **overwritten_value )
{
  int
    rv = 0;

  struct lfds720_freelist_n_element
    *fe;

  struct lfds720_queue_numm_element
    *qumme;

  struct lfds720_ringbuffer_n_element
    *re = NULL;

  LFDS720_PAL_ASSERT( rs != NULL );
  // TRD : key can be NULL
  // TRD : value can be NULL
  // TRD : overwrite_occurred_flag can be NULL
  // TRD : overwritten_key can be NULL
  // TRD : overwritten_value can be NULL
  // TRD : psts can be NULL

  if( overwrite_occurred_flag != NULL )
    *overwrite_occurred_flag = LFDS720_MISC_FLAG_LOWERED;

  do
  {
    rv = lfds720_freelist_n_threadsafe_pop( &rs->fs, NULL, &fe );

    if( rv == 1 )
      re = LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT( *fe );

    if( rv == 0 )
    {
      // TRD : the queue can return empty as well - remember, we're lock-free; anything could have happened since the previous instruction
      rv = lfds720_queue_numm_dequeue( &rs->qumms, &qumme );

      if( rv == 1 )
      {
        re = LFDS720_QUEUE_NUMM_GET_VALUE_FROM_ELEMENT( *qumme );
        re->qumme_use = (struct lfds720_queue_numm_element *) qumme;

        if( overwrite_occurred_flag != NULL )
          *overwrite_occurred_flag = LFDS720_MISC_FLAG_RAISED;

        if( overwritten_key != NULL )
          *overwritten_key = re->key;

        if( overwritten_value != NULL )
          *overwritten_value = re->value;
      }
    }
  }
  while( rv == 0 );

  re->key = key;
  re->value = value;

  LFDS720_QUEUE_NUMM_SET_VALUE_IN_ELEMENT( *re->qumme_use, re );
  lfds720_queue_numm_enqueue( &rs->qumms, re->qumme_use );

  return;
}

