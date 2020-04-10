/***** includes *****/
#include "lfds720_ringbuffer_n_internal.h"





/****************************************************************************/
int lfds720_ringbuffer_n_read( struct lfds720_ringbuffer_n_state *rs,
                             void **key,
                             void **value )
{
  int
    rv;

  struct lfds720_queue_numm_element
    *qumme;

  struct lfds720_ringbuffer_n_element
    *re;

  LFDS720_PAL_ASSERT( rs != NULL );
  // TRD : key can be NULL
  // TRD : value can be NULL
  // TRD : psts can be NULL

  rv = lfds720_queue_numm_dequeue( &rs->qumms, &qumme );

  if( rv == 1 )
  {
    re = LFDS720_QUEUE_NUMM_GET_VALUE_FROM_ELEMENT( *qumme );
    re->qumme_use = (struct lfds720_queue_numm_element *) qumme;
    if( key != NULL )
      *key = re->key;
    if( value != NULL )
      *value = re->value;
    LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT( re->fe, re );
    lfds720_freelist_n_threadsafe_push( &rs->fs, NULL, &re->fe );
  }

  return rv;
}

