#include <iostream>  
#include <cstring>  
#include <errno.h>  
  
#include <unistd.h>  
#include <fcntl.h>  
#include <sys/shm.h>  
  
using namespace std;  
  
#define PATH_NAME "/tmp/shm"  
  
int main()  
{  
	 int fd;  
		  
	 if ((fd = open(PATH_NAME, O_CREAT, 0666)) < 0)  
	 {  
	      cout<<"open file "<<PATH_NAME<<"failed.";  
		  cout<<strerror(errno)<<endl;  
		  return -1;  
	 }  
			  
	 close(fd);  
				  
	 key_t key = ftok(PATH_NAME, 0);  
	
	 int shmID;  
     
	 if ((shmID = shmget(key, 4, IPC_CREAT | 0666)) < 0)  
	 {  
		 cout<<"shmget failed..."<<strerror(errno)<<endl;  
		 return -1;  
	 }  
	  
	 shmid_ds shmInfo;  
	
	 shmctl(shmID, IPC_STAT, &shmInfo);  
			  
	 cout<<"shm key:0x"<<hex<<key<<dec<<endl;  
	 cout<<"shm id:"<<shmID<<endl;  
	 cout<<"shm_segsz:"<<shmInfo.shm_segsz<<endl;  
	 cout<<"shm_nattch:"<<shmInfo.shm_nattch<<endl;  
	 return 0;  
}  






