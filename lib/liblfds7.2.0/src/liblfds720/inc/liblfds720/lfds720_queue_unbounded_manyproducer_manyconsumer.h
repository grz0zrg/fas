/***** defines *****/
#define LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_ENQUEUE_POINTER       0
#define LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_ENQUEUE_NEXT_POINTER  1

#define LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_DEQUEUE_POINTER       0
#define LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_DEQUEUE_NEXT_POINTER  1

/***** enums *****/
enum lfds720_queue_umm_query
{
  LFDS720_QUEUE_UMM_QUERY_SINGLETHREADED_GET_COUNT,
  LFDS720_QUEUE_UMM_QUERY_SINGLETHREADED_VALIDATE
};

/***** structures *****/
struct lfds720_queue_umm_element
{
  struct lfds720_queue_umm_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile next;

  struct lfds720_smrhp_allocation_state
    smrhpas;

  struct lfds720_queue_umm_state
    *qummhps;

  void
    (*element_cleaned_callback)( struct lfds720_queue_umm_state *qummhps,
                                 struct lfds720_queue_umm_element *qummhpe,
                                 void *element_user_state,
                                 struct lfds720_smrhp_per_thread_state *smrhpts ),
    *key,
    *value;
};

struct lfds720_queue_umm_state
{
  struct lfds720_queue_umm_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *volatile enqueue,
    *volatile dequeue;

  struct lfds720_smrhp_state LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *smrhps;

  struct lfds720_smrhp_allocation_state
    smrhpas;

  struct lfds720_misc_backoff_state
    dequeue_backoff,
    enqueue_backoff;

  void
    (*element_cleaned_callback)( struct lfds720_queue_umm_state *qummhps,
                                 struct lfds720_queue_umm_element *qummhpe,
                                 void *element_user_state,
                                 struct lfds720_smrhp_per_thread_state *smrhpts,
                                 enum lfds720_misc_flag dummy_element_flag ),
    (*state_cleanup_callback)( struct lfds720_queue_umm_state *qummhps,
                               void *element_user_state,
                               struct lfds720_smrhp_per_thread_state *smrhpts ),
    *user_state;
};

/***** public macros and prototypes *****/
void lfds720_queue_umm_init_valid_on_current_logical_core( struct lfds720_queue_umm_state *qummhps,
                                                                  struct lfds720_queue_umm_element *qummhpe_dummy,
                                                                  struct lfds720_smrhp_state *smrhps,
                                                                  void *user_state );
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_queue_umm_cleanup( struct lfds720_queue_umm_state *qummhps,
                                      void (*element_cleaned_callback)( struct lfds720_queue_umm_state *qummhps, struct lfds720_queue_umm_element *qummhpe, void *element_user_state, struct lfds720_smrhp_per_thread_state *smrhpts, enum lfds720_misc_flag dummy_element_flag ),
                                      void (*state_cleanup_callback)( struct lfds720_queue_umm_state *qummhps, void *element_user_state, struct lfds720_smrhp_per_thread_state *smrhpts ),
                                      struct lfds720_smrhp_per_thread_state *smrhpts );

#define LFDS720_QUEUE_UMM_GET_KEY_FROM_ELEMENT( queue_nummhp_element )             ( (queue_nummhp_element).key )
#define LFDS720_QUEUE_UMM_SET_KEY_IN_ELEMENT( queue_nummhp_element, new_key )      ( (queue_nummhp_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_QUEUE_UMM_GET_VALUE_FROM_ELEMENT( queue_nummhp_element )           ( (queue_nummhp_element).value )
#define LFDS720_QUEUE_UMM_SET_VALUE_IN_ELEMENT( queue_nummhp_element, new_value )  ( (queue_nummhp_element).value = (void *) (lfds720_pal_uint_t) (new_value) )
#define LFDS720_QUEUE_UMM_GET_USER_STATE_FROM_STATE( queue_nummhp_state )          ( (queue_nummhp_state).user_state )

void lfds720_queue_umm_enqueue( struct lfds720_queue_umm_state *qummhps,
                                      struct lfds720_queue_umm_element *qummhpe,
                                      struct lfds720_smrhp_per_thread_state *smrhpts );

int lfds720_queue_umm_dequeue( struct lfds720_queue_umm_state *qummhps,
                                     struct lfds720_queue_umm_element **qummhpe,
                                     struct lfds720_smrhp_per_thread_state *smrhpts );

void lfds720_queue_umm_clean_dirty_element( struct lfds720_queue_umm_state *qummhps,
                                                  struct lfds720_queue_umm_element *qummhpe,
                                                  void *element_user_state,
                                                  void (*element_cleaned_callback)( struct lfds720_queue_umm_state *qummhps,
                                                                                    struct lfds720_queue_umm_element *qummhpe,
                                                                                    void *element_user_state,
                                                                                    struct lfds720_smrhp_per_thread_state *smrhpts ),
                                                  struct lfds720_smrhp_per_thread_state *smrhpts );

void lfds720_queue_umm_query( struct lfds720_queue_umm_state *qummhps,
                                    enum lfds720_queue_umm_query query_type,
                                    void *query_input,
                                    void *query_output );

