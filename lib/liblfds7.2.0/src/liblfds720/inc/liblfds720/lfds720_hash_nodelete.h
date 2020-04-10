/***** defines *****/
// TRD : a quality hash function, provided for user convenience - note hash must be initialized to 0 before the first call by the user

#if( LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 4 )
  // TRD : void *data, lfds720_pal_uint_t data_length_in_bytes, lfds720_pal_uint_t hash
  #define LFDS720_HASH_N_HASH_FUNCTION( data, data_length_in_bytes, hash )  {                                                           \
                                                                              lfds720_pal_uint_t                                        \
                                                                                loop;                                                   \
                                                                                                                                        \
                                                                              for( loop = 0 ; loop < (data_length_in_bytes) ; loop++ )  \
                                                                              {                                                         \
                                                                                (hash) += *( (char unsigned *) (data) + loop );         \
                                                                                (hash) = ((hash) ^ ((hash) >> 16)) * 0x85ebca6bUL;      \
                                                                                (hash) = ((hash) ^ ((hash) >> 13)) * 0xc2b2ae35UL;      \
                                                                                (hash) = (hash ^ (hash >> 16));                         \
                                                                              }                                                         \
                                                                            }
#endif

#if( LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 8 )
  // TRD : void *data, lfds720_pal_uint_t data_length_in_bytes, lfds720_pal_uint_t hash
  #define LFDS720_HASH_N_HASH_FUNCTION( data, data_length_in_bytes, hash )  {                                                                \
                                                                              lfds720_pal_uint_t                                             \
                                                                                loop;                                                        \
                                                                                                                                             \
                                                                              for( loop = 0 ; loop < (data_length_in_bytes) ; loop++ )       \
                                                                              {                                                              \
                                                                                (hash) += *( (char unsigned *) (data) + loop );              \
                                                                                (hash) = ((hash) ^ ((hash) >> 30)) * 0xBF58476D1CE4E5B9ULL;  \
                                                                                (hash) = ((hash) ^ ((hash) >> 27)) * 0x94D049BB133111EBULL;  \
                                                                                (hash) = (hash ^ (hash >> 31));                              \
                                                                              }                                                              \
                                                                            }
#endif

/***** enums *****/
enum lfds720_hash_n_existing_key
{
  LFDS720_HASH_N_EXISTING_KEY_OVERWRITE,
  LFDS720_HASH_N_EXISTING_KEY_FAIL
};

enum lfds720_hash_n_insert_result
{
  LFDS720_HASH_N_PUT_RESULT_FAILURE_EXISTING_KEY,
  LFDS720_HASH_N_PUT_RESULT_SUCCESS_OVERWRITE,
  LFDS720_HASH_N_PUT_RESULT_SUCCESS
};

enum lfds720_hash_n_query
{
  LFDS720_HASH_N_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LFDS720_HASH_N_QUERY_SINGLETHREADED_VALIDATE
};

/***** structs *****/
struct lfds720_hash_n_element
{
  struct lfds720_btree_nu_element
    baue;

  void
    *key;

  void LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile value;
};

struct lfds720_hash_n_iterate
{
  struct lfds720_btree_nu_element
    *baue;

  struct lfds720_btree_nu_state
    *baus,
    *baus_end;
};

struct lfds720_hash_n_state
{
  enum lfds720_hash_n_existing_key
    existing_key;

  int
    (*key_compare_function)( void const *new_key, void const *existing_key );

  lfds720_pal_uint_t
    array_size;

  struct lfds720_btree_nu_state
    *baus_array;

  void
    (*element_cleanup_callback)( struct lfds720_hash_n_state *has, struct lfds720_hash_n_element *hae ),
    (*key_hash_function)( void const *key, lfds720_pal_uint_t *hash ),
    *user_state;
};

/***** public macros and prototypes *****/
void lfds720_hash_n_init_valid_on_current_logical_core( struct lfds720_hash_n_state *has,
                                                         struct lfds720_btree_nu_state *baus_array,
                                                         lfds720_pal_uint_t array_size,
                                                         int (*key_compare_function)(void const *new_key, void const *existing_key),
                                                         void (*key_hash_function)(void const *key, lfds720_pal_uint_t *hash),
                                                         enum lfds720_hash_n_existing_key existing_key,
                                                         void *user_state );
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_hash_n_cleanup( struct lfds720_hash_n_state *has,
                             void (*element_cleanup_function)(struct lfds720_hash_n_state *has, struct lfds720_hash_n_element *hae) );

#define LFDS720_HASH_N_GET_KEY_FROM_ELEMENT( hash_n_element )             ( (hash_n_element).key )
#define LFDS720_HASH_N_SET_KEY_IN_ELEMENT( hash_n_element, new_key )      ( (hash_n_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_HASH_N_GET_VALUE_FROM_ELEMENT( hash_n_element )           ( LFDS720_MISC_BARRIER_LOAD, (hash_n_element).value )
#define LFDS720_HASH_N_SET_VALUE_IN_ELEMENT( hash_n_element, new_value )  { LFDS720_PAL_ATOMIC_SET( (hash_n_element).value, (void *) (lfds720_pal_uint_t) (new_value) ); }
#define LFDS720_HASH_N_GET_USER_STATE_FROM_STATE( hash_n_state )          ( (hash_n_state).user_state )

enum lfds720_hash_n_insert_result lfds720_hash_n_insert( struct lfds720_hash_n_state *has,
                                                         struct lfds720_hash_n_element *hae,
                                                         struct lfds720_hash_n_element **existing_hae );
  // TRD : if the key in hae is already present in the hash, existing_hae is set to point to the struct lfds720_hash_n_element of the existing element in the hash

int lfds720_hash_n_get_by_key( struct lfds720_hash_n_state *has,
                               int (*key_compare_function)(void const *new_key, void const *existing_key),
                               void (*key_hash_function)(void const *key, lfds720_pal_uint_t *hash),
                               void *key,
                               struct lfds720_hash_n_element **hae );

void lfds720_hash_n_iterate_init( struct lfds720_hash_n_state *has, struct lfds720_hash_n_iterate *hai );
int lfds720_hash_n_iterate( struct lfds720_hash_n_iterate *hai, struct lfds720_hash_n_element **hae );

void lfds720_hash_n_query( struct lfds720_hash_n_state *has,
                           enum lfds720_hash_n_query query_type,
                           void *query_input,
                           void *query_output );

