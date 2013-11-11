#include <iostream>
#include <cstring>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>

#include <cstdlib>
#include <pthread.h>

#define PATH_NAME "/tmp/shm"

using namespace std;

void * create(void * ptr)
{
   int fd;  
     
   if ((fd = open(PATH_NAME, O_CREAT, 0666)) < 0)
   {  
	   cout<<"open file "<<PATH_NAME<<"failed.";  
	   cout<<strerror(errno)<<endl;  
   }  
	     
   close(fd);  
		     
   key_t key = ftok(PATH_NAME, 0);  
			     
   int shmID;  
				         
   if ((shmID = shmget(key, 4096, IPC_CREAT | 0666)) < 0)  
   {  
	   cout<<"shmget failed..."<<strerror(errno)<<endl;  
   }  
					     
   shmid_ds shmInfo;  
   shmctl(shmID, IPC_STAT, &shmInfo);  

   cout<<"--->Create a Shared Memory!<---"<<endl;
   cout<<"shm key:0x"<<hex<<key<<dec<<endl;  
   cout<<"shm id:"<<shmID<<endl;  
   cout<<"shm_segsz:"<<shmInfo.shm_segsz<<endl;  
   cout<<"shm_nattch:"<<shmInfo.shm_nattch<<endl;
   cout<<endl;

}

void * write(void * ptr)
{
	int fd;

	if((fd = open(PATH_NAME, O_CREAT, 0666))<0)
	{
		cout<<"open file "<<PATH_NAME<<"failed."<<endl;
		cout<<strerror(errno)<<endl;
	}
	
	close(fd);
	
	key_t key = ftok(PATH_NAME, 0);

	int shmID;
	
	if((shmID = shmget(key,0,0))<0)
	{
		cout<<"shmget gailed ..."<<strerror(errno)<<endl;
	}

	char *buf = (char *)shmat(shmID, 0,0);

	buf[0]='h';buf[1]='e';buf[2]='l';buf[3]='l';buf[4]='o';
	buf[5]=' ';buf[6]='w';buf[7]='o';buf[8]='r';buf[9]='l';
	buf[10]='d';buf[11]='!';buf[12]='\0';
	cout<<"--->process1 Write " <<buf <<"<---"<<endl;

	shmid_ds shmInfo;  
	shmctl(shmID, IPC_STAT, &shmInfo);  
		  
	cout<<"shm key:0x"<<hex<<key<<dec<<endl;  
	cout<<"shm id:"<<shmID<<endl;  
	cout<<"shm_segsz:"<<shmInfo.shm_segsz<<endl;  
	cout<<"shm_nattch:"<<shmInfo.shm_nattch<<endl;  
	cout<<endl;
						  
}

void * read(void * ptr)
{
	int fd;  
	    
	if ((fd = open(PATH_NAME, O_RDONLY)) < 0)  
	{  
		cout<<"open file "<<PATH_NAME<<"failed.";  
		cout<<strerror(errno)<<endl;  
	}  
		    
	close(fd);

	key_t key = ftok(PATH_NAME,0);

	int shmID;

	if ((shmID = shmget(key,0, 0)) < 0)  
	{  
		cout<<"shmget failed..."<<strerror(errno)<<endl;  
	}  
	
	char *buf = (char *)shmat(shmID,0,0);
	

	cout<<"--->process2 Read "<<buf<<"<---"<<endl;
	sleep(1);
	
	
	cout<<"--->process2 Read "<<buf<<"<---"<<endl;

	shmid_ds shmInfo;  
        shmctl(shmID, IPC_STAT, &shmInfo);  
	  
	cout<<"shm key:0x"<<hex<<key<<dec<<endl;  
	cout<<"shm id:"<<shmID<<endl;  
	cout<<"shm_segsz:"<<shmInfo.shm_segsz<<endl;  
	cout<<"shm_nattch:"<<shmInfo.shm_nattch<<endl;  
	cout<<endl;

}
int main()
{
	pthread_t tid1,tid2,tid3;

	pthread_create(&tid1,NULL,create,NULL);
	sleep(1);
	pthread_create(&tid2,NULL,write,NULL);
	sleep(1);
	pthread_create(&tid3,NULL,read,NULL);
	sleep(1);

	void *retVal;

	pthread_join(tid1,&retVal);
	pthread_join(tid2,&retVal);
	pthread_join(tid3,&retVal);

	return 0;

}





