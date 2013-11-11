#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void printsigset(const sigset_t *set)
{
	int i;
	for (i = 1; i < 32; i++)
		if (sigismember(set, i) == 1)
			putchar('1');
		else
			putchar('0');
	puts("");
}

int main(void)
{
	sigset_t s, p;
	sigemptyset(&s);
        printf("the pid is :%d\n",getpid());
	sigaddset(&s, SIGINT);
        sigaddset(&s,SIGTERM);
        
	sigprocmask(SIG_BLOCK, &s, NULL);
	while (1) {
		sigpending(&p);
		printsigset(&p);
		sleep(1);
	}
	return 0;
}
