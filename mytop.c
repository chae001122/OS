#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <utmp.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <ncurses.h>
#include <signal.h>

int cur_cpu[9];
int past_cpu[9];
int w_row;
int w_col;
char head[5][1024];
int firstLine=0;
int op=0;
long long int total[1024];
typedef struct Proc{
	int PID;
	char USER[1024];
	char PR[3];
	int NI;
	long long int VIRT;
	long long RES;
	long long SHR;
	char S;
	double CPU;
	double MEM;
	char TIME[8];
	char COMMAND[1024];

}Proc;

Proc proc_struct[1024];
int run=0,slep=0,stop=0,zom=0, proc_count;

void FirstLine(){
	
	//-----------------현재 시간 구하기--------------------
	time_t curTime = time(NULL);
	struct tm *pLocal = localtime(&curTime);
	//----------------현재 시간 구하기 완료---------------
	

	//---------------- uptime 구하는 과정---------------------
	int fd;
	if((fd=open("/proc/uptime",O_RDONLY))<0){
		fprintf(stderr,"open error for uptime\n");
		exit(1);
	}

	FILE *fp = fdopen(fd,"r");
	char buf[20];
	memset(buf,'\0',sizeof(buf));
	fgets(buf,20,fp);
	buf[strlen(buf)] ='\0';
	//printf("%s\n",buf);
		
	char uptime_s[20];
	for(int i=0; i<strlen(buf); i++){
		if(buf[i] == ' ') break;
		else uptime_s[i] = buf[i];
	}
	float uptime;
	uptime = atof(uptime_s);
	//printf("utime = %d\n",utime);
	
	float check, hour_tmp;
	int day=0, min, hour;

	check = uptime / 3600;
	//printf("check : %f\n",check);
	if(check > 24){
		day = (int)(check/24);
		hour_tmp = (check-(day*24));
		hour = (int)hour_tmp;
		min = (hour_tmp - hour) * 60;
	}
	else{
		hour = (int)check;
		min = (check-hour) * 60;
	}
//----------------uptime 구하기 완료 -----------------------------
	
//--------------------user 수 구하기--------------------------
	int user;
	struct utmp *ut;
	setutent();
	while((ut=getutent()) !=NULL ){
		if(ut->ut_type == USER_PROCESS) user++;
	}
	//printf("user:%d\n",user);
	endutent();
//------------------------user 수 구하기 완료------------------------

//---------------loadavg 구하기 -------------------------
	int loadavg_fd;
	if((loadavg_fd=open("/proc/loadavg",O_RDONLY))<0){
		fprintf(stderr,"open error for loadavg\n");
		exit(1);
	}
	
	char loadavg_buf[20];
	memset(loadavg_buf,'\0',sizeof(loadavg_buf));
	FILE *loadavg_fp=fdopen(loadavg_fd,"r");
	fgets(loadavg_buf,20,loadavg_fp);
	loadavg_buf[strlen(loadavg_buf)] = '\0';

	char *loadavg_s[10];
	int idx=0;

	char *ptr = strtok(loadavg_buf," ");
	while(ptr != NULL){
		loadavg_s[idx] = ptr;
		idx++;

		ptr = strtok(NULL, " ");
	}
	
	float loadavg[3];
	for(int i=0; i<3; i++){
		loadavg[i] = atof(loadavg_s[i]);
	}

	//printf("%f %f %f\n",loadavg[0], loadavg[1], loadavg[2]);
//-----------------loadavg 구하기 끝---------------------




//-----------------출력-----------------------------------------
//	printf("top - ");
//	printf("%02d:%02d:%02d",pLocal->tm_hour,pLocal->tm_min,pLocal->tm_sec);
//	printf(" up  ");
	if(day == 0){
//		printf("%2d:%02d, ",hour,min);
		sprintf(head[0],"top - %02d:%02d:%02d up  %02d:%02d, %d user, load average: %3.2lf, %3.2lf. %3.2lf",pLocal->tm_hour,pLocal->tm_min,pLocal->tm_sec,hour,min,user,loadavg[0],loadavg[1],loadavg[2]);
	}
	else{
		//printf("%2ddays %2d:%2d, ",day,hour,min);
	sprintf(head[0],"top - %02d:%02d:%02d up %3d day %02d:%02d, %d user, load average: %3.2lf, %3.2lf. %3.2lf",pLocal->tm_hour,pLocal->tm_min,pLocal->tm_sec,hour,day,min,user,loadavg[0],loadavg[1],loadavg[2]);
	
	}
//	printf("%d users, ",user);
//	printf("load average: %.2f, %.2f, %.2f\n",loadavg[0], loadavg[1], loadavg[2]);

//-------------------출력 완료--------------------------------------

	fclose(loadavg_fp);
	close(loadavg_fd);
	fclose(fp);
	close(fd);
}

