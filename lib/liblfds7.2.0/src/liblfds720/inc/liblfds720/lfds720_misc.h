/***** defines *****/
#define LFDS720_MISC_VERSION_STRING   "7.2.0"
#define LFDS720_MISC_VERSION_INTEGER  720

#ifndef NULL
  #define NULL ( (void *) 0 )
#endif

// TRD : counter must be 0 for IA64 DWCAS
#define LFDS720_MISC_COUNTER   0
#define LFDS720_MISC_POINTER   1
#define LFDS720_MISC_OFFSET    1
#define LFDS720_MISC_PAC_SIZE  2

#define LFDS720_MISC_DELIBERATELY_CRASH  { char *c = 0; *c = 0; }

// TRD : random number from on-line atmospheric noise RNG
#define LFDS720_MISC_INIT_HAS_BEEN_CALLED_SAFETY_CHECK_BITPATTERN  0x0a34655dUL

#define LFDS720_MISC_BEGIN_SINGLEHREADED_OPERATIONS  LFDS720_MISC_BARRIER_LOAD
#define LFDS720_MISC_END_SINGLEHREADED_OPERATIONS    LFDS720_MISC_FLUSH

#if( !defined LFDS720_PAL_ATOMIC_ADD )
  #define LFDS720_PAL_NO_ATOMIC_ADD
  #define LFDS720_MISC_ATOMIC_SUPPORT_ADD 0
  #define LFDS720_PAL_ATOMIC_ADD( target, value, result, result_type )        \
  {                                                                                      \
    LFDS720_PAL_ASSERT( !"LFDS720_PAL_ATOMIC_ADD not implemented for this platform." );  \
    LFDS720_MISC_DELIBERATELY_CRASH;                                                     \
  }
#else
  #define LFDS720_MISC_ATOMIC_SUPPORT_ADD 1
#endif

#if( !defined LFDS720_PAL_ATOMIC_CAS )
  #define LFDS720_PAL_NO_ATOMIC_CAS
  #define LFDS720_MISC_ATOMIC_SUPPORT_CAS 0
  #define LFDS720_PAL_ATOMIC_CAS( destination, pointer_to_compare, new_destination, cas_strength, result )  \
  {                                                                                                                    \
    LFDS720_PAL_ASSERT( !"LFDS720_PAL_ATOMIC_CAS not implemented for this platform." );                                \
    (result) = 0;                                                                                                      \
    LFDS720_MISC_DELIBERATELY_CRASH;                                                                                   \
  }
#else
  #define LFDS720_MISC_ATOMIC_SUPPORT_CAS 1
#endif

#if( !defined LFDS720_PAL_ATOMIC_DWCAS )
  #define LFDS720_PAL_NO_ATOMIC_DWCAS
  #define LFDS720_MISC_ATOMIC_SUPPORT_DWCAS 0
  #define LFDS720_PAL_ATOMIC_DWCAS( destination, pointer_to_compare, pointer_to_new_destination, cas_strength, result )  \
  {                                                                                                                                 \
    LFDS720_PAL_ASSERT( !"LFDS720_PAL_ATOMIC_DWCAS not implemented for this platform." );                                           \
    (result) = 0;                                                                                                                   \
    LFDS720_MISC_DELIBERATELY_CRASH;                                                                                                \
  }
#else
  #define LFDS720_MISC_ATOMIC_SUPPORT_DWCAS 1
#endif

#if( !defined LFDS720_PAL_ATOMIC_EXCHANGE )
  #define LFDS720_PAL_NO_ATOMIC_EXCHANGE
  #define LFDS720_MISC_ATOMIC_SUPPORT_EXCHANGE 0
  #define LFDS720_PAL_ATOMIC_EXCHANGE( destination, new_value, original_value, value_type )  \
  {                                                                                                     \
    LFDS720_PAL_ASSERT( !"LFDS720_PAL_ATOMIC_EXCHANGE not implemented for this platform." );            \
    LFDS720_MISC_DELIBERATELY_CRASH;                                                                    \
  }
#else
  #define LFDS720_MISC_ATOMIC_SUPPORT_EXCHANGE 1
#endif

#if( !defined LFDS720_PAL_ATOMIC_SET )
  #define LFDS720_PAL_NO_ATOMIC_SET
  #define LFDS720_MISC_ATOMIC_SUPPORT_SET 0
  #define LFDS720_PAL_ATOMIC_SET( destination, new_value )                    \
  {                                                                                      \
    LFDS720_PAL_ASSERT( !"LFDS720_PAL_ATOMIC_SET not implemented for this platform." );  \
    LFDS720_MISC_DELIBERATELY_CRASH;                                                     \
  }
