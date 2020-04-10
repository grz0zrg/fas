/***** includes *****/
#include "lfds720_queue_bss_internal.h"





/****************************************************************************/
int lfds720_queue_bss_enqueue( struct lfds720_queue_bss_state *qbsss,
                               void *key,
                               void *value )
{
  struct lfds720_queue_bss_element
    *qbsse;

  LFDS720_PAL_ASSERT( qbsss != NULL );
  // TRD : key can be NULL
  // TRD : value can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( ( (qbsss->write_index+1) & qbsss->mask ) != qbsss->read_index )
  {
    qbsse = qbsss->element_array + qbsss->write_index;

    qbsse->key = key;
    qbsse->value = value;

    LFDS720_MISC_BARRIER_STORE;

    qbsss->write_index = (qbsss->write_index + 1) & qbsss->mask;

    return 1;
  }

  return 0;
}

