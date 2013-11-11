#include <iostream>  
#include <cstring>  
#include <errno.h>  
  
#include <unistd.h>  
#include <fcntl.h>  
#include <sys/shm.h>  
#include <sys/sem.h>  
  
using namespace std;  
  
#define PATH_NAME "/tmp/shm"  
  
union semun    
{    
	int val;                               
	struct semid_ds *buf;                 
	unsigned short int *array;          
	struct seminfo *__buf;              
};    
  
int main()  
{  
	    int fd;  
		  
		if ((fd = open(PATH_NAME, O_RDONLY)) < 0)  
		{  
			cout<<"open file "<<PATH_NAME<<"failed.";  
			cout<<strerror(errno)<<endl;  
			return -1;  
		}  
			  
		close(fd);  
				  
		key_t keyShm = ftok(PATH_NAME, 0);  
		key_t keySem = ftok(PATH_NAME, 1);  
						  
		int shmID, semID;  
							      
		if ((shmID = shmget(keyShm, sizeof(int), 0)) < 0)  
		{  
			cout<<"shmget failed..."<<strerror(errno)<<endl;  
			return -1;  
		}  
								  
		int *buf = (int *)shmat(shmID, 0, 0);  
									  
		if ((semID = semget(keySem, 1, 0)) < 0)  
		{  
			cout<<"semget failed..."<<strerror(errno)<<endl;  
			return -1;  
		}  
										      
		struct sembuf buffer;    
		buffer.sem_num = 0;    
		buffer.sem_op = -1;    
		buffer.sem_flg = 0;    
														  
		//获得信号量资源  
		semop(semID, &buffer, 1);  
															  
		cout<<"process 2:recv "<<*buf<<endl;  
																  
		return 0;  
} 

