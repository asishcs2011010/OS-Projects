#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include<sys/time.h>
#include <cstdlib>
#include <algorithm>
#include <semaphore.h>
#include <fstream>
#include <random>
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

FILE*file_output=fopen("FairRW-Log.txt","w");

FILE*file_output_avg=fopen("average_time.txt","w");

int n_w,n_r,k_w,k_r;
int read_count=0;
float lambda_cs,lambda_rem;

 default_random_engine generator1,generator2;
 exponential_distribution<double> cs(1.0/lambda_cs); 
 exponential_distribution<double> rem(1.0/lambda_rem);

double reader_avg_time=0,reader_max_time=0;
double writer_avg_time=0,writer_max_time=0;

sem_t mutex , rw_mutex , queue;
// mutex helps in increment of read_count with out inconsistency
// rw_mutex allows mutual exclusibilty in CS
// queue handles service_queue

void *reader(void *param)
{   
  int id = *(int*)param; // thread ID

  for(int i=0;i<k_r;i++)
    {
      //entry section
     time_t Time_req = time(NULL);                       
     tm *reqEnterTime = localtime(&Time_req);
    
		fprintf(file_output,"%dth CS Requested at %.1d :%.1d :%.1d by reader thread %d \n" , i+1,reqEnterTime->tm_hour,reqEnterTime->tm_min,reqEnterTime->tm_sec, id);

  double start=clock();
      
    sem_wait(&queue); // block other threads 
		sem_wait(&mutex);
      
		read_count=read_count+1;
      
		if (read_count == 1) // block writers thread 
    {
      sem_wait(&rw_mutex);
    }
      
		sem_post(&queue);// release semaphore 
		sem_post(&mutex);// rw_mutex allows mutual exclusibilty in CS
      
  double end=clock();

  double total = ((double)(end- start)/double(CLOCKS_PER_SEC));
      
   reader_avg_time = reader_avg_time+total;
	 reader_max_time =  max(reader_max_time,total);

  //critical section
   time_t Time_act = time(NULL);
   tm *actEnterTime = localtime(&Time_act);

fprintf(file_output,"%dth CS Entered at %.1d :%.1d :%.1d by reader thread %d\n" , i+1,actEnterTime->tm_hour,actEnterTime->tm_min,actEnterTime->tm_sec, id);

  usleep(cs(generator1)*1e3);// simulating real time crtitical cetion

  // exit section
      
    sem_wait(&mutex);
      
		read_count=read_count-1;
      
		if (read_count == 0)
    {
      sem_post(&rw_mutex);
      //sem_post signals to allow a thread in to CS(basically adds one to rw_mutex semaphore)
    }
      
		sem_post(&mutex); // signals to allow other threads 

   time_t Time_exit = time(NULL);
   tm *exitTime = localtime(&Time_exit);

fprintf(file_output,"%dth CS Exited at %.1d :%.1d :%.1d by reader thread %d \n" , i+1,exitTime->tm_hour,exitTime->tm_min,exitTime->tm_sec, id);
      
  // remainder section
  usleep(rem(generator2)*1e3);
      
    }
  pthread_exit(0);
  }


void *writer(void *param)
{   
  int id = *(int*)param; // thread ID

  for(int i=0;i<k_w;i++)
    {
      //entry section
  
     time_t Time_req = time(NULL);                       
     tm *reqEnterTime = localtime(&Time_req);
    
		fprintf(file_output,"%dth CS Requested at %.1d :%.1d :%.1d by writer thread %d \n" , i+1,reqEnterTime->tm_hour,reqEnterTime->tm_min,reqEnterTime->tm_sec, id);
    
    double start=clock();

      sem_wait(&queue); // block queue
		  sem_wait(&rw_mutex); // block readers and writers
		  sem_post(&queue);
     
    double end=clock();
      
 double total=((double)(end- start)/double(CLOCKS_PER_SEC));

  
    writer_avg_time = writer_avg_time + total;
		writer_max_time =  max(writer_max_time, total);

  
      //critical section
      
    time_t Time_act = time(NULL);
   tm *actEnterTime = localtime(&Time_act);

fprintf(file_output,"%dth CS Entered at %.1d :%.1d :%.1d by writer thread %d\n" , i+1,actEnterTime->tm_hour,actEnterTime->tm_min,actEnterTime->tm_sec, id);
      
    usleep(cs(generator1)*1e3);//simulating real time crtitical section    
    sem_post(&rw_mutex);
      
  // exit section
   time_t Time_exit = time(NULL);
   tm *exitTime = localtime(&Time_exit);

fprintf(file_output,"%dth CS Exited at %.1d :%.1d :%.1d by writer thread %d \n" , i+1,exitTime->tm_hour,exitTime->tm_min,exitTime->tm_sec, id);

  // remainder section
    usleep(rem(generator2)*1e3);
      
    }
  pthread_exit(0);
  }

int main() {
  
  char line[100];
  ifstream myfile("inp-params.txt");
  myfile.getline(line,100);
  sscanf(line,"%d" "%d" "%d" "%d" "%f" "%f " ,&n_w,&n_r,&k_w,&k_r,&lambda_cs,&lambda_rem);

  cs = exponential_distribution<double>(1/lambda_cs);
	rem = exponential_distribution<double>(1/lambda_rem);

  // semaphores initialisations
	sem_init(&mutex, 0, 1);
	sem_init(&rw_mutex, 0, 1);
	sem_init(&queue,0,1);

    //writer threads
  pthread_t writers[n_w];
  int id_w[n_w];
  pthread_attr_t attr_w[n_w];
  
//reader threads
  pthread_t readers[n_r];
  int id_r[n_r];
  pthread_attr_t attr_r[n_r];


for (int i = 0;i < n_w; i++)
{ 
  id_w[i]=i+1;
  pthread_attr_init(&attr_w[i]);
  pthread_create(&writers[i],&attr_w[i],writer,&id_w[i]);
}

for (int i = 0; i < n_r; i++)
{ 
  id_r[i]=i+1;
  pthread_attr_init(&attr_r[i]);
  pthread_create(&readers[i],&attr_r[i],reader,&id_r[i]);
}
  
for(int i=0;i<n_w;i++)
{
  pthread_join(writers[i],NULL);
}
   for(int i=0;i<n_r;i++)
{
  pthread_join(readers[i],NULL);
}

	fprintf(file_output_avg,"Average time in standard reader-wrters is %lf milliseconds\n" , (writer_avg_time + reader_avg_time)*(1000)/(float)(n_r*k_r + n_w*k_w));
  
  fprintf(file_output_avg,"Average time taken for reader threads to enter Cs is %lf milliseconds\n" ,reader_avg_time*(1000)/(n_r*k_r));
  
  fprintf(file_output_avg,"Average time taken for writer threads to enter Cs is %lf milliseconds\n" ,writer_avg_time*(1000)/(n_w*k_w));

   fprintf(file_output_avg,"Worst case time taken for reader threads to enter Cs is %lf milliseconds\n" ,reader_max_time*(1000));
  
	  fprintf(file_output_avg,"Worst case time taken for writer threads to enter Cs is %lf milliseconds\n" ,writer_max_time*(1000));
return 0;
}