#else
  #define LFDS720_MISC_ATOMIC_SUPPORT_SET 1
#endif

#if( defined LFDS720_PAL_BARRIER_COMPILER_LOAD && defined LFDS720_PAL_BARRIER_PROCESSOR_LOAD )
  #define LFDS720_MISC_BARRIER_LOAD  ( LFDS720_PAL_BARRIER_COMPILER_LOAD, LFDS720_PAL_BARRIER_PROCESSOR_LOAD, LFDS720_PAL_BARRIER_COMPILER_LOAD )
#endif

#if( (!defined LFDS720_PAL_BARRIER_COMPILER_LOAD || defined LFDS720_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME) && defined LFDS720_PAL_BARRIER_PROCESSOR_LOAD )
  #define LFDS720_MISC_BARRIER_LOAD  LFDS720_PAL_BARRIER_PROCESSOR_LOAD
#endif

#if( defined LFDS720_PAL_BARRIER_COMPILER_LOAD && !defined LFDS720_PAL_BARRIER_PROCESSOR_LOAD )
  #define LFDS720_MISC_BARRIER_LOAD  LFDS720_PAL_BARRIER_COMPILER_LOAD
#endif

#if( !defined LFDS720_PAL_BARRIER_COMPILER_LOAD && !defined LFDS720_PAL_BARRIER_PROCESSOR_LOAD )
  #define LFDS720_MISC_BARRIER_LOAD
#endif

#if( defined LFDS720_PAL_BARRIER_COMPILER_STORE && defined LFDS720_PAL_BARRIER_PROCESSOR_STORE )
  #define LFDS720_MISC_BARRIER_STORE  ( LFDS720_PAL_BARRIER_COMPILER_STORE, LFDS720_PAL_BARRIER_PROCESSOR_STORE, LFDS720_PAL_BARRIER_COMPILER_STORE )
#endif

#if( (!defined LFDS720_PAL_BARRIER_COMPILER_STORE || defined LFDS720_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME) && defined LFDS720_PAL_BARRIER_PROCESSOR_STORE )
  #define LFDS720_MISC_BARRIER_STORE  LFDS720_PAL_BARRIER_PROCESSOR_STORE
#endif

#if( defined LFDS720_PAL_BARRIER_COMPILER_STORE && !defined LFDS720_PAL_BARRIER_PROCESSOR_STORE )
  #define LFDS720_MISC_BARRIER_STORE  LFDS720_PAL_BARRIER_COMPILER_STORE
#endif

#if( !defined LFDS720_PAL_BARRIER_COMPILER_STORE && !defined LFDS720_PAL_BARRIER_PROCESSOR_STORE )
  #define LFDS720_MISC_BARRIER_STORE
#endif

#if( defined LFDS720_PAL_BARRIER_COMPILER_FULL && defined LFDS720_PAL_BARRIER_PROCESSOR_FULL )
  #define LFDS720_MISC_BARRIER_FULL  ( LFDS720_PAL_BARRIER_COMPILER_FULL, LFDS720_PAL_BARRIER_PROCESSOR_FULL, LFDS720_PAL_BARRIER_COMPILER_FULL )
#endif

#if( (!defined LFDS720_PAL_BARRIER_COMPILER_FULL || defined LFDS720_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME) && defined LFDS720_PAL_BARRIER_PROCESSOR_FULL )
  #define LFDS720_MISC_BARRIER_FULL  LFDS720_PAL_BARRIER_PROCESSOR_FULL
#endif

#if( defined LFDS720_PAL_BARRIER_COMPILER_FULL && !defined LFDS720_PAL_BARRIER_PROCESSOR_FULL )
  #define LFDS720_MISC_BARRIER_FULL  LFDS720_PAL_BARRIER_COMPILER_FULL
#endif

#if( !defined LFDS720_PAL_BARRIER_COMPILER_FULL && !defined LFDS720_PAL_BARRIER_PROCESSOR_FULL )
  #define LFDS720_MISC_BARRIER_FULL
#endif

#if( (defined LFDS720_PAL_BARRIER_COMPILER_LOAD && defined LFDS720_PAL_BARRIER_COMPILER_STORE && defined LFDS720_PAL_BARRIER_COMPILER_FULL) || (defined LFDS720_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME) )
  #define LFDS720_MISC_ATOMIC_SUPPORT_COMPILER_BARRIERS  1
#else
  #define LFDS720_MISC_ATOMIC_SUPPORT_COMPILER_BARRIERS  0
#endif

