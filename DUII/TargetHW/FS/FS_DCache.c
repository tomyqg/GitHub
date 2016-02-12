#include "SEGGER.h"
#include "MMC_MCI_HW.h"
#include "RTOS.h"

/*********************************************************************
* *
FS_MCI_HW_CleanDCacheRange
* *
Parameters:
* p - Pointer to the region that shall be flushed from cache.
* NumBytes - Number of bytes to flush
*
*/
void FS_MCI_HW_CleanDCacheRange(void * p, unsigned NumBytes) {
	OS_ARM_DCACHE_CleanRange(p, NumBytes);
}

/*********************************************************************
*
* FS_MCI_HW_InvalidateDCache
*
* Parameters:
* p - Pointer to the buffer that shall be invalidated in cache.
* NumBytes - Number of bytes to invalidate
*
*/
void FS_MCI_HW_InvalidateDCache(void * p, unsigned NumBytes) {
	OS_ARM_DCACHE_InvalidateRange(p, NumBytes);
}