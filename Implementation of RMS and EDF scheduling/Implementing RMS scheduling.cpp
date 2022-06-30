#include <iostream>
#include <string>
#include<fstream>

using namespace std;

struct Process
{ 
  int in_time;            //  the time at which process becomes ready
  int p_id;               //  process id
  int t;                  //  processing time
  int p;                  //  period
  char status;            // 't' is terminated
  int time_left;          //  proccessing time left for a process
  int wait_time;          //  wait time of the process
};

typedef struct Process process;

process wait[1000];       // array of processes with their in_times & their metadeta

ofstream file_log("RMS-log.txt");

int count_w=0, terminated_count=0;

ofstream fout;

float avg_wait[1000];     //avg waiting of each process
int num_process[1000];    //no of times a process runs


void insert(int p_id,int t,int p,int time_in)  //function to insert in_times to their respective process along with the meta data
{
  wait[count_w].p_id=p_id;
  wait[count_w].t=t;
  wait[count_w].p=p;
  wait[count_w].in_time=time_in;
  wait[count_w].time_left=t;
   count_w=count_w+1;
}
 void write_1(int index_run,int curr_time,int prev_index,std::ofstream &output_file)// function to print start and resume statments into the output file
 {
     
    if(wait[index_run].time_left==wait[index_run].t && curr_time+wait[index_run].time_left<wait[index_run].p+wait[index_run].in_time)
      {
        output_file<<"Process P"<<wait[index_run].p_id<< " starts execution at time "<<curr_time<<endl;
      }

    if(prev_index!=-1)
      {
        if(wait[index_run].time_left<wait[index_run].t && prev_index!=index_run && curr_time+wait[index_run].time_left<=wait[index_run].p+
           wait[index_run].in_time)
           {  
             output_file<<"Process P"<<wait[index_run].p_id<< " resumes its execution at time "<<curr_time<<endl;
           }  
      }
 }

 void write_2(int prev_index,int curr_time, int index_run, std::ofstream &output_file)// function to print finish and preempt statments into the output file
 {
     
   if(prev_index!=-1)
   {
      if(wait[prev_index].time_left==0)
        {
           output_file<<"Process P"<<wait[prev_index].p_id<<" finishes execution at time " <<curr_time<<endl;
        }

      if(wait[prev_index].p_id!=wait[index_run].p_id && wait[prev_index].time_left!=0 && index_run!=-1 && wait[prev_index].time_left!=wait[prev_index].t)
        {
          output_file<<"Process P"<<wait[prev_index].p_id<<" is preempted by Process P"<<wait[index_run].p_id<<" at time "<<curr_time<<". Remaining processing time: "<<wait[prev_index].time_left<<endl;
        }
   }
 }

void write_3(int prev_index,int curr_time, int index_run,std::ofstream &output_file)// function to print CPU idle statment into the output file
{
   if(prev_index==-1 && index_run!=-1 && curr_time>0)
   { 
      output_file<<"CPU idle till "<<curr_time-1<<endl;
   }

}

int find_max_priority(int time)    // function to find index number of max priority process from the array
 {
   process ready[1000];                      //array contains all process which are active to run
   int return_index, count_r=0, ind_arr[1000], check=0;

    for(int i=0;i<count_w;i++)
      {
        if(wait[i].time_left!=0 && wait[i].status!='t' && wait[i].in_time<=time)
          {
             ready[count_r]=wait[i];
             ind_arr[count_r]=i;
             count_r=count_r+1;
             check++;
          }
      }

   if(check!=0)
     {
         process temp=ready[0];
         return_index=ind_arr[0];


        for(int i=1;i<count_r;i++)
            {
              if(ready[i].p<temp.p)         
                  {
                     temp=ready[i];
                     return_index=ind_arr[i];
                  }
             else if(ready[i].p==temp.p) // if periods are same then we compare process id
                    {
                      if(ready[i].p_id<temp.p_id)
                         {
                            temp=ready[i];
                            return_index=ind_arr[i];
                         }
    
                     }

             }
     return return_index;
       }
  return -1;
 }



int check()             // function to check if all process are either runned are terminated
{
  for(int i=0;i<count_w;i++)
  {
    if(wait[i].time_left!=0&&wait[i].status!='t')
    {
      return 0;
    }
  }
 return 1;
}

void wait_time(int time,int index_run)// increase the wait time of all other process other than process which is currently is running 
{ 
   process temp[1000]; //array contains all process which are active to run
   int count_t=0, arr_index[1000];

    for(int i=0;i<count_w;i++)
      {
         if(wait[i].in_time<=time&&wait[i].status!='t'&&wait[i].time_left!=0)
            {
              temp[count_t]=wait[i];
              arr_index[count_t]=i;
              count_t++;
            }
      }

    for(int j=0;j<count_t;j++)
      {
         if(arr_index[j]!=index_run)
             {
               wait[arr_index[j]].wait_time++;
             }
        }
}


