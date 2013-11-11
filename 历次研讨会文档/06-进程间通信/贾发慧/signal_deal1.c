#include<signal.h>
#include<stdio.h>
void handler(int sig)
{
   printf("Receive signal:%u\n",sig);

}

void main()
{
   struct sigaction sa;  
   int count;
   printf("pid is :%d\n",getpid());
   sa.sa_handler=handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags=0;
   
   sigaction(SIGTERM,&sa,NULL);
   
   sigprocmask(SIG_SETMASK,&sa.sa_mask,NULL);
   
   while(1)
   {
      sigsuspend(&sa.sa_mask);
      printf("loop\n");
   }
}
