/***** includes *****/
#include "lfds720_ringbuffer_n_internal.h"

/***** private prototypes *****/
static void lfds720_ringbuffer_n_internal_validate( struct lfds720_ringbuffer_n_state *rs,
                                                  struct lfds720_misc_validation_info *vi,
                                                  enum lfds720_misc_validity *lfds720_queue_numm_validity,
                                                  enum lfds720_misc_validity *lfds720_freelist_n_validity );



/****************************************************************************/
void lfds720_ringbuffer_n_query( struct lfds720_ringbuffer_n_state *rs,
                               enum lfds720_ringbuffer_n_query query_type,
                               void *query_input,
                               void *query_output )
{
  LFDS720_PAL_ASSERT( rs != NULL );
  // TRD : query_type can be any value in its range

  LFDS720_MISC_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS720_RINGBUFFER_N_QUERY_SINGLETHREADED_GET_COUNT:
      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_queue_numm_query( &rs->qumms, LFDS720_QUEUE_NUMM_QUERY_SINGLETHREADED_GET_COUNT, NULL, query_output );
    break;

    case LFDS720_RINGBUFFER_N_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_ringbuffer_n_internal_validate( rs, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output, ((enum lfds720_misc_validity *) query_output)+1 );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_ringbuffer_n_internal_validate( struct lfds720_ringbuffer_n_state *rs,
                                                  struct lfds720_misc_validation_info *vi,
                                                  enum lfds720_misc_validity *lfds720_queue_numm_validity,
                                                  enum lfds720_misc_validity *lfds720_freelist_n_validity )
{
  LFDS720_PAL_ASSERT( rs != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_queue_numm_validity != NULL );
  LFDS720_PAL_ASSERT( lfds720_freelist_n_validity != NULL );

  if( vi == NULL )
  {
    lfds720_queue_numm_query( &rs->qumms, LFDS720_QUEUE_NUMM_QUERY_SINGLETHREADED_VALIDATE, NULL, lfds720_queue_numm_validity );
    lfds720_freelist_n_query( &rs->fs, LFDS720_FREELIST_N_QUERY_SINGLETHREADED_VALIDATE, NULL, lfds720_freelist_n_validity );
  }

  if( vi != NULL )
  {
    struct lfds720_misc_validation_info
      freelist_n_vi,
      queue_vi;

    queue_vi.min_elements = 0;
    freelist_n_vi.min_elements = 0;
    queue_vi.max_elements = vi->max_elements;
    freelist_n_vi.max_elements = vi->max_elements;

    lfds720_queue_numm_query( &rs->qumms, LFDS720_QUEUE_NUMM_QUERY_SINGLETHREADED_VALIDATE, &queue_vi, lfds720_queue_numm_validity );
    lfds720_freelist_n_query( &rs->fs, LFDS720_FREELIST_N_QUERY_SINGLETHREADED_VALIDATE, &freelist_n_vi, lfds720_freelist_n_validity );
  }

  return;
}