void SecondLine(){
	sprintf(head[1],"Tasks: %3d total, %3d running, %3d sleeping, %3d stopped, %3d zombie",proc_count,run,slep,stop,zom);
}

void ThirdLine(){
	long long int sum;
	FILE *stat_fp;
	if((stat_fp=fopen("/proc/stat","r")) == NULL){
		fprintf(stderr,"open error for stat\n");
		exit(1);
	}
	char tmp[1024];
	memset(tmp,0,1024);
	fscanf(stat_fp,"%s",tmp);
	for(int i=0; i<8; i++){
		fscanf(stat_fp,"%d",&cur_cpu[i]);
		sum+=cur_cpu[i];
	}
	cur_cpu[8]=sum;

	double us = (double)(cur_cpu[0]-past_cpu[0])/(double)(cur_cpu[8]-past_cpu[8])*100;
	double sy = (double)(cur_cpu[2]-past_cpu[2])/(double)(cur_cpu[8]-past_cpu[8])*100;
	double ui = (double)(cur_cpu[1]-past_cpu[1])/(double)(cur_cpu[8]-past_cpu[8])*100;
	double id = (double)(cur_cpu[3]-past_cpu[3])/(double)(cur_cpu[8]-past_cpu[8])*100;
	double wa = (double)(cur_cpu[4]-past_cpu[4])/(double)(cur_cpu[8]-past_cpu[8])*100;
	double hi = (double)(cur_cpu[5]-past_cpu[5])/(double)(cur_cpu[8]-past_cpu[8])*100;
	double si = (double)(cur_cpu[6]-past_cpu[6])/(double)(cur_cpu[8]-past_cpu[8])*100;
	double st = (double)(cur_cpu[7]-past_cpu[7])/(double)(cur_cpu[8]-past_cpu[8])*100;
	memcpy(past_cpu,cur_cpu, sizeof(past_cpu));
	

	sprintf(head[2],"%%Cpu(s): %3.1lf us, %3.1lf sy, % 3.1lf ni, %3.1lf id, %3.1lf wa, %3.1lf hi, %3.1lf si, %3.1lf st",us,sy,ui,id,wa,hi,si,st);
	fclose(stat_fp);
}


