#include<stdio.h>
#include<unistd.h>
#include<signal.h>
void sig_int(int signo)
{
        printf("\nint sig_int\n");
}
 
int main(void)
{
        struct sigaction new_action;
        new_action.sa_handler=sig_int;
        if(sigemptyset(&new_action.sa_mask)==-1)
       {
                printf("set new action mask error\n");
                exit(1);
        }
        new_action.sa_flags=0;
       //new_action.sa_flags=SA_RESTART;
 
        if(sigaction(SIGINT,&new_action,NULL)==-1)
        {
                printf("set SIGINT process error\n");
                exit(1);
         }
         char c;
         read(0,&c,1);
         printf("read :%c\n",c);
         exit(0);
}