#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>

/*
理想结果：输出 Test 04_1 OK!
*/

int main()
{
	uint64 start = 0x10000000;
	uint64 len = 4096;
	int prot = PROT_READ | PROT_WRITE;
	assert_eq(0, mmap((void *)start, len, prot, MAP_ANONYMOUS));
	for (uint64 i = start; i < (start + len); ++i) {
		uint8 *addr = (uint8 *)i;
		*addr = (uint8)i;
	}
	for (uint64 i = start; i < (start + len); ++i) {
		uint8 *addr = (uint8 *)i;
		assert_eq(*addr, (uint8)i);
	}
	puts("Test 04_0 OK!");
	return 0;
}