void FourthLine(){
	long long int memTotal_KB;
	double memTotal_MIB;
	int meminfo_fd;
	
	FILE *meminfo_fp;
	if((meminfo_fd=open("/proc/meminfo",O_RDONLY))<0){
		fprintf(stderr,"open error for meminfo\n");
		exit(1);
	}
	meminfo_fp=fdopen(meminfo_fd,"r");
	
	//---------------전체 메모리 크기 구하기--------------------
	char tmp[1024];
	memset(tmp,'\0',1024);
	fgets(tmp,1024,meminfo_fp);
	tmp[strlen(tmp)]='\0';
	for(int i=0; i<strlen(tmp); i++){
		if(!isdigit(tmp[i])){
			continue;
		}
		else{
			memTotal_KB = atoll(&tmp[i]);
			break;
		}
	}
	memTotal_MIB = memTotal_KB*1000 / 1048576; // 전체 물리 메모리 크기
	//------------------전체 메모리 완료----------------------------

	//-------------------free 메모리 크기 구하기---------------------
	long long int freeMem_KB;
	double freeMem_MIB;
	memset(tmp,'\0',1024);
	fgets(tmp,1024,meminfo_fp);
	tmp[strlen(tmp)] = '\0';
	for(int i=0; i<strlen(tmp); i++){
		if(!isdigit(tmp[i])){
			continue;
		}
		else{
			freeMem_KB = atoll(&tmp[i]);
			break;
		}
	}
	freeMem_MIB = freeMem_KB*1000 / 1048576; 

	//------------------free 메모리 완료 -------------------------------------

	
	//-------------------사용중인 메모리 크기 구하기----------------------

	long long int memUsed_KB;
	double memUsed_MIB;

		//------------buffer 구하기--------------
	long long int buffer_KB;
	double buffer_MIB;
	int check = 0;
	char line[1024];
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,meminfo_fp)){
		if(strncmp(line,"Buffers",7) == 0){
			check = 1;
			break;
		}
		else{
			memset(line,'\0',1024);
			continue;
		}
	}
	if(check==1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				buffer_KB = atoll(&line[i]);
				break;
			}
		}
	}
	buffer_MIB = buffer_KB*1000 / 1048576;
		//----------------buffer 구하기 완료---------------------

		//---------------cache 구하기 ---------------------------

	long long int cache_KB;
	double cache_MIB;
	memset(tmp,'\0',1024);
	fgets(tmp,1024,meminfo_fp);
	tmp[strlen(tmp)] = '\0';
	for(int i=0; i<strlen(tmp); i++){
		if(!isdigit(tmp[i])){
			continue;
		}
		else{
			cache_KB = atoll(&tmp[i]);
			break;
		}
	}
	cache_MIB = cache_KB*1000 / 1048576;
		//-----------------cache 구하기 완료 ------------------------

		//-----------------SReclaimable 구하기------------------------

	long long int SReclaimable_KB;
	double SReclaimable_MIB;
	check = 0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,meminfo_fp)){
		if(strncmp(line,"SReclaimable",12) == 0){
			check = 1;
			break;
		}
		else{
			memset(line,'\0',1024);
			continue;
		}
	}
	if(check==1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				SReclaimable_KB = atoll(&line[i]);
				break;
			}
		}
	}
	SReclaimable_MIB = SReclaimable_KB*1000 / 1048576;
		//-----------------SReclaimable 구하기 완료--------------------

	memUsed_KB = memTotal_KB - freeMem_KB - buffer_KB - cache_KB - SReclaimable_KB; 
	memUsed_MIB = memUsed_KB*1000/1048576;

	//----------------- 사용중인 메모리 크기 구하기 완료-------------------


	//-----------------buffer/cache 메모리 크기 구하기----------------------
	long long int buffer_cache_KB;
	double buffer_cache_MIB;

	buffer_cache_KB = buffer_KB + cache_KB + SReclaimable_KB;
	buffer_cache_MIB = buffer_cache_KB*1000/1048576;

	//----------------buffer/cache 메모리 크기 구하기 완료--------------------

	//------------------------출력-------------------------------------
	sprintf(head[3],"Mib Mem : %8.1lf total, %8.1lf free, %8.1lf used, %8.1lf buff/cache",memTotal_MIB,freeMem_MIB,memUsed_MIB,buffer_cache_MIB);
	//------------------------출력-----------------------------------

	fclose(meminfo_fp);
	close(meminfo_fd);

}

