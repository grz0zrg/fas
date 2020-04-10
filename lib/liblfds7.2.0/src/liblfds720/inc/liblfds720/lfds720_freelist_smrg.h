/***** enums *****/
enum lfds720_freelist_smrg_query
{
  LFDS720_FREELIST_SMRG_QUERY_SINGLETHREADED_GET_COUNT,
  LFDS720_FREELIST_SMRG_QUERY_SINGLETHREADED_VALIDATE
};

/***** incomplete types *****/
struct lfds720_freelist_smrg_state;

/***** structures *****/
struct lfds720_freelist_smrg_element
{
  struct lfds720_freelist_smrg_element
    *next;

  struct lfds720_smrg_allocation_state
    smrgas;

  struct lfds720_freelist_smrg_state
    *fsgs;

  void
    (*element_cleaned_callback)( struct lfds720_freelist_smrg_state *fsgs,
                                 struct lfds720_freelist_smrg_element *fsge,
                                 void *element_user_state,
                                 struct lfds720_smrg_thread_state *smrts ),
    *key,
    *value;
};

struct lfds720_freelist_smrg_state
{
  struct lfds720_freelist_smrg_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *volatile top;

  void LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *user_state;

  struct lfds720_smrg_state
    *smrgs;

  struct lfds720_smrg_allocation_state
    smrgas;

  void
    (*element_cleanup_callback)( struct lfds720_freelist_smrg_state *fsgs, struct lfds720_freelist_smrg_element *fsge ),
    (*state_cleanup_callback)( struct lfds720_freelist_smrg_state *fsgs );

  struct lfds720_misc_backoff_state
    pop_backoff,
    push_backoff;
};

/***** public macros and prototypes *****/
void lfds720_freelist_smrg_init_valid_on_current_logical_core( struct lfds720_freelist_smrg_state *fsgs,
                                                               struct lfds720_smrg_state *smrgs,
                                                               void *user_state );
  // TRD : ufsged in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_freelist_smrg_cleanup( struct lfds720_freelist_smrg_state *fsgs,
                                    void (*element_cleanup_callback)( struct lfds720_freelist_smrg_state *fsgs, struct lfds720_freelist_smrg_element *fsge ),
                                    void (*state_cleanup_callback)( struct lfds720_freelist_smrg_state *fsgs ),
                                    struct lfds720_smrg_thread_state *smrts );

#define LFDS720_FREELIST_SMRG_GET_KEY_FROM_ELEMENT( freelist_element )             ( (freelist_element).key )
#define LFDS720_FREELIST_SMRG_SET_KEY_IN_ELEMENT( freelist_element, new_key )      ( (freelist_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_FREELIST_SMRG_GET_VALUE_FROM_ELEMENT( freelist_element )           ( (freelist_element).value )
#define LFDS720_FREELIST_SMRG_SET_VALUE_IN_ELEMENT( freelist_element, new_value )  ( (freelist_element).value = (void *) (lfds720_pal_uint_t) (new_value) )
#define LFDS720_FREELIST_SMRG_GET_USER_STATE_FROM_STATE( freelist_state )          ( (freelist_state).user_state )

void lfds720_freelist_smrg_push_clean_element( struct lfds720_freelist_smrg_state *fsgs,
                                               struct lfds720_freelist_smrg_element *fsge,
                                               struct lfds720_smrg_thread_state *smrts );

int lfds720_freelist_smrg_pop_dirty_element( struct lfds720_freelist_smrg_state *fsgs,
                                             struct lfds720_freelist_smrg_element **fsge,
                                             struct lfds720_smrg_thread_state *smrts );

void lfds720_freelist_smrg_clean_dirty_element( struct lfds720_freelist_smrg_state *fsgs,
                                                struct lfds720_freelist_smrg_element *fsge,
                                                void *element_user_state,
                                                void (*element_cleaned_callback)( struct lfds720_freelist_smrg_state *fsgs,
                                                                                  struct lfds720_freelist_smrg_element *fsge,
                                                                                  void *element_user_state,
                                                                                  struct lfds720_smrg_thread_state *smrts ),
                                                struct lfds720_smrg_thread_state *smrts );

void lfds720_freelist_smrg_query( struct lfds720_freelist_smrg_state *fsgs,
                                  enum lfds720_freelist_smrg_query query_type,
                                  void *query_input,
                                  void *query_output );


