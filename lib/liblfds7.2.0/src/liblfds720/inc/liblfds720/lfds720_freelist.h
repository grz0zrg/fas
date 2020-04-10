/***** defines *****/
#define LFDS720_FREELIST_HAZARD_POINTER_INDEX_HEAD_POINTER  0
#define LFDS720_FREELIST_HAZARD_POINTER_INDEX_NEXT_POINTER  1

/***** enums *****/
enum lfds720_freelist_query
{
  LFDS720_FREELIST_QUERY_SINGLETHREADED_GET_COUNT,
  LFDS720_FREELIST_QUERY_SINGLETHREADED_VALIDATE
};

/***** incomplete types *****/
struct lfds720_freelist_state;

/***** structures *****/
struct lfds720_freelist_element
{
  struct lfds720_freelist_element
    *next;

  struct lfds720_smrhp_allocation_state
    smrhpas;

  struct lfds720_freelist_state
    *fs;

  void
    (*element_cleaned_callback)( struct lfds720_freelist_state *fs,
                                 struct lfds720_freelist_element *fe,
                                 void *element_user_state,
                                 struct lfds720_smrhp_per_thread_state *smrhpts ),
    *key,
    *value;
};

struct lfds720_freelist_state
{
  struct lfds720_freelist_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *volatile top;

  struct lfds720_smrhp_state LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *smrhps;

  struct lfds720_smrhp_allocation_state
    smrhpas;

  void
    (*element_cleaned_callback)( struct lfds720_freelist_state *fs,
                                 struct lfds720_freelist_element *fe,
                                 void *element_user_state,
                                 struct lfds720_smrhp_per_thread_state *smrhpts ),
    (*state_cleanup_callback)( struct lfds720_freelist_state *fs,
                               void *element_user_state,
                               struct lfds720_smrhp_per_thread_state *smrhpts ),
    *user_state;

  struct lfds720_misc_backoff_state
    pop_backoff,
    push_backoff;
};

/***** public macros and prototypes *****/
void lfds720_freelist_init_valid_on_current_logical_core( struct lfds720_freelist_state *fs,
                                                          struct lfds720_smrhp_state *smrhps,
                                                          void *user_state );

void lfds720_freelist_cleanup( struct lfds720_freelist_state *fs,
                               void *element_user_state,
                               void (*element_cleaned_callback)( struct lfds720_freelist_state *fs,
                                                                 struct lfds720_freelist_element *fe,
                                                                 void *element_user_state,
                                                                 struct lfds720_smrhp_per_thread_state *smrhpts ),
                               void (*state_cleanup_callback)( struct lfds720_freelist_state *fs,
                                                               void *element_user_state,
                                                               struct lfds720_smrhp_per_thread_state *smrhpts ),
                               struct lfds720_smrhp_per_thread_state *smrhpts );

#define LFDS720_FREELIST_GET_KEY_FROM_ELEMENT( freelist_element )             ( (freelist_element).key )
#define LFDS720_FREELIST_SET_KEY_IN_ELEMENT( freelist_element, new_key )      ( (freelist_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_FREELIST_GET_VALUE_FROM_ELEMENT( freelist_element )           ( (freelist_element).value )
#define LFDS720_FREELIST_SET_VALUE_IN_ELEMENT( freelist_element, new_value )  ( (freelist_element).value = (void *) (lfds720_pal_uint_t) (new_value) )
#define LFDS720_FREELIST_GET_USER_STATE_FROM_STATE( freelist_state )          ( (freelist_state).user_state )

int lfds720_freelist_pop_dirty_element( struct lfds720_freelist_state *fs,
                                        struct lfds720_freelist_element **fe,
                                        struct lfds720_smrhp_per_thread_state *smrhpts );

void lfds720_freelist_clean_dirty_element( struct lfds720_freelist_state *fs,
                                           struct lfds720_freelist_element *fe,
                                           void *element_user_state,
                                           void (*element_cleaned_callback)( struct lfds720_freelist_state *fs,
                                                                             struct lfds720_freelist_element *fe,
                                                                             void *element_user_state,
                                                                             struct lfds720_smrhp_per_thread_state *smrhpts ),
                                           struct lfds720_smrhp_per_thread_state *smrhpts );

void lfds720_freelist_push_clean_element( struct lfds720_freelist_state *fs,
                                          struct lfds720_freelist_element *fe,
                                          struct lfds720_smrhp_per_thread_state *smrhpts );

void lfds720_freelist_query( struct lfds720_freelist_state *fs,
                             enum lfds720_freelist_query query_type,
                             void *query_input,
                             void *query_output );

