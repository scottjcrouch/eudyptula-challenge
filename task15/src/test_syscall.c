#include <assert.h>

#include "sys_eudyptula.h"

#include <asm/eudyptula_64.h>

int main(int argc, char *argv[])
{
	int err;

	/* Test 1 */
	{
		err = sys_eudyptula(0xbeefbad, 0xb0ba900d);
		assert(!err);
	}

	/* Test 2 */
	{
		err = sys_eudyptula(0, 0);
		assert(err);
	}

	/* Test 3 */
	{
		err = sys_eudyptula(0xb0ba900d, 0xbeefbad);
		assert(err);
	}

	/* Test 4 */
	{
		err = sys_eudyptula(0, 0xb0ba900d);
		assert(err);
	}

	/* Test 5 */
	{
		err = sys_eudyptula(0xbeefbad, 0);
		assert(err);
	}

	/* Test 6 */
	{
		err = sys_eudyptula((EUDYPTULA_ID >> 32) & 0xFFFFFFFF, EUDYPTULA_ID & 0xFFFFFFFF);
		assert(!err);
	}

	/* Test 7 */
	{
		err = sys_eudyptula((EUDYPTULA_ID >> 32) & 0xFFFFFFFF, 0);
		assert(err);
	}

	/* Test 8 */
	{
		err = sys_eudyptula(0, EUDYPTULA_ID & 0xFFFFFFFF);
		assert(err);
	}

	return 0;
}
