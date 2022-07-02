#include <pthread.h>
#include <iostream>
#include <atomic>
#include <unistd.h>
#include <fstream>
#include <random>
#include <limits.h>

using namespace std;

float max_1(float a,float b)
{
  if(a>b)
  {
    return a;
  }
  else 
  {
    return b;
  }
}
FILE*file_output=fopen("CAS-Bounded-Log.txt","w"); 

int n , k ; 
float lambda_1 , lambda_2 ;

atomic<int> lock (0); // lock variable

atomic<bool>* wait; //atomic waiting array

float total_wait_time = 0 ; // total waiting time
float max_wait_time =  INT_MIN; // max waiting time


void *testCAS_bounded(void *param){

	int id = *(int*)param;  // thread ID

	default_random_engine generator;
	exponential_distribution<double> d1(1.0/lambda_1);
	exponential_distribution<double> d2(1.0/lambda_2); 

	for(int i=0;i<k;i++){
		// entry section
    
		wait[id-1] = true;
    
		bool key = false;
    
    time_t Time_req = time(NULL);                       
     tm *reqEnterTime = localtime(&Time_req);
   
		fprintf(file_output,"%dth CS Requested at %.1d :%.1d :%.1d by thread %d \n" , i+1,reqEnterTime->tm_hour,reqEnterTime->tm_min,reqEnterTime->tm_sec, id);

     double start=clock();
    
		while (wait[id-1] && !key){
			int x =0 , y=1;
			key = lock.compare_exchange_strong(x,y);
		}
		wait[id-1] = false;	
    
    double end=clock();
    
		// critical section stimulation	
  	
    time_t Time_act = time(NULL);
  
    tm *actEnterTime = localtime(&Time_act);

    double total=((double)(end- start)/double(CLOCKS_PER_SEC));
      
      total_wait_time = total_wait_time+ total; 

  		max_wait_time = max_1(max_wait_time,total); 

fprintf(file_output," %dth CS Entered at %.1d :%.1d :%.1d by thread %d\n" , i+1,actEnterTime->tm_hour,actEnterTime->tm_min,actEnterTime->tm_sec, id);
    
  		usleep(d1(generator)*1e6);
    
  time_t Time_exit = time(NULL);
   tm *exitTime = localtime(&Time_exit);

fprintf(file_output," %dth CS Exited at %.1d :%.1d :%.1d by thread %d \n" , i+1,exitTime->tm_hour,exitTime->tm_min,exitTime->tm_sec, id);

  		// exit section
  		int j = (id)%n; 
  		while((j!=id-1) && !wait[j]) // selecting which thread should go in CS
  			j = (j+1)%n;

  		if (j==id-1)
  			lock = 0;
  		else
  			wait[j] = false; 
  		// remainder section
  		usleep(d2(generator)*1e6);
	}
  pthread_exit(0);
}

int main(){

  char line[100];
  ifstream myfile("inp-params.txt");
  myfile.getline(line,100);
  sscanf(line," %d" "%d" "%f" "%f " ,&n,&k,&lambda_1,&lambda_2);
  
	fprintf(file_output,"Bounded CAS ME Output\n");
  
	pthread_t workers[n];
	int id_number[n];
	pthread_attr_t attr[n];
	wait = (atomic<bool>*)malloc(n*sizeof(atomic<bool>));
	for(int i=0;i<n;i++){  // crrate n threads
		id_number[i] = i+1;
		pthread_attr_init(&attr[i]);
		pthread_create(&workers[i] ,&attr[i],testCAS_bounded,&id_number[i]);
	}

	for(int i=0;i<n;i++){  // wait for all threads to finish
		pthread_join(workers[i],NULL);
	}
	free(wait);

cout<<"waiting time is "<< float(total_wait_time)/float(n*k)<<endl;

cout<<"max_waiting time is "<<float(max_wait_time)<<endl;

	return 0;
}