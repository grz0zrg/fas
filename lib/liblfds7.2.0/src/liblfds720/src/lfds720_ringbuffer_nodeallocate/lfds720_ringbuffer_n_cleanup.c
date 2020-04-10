/***** includes *****/
#include "lfds720_ringbuffer_n_internal.h"

/***** private prototypes *****/
static void lfds720_ringbuffer_n_internal_queue_numm_element_cleanup_callback( struct lfds720_queue_numm_state *qumms,
                                                                            struct lfds720_queue_numm_element *qumme,
                                                                            enum lfds720_misc_flag dummy_element_flag );
static void lfds720_ringbuffer_n_internal_freelist_n_element_cleanup_callback( struct lfds720_freelist_n_state *fs,
                                                                           struct lfds720_freelist_n_element *fe );





/****************************************************************************/
void lfds720_ringbuffer_n_cleanup( struct lfds720_ringbuffer_n_state *rs,
                                 void (*element_cleanup_callback)(struct lfds720_ringbuffer_n_state *rs, void *key, void *value, enum lfds720_misc_flag unread_flag) )
{
  LFDS720_PAL_ASSERT( rs != NULL );
  // TRD : element_cleanup_callback can be NULL

  if( element_cleanup_callback != NULL )
  {
    rs->element_cleanup_callback = element_cleanup_callback;
    lfds720_queue_numm_cleanup( &rs->qumms, lfds720_ringbuffer_n_internal_queue_numm_element_cleanup_callback );
    lfds720_freelist_n_cleanup( &rs->fs, lfds720_ringbuffer_n_internal_freelist_n_element_cleanup_callback );
  }

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

static void lfds720_ringbuffer_n_internal_queue_numm_element_cleanup_callback( struct lfds720_queue_numm_state *qumms,
                                                                            struct lfds720_queue_numm_element *qumme,
                                                                            enum lfds720_misc_flag dummy_element_flag )
{
  struct lfds720_ringbuffer_n_element
    *re;

  struct lfds720_ringbuffer_n_state
    *rs;

  LFDS720_PAL_ASSERT( qumms != NULL );
  LFDS720_PAL_ASSERT( qumme != NULL );
  // TRD : dummy_element can be any value in its range

  rs = (struct lfds720_ringbuffer_n_state *) LFDS720_QUEUE_NUMM_GET_USER_STATE_FROM_STATE( *qumms );
  re = (struct lfds720_ringbuffer_n_element *) LFDS720_QUEUE_NUMM_GET_VALUE_FROM_ELEMENT( *qumme );

  if( dummy_element_flag == LFDS720_MISC_FLAG_LOWERED )
    rs->element_cleanup_callback( rs, re->key, re->value, LFDS720_MISC_FLAG_RAISED );

  return;
}

#pragma warning( default : 4100 )





/****************************************************************************/
#pragma warning( disable : 4100 )

static void lfds720_ringbuffer_n_internal_freelist_n_element_cleanup_callback( struct lfds720_freelist_n_state *fs,
                                                                           struct lfds720_freelist_n_element *fe )
{
  struct lfds720_ringbuffer_n_element
    *re;

  struct lfds720_ringbuffer_n_state
    *rs;

  LFDS720_PAL_ASSERT( fs != NULL );
  LFDS720_PAL_ASSERT( fe != NULL );

  rs = (struct lfds720_ringbuffer_n_state *) LFDS720_FREELIST_N_GET_USER_STATE_FROM_STATE( *fs );
  re = (struct lfds720_ringbuffer_n_element *) LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT( *fe );

  rs->element_cleanup_callback( rs, re->key, re->value, LFDS720_MISC_FLAG_LOWERED );

  return;
}

#pragma warning( default : 4100 )

