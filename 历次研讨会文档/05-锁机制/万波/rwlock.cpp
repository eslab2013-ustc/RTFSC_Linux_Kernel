#include <iostream>
#include <cstdlib>

#include <unistd.h>
#include <pthread.h>

using namespace std;

struct sharedData{
    pthread_rwlock_t rwlock;
    int product;
}sharedData = {PTHREAD_RWLOCK_INITIALIZER, 0};

void * produce(void *ptr)   //写锁使用者
{
    for (int i = 0; i < 5; ++i)
    {
        pthread_rwlock_wrlock(&sharedData.rwlock);
        sharedData.product = i;
        pthread_rwlock_unlock(&sharedData.rwlock);

        sleep(1);
    }
}

void * consume1(void *ptr)  //读锁使用者
{
    for (int i = 0; i < 5;)
    {
        pthread_rwlock_rdlock(&sharedData.rwlock);
        cout<<"consume1:"<<sharedData.product<<endl;
        pthread_rwlock_unlock(&sharedData.rwlock);

        ++i;
        sleep(1);
    }
}

void * consume2(void *ptr) //读锁使用者
{
    for (int i = 0; i < 5;)
    {
        pthread_rwlock_rdlock(&sharedData.rwlock);
        cout<<"consume2:"<<sharedData.product<<endl;
        pthread_rwlock_unlock(&sharedData.rwlock);

        ++i;
        sleep(1);
    }
}

int main()
{
    pthread_t tid1, tid2, tid3;
    unsigned int flag = 0;

	/*创建三个线程，分别
	*int pthread_create(pthread_t*restrict tidp, const pthread_attr_t *restrict_attr, void*(*start_rtn)(void*), void *restrict arg);
	*1.由tidp指向的内存单元被设置为新创建线程的线程ID
	*2.attr参数用于指定各种不同的线程属性
	*3.新创建的线程从start_rtn函数的地址开始运行
	*4.该函数只有一个万能指针参数arg，如果需要向start_rtn函数传递的参数不止一个，
	*那么需要把这些参数放到一个结构中，然后把这个结构的地址作为arg的参数传入
	*/
    	pthread_create(&tid1, NULL, produce, NULL);
    	sleep(2);

    	pthread_create(&tid2, NULL, consume1, NULL);
    	pthread_create(&tid3, NULL, consume2, NULL);

    void *retVal;

	/*
	*以阻塞的方式等待thread指定的线程结束
	*thread: 线程标识符，即线程ID，标识唯一线程。
	*retval: 用户定义的指针，用来存储被等待线程的返回值
	*返回值：0代表成功；失败，返回的则是错误号。
	*/
    pthread_join(tid1, &retVal);
    pthread_join(tid2, &retVal);
    pthread_join(tid3, &retVal);

    return 0;
}