#if( defined LFDS720_PAL_BARRIER_PROCESSOR_LOAD && defined LFDS720_PAL_BARRIER_PROCESSOR_STORE && defined LFDS720_PAL_BARRIER_PROCESSOR_FULL )
  #define LFDS720_MISC_ATOMIC_SUPPORT_PROCESSOR_BARRIERS  1
#else
  #define LFDS720_MISC_ATOMIC_SUPPORT_PROCESSOR_BARRIERS  0
#endif

#define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE  LFDS720_MISC_BARRIER_LOAD
#define LFDS720_MISC_FLUSH                                                                                      { LFDS720_MISC_BARRIER_STORE; lfds720_misc_force_store(); }

/***** enums *****/
enum lfds720_misc_cas_strength
{
  // TRD : GCC defined these values, not me :-)
  LFDS720_MISC_CAS_STRENGTH_STRONG = 0,
  LFDS720_MISC_CAS_STRENGTH_WEAK   = 1,
};

enum lfds720_misc_data_structure
{
  LFDS720_MISC_DATA_STRUCTURE_BTREE_NPU,
  LFDS720_MISC_DATA_STRUCTURE_BTREE_NU,
  LFDS720_MISC_DATA_STRUCTURE_FREELIST,
  LFDS720_MISC_DATA_STRUCTURE_FREELIST_N,
  LFDS720_MISC_DATA_STRUCTURE_FREELIST_NP,
  LFDS720_MISC_DATA_STRUCTURE_HASH_N,
  LFDS720_MISC_DATA_STRUCTURE_LIST_NSO,
  LFDS720_MISC_DATA_STRUCTURE_LIST_NSU,
  LFDS720_MISC_DATA_STRUCTURE_LIST_SO,
  LFDS720_MISC_DATA_STRUCTURE_QUEUE_BMM,
  LFDS720_MISC_DATA_STRUCTURE_QUEUE_BSS,
  LFDS720_MISC_DATA_STRUCTURE_QUEUE_NPUMM,
  LFDS720_MISC_DATA_STRUCTURE_QUEUE_NPUSS,
  LFDS720_MISC_DATA_STRUCTURE_QUEUE_NUMM,
  LFDS720_MISC_DATA_STRUCTURE_QUEUE_NUSS,
  LFDS720_MISC_DATA_STRUCTURE_RINGBUFFER_N,
  LFDS720_MISC_DATA_STRUCTURE_STACK_N,
  LFDS720_MISC_DATA_STRUCTURE_STACK_NP,
  LFDS720_MISC_DATA_STRUCTURE_COUNT
};

enum lfds720_misc_flag
{
  LFDS720_MISC_FLAG_LOWERED,
  LFDS720_MISC_FLAG_RAISED
};

enum lfds720_misc_query
{
  LFDS720_MISC_QUERY_GET_BUILD_AND_VERSION_STRING
};

enum lfds720_misc_validity
{
  LFDS720_MISC_VALIDITY_UNKNOWN,
  LFDS720_MISC_VALIDITY_VALID,
  LFDS720_MISC_VALIDITY_INVALID,
  LFDS720_MISC_VALIDITY_INVALID_LOOP,
  LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS,
  LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS,
  LFDS720_MISC_VALIDITY_INVALID_TEST_DATA,
  LFDS720_MISC_VALIDITY_INVALID_ORDER,
  LFDS720_MISC_VALIDITY_INVALID_ATOMIC_FAILED,
  LFDS720_MISC_VALIDITY_INDETERMINATE_NONATOMIC_PASSED,
};

/***** struct *****/
struct lfds720_misc_backoff_state
{
  lfds720_pal_uint_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    lock;

  lfds720_pal_uint_t
    backoff_iteration_frequency_counters[2],
    metric,
    total_operations;
};

struct lfds720_misc_globals
{
  struct lfds720_prng_state
    ps;
};

struct lfds720_misc_validation_info
{
  lfds720_pal_uint_t
    min_elements,
    max_elements;
};

/***** externs *****/
extern struct lfds720_misc_globals
  lfds720_misc_globals;

/***** public prototypes *****/
static LFDS720_PAL_INLINE void lfds720_misc_force_store( void );

void lfds720_misc_query( enum lfds720_misc_query query_type, void *query_input, void *query_output );

/***** public in-line functions *****/
#pragma prefast( disable : 28112, "blah" )

static LFDS720_PAL_INLINE void lfds720_misc_force_store()
{
  lfds720_pal_uint_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    destination;

  LFDS720_PAL_ATOMIC_SET( destination, 0 );

  return;
}