void FifthLine(){
	int meminfo_fd;
	FILE *meminfo_fp;
	if((meminfo_fd=open("/proc/meminfo",O_RDONLY))<0){
		fprintf(stderr,"open error for meminfo\n");
		exit(1);
	}
	meminfo_fp=fdopen(meminfo_fd,"r");
	
	//----------------사용 가능한 메모리 크기 구하기----------------
	long long int MemAvailable_KB;
	double MemAvailable_MIB;
	int check = 0;
	char line[1024];
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,meminfo_fp)){
		if(strncmp(line,"MemAvailable",12) == 0){
			check = 1;
			break;
		}
		else{
			memset(line,'\0',1024);
			continue;
		}
	}
	if(check==1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				MemAvailable_KB = atoll(&line[i]);
				break;
			}
		}
	}
	MemAvailable_MIB = MemAvailable_KB*1000 / 1048576;
	//----------------사용 가능한 메모리 크기 완료----------------

	//-------------SwapTotal 구하기------------------------
	long long int SwapTotal_KB;
	double SwapTotal_MIB;
	check = 0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,meminfo_fp)){
		if(strncmp(line,"SwapTotal",9) == 0){
			check = 1;
			break;
		}
		else{
			memset(line,'\0',1024);
			continue;
		}
	}
	if(check==1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				SwapTotal_KB = atoll(&line[i]);
				break;
			}
		}
	}else{
		printf("not checked\n");
	}
	SwapTotal_MIB = SwapTotal_KB*1000/1048576;
	//---------------SwapTotal 구하기 완료----------------------

	//----------------SwapFree 구하기-------------------------
	long long int SwapFree_KB;
	double SwapFree_MIB;
	char tmp[1024];
	memset(tmp,'\0',1024);
	fgets(tmp,1024,meminfo_fp);
	tmp[strlen(tmp)]='\0';
	for(int i=0; i<strlen(tmp); i++){
		if(!isdigit(tmp[i])){
			continue;
		}
		else{
			SwapFree_KB = atoll(&tmp[i]);
			break;
		}
	}

	SwapFree_MIB = SwapFree_KB*1000/1048576;
	//----------------SwapFree 구하기 완료---------------------------

	//------------------Swap used = SwapTotal-SwapFree구하기------
	long long int SwapUsed_KB = SwapTotal_KB-SwapFree_KB;
	double SwapUsed_MIB = SwapUsed_KB*1000 / 1048576;
	//------------------Swap used 구하기 완료------------------


	//------------------출력---------------------------

	sprintf(head[4],"Mib Swap: %8.1lf total, %8.1lf free, %8.1lf used, %8.1lf avail Mem", SwapTotal_MIB, SwapFree_MIB, SwapUsed_MIB, MemAvailable_MIB);
	//-------------------출력-----------------------------


//	snprintf(head[3],"Mib Mem : %8.1lf total, %8.1lf free, %8.1lf used, %8.1lf buff/cache\n",memTotal_MIB,freeMem_MIB,memUsed_MIB,buffer_cache_MIB);
	

	fclose(meminfo_fp);
	close(meminfo_fd);
}

long long int getUptime(){
	int uptime_fd;
	if((uptime_fd=open("/proc/uptime",O_RDONLY))<0){
		fprintf(stderr,"open error for uptime\n");
		exit(1);
	}
	char tmp[1024];
	memset(tmp,'\0',1024);

	read(uptime_fd,tmp,1024);
	close(uptime_fd);

	for(int i=0; i<sizeof(tmp); i++){
		if(tmp[i] == ' '){
			for(int j=i; j<sizeof(tmp); j++){
				tmp[j] = '\0';
			}
			break;
		}
	}

	long long int uptime = atoll(tmp);
	
	return uptime;
}	

long long getSHR(int PID){
	char path[1024];
	FILE *SHR_fp;
	memset(path,'\0',1024);
	sprintf(path,"/proc/%d/status",PID);
	path[strlen(path)] = '\0';
	if((SHR_fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,SHR_fp)){
		if(strncmp(line,"RssFile",7)==0){
			check = 1;
			break;
		}
	}
	if(check == 1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				fclose(SHR_fp);
				return atoll(&line[i]);
			}
		}
	}
	else{
		fclose(SHR_fp);
		return 0;
	}

}

long long getRES(int PID){
	char path[1024];
	FILE *RES_fp;
	memset(path,'\0',1024);
	sprintf(path,"/proc/%d/status",PID);
	path[strlen(path)] = '\0';
	if((RES_fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,RES_fp)){
		if(strncmp(line,"VmHWM",5)==0){
			check = 1;
			break;
		}
	}
	if(check == 1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				fclose(RES_fp);
				return atoll(&line[i]);
			}
		}
	}
	else{
		fclose(RES_fp);
		return 0;
	}

}