int check_terminate(int current_time) // functions gives index of a process which had reached deadline
{
   process ready[1000];
   int count_r=0, ind_arr[1000], check=0;

     for(int i=0;i<count_w;i++)
         {
            if(current_time==wait[i].in_time+wait[i].p&&wait[i].status!='t'&&wait[i].time_left!=0)
                {
                   ready[count_r]=wait[i];
                   ind_arr[count_r]=i;
                  count_r=count_r+1;
                   check++;
                }
         }

  int return_index;

     if(check!=0)
        {
          process temp=ready[0];
          return_index=ind_arr[0];

            for(int i=1;i<count_r;i++)
                 {
                    if(ready[i].p_id<temp.p_id)
                        {
                           temp=ready[i];
                           return_index=ind_arr[i];
                        }
                 }
         return return_index;
       }
  return -1;
}



void start_schedulling()// function starts schedulling
{
    int curr_time=0;
    int prev_index=-1;

  while(check()!=1)
    { 
      int index_run=find_max_priority(curr_time);

      int index_wait=index_run/20;

    if(prev_index!=-1&&wait[prev_index].time_left!=0&prev_index!=index_run&&wait[prev_index].status!='t')// preempted process gets n status
       {
          wait[prev_index].status='n';
       }

        if(index_run!=-1)
          {
            write_3( prev_index, curr_time, index_run,file_log);        // function to print statements
            write_2( prev_index, curr_time, index_run,file_log);
            write_1(index_run,curr_time,prev_index,file_log);
  
             if(check_terminate(curr_time)!=-1) // if process misses deadline it is been removes
                {
                   int check_ter=check_terminate(curr_time);
                   wait[check_ter].status='t';
                   terminated_count=terminated_count+1;
                   file_log<<"Process P"<<wait[check_ter].p_id<<" does not met its deadline and it is removes at time "<<curr_time<<endl;
                }
           else if(curr_time+wait[index_run].time_left>wait[index_run].p+wait[index_run].in_time&&wait[index_run].status!='r')//if the highest priority process misses the deadline in future, it is termintaed here
                {   wait[index_run].status='t';
                    terminated_count=terminated_count+1;
                    file_log<<"Process P"<<wait[index_run].p_id<<" does not met its deadline and it is removes at time "<<curr_time<<endl;
                 }
            else     // the higehst priority process runs for a iterartion
                 {
                    wait[index_run].time_left= wait[index_run].time_left-1;
                    wait_time(curr_time, index_run);
                    curr_time=curr_time+1;   
                    wait[index_run].status='r'; 
                 }

                 prev_index=index_run;
        
         }
    else
         { 
            write_2( prev_index, curr_time, index_run,file_log);// cpu idle statement
            prev_index=-1;
            curr_time=curr_time+1;
        }
   }

  file_log<<"Process P"<<wait[prev_index].p_id<<" finishes execution at time "<<curr_time<<endl;
  }

int main() {

int n,c=-1, checker=0, num_1=0;

char line[100];

ifstream file("inp-params.txt");

while(file.getline(line,100))
{
   if(c==-1)
   {
       n=atoi(line);
       c=c+1;
   }
  else if(checker<n)
   {  
      int p_id,t,p,k,int_time=0;
      sscanf(line,"%d""%d""%d""%d",&p_id,&t,&p,&k);

     file_log<<"Process P"<<p_id<<": processing time="<<t<<"; deadline:"<<p<<"; period:"<<p<<" joined the system at time "<<0<<endl;

      num_process[num_1]=k;

       num_1=num_1+1;

       for(int j=0;j<k;j++)
         {
           insert(p_id,t,p,int_time);
           int_time=int_time+p;
         }

      checker++;
   }
}


start_schedulling();

int total=0;
 for(int i=0;i<n;i++) // finding avg waiting times
  {
    float sum=0;
    for(int j=total;j<total+num_process[i];j++)
      {
       sum=sum+wait[j].wait_time;
      }
     total=total+num_process[i];
     avg_wait[i]=sum/num_process[i]; 
  }


int total_process=0;

for(int i=0;i<n;i++)       
{
  total_process=total_process+num_process[i];
}

ofstream fout;

fout.open("RMS-Stats.txt") ;

  //Printing out the final statistics results int to stats file
  fout<<"Number of processes that came into the system: "<<total_process<<endl;
  fout<<"Number of processes that successfully completed: "<<total_process - terminated_count<<endl;
  fout<<"Number of processes that missed their deadline: "<<terminated_count<<"\n"<<endl;
  float total_avg = 0 ;
  for(int i=0;i<n;i++)
  {
    
    fout<<"Average waiting time for each process"<<i+1<<" is: "<<avg_wait[i]<<" milliseconds"<<endl ;
    total_avg= total_avg +avg_wait[i]*num_process[i] ;
  }
  fout<<"\nAverage waiting time for all the processes is "<<(float)total_avg/total_process<<" milliseconds"<<endl;

  fout.close(); 
  //closing the stats file after the statistics are printed.

}