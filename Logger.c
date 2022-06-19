#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define Email "PutYourEmail@myemail.com"
#define PathOfExecutable1 "Path/of/myfirstscript"
#define NameOfExecutable1 "nameofscript"
#define PathOfExecutable2 ""Path/of/mysecondscript""
#define NameOfExecutable2 "nameofscript"
#define Max_Email_Counter 48 
#define SleepTime 300
#define BUFFERSIZE 4096

#define cknull(x) if((x)==NULL) {perror(""); exit(EXIT_FAILURE);}
#define cknltz(x) if((x)<0) {perror(""); exit(EXIT_FAILURE);}
#define LIST_LEN 1

void email_it(char *filename,FILE *logf);
void GenerateFileName(time_t Rtime,struct tm *TimeInfo,char *filename,char *NewFname,int mode);
int EndExecution(time_t Rtime,struct tm *TimeInfo);
FILE *OpenFile(char *filename);
int PipeError(int pipevalue,FILE *file);
void WriteToFile(char *buffer,FILE *file,struct tm * TimeInfo,pid_t PID);
void ReadPipe(int Pipe[],char *buffer,FILE *file);

int main() {

	int ForkValue1,ForkValue2,PipeValue1,PipeValue2;
	int Pipe1[2],Pipe2[2];
	char buffer[4096];
	char Pingbuffer[4096];
	int MailBool=0;
	char FileName[128],NewFileName[128],PingFileName[128],NewPingFileName[128];
	FILE *logf;
	FILE *logPing;
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	int MainPID=getpid();
	int init = 1;
	  
	GenerateFileName(rawtime,timeinfo,FileName,NewFileName,1);
	GenerateFileName(rawtime,timeinfo,PingFileName,NewPingFileName,2);

	logf=OpenFile(NewFileName);
	logPing=OpenFile(NewPingFileName);
	if(logf==NULL || logPing==NULL){
		exit(1);
	}

    while(1){

    	PipeValue1=pipe(Pipe1);
    	PipeValue2=pipe(Pipe2);
    	PipeError(PipeValue1,logf);
    	PipeError(PipeValue2,logf);
    	ForkValue1=fork();

    	if (init==1 || MainPID==getpid()){
			ForkValue2=fork();
			init=0;
		}

    	if(ForkValue1==0 && ForkValue2>0){
    		//Child Process.
    		dup2(Pipe1[1],STDOUT_FILENO);
    		sleep(SleepTime);
    		execl(PathOfExecutable1,NameOfExecutable1,NULL);
    		//Error if the below is executed.
    		perror("Exec ERROR!Fork1\n");
    	}
    	else if (ForkValue1 > 0 && ForkValue2 == 0){
    		//Child Process.
    		dup2(Pipe2[1],STDOUT_FILENO);
    		sleep(SleepTime);
    		execl(PathOfExecutable2,NameOfExecutable2,NULL);
    		//Error if the below is executed.
    		perror("Exec ERROR,Fork2!\n");
    	}

    	else if (ForkValue1 > 0 && ForkValue2 > 0){
    		//Parent Process.
    		close(Pipe1[1]);
    		close(Pipe2[1]);
    		waitpid(ForkValue2,NULL,0);
    		time(&rawtime);
    		timeinfo = localtime (&rawtime);
    		ReadPipe(Pipe2,Pingbuffer,logPing);
    		WriteToFile(Pingbuffer,logPing,timeinfo,ForkValue2);
    		waitpid(ForkValue1,NULL,0);
    		time(&rawtime);
    		timeinfo = localtime (&rawtime);
    		ReadPipe(Pipe1,buffer,logf);
    		WriteToFile(buffer,logf,timeinfo,ForkValue1);

    		time(&rawtime);
    		MailBool++;
    		if ((EndExecution(rawtime,timeinfo))==1){
    			WriteToFile("TIME-TRIGGERED:END OF EXECUTION.\n",logf,timeinfo,0);
    			WriteToFile("TIME-TRIGGERED:END OF EXECUTION.\n",logPing,timeinfo,0);
    			fclose(logf);
    			fclose(logPing);
    			return 1;
    		}else{
			
    			if (MailBool>=Max_Email_Counter){
    				timeinfo = localtime (&rawtime);
    				time (&rawtime);
    				fprintf(logf,"%s%s\n","Email is about to be sent.TimeStamp: ",asctime(timeinfo));
    				email_it(NewFileName,logf);
    				sleep(15);
    				email_it(NewPingFileName,logPing);
    				MailBool=0; //Reset The Email Trigger Counter.
    			}
    		}
    	}else{
    		//no use for the 3rd fork().
    		exit(3);
    	}
    }
    return 0;
}


void ReadPipe(int Pipe[],char *buffer,FILE *file){
	int R=read(Pipe[0],buffer,BUFFERSIZE*sizeof(char));
	if (R==0)
    {
    	fprintf(file,"%s\n","Error While Reading Value!");
    }
    fflush(file);

}

void WriteToFile(char *buffer,FILE *file,struct tm * TimeInfo,pid_t PID){
	int fpfv=fprintf(file,"%s%d%s%s%s\n","PID is : ",PID,"  Timestamp : ",asctime(TimeInfo),buffer);
    if(fpfv<0){
    	perror("Print to File ERROR!\n");
    }
    int fv=fflush(file);
    if (fv == EOF ){
    	perror("Flushing File ERROR!\n");
    }	
}

int PipeError(int pipevalue,FILE *file){

    if(pipevalue<0){
            perror("pipe()\n");
            fprintf(file,"%s\n","Error in pipe");
            fflush(file);
            exit(1);
        }
}

FILE *OpenFile(char *filename){

	FILE *file=fopen(filename,"a+");
    if (file==NULL){
    	perror("Error Creating File!");
    }
    return file;
}

int EndExecution(time_t Rtime,struct tm *TimeInfo){
	
	if((TimeInfo->tm_hour==0) && (TimeInfo -> tm_min <= 11 )){

		return 1;
	}

	return 0;

}

void GenerateFileName(time_t Rtime,struct tm *TimeInfo,char *filename,char *NewFname,int mode){
	//This Function is used to make the File Name of the txt file. 
	// => Day-Month-Hour:Minutes:Seconds-Year.txt with mode = 1
	// or => Day-Month-Hour:Minutes:Seconds-YearPING.txt with mode != 1

	sprintf(filename,"%s",asctime(TimeInfo));
    for(int i=0;i<strlen(filename)-1;i++){ //we dont want the year.
        if(filename[i]==' '){
            NewFname[i]='-';
        }else{
            NewFname[i]=filename[i];
        }
    }
    if (mode==1){
    	NewFname[strlen(NewFname)]= '\0';
    	strcat(NewFname,".txt");
    }else{
    	NewFname[strlen(NewFname)]= '\0';
    	strcat(NewFname,"PING.txt");
    }
}

void email_it(char *filename,FILE *logf)                                           
{                                                                       
	char content[256]={0};                                                
	char fpBuffer[512]={0};                                           
	char email_recepients[LIST_LEN][256]={{Email}};                                  
	int i=0;                                                            

	for(i=0;*email_recepients[i]>0;i++)                                
	{                                                            
		cknull(strcpy(content, email_recepients[i]));                             
		cknltz(sprintf (fpBuffer,"/usr/bin/mailx -s '%s %s' %s < %s","Logging:",filename,content,filename)); 

	if(system (fpBuffer)==(-1))                                     
	{
		perror("Email Failure!");
		fprintf(logf,"%s%s\n","Problem with sending the email!");
		fflush(logf);
	}                                                               
	}                                                                
}