long long getVIRT(int PID){
	char path[1024];
	FILE *VIRT_fp;
	memset(path,'\0',1024);
	sprintf(path,"/proc/%d/status",PID);
	path[strlen(path)] = '\0';
	if((VIRT_fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,VIRT_fp)){
		if(strncmp(line,"VmSize",6)==0){
			check = 1;
			break;
		}
	}
	if(check == 1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				fclose(VIRT_fp);
				return atoll(&line[i]);
			}
		}
	}
	else{
		fclose(VIRT_fp);
		return 0;
	}

}

long long int getMemTotal(){
	FILE *MEM_fp;
	long long int memTotal;
	if((MEM_fp=fopen("/proc/meminfo","r"))==NULL){
		fprintf(stderr,"open error for meminfo");
		exit(1);
	}

	char line[1024];
	memset(line,'\0',1024);
	fgets(line,1024,MEM_fp);
	for(int i=0; i<strlen(line); i++){
		if(isdigit(line[i])){
			memTotal = atoll(&line[i]);
			break;
		}
	}
	fclose(MEM_fp);
	return memTotal;
}

int digit_filter(const struct dirent *dirent){
	if(isdigit(dirent->d_name[0]) != 0) return 1;
	else return 0;
}

void loadData(){
	struct dirent **dirlist;
	int count=0;
	if((count = scandir("/proc",&dirlist,*digit_filter,alphasort)) == -1){ //숫자만 거르기
		fprintf(stderr,"/proc Directory Scacn Error : %s\n",strerror(errno));
		exit(1);
	}
	proc_count = count;
	memset(proc_struct,0,sizeof(proc_struct));

	for(int i=0; i<count; i++){ // 문자로 된 dir 숫자로 바꿔서 저장
		proc_struct[i].PID = atoi(dirlist[i]->d_name);
	}
	Proc tmp;
	for(int i=0; i<count; i++){ //PID기준 정렬
		for(int j=i+1; j<count; j++){
			if(proc_struct[i].PID > proc_struct[j].PID){
				tmp = proc_struct[i];
				proc_struct[i] = proc_struct[j];
				proc_struct[j] = tmp;
			}
		}
	}
	

	struct stat statbuf;
	struct passwd *pwd;
	char statpath[1024];


	for(int i=0; i<count; i++){
		memset(statpath,'\0',sizeof(statpath));
		sprintf(statpath,"/proc/%d",proc_struct[i].PID);

		if(stat(statpath, &statbuf)==-1){ //pid로 uid가져오기
			fprintf(stderr,"%d stat error\n",proc_struct[i].PID);
			exit(1);
		}

		if((pwd=getpwuid(statbuf.st_uid))==NULL){ //uid로 user가져오기
			fprintf(stderr,"%d getpwuid error\n",proc_struct[i].PID);
		}
		//USER이름 저장
		char users[1024];
		memset(users,'\0',sizeof(users));
		strcpy(users,pwd->pw_name);
		if(strlen(users)>8){
			users[7]='+';
			for(int j=8; j<strlen(users); j++){
				users[j]='\0';
			}
		}
		strncpy(proc_struct[i].USER,users,10);

		//stat 내용 불러오기
		int process_fd;
		sprintf(statpath,"%s/stat",statpath);
		if((process_fd=open(statpath,O_RDONLY)) < 0){
			fprintf(stderr,"open error for %s\n",statpath);
			exit(1);
		}

		//프로세스의 stat을 저장
		char process_stat_info[1024];
		memset(process_stat_info,'\0',sizeof(process_stat_info));
		read(process_fd,process_stat_info,1024);
		close(process_fd);

		
		//프로세스 stat을 공백문자를 기준으로 저장
		int idx=0;
		char token[1024][1024];
		for(int i=0; i<1024; i++){
			memset(token[i],'\0',1024);
		}

		char *ptr = strtok(process_stat_info," ");
		while(ptr != NULL){
			strcpy(token[idx],ptr);
			idx++;
			ptr=strtok(NULL," ");
		}

		//PR 구하기
		memset(proc_struct[i].PR, '\0', sizeof(proc_struct[i].PR));
		if(atoi(token[17]) <= -100){
			strcpy(proc_struct[i].PR, "rt");
		}
		else{
			strcpy(proc_struct[i].PR, token[17]);
		}

		//NI 구하기
		proc_struct[i].NI = atoi(token[18]);
		
		//VIRT 구하기
		proc_struct[i].VIRT = getVIRT(proc_struct[i].PID);

		//RES 구하기
		long long int rss = getRES(proc_struct[i].PID);
		proc_struct[i].RES = rss;

		//SHR 구하기
		proc_struct[i].SHR = getSHR(proc_struct[i].PID);

		//STATE 구하기
		proc_struct[i].S = token[2][0];
		if(proc_struct[i].S == 'R') run++;
		else if(proc_struct[i].S == 'S') slep++;
		else if(proc_struct[i].S == 'T' || proc_struct[i].S == 't') stop++;
		else if(proc_struct[i].S == 'Z') zom++;
		
		//cpu 사용률 구하기
		long long int utime = atoll(token[13]);
		long long int stime = atoll(token[14]);
		long long int startTime = atoll(token[21]);
		long long int uptime  = getUptime();	

		long long int totalTime = utime+stime;
		double totalTime_tic = (double)totalTime/sysconf(_SC_CLK_TCK);
		proc_struct[i].CPU = (double)totalTime_tic/uptime*100;

		//메모리 사용률 구하기
		long long int memTotal = getMemTotal();
		proc_struct[i].MEM = (double)rss/(double)memTotal *100 * 1.024;
		
		//CPU 사용시간 구하기
		long long int ticTime = totalTime;
		total[i] = totalTime;
		long long int minTime = ticTime/3600;
		long long int secTime = (ticTime-minTime*3600)/60;
		ticTime = ticTime - minTime*3600 - secTime*60;
		memset(proc_struct[i].TIME,'\0',sizeof(proc_struct[i].TIME));
		sprintf(proc_struct[i].TIME, "%lld:%02lld.%02lld",minTime,secTime,ticTime);

		//Command 가져오기
		memset(proc_struct[i].COMMAND,'\0',1024);
		strcpy(proc_struct[i].COMMAND, token[1]+1);
		proc_struct[i].COMMAND[strlen(proc_struct[i].COMMAND)-1]='\0';

	}

}

