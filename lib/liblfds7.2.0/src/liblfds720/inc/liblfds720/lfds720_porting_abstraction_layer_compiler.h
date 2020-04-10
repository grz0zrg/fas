/* TRD : the internal single-threaded library includes this file
         so it does not have to have the user repeat the work
         so this file has to be protected from double inclusion
         or the "more than one amtching layer" error kicks in
*/

#ifndef LFDS720_PAL_COMPILER

  /****************************************************************************/
  #if( defined __GNUC__ )
    // TRD : makes checking GCC versions much tidier
    #define LFDS720_PAL_GCC_VERSION ( __GNUC__ * 100 + __GNUC_MINOR__ * 10 + __GNUC_PATCHLEVEL__ )
  #endif





  /****************************************************************************/
  #if( defined _MSC_VER && _MSC_VER >= 1400 )

    #ifdef LFDS720_PAL_COMPILER
      #error More than one porting abstraction layer matches the current platform in lfds720_porting_abstraction_layer_compiler.h
    #endif

    #define LFDS720_PAL_COMPILER

    #define LFDS720_PAL_COMPILER_STRING            "MSVC"

    #define LFDS720_PAL_ALIGN(alignment)           __declspec( align(alignment) )
    #define LFDS720_PAL_INLINE                     __forceinline

    #define LFDS720_PAL_BARRIER_COMPILER_LOAD      _ReadBarrier()
    #define LFDS720_PAL_BARRIER_COMPILER_STORE     _WriteBarrier()
    #define LFDS720_PAL_BARRIER_COMPILER_FULL      _ReadWriteBarrier()

    /* TRD : there are four processors to consider;

             . ARM32    (32 bit, ADD, CAS, DWCAS, EXCHANGE, SET) (defined _M_ARM)
             . Itanium  (64 bit, ADD, CAS,        EXCHANGE, SET) (defined _M_IA64)
             . x64      (64 bit, ADD, CAS, DWCAS, EXCHANGE, SET) (defined _M_X64 || defined _M_AMD64)
             . x86      (32 bit, ADD, CAS, DWCAS, EXCHANGE, SET) (defined _M_IX86)

             can't find any indications of 64-bit ARM support yet

             ARM has better intrinsics than the others, as there are no-fence variants

             in theory we also have to deal with 32-bit Windows on a 64-bit platform,
             and I presume we'd see the compiler properly indicate this in its macros,
             but this would require that we use 32-bit atomics on the 64-bit platforms,
             while keeping 64-bit cache line lengths and so on, and this is just so
             wierd a thing to do these days that it's not supported
    */

    #if( defined _M_ARM )
      #define LFDS720_PAL_BARRIER_PROCESSOR_LOAD   __dmb( _ARM_BARRIER_ISH   )
      #define LFDS720_PAL_BARRIER_PROCESSOR_STORE  __dmb( _ARM_BARRIER_ISHST )
      #define LFDS720_PAL_BARRIER_PROCESSOR_FULL   __dmb( _ARM_BARRIER_ISH   )

      #define LFDS720_PAL_ATOMIC_ADD( target, value, result, result_type )                                   \
      {                                                                                                      \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                   \
        (result) = (result_type) _InterlockedAdd_nf( (int long volatile *) &(target), (int long) (value) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                   \
      }

      #define LFDS720_PAL_ATOMIC_CAS( destination, compare, exchange, cas_strength, result )                                                       \
      {                                                                                                                                                                         \
        long                                                                                                                                                                    \
          original_compare;                                                                                                                                                     \
                                                                                                                                                                                \
        original_compare = (long) (compare);                                                                                                                        \
                                                                                                                                                                                \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                      \
        *(long *) &(compare) = _InterlockedCompareExchange_nf( (long volatile *) &(destination), (long) (exchange), (long) (compare) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                      \
                                                                                                                                                                                \
        result = (char unsigned) ( original_compare == (long) (compare) );                                                                                          \
      }

      #define LFDS720_PAL_ATOMIC_DWCAS( destination, compare, exchange, cas_strength, result )                                                                        \
      {                                                                                                                                                                                                       \
        __int64                                                                                                                                                                                               \
          original_compare;                                                                                                                                                                                   \
                                                                                                                                                                                                              \
        original_compare = *(__int64 *) (compare);                                                                                                                                                 \
                                                                                                                                                                                                              \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                    \
        *(__int64 *) &(compare) = _InterlockedCompareExchange64_nf( (__int64 volatile *) (destination), *(__int64 *) (exchange), *(__int64 *) (compare) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                    \
                                                                                                                                                                                                              \
        (result) = (char unsigned) ( *(__int64 *) (compare) == original_compare );                                                                                                                 \
      }

      #define LFDS720_PAL_ATOMIC_EXCHANGE( destination, exchange, exchange_type )                                            \
      {                                                                                                                                 \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                              \
        (exchange) = (exchange_type) _InterlockedExchange_nf( (int long volatile *) &(destination), (int long) (exchange) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                              \
      }

      #define LFDS720_PAL_ATOMIC_SET( destination, new_value )                                          \
      {                                                                                                            \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                         \
        (void) _InterlockedExchange_nf( (int long volatile *) &(destination), (int long) (new_value) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                         \
      }
    #endif

    #if( defined _M_IA64 )
      #define LFDS720_PAL_BARRIER_PROCESSOR_LOAD   __mf()
      #define LFDS720_PAL_BARRIER_PROCESSOR_STORE  __mf()
      #define LFDS720_PAL_BARRIER_PROCESSOR_FULL   __mf()

      #define LFDS720_PAL_ATOMIC_ADD( target, value, result, result_type )                               \
      {                                                                                                             \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                          \
        (result) = (result_type) _InterlockedAdd64( (__int64 volatile *) &(target), (__int64) (value) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                          \
      }

      #define LFDS720_PAL_ATOMIC_CAS( destination, compare, exchange, cas_strength, result )                                                                  \
      {                                                                                                                                                                                    \
        __int64                                                                                                                                                                            \
          original_compare;                                                                                                                                                                \
                                                                                                                                                                                           \
        original_compare = (__int64) (compare);                                                                                                                                \
                                                                                                                                                                                           \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                 \
        *(__int64 *) &(compare) = _InterlockedCompareExchange64( (__int64 volatile *) &(destination), (__int64) (exchange), (__int64) (compare) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                 \
                                                                                                                                                                                           \
        result = (char unsigned) ( original_compare == (__int64) (compare) );                                                                                                  \
      }

      #define LFDS720_PAL_ATOMIC_DWCAS( destination, compare, exchange, cas_strength, result )                                                                                                                                                             \
      {                                                                                                                                                                                                                                                                                            \
        __int64                                                                                                                                                                                                                                                                                    \
          original_compare;                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                   \
        original_compare = (__int64) (compare)[0];                                                                                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                   \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                                                                                                         \
        *(__int64 *) &(compare)[0] = _InterlockedCompare64Exchange128( (__int64 volatile *) &(destination), (__int64) (exchange)[1], (__int64) (exchange)[0], (__int64) (compare)[0] );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                                                                                                         \
                                                                                                                                                                                                                                                                                                   \
        result = (char unsigned) ( original_compare == (__int64) (compare)[0] );                                                                                                                                                                                     \
                                                                                                                                                                                                                                                                                                   \
        if( result == 0 )                                                                                                                                                                                                                                                                          \
        {                                                                                                                                                                                                                                                                                          \
          (compare)[0] = (destination)[0];                                                                                                                                                                                             \
          (compare)[1] = (destination)[1];                                                                                                                                                                                             \
        }                                                                                                                                                                                                                                                                                          \
      }

      #define LFDS720_PAL_ATOMIC_EXCHANGE( destination, exchange, exchange_type )                                         \
      {                                                                                                                              \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                           \
        (exchange) = (exchange_type) _InterlockedExchange64( (__int64 volatile *) &(destination), (__int64) (exchange) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                           \
      }

      #define LFDS720_PAL_ATOMIC_SET( destination, new_value )                                       \
      {                                                                                                         \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                      \
        (void) _InterlockedExchange64( (__int64 volatile *) &(destination), (__int64) (new_value) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                      \
      }
    #endif

    #if( defined _M_X64 || defined _M_AMD64 )
      #define LFDS720_PAL_BARRIER_PROCESSOR_LOAD   _mm_lfence()
      #define LFDS720_PAL_BARRIER_PROCESSOR_STORE  _mm_sfence()
      #define LFDS720_PAL_BARRIER_PROCESSOR_FULL   _mm_mfence()

      // TRD : no _InterlockedAdd64 for x64 - only the badly named _InterlockedExchangeAdd64, which is the same as _InterlockedAdd64 but returns the *original* value (which we must then add to before we return)
      #define LFDS720_PAL_ATOMIC_ADD( target, value, result, result_type )                                       \
      {                                                                                                                     \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                  \
        (result) = (result_type) _InterlockedExchangeAdd64( (__int64 volatile *) &(target), (__int64) (value) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                  \
        result += value;                                                                                                    \
      }

      #define LFDS720_PAL_ATOMIC_CAS( destination, compare, exchange, cas_strength, result )                                                                                                  \
      {                                                                                                                                                                                                                    \
        __int64                                                                                                                                                                                                 \
          original_compare;                                                                                                                                                                                                \
                                                                                                                                                                                                                           \
        original_compare = (__int64) (compare);                                                                                                                                                     \
                                                                                                                                                                                                                           \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                                 \
        *(__int64 *) &(compare) = _InterlockedCompareExchange64( (__int64 volatile *) &(destination), (__int64) (exchange), (__int64) (compare) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                                 \
                                                                                                                                                                                                                           \
        result = (char unsigned) ( original_compare == (__int64) (compare) );                                                                                                                       \
      }

      #if( _MSC_VER >= 1500 )
        #define LFDS720_PAL_ATOMIC_DWCAS( destination, compare, exchange, cas_strength, result )                                                                                                       \
        {                                                                                                                                                                                                                                      \
          LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                                                   \
          (result) = (char unsigned) _InterlockedCompareExchange128( (__int64 volatile *) (destination), (__int64) (exchange)[1], (__int64) (exchange)[0], (__int64 *) (compare) );  \
          LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                                                   \
        }
      #endif

      #define LFDS720_PAL_ATOMIC_EXCHANGE( destination, exchange, exchange_type )                                         \
      {                                                                                                                              \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                           \
        (exchange) = (exchange_type) _InterlockedExchange64( (__int64 volatile *) &(destination), (__int64) (exchange) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                           \
      }

      #define LFDS720_PAL_ATOMIC_SET( destination, new_value )                                       \
      {                                                                                                         \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                      \
        (void) _InterlockedExchange64( (__int64 volatile *) &(destination), (__int64) (new_value) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                      \
      }
    #endif

    #if( defined _M_IX86 )
      #define LFDS720_PAL_BARRIER_PROCESSOR_LOAD   lfds720_misc_force_store()
      #define LFDS720_PAL_BARRIER_PROCESSOR_STORE  lfds720_misc_force_store()
      #define LFDS720_PAL_BARRIER_PROCESSOR_FULL   lfds720_misc_force_store()

      // TRD : no _InterlockedAdd for x86 - only the badly named _InterlockedExchangeAdd, which is the same as _InterlockedAdd but returns the *original* value (which we must then add to before we return)
      #define LFDS720_PAL_ATOMIC_ADD( target, value, result, result_type )                                     \
      {                                                                                                                   \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                \
        (result) = (result_type) _InterlockedExchangeAdd( (__int64 volatile *) &(target), (__int64) (value) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                \
        result += value;                                                                                                  \
      }

      #define LFDS720_PAL_ATOMIC_CAS( destination, compare, exchange, cas_strength, result )                                                                                       \
      {                                                                                                                                                                                                         \
        long                                                                                                                                                                                      \
          original_compare;                                                                                                                                                                                     \
                                                                                                                                                                                                                \
        original_compare = (long) (compare);                                                                                                                                          \
                                                                                                                                                                                                                \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                      \
        *(long *) &(compare) = _InterlockedCompareExchange( (long volatile *) &(destination), (long) (exchange), (long) (compare) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                      \
                                                                                                                                                                                                                \
        result = (char unsigned) ( original_compare == (long) (compare) );                                                                                                            \
      }

      #define LFDS720_PAL_ATOMIC_DWCAS( destination, compare, exchange, cas_strength, result )                                                                     \
      {                                                                                                                                                                                                    \
        __int64                                                                                                                                                                                            \
          original_compare;                                                                                                                                                                                \
                                                                                                                                                                                                           \
        original_compare = (__int64) (compare);                                                                                                                                              \
                                                                                                                                                                                                           \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                 \
        *(__int64 *) &(compare) = _InterlockedCompareExchange64( (__int64 volatile *) (destination), *(__int64 *) (exchange), *(__int64 *) (compare) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                                 \
                                                                                                                                                                                                           \
        (result) = (char unsigned) ( *(__int64 *) (compare) == original_compare );                                                                                                              \
      }

      #define LFDS720_PAL_ATOMIC_EXCHANGE( destination, exchange, exchange_type )                                         \
      {                                                                                                                              \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                           \
        (exchange) = (exchange_type) _InterlockedExchange( (int long volatile *) &(destination), (int long) (exchange) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                           \
      }

      #define LFDS720_PAL_ATOMIC_SET( destination, new_value )                                       \
      {                                                                                                         \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                      \
        (void) _InterlockedExchange( (int long volatile *) &(destination), (int long) (new_value) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                      \
      }
    #endif

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && LFDS720_PAL_GCC_VERSION >= 412 && LFDS720_PAL_GCC_VERSION < 473 )

    // TRD : __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 is a proxy for being an i486 or better, due to the lack of a better method

    #ifdef LFDS720_PAL_COMPILER
      #error More than one porting abstraction layer matches the current platform in lfds720_porting_abstraction_layer_compiler.h
    #endif

    #define LFDS720_PAL_COMPILER

    #define LFDS720_PAL_COMPILER_STRING          "GCC < 4.7.3"

    #define LFDS720_PAL_ALIGN(alignment)         __attribute__( (aligned(alignment)) )
    #define LFDS720_PAL_INLINE                   inline

    static LFDS720_PAL_INLINE void lfds720_pal_barrier_compiler( void )
    {
      __asm__ __volatile__ ( "" : : : "memory" );
    }

    #define LFDS720_PAL_BARRIER_COMPILER_LOAD    lfds720_pal_barrier_compiler()
    #define LFDS720_PAL_BARRIER_COMPILER_STORE   lfds720_pal_barrier_compiler()
    #define LFDS720_PAL_BARRIER_COMPILER_FULL    lfds720_pal_barrier_compiler()

    #define LFDS720_PAL_BARRIER_PROCESSOR_LOAD   __sync_synchronize()
    #define LFDS720_PAL_BARRIER_PROCESSOR_STORE  __sync_synchronize()
    #define LFDS720_PAL_BARRIER_PROCESSOR_FULL   __sync_synchronize()

    #define LFDS720_PAL_ATOMIC_ADD( target, value, result, result_type )                                                \
    {                                                                                                                   \
      LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                \
      (result) = (result_type) __sync_add_and_fetch( (lfds720_pal_uint_t *) &(target), (lfds720_pal_uint_t) (value) );  \
      LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                \
    }

    #define LFDS720_PAL_ATOMIC_CAS( destination, compare, exchange, cas_strength, result )  \
    {                                                                                       \
      lfds720_pal_uint_t                                                                    \
        original_compare;                                                                   \
                                                                                            \
      original_compare = (lfds720_pal_uint_t) (compare);                                    \
                                                                                            \
      LFDS720_PAL_BARRIER_COMPILER_FULL;                                                    \
      (compare) = __sync_val_compare_and_swap( &(destination), compare, exchange );         \
      LFDS720_PAL_BARRIER_COMPILER_FULL;                                                    \
                                                                                            \
      result = (unsigned char) ( original_compare == (lfds720_pal_uint_t) (compare) );      \
    }

    /* TRD : this is badly insensitive
             but it's the best we can do with GCC

             an actual i386 has no atomic support whatsoever
             an actual i486 has everything but no DWCAS
             i586 and later have everything
             the problem is you do not by any means always *get* __i486__, __i586__ or __i686__ predefined by GCC
             the only consistent predefine seems to be __i386__, for 32-bit processors
             as an aside, the __GCC_HAVE_SYNC_COMPARE_AND_SWAP_N predefines are totally useless
             __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 isn't defined on __arch64__
             I suspect __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 (four!) isn't even defined on __arm__
             __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 is sometimes only defined if -mcx16 is given, even if the target supports DWCASs

             arm has DWCAS from arm6k and later
    */

    #if( defined __i386__ || defined __arm__ )
      #define LFDS720_PAL_ATOMIC_DWCAS( destination, compare, exchange, cas_strength, result )                                                                                                   \
      {                                                                                                                                                                                          \
        int long long unsigned                                                                                                                                                                   \
          original_destination;                                                                                                                                                                  \
                                                                                                                                                                                                 \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                       \
        original_destination = __sync_val_compare_and_swap( (int long long unsigned volatile *) (destination), *(int long long unsigned *) (compare), *(int long long unsigned *) (exchange) );  \
        LFDS720_PAL_BARRIER_COMPILER_FULL;                                                                                                                                                       \
                                                                                                                                                                                                 \
        (result) = (char unsigned) ( original_destination == *(int long long unsigned *) (compare) );                                                                                            \
                                                                                                                                                                                                 \
        *(int long long unsigned *) (compare) = original_destination;                                                                                                                            \
      }
    #endif

    #define LFDS720_PAL_ATOMIC_EXCHANGE( destination, exchange, exchange_type )             \
    {                                                                                       \
      LFDS720_PAL_BARRIER_COMPILER_FULL;                                                    \
      (exchange) = (exchange_type) __sync_lock_test_and_set( &(destination), (exchange) );  \
      LFDS720_PAL_BARRIER_COMPILER_FULL;                                                    \
    }

    #define LFDS720_PAL_ATOMIC_SET( destination, new_value )           \
    {                                                                  \
      LFDS720_PAL_BARRIER_COMPILER_FULL;                               \
      (void) __sync_lock_test_and_set( (&destination), (new_value) );  \
      LFDS720_PAL_BARRIER_COMPILER_FULL;                               \
    }

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && LFDS720_PAL_GCC_VERSION >= 473 )

    #ifdef LFDS720_PAL_COMPILER
      #error More than one porting abstraction layer matches the current platform in lfds720_porting_abstraction_layer_compiler.h
    #endif

    #define LFDS720_PAL_COMPILER

    #define LFDS720_PAL_COMPILER_STRING          "GCC >= 4.7.3"

    #define LFDS720_PAL_ALIGN(alignment)         __attribute__( (aligned(alignment)) )
    #define LFDS720_PAL_INLINE                   inline

    // TRD : GCC >= 4.7.3 compiler barriers are built into the intrinsics
    #define LFDS720_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME

    #define LFDS720_PAL_BARRIER_PROCESSOR_LOAD   __atomic_thread_fence( __ATOMIC_ACQUIRE )
    #define LFDS720_PAL_BARRIER_PROCESSOR_STORE  __atomic_thread_fence( __ATOMIC_RELEASE )
    #define LFDS720_PAL_BARRIER_PROCESSOR_FULL   __atomic_thread_fence( __ATOMIC_ACQ_REL )

    #define LFDS720_PAL_ATOMIC_ADD( target, value, result, result_type )                    \
    {                                                                                       \
      (result) = (result_type) __atomic_add_fetch( &(target), (value), __ATOMIC_CONSUME );  \
    }

    #define LFDS720_PAL_ATOMIC_CAS( destination, compare, exchange, cas_strength, result )                                                                                                                                           \
    {                                                                                                                                                                                                                                \
      /* TRD : GCC throws an error without the casts, disgarding volatile type (for second arg) */                                                                                                                                   \
      result = (char unsigned) __atomic_compare_exchange_n( (lfds720_pal_uint_t volatile *) &(destination), (lfds720_pal_uint_t *) &(compare), (lfds720_pal_uint_t) (exchange), cas_strength, __ATOMIC_CONSUME, __ATOMIC_CONSUME );  \
    }

    #if( defined __i386__ || defined __arm__ )
      #define LFDS720_PAL_ATOMIC_DWCAS( destination, compare, exchange, cas_strength, result )                                                                                                                                                          \
      {                                                                                                                                                                                                                                                 \
        (result) = (char unsigned) __atomic_compare_exchange_n( (int long long unsigned volatile *) (destination), (int long long unsigned *) (compare), *(int long long unsigned *) (exchange), (cas_strength), __ATOMIC_CONSUME, __ATOMIC_CONSUME );  \
      }
    #endif

    // TRD : __ATOMIC_CONSUME not supported for exchange
    #define LFDS720_PAL_ATOMIC_EXCHANGE( destination, exchange, exchange_type )                          \
    {                                                                                                    \
      (exchange) = (exchange_type) __atomic_exchange_n( &(destination), (exchange), __ATOMIC_ACQUIRE );  \
    }

    // TRD : __ATOMIC_CONSUME not supported for exchange
    #define LFDS720_PAL_ATOMIC_SET( destination, new_value )                        \
    {                                                                               \
      (void) __atomic_exchange_n( &(destination), (new_value), __ATOMIC_ACQUIRE );  \
    }

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __x86_64__ )

    // TRD : lfds720_pal_uint_t volatile (*destination)[2], lfds720_pal_uint_t (*compare)[2], lfds720_pal_uint_t (*new_destination)[2], enum lfds720_misc_cas_strength cas_strength, char unsigned result

    #define LFDS720_PAL_ATOMIC_DWCAS( destination, compare, exchange, cas_strength, result )                         \
    {                                                                                                                \
      (result) = 0;                                                                                                  \
                                                                                                                     \
      __asm__ __volatile__                                                                                           \
      (                                                                                                              \
        "lock;"           /* make cmpxchg16b atomic        */                                                        \
        "cmpxchg16b %0;"  /* cmpxchg16b sets ZF on success */                                                        \
        "setz       %4;"  /* if ZF set, set result to 1    */                                                        \
                                                                                                                     \
        /* output */                                                                                                 \
        : "+m" ((destination)[0]), "+m" ((destination)[1]), "+a" ((compare)[0]), "+d" ((compare)[1]), "=q" (result)  \
                                                                                                                     \
        /* input */                                                                                                  \
        : "b" ((exchange)[0]), "c" ((exchange)[1])                                                                   \
                                                                                                                     \
        /* clobbered */                                                                                              \
        : "cc"                                                                                                       \
      );                                                                                                             \
    }

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __aarch64__ )

    // TRD : lfds720_pal_uint_t volatile destination[2], lfds720_pal_uint_t compare[2], lfds720_pal_uint_t exchange[2], enum lfds720_misc_cas_strength cas_strength, char unsigned result

    /* TRD : time for lots of notes on this code
             as it took a full day to research and develop

             with aarch64 registers have both 32 bit and 64 bit names
             the "%x" is a 64 bit register
             the "%w" is a 32 bit register

             ldxp and stxp are the bare LL/SC instructions - no memory barriers
             we however are usng ldaxp, which is combined load barrier and LL

             I *think* the monitor is set and *then* the load barrier is issued
             and so any invalidations break the monitor

             we need this barrier because : before this DWCAS, we load the compare
             in the DWCAS we then load the current value, and then compare - and if no one else
             broke the monitor by a write, then we store

             the problem is we could be reading the compare wrongly
             we read it - someone else writes but we don't see the invalidation request - and THEN we issue the LL

             no one else has broken the monitor (the invalidation request hasn't been seen yet) and so we load the
             now-wrong (but we don't know it yet) value AGAIN in the LL, and so the compare PASSES, and we store

             so we have to load the compare, put the monitor on, and THEN issue a load barrier

             moving on...

             ldaxp and stxp basically take a pointer argument (the final argument, alias_destination, which is type "Q")
             and then load two 64-bit values from that address

             temp[0] and temp[1] are both write-only ('=') since we don't care about their initial values
             both are earlyclobber since they are written to before we finish using input operands

             compare and result are both read/write since we want GCC to load their original values into registers for us
             and then to retain whatever values we write into them

             we do not clobber memory as we specifically indicate which memory locations are modified

             note we have to use local labels ("[n]:") because named labels are global and so the second
             time the compiler sees this macro, it throws an error, because the labels are already defined

             when jumping to a label, 'b' indicates backwards jump, 'f' indicates a forward jump

             now there's some slightly sneaky stuff to be aware of with sxtp and how it indicates the result of its operation

             stxp sets result to 1 on failure and 0 on success (which is the opposite of our API)

             however, we might not call stxp at all (if the compares fail right away) or - and this is important - 
             we might call it more than once, because we loop

             to deal with all this, we follow stxp's convention, and then logical not on the return
             so we start with result set to failed (1), in case the cmp fails right away
             after that we loop, with stxp controlling the loop by its result, until we're done
             and then we flip the result

             finally note this code right now is always "strong" CAS
    */

  #define LFDS720_PAL_ATOMIC_DWCAS( destination, compare, exchange, cas_strength, result )                        \
  {                                                                                                               \
    int unsigned                                                                                                  \
      local_result = 1;                                                                                           \
                                                                                                                  \
    lfds720_pal_uint_t volatile __attribute__( (aligned(16)) )                                                    \
      temp[2];                                                                                                    \
                                                                                                                  \
    __asm__ __volatile__                                                                                          \
    (                                                                                                             \
      "0:;"                                                                                                       \
      "  ldaxp  %x[alias_zero_temp], %x[alias_one_temp], %x[alias_destination];"                                  \
      "  cmp    %x[alias_zero_temp], %x[alias_zero_compare];"                                                     \
      "  bne    1f;"                                                                                              \
      "  cmp    %x[alias_one_temp], %x[alias_one_compare];"                                                       \
      "  bne    1f;"                                                                                              \
      "  stxp   %w[alias_local_result], %x[alias_zero_exchange], %x[alias_one_exchange], %x[alias_destination];"  \
      "  cbnz   %w[alias_local_result], 0b;"                                                                      \
      "1:;"                                                                                                       \
      "  mov    %x[alias_zero_compare], %x[alias_zero_temp];"                                                     \
      "  mov    %x[alias_one_compare], %x[alias_one_temp];"                                                       \
                                                                                                                  \
      /* output */                                                                                                \
      : [alias_zero_temp] "=&r" (temp[0]),                                                                        \
        [alias_one_temp] "=&r" (temp[1]),                                                                         \
        [alias_zero_compare] "+r" ((compare)[0]),                                                                 \
        [alias_one_compare] "+r" ((compare)[1]),                                                                  \
        [alias_local_result] "+r" (local_result),                                                                 \
        "=m" ((destination)[0]),                                                                                  \
        "=m" ((destination)[1])                                                                                   \
                                                                                                                  \
      /* input */                                                                                                 \
      : [alias_destination] "Q" (destination),                                                                    \
        [alias_zero_exchange] "r" ((exchange)[0]),                                                                \
        [alias_one_exchange] "r" ((exchange)[1])                                                                  \
                                                                                                                  \
      /* clobbered */                                                                                             \
      : "cc"                                                                                                      \
    );                                                                                                            \
                                                                                                                  \
    (result) = (char unsigned) !local_result;                                                                     \
  }

  #endif





  /****************************************************************************/
  #if( !defined LFDS720_PAL_COMPILER )

    #error No matching porting abstraction layer in lfds720_porting_abstraction_layer_compiler.h

  #endif

#endif

