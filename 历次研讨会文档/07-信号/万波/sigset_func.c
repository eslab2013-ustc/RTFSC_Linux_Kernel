#include<signal.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

static void my_op(int);
int main()
{
	sigset_t new_mask,old_mask,pending_mask;
	struct sigaction act;
	
	printf("process start\n");
	
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	act.sa_handler=my_op;
	if(sigaction(SIGQUIT,&act,NULL)<0)    //安装信号
		printf("install signal SIGQUIT error\n");

	sigemptyset(&new_mask);	  //清除信号
	sigaddset(&new_mask,SIGQUIT);   //添加信号
	if(!sigprocmask(SIG_BLOCK, &new_mask,&old_mask))  //阻塞添加的信号
		printf("block signal SIGQUIT\n");

	sleep(10);	
	printf("now begin to get pending mask and unblock SIGQUIT\n");
	if(sigpending(&pending_mask)<0)   //得到挂起的信号 man sigpending
		printf("get pending mask error\n");  
	if(sigismember(&pending_mask,SIGQUIT))  //判断是否有此信号被挂起
		printf("signal SIGQUIT is pending\n");

	if(sigprocmask(SIG_SETMASK,&old_mask,NULL)<0)   //使所有信号不被阻塞
		printf("unblock signal error\n");
	printf("signal unblocked\n");

	sleep(10);
}
static void my_op(int signum)
{
	printf("receive signal %d \n",signum);  //打印进程号
}



SIGHUP     终止进程     终端线路挂断
SIGINT     终止进程     中断进程
SIGQUIT   建立CORE文件终止进程，并且生成core文件
SIGILL   建立CORE文件       非法指令
SIGTRAP   建立CORE文件       跟踪自陷
SIGBUS   建立CORE文件       总线错误
SIGSEGV   建立CORE文件       段非法错误
SIGFPE   建立CORE文件       浮点异常
SIGIOT   建立CORE文件       执行I/O自陷
SIGKILL   终止进程     杀死进程
SIGPIPE   终止进程     向一个没有读进程的管道写数据
SIGALARM   终止进程     计时器到时
SIGTERM   终止进程     软件终止信号
SIGSTOP   停止进程     非终端来的停止信号
SIGTSTP   停止进程     终端来的停止信号
SIGCONT   忽略信号     继续执行一个停止的进程
SIGURG   忽略信号     I/O紧急信号
SIGIO     忽略信号     描述符上可以进行I/O
SIGCHLD   忽略信号     当子进程停止或退出时通知父进程
SIGTTOU   停止进程     后台进程写终端
SIGTTIN   停止进程     后台进程读终端
SIGXGPU   终止进程     CPU时限超时
SIGXFSZ   终止进程     文件长度过长
SIGWINCH   忽略信号     窗口大小发生变化
SIGPROF   终止进程     统计分布图用计时器到时
SIGUSR1   终止进程     用户定义信号1
SIGUSR2   终止进程     用户定义信号2
SIGVTALRM 终止进程     虚拟计时器到时