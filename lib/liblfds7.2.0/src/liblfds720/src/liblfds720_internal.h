/***** public prototypes *****/
#include "../inc/liblfds720.h"

/***** defines *****/
#define and &&
#define or  ||

#define NO_FLAGS 0x0

#if( defined KERNEL_MODE )
  #define MODE_TYPE_STRING "kernel-mode"
#endif

#if( !defined KERNEL_MODE )
  #define MODE_TYPE_STRING "user-mode"
#endif

#if( defined NDEBUG && !defined COVERAGE && !defined TSAN && !defined PROF )
  #define BUILD_TYPE_STRING "release"
#endif

#if( !defined NDEBUG && !defined COVERAGE && !defined TSAN && !defined PROF )
  #define BUILD_TYPE_STRING "debug"
#endif

#if( !defined NDEBUG && defined COVERAGE && !defined TSAN && !defined PROF )
  #define BUILD_TYPE_STRING "coverage"
#endif

#if( !defined NDEBUG && !defined COVERAGE && defined TSAN && !defined PROF )
  #define BUILD_TYPE_STRING "threadsanitizer"
#endif

#if( !defined NDEBUG && !defined COVERAGE && !defined TSAN && defined PROF )
  #define BUILD_TYPE_STRING "profiling"
#endif

#define LFDS720_LARGERST_64BIT_PRIME        0xFFFFFFFFFFFFFFCF
#define LFDS720_SECOND_LARGERST_64BIT_PRIME 0xFFFFFFFFFFFFFFB1

#define LFDS720_BACKOFF_INITIAL_VALUE  0
#define LFDS720_BACKOFF_LIMIT          10

#define LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( backoff_state, backoff_iteration )                \
{                                                                                              \
  lfds720_pal_uint_t volatile                                                                  \
    loop;                                                                                      \
                                                                                               \
  lfds720_pal_uint_t                                                                           \
    endloop;                                                                                   \
                                                                                               \
  if( (backoff_iteration) == LFDS720_BACKOFF_LIMIT )                                           \
    (backoff_iteration) = LFDS720_BACKOFF_INITIAL_VALUE;                                       \
  else                                                                                         \
  {                                                                                            \
    endloop = ( ((lfds720_pal_uint_t) 0x1) << (backoff_iteration) ) * (backoff_state).metric;  \
    for( loop = 0 ; loop < endloop ; loop++ );                                                 \
  }                                                                                            \
                                                                                               \
  (backoff_iteration)++;                                                                       \
}

#define LFDS720_BACKOFF_AUTOTUNE( bs, backoff_iteration )                                                                           \
{                                                                                                                                   \
  if( (backoff_iteration) < 2 )                                                                                                     \
    (bs).backoff_iteration_frequency_counters[(backoff_iteration)]++;                                                               \
                                                                                                                                    \
  if( ++(bs).total_operations >= 10000 and (bs).lock == LFDS720_MISC_FLAG_LOWERED )                                                 \
  {                                                                                                                                 \
    char unsigned                                                                                                                   \
      result;                                                                                                                       \
                                                                                                                                    \
    lfds720_pal_uint_t LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)                                              \
      compare = LFDS720_MISC_FLAG_LOWERED;                                                                                          \
                                                                                                                                    \
    LFDS720_PAL_ATOMIC_CAS( (bs).lock, compare, LFDS720_MISC_FLAG_RAISED, LFDS720_MISC_CAS_STRENGTH_WEAK, result );                 \
                                                                                                                                    \
    if( result == 1 )                                                                                                               \
    {                                                                                                                               \
      /* TRD : if E[1] is less than 1/100th of E[0], decrease the metric, to increase E[1] */                                       \
      if( (bs).backoff_iteration_frequency_counters[1] < (bs).backoff_iteration_frequency_counters[0] / 100 )                       \
      {                                                                                                                             \
        if( (bs).metric >= 11 )                                                                                                     \
          (bs).metric -= 10;                                                                                                        \
      }                                                                                                                             \
      else                                                                                                                          \
        (bs).metric += 10;                                                                                                          \
                                                                                                                                    \
      (bs).backoff_iteration_frequency_counters[0] = 0;                                                                             \
      (bs).backoff_iteration_frequency_counters[1] = 0;                                                                             \
      (bs).total_operations = 0;                                                                                                    \
                                                                                                                                    \
      LFDS720_MISC_BARRIER_STORE;                                                                                                   \
                                                                                                                                    \
      LFDS720_PAL_ATOMIC_SET( (bs).lock, LFDS720_MISC_FLAG_LOWERED );                                                               \
    }                                                                                                                               \
  }                                                                                                                                 \
}

/***** library-wide macros and prototypes *****/
void lfds720_misc_internal_backoff_init( struct lfds720_misc_backoff_state *bs );

#define LFDS720_MISC_POINTER_TO_OFFSET( base, pointer )             ( (void *) (pointer) - (void *) (base) )
#define LFDS720_MISC_OFFSET_TO_POINTER( base, offset, type )        ( (type *) ( (void *) (base) + (offset) ) )




