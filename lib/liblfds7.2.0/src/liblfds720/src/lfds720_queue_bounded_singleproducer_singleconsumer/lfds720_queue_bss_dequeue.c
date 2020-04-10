/***** includes *****/
#include "lfds720_queue_bss_internal.h"





/****************************************************************************/
int lfds720_queue_bss_dequeue( struct lfds720_queue_bss_state *qbsss,
                               void **key,
                               void **value )
{
  struct lfds720_queue_bss_element
    *qbsse;

  LFDS720_PAL_ASSERT( qbsss != NULL );
  // TRD : key can be NULL
  // TRD : value can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( qbsss->read_index != qbsss->write_index )
  {
    qbsse = qbsss->element_array + qbsss->read_index;

    if( key != NULL )
      *key = qbsse->key;

    if( value != NULL )
      *value = qbsse->value;

    qbsss->read_index = (qbsss->read_index + 1) & qbsss->mask;

    LFDS720_MISC_BARRIER_STORE;

    return 1;
  }

  return 0;
}

