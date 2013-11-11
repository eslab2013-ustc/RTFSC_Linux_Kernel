#include<signal.h>
#include<stdio.h>
void handler(int sig)
{
   printf("Receive signal:%u\n",sig);
}

void main()
{
   struct sigaction sa;  
   int count,i=1;
   sa.sa_handler=handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags=0;
   
   sigaction(SIGINT,&sa,NULL);
   
   sigprocmask(SIG_SETMASK,&sa.sa_mask,NULL);
   
   while(i<3)
   {
     sigsuspend(&sa.sa_mask);
     printf("loop\n");
     i++;
   }
   sigaddset(&sa.sa_mask,SIGINT);
   sigprocmask(SIG_SETMASK,&sa.sa_mask,NULL);
  // sigprocmask(SIG_UNBLOCK,&sa.sa_mask,NULL);
   while(i>2)
   {
      sigsuspend(&sa.sa_mask);
      printf("loop\n");
      
   }
  
}
