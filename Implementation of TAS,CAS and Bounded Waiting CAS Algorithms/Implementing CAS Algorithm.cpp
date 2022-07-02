#include <pthread.h>
#include <iostream>
#include <atomic>
#include <unistd.h>
#include <fstream>
#include <random>
#include <limits.h>
#include<time.h>

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

FILE* file_output=fopen("CAS-Log.txt","w");

int n,k;
float lambda_1,lambda_2;

atomic<int> lock (0); // lock variable

double total_wait_time = 0 ; // total waiting time
double max_wait_time = 0; // max waiting time


void *testCAS(void *param)
{   
  
 int id = *(int*)param; // thread ID

  default_random_engine generator;
	exponential_distribution<double> d1(1.0/lambda_1); 
	exponential_distribution<double> d2(1.0/lambda_2); 
  
  for(int i=0;i<k;i++)
    {
      //entry section
     time_t Time_req = time(NULL);                       
     tm *reqEnterTime = localtime(&Time_req);
      
      
		fprintf(file_output,"%dth CS Requested at %.1d :%.1d :%.1d by thread %d \n" , i+1,reqEnterTime->tm_hour,reqEnterTime->tm_min,reqEnterTime->tm_sec, id);
      
    double start=clock();
      while(1)
        {
          int a=0;
          int b=1;
          if(lock.compare_exchange_strong(a,b))
          {
             break;
          }
        }
    //crtical section
      
       double end=clock();
      
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
  lock=0;
      
  // remainder section
  usleep(d2(generator)*1e6);
      
    }
  pthread_exit(0);
  }


int main() {
  
  char line[100];
  ifstream myfile("inp-params.txt");
  myfile.getline(line,100);
  sscanf(line," %d" "%d" "%f" "%f " ,&n,&k,&lambda_1,&lambda_2);

fprintf(file_output,"CAS ME Output\n");
  
  pthread_t workers[n];
  int id_number[n];
  pthread_attr_t attr[n];
  
for (int i = 0; i < n; i++)
{ 
  id_number[i]=i+1;
  pthread_attr_init(&attr[i]);
  pthread_create(&workers[i],&attr[i],testCAS,&id_number[i]);
}
  
for(int i=0;i<n;i++)
{
  pthread_join(workers[i],NULL);
}


cout<<"waiting time is "<< float(total_wait_time)/float(n*k)<<endl;

cout<<"max_waiting time is "<<float(max_wait_time)<<endl;

  
return 0;
}