#include <unistd.h> /* syscall() definition */
#include <sys/syscall.h> /* syscall numbers (ours won't be here if we haven't
                          * installed the uapi headers from our kernel
                          * build) */

/* #define __NR_eudyptula 451 */
static inline int sys_eudyptula(int high_id, int low_id)
{
	return syscall(__NR_eudyptula, high_id, low_id);
}