void printHead(){
	memset(head,0,sizeof(head));
	FirstLine();
	SecondLine();
	ThirdLine();
	FourthLine();
	FifthLine();
}

void sortCpu(){
	Proc tmp;
	for(int i=0; i<proc_count-1; i++){
		for(int j=i+1; j<proc_count; j++){
			if(proc_struct[i].CPU < proc_struct[j].CPU){
				tmp=proc_struct[i];
				proc_struct[i] = proc_struct[j];
				proc_struct[j] = tmp;
			}
		}
	}
}
void sortMem(){
	Proc tmp;
	for(int i=0; i<proc_count-1; i++){
		for(int j=i+1; j<proc_count; j++){
			if(proc_struct[i].MEM < proc_struct[j].MEM){
				tmp=proc_struct[i];
				proc_struct[i] = proc_struct[j];
				proc_struct[j] = tmp;
			}
		}
	}

}
void sortProcTime(){
	Proc tmp;	
	long long int ttmp;
	for(int i=0; i<proc_count-1; i++){
		for(int j=i+1; j<proc_count; j++){
			if(total[i] < total[j]){
				ttmp = total[i];
				total[i] = total[j];
				total[j] = ttmp;

				tmp=proc_struct[i];
				proc_struct[i] = proc_struct[j];
				proc_struct[j] = tmp;
			}
		}
	}

}
void printMytop(){
	struct winsize size;

	if(ioctl(0,TIOCGWINSZ,(char*)&size)<0){
		fprintf(stderr,"ioctl error\n");
		exit(1);
	}

	w_row = size.ws_row;
	w_col = size.ws_col;

	char print[1024][1024];
	for(int i=0; i<1024; i++){
		memset(print,'\0',1024);
	}

	printHead();
	for(int i=0; i<5; i++){
		strncpy(print[i],head[i],w_col);
	}
	memset(print[5],' ', w_col);
	snprintf(print[6], w_col, "%5s %-8s %3s %3s %7s %6s %6s %s %4s %4s  %7s   %s","PID","USER","PR","NI","VIRT","RES","SHR","S","%CPU","%MEM","TIME+","COMMAND");

	if(op == 1){ //shift+p cpu 내림차순
		sortCpu();
	}
	else if(op == 2){//shift+m 메모리 내림차순
		sortMem();
	}
	else if(op == 3){//shift+t 프로세스가 돌아가고 있는 시간 순
		sortProcTime();
	}
	else if(op == 0){ //마지막꺼 유지
		//nothing
	}
	for(int i=7; i<w_row-1+firstLine; i++){
		snprintf(print[i],w_col,"%5d %-8s %3s %3d %7lld %6lld %6lld %c %4.1lf %4.1lf   %7s  %s",proc_struct[i-7+firstLine].PID,proc_struct[i-7+firstLine].USER,proc_struct[i-7+firstLine].PR,proc_struct[i-7+firstLine].NI,proc_struct[i-7+firstLine].VIRT,proc_struct[i-7+firstLine].RES,proc_struct[i-7+firstLine].SHR,proc_struct[i-7+firstLine].S,proc_struct[i-7+firstLine].CPU,proc_struct[i-7+firstLine].MEM,proc_struct[i-7+firstLine].TIME,proc_struct[i-7+firstLine].COMMAND);
	}
	
	for(int i=0; i<w_row-1; i++){
		printf("%s\n",print[i]);
	}
	
}

