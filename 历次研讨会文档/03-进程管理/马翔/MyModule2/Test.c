#include <stdio.h>
#include <unistd.h>

int main()
{
	int ret;
	ret = syscall(223);
	printf("The Ret : %d\n", ret);
	return 0;
}
