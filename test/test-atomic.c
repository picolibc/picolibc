#include <stdatomic.h>
#include <inttypes.h>
#include <stdio.h>

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
_Atomic uint32_t    atomic_4;
#endif
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
_Atomic uint16_t    atomic_2;
#endif

int
main(void)
{
    int ret = 0;
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
    uint32_t    zero_4 = 0;
    if (atomic_compare_exchange_strong(&atomic_4, &zero_4, 1)) {
        printf("atomic_compare_exchange 4 worked, value %" PRIu32 " zero %" PRIu32 "\n",
               atomic_4, zero_4);
        uint32_t        old;
        old = atomic_exchange_explicit(&atomic_4, 0, memory_order_relaxed);
        if (atomic_4 == 0 && old == 1) {
            printf("atomic_exchange_explicit 4 worked %" PRIu32 " value now %" PRIu32 "\n",
                   old, atomic_4);
        } else {
            printf("atomic_exchange_explicit failed %" PRIu32 " value now %" PRIu32 "\n",
                   old, atomic_4);
            ret = 1;
        }
    } else {
        printf("atomic_compare_exchange 4 failed, value %" PRIu32 "\n", atomic_4);
        ret = 1;
    }
#endif
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
    uint16_t zero_2 = 0;
    if (atomic_compare_exchange_strong(&atomic_2, &zero_2, 1)) {
        printf("atomic_compare_exchange 2 worked, value %" PRIu16 " zero %" PRIu16 "\n", atomic_2, zero_2);
        uint16_t        old;
        old = atomic_exchange_explicit(&atomic_2, 0, memory_order_relaxed);
        if (atomic_2 == 0 && old == 1) {
            printf("atomic_exchange_explicit 2 worked %" PRIu16 " value now %" PRIu16 "\n",
                   old, atomic_2);
        } else {
            printf("atomic_exchange_explicit 2 failed %" PRIu16 " value now %" PRIu16 "\n",
                   old, atomic_2);
            ret = 1;
        }
    } else {
        printf("atomic_compare_exchange 2 failed, value %" PRIu16 "\n", atomic_2);
        ret = 1;
    }
#endif
    return ret;
}