void sigint_handler(int signo){
	write(1,"\33[3J\33[H\33[2J",11); //clear inpterpreter console
	run =0; slep=0; stop=0; zom=0;
	loadData();
	printMytop();
	alarm(3);
}

int myGetch(){
	int ch;
	struct termios buf, save;
	tcgetattr(0,&save);
	buf = save;
	buf.c_lflag &= ~ICANON;
	buf.c_lflag &= ~ECHO;

	tcsetattr(0, TCSAFLUSH, &buf);
	ch = getchar();
	tcsetattr(0, TCSAFLUSH, &save);
	return ch;
}

int checkinput(){
	char a = myGetch();
	int ascii = 0;
	if(a=='q') return 1;
	else if((int)a==77){ //메모리 순
		return 4;
	}
	else if((int)a==80){ //cpu 사용률
		return 5;
	}
	else if((int)a==84){ //프로세스가 돌아가고 있는 시간 순
		return 6;
	}
	else{
		ascii += (int)a;
		char b = myGetch();
		ascii += (int)b;
		char c = myGetch();
		ascii += (int)c;
		if(ascii==183) return 2;
		else if(ascii == 184) return 3;
	}
	return 0;
	
}
int main(){
	fflush(stdout);
	loadData();
	printMytop();

	signal(SIGALRM,sigint_handler);

	while(1){
		alarm(3);
		int input = checkinput();
		if(input == 1) exit(0);
		else if(input == 2){
			if(firstLine > 0){
				firstLine--;
				loadData();
				printMytop();
			}
			else continue;
		}
		else if(input == 3){
			if(firstLine<proc_count){
				firstLine++;
				loadData();
				printMytop();
			}
			else continue;
		}
		else if(input == 4){ //메모리
			loadData();
			op = 2;
			printMytop();
		}
		else if(input == 5){ //cpu
			op = 1;
			printMytop();
		}
		else if(input == 6){ //프로세스 시간
			op=3;
			printMytop();
		}
	}
	return 0;
}
