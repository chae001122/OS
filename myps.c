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

#define PATH_LEN 1024
#define TTY_LEN 1024
#define BUFFER_SIZE 1024

typedef struct Proc{
	int PID;
	char USER[1024];
	long long int VSZ;
	long long int RSS;
	double CPU;
	double MEM;
	char TTY[1024];
	char STAT[1024];
	char START[6];
	char TIME[10];
	char COMMAND[1024];

}Proc;
struct stat statbuf;
struct passwd *pwd;
char statpath[1024];

struct winsize size;
int w_col;

Proc proc_struct[1024];
int proc_count;
int p=1;
char print[1024][1024];

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

long long getVmLck(int PID){
	char path[1024];
	FILE *Vm_fp;
	memset(path,'\0',1024);
	sprintf(path,"/proc/%d/status",PID);
	path[strlen(path)] = '\0';
	if((Vm_fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,Vm_fp)){
		if(strncmp(line,"VmLck",5)==0){
			check = 1;
			break;
		}
		else{
			memset(line,'\0',1024);
			continue;
		}
	}
	if(check == 1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				fclose(Vm_fp);
				return atoll(&line[i]);
			}
		}
	}
	else{
		fclose(Vm_fp);
		return 0;
	}

}

long long getRSS(int PID){
	char path[1024];
	FILE *RSS_fp;
	memset(path,'\0',1024);
	sprintf(path,"/proc/%d/status",PID);
	path[strlen(path)] = '\0';
	if((RSS_fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,RSS_fp)){
		if(strncmp(line,"VmRSS",5)==0){
			check = 1;
			break;
		}
		else{
			memset(line,'\0',1024);
			continue;
		}
	}
	if(check == 1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				fclose(RSS_fp);
				return atoll(&line[i]);
			}
		}
	}
	else{
		fclose(RSS_fp);
		return 0;
	}

}



long long getVmSize(int PID){
	char path[1024];
	FILE *V_fp;
	memset(path,'\0',1024);
	sprintf(path,"/proc/%d/status",PID);
	path[strlen(path)] = '\0';
	if((V_fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,V_fp)){
		if(strncmp(line,"VmSize",6)==0){
			check = 1;
			break;
		}
		else{
			memset(line,'\0',1024);
			continue;
		}
	}
	if(check == 1){
		for(int i=0; i<strlen(line); i++){
			if(isdigit(line[i])){
				fclose(V_fp);
				return atoll(&line[i]);
			}
		}
	}
	else{
		fclose(V_fp);
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
void getTTY(char path[1024], char tty[1024])
{
	char fdZeroPath[PATH_LEN];			//0??? fd??? ?????? ?????? ??????
	memset(tty, '\0', PATH_LEN);
	memset(fdZeroPath, '\0', TTY_LEN);
	strcpy(fdZeroPath, path);
	strcat(fdZeroPath, "/fd/0");

	if(access(fdZeroPath, F_OK) < 0){	//fd 0??? ?????? ??????

		char statPath[PATH_LEN];		// /proc/pid/stat??? ?????? ?????? ??????
		memset(statPath, '\0', PATH_LEN);
		strcpy(statPath, path);
		strcat(statPath, "/stat");

		FILE *statFp;
		if((statFp = fopen(statPath, "r")) == NULL){	// /proc/pid/stat open
			fprintf(stderr, "fopen error %s %s\n", strerror(errno), statPath);
			sleep(1);
			return;
		}

		char buf[BUFFER_SIZE];
		for(int i = 0; i <= 6; i++){		// 7????????? read??? TTY_NR ??????
			memset(buf, '\0', BUFFER_SIZE);
			fscanf(statFp, "%s", buf);
		}
		fclose(statFp);

		int ttyNr = atoi(buf);		//ttyNr ?????? ????????? ??????

		DIR *dp;
		struct dirent *dentry;
		if((dp = opendir("/dev")) == NULL){		// ????????? ?????? ?????? /dev ???????????? open
			fprintf(stderr, "opendir error for %s\n", "dev");
			exit(1);
		}
		char nowPath[PATH_LEN];

		while((dentry = readdir(dp)) != NULL){	// /dev ???????????? ??????
			memset(nowPath, '\0', PATH_LEN);	// ?????? ?????? ?????? ?????? ?????? ??????
			strcpy(nowPath, "/dev");
			strcat(nowPath, "/");
			strcat(nowPath, dentry->d_name);

			struct stat statbuf;
			if(stat(nowPath, &statbuf) < 0){	// stat ??????
				fprintf(stderr, "stat error for %s\n", nowPath);
				exit(1);
			}
			if(!S_ISCHR(statbuf.st_mode))		//?????? ???????????? ????????? ?????? ?????? skip
				continue;
			else if(statbuf.st_rdev == ttyNr){	//?????? ???????????? ????????? ???????????? ID??? ttyNr??? ?????? ??????
				strcpy(tty, dentry->d_name);	//tty??? ?????? ????????? ??????
				break;
			}
		}
		closedir(dp);

		if(!strlen(tty))					// /dev????????? ?????? ?????? ??????
			strcpy(tty, "?");				//nonTerminal
	}
	else{
		char symLinkName[128];
		memset(symLinkName, '\0', 128);
		if(readlink(fdZeroPath, symLinkName, 128) < 0){
			fprintf(stderr, "readlink error for %s\n", fdZeroPath);
			exit(1);
		}
		if(!strcmp(symLinkName, "/dev/null"))		//symbolic link??? ???????????? ????????? /dev/null??? ??????
			strcpy(tty, "?");					//nonTerminal
		else
			sscanf(symLinkName, "/dev/%s", tty);	//??? ?????? ?????? tty ??????

	}
	return;
}

void loadData(){
	struct dirent **dirlist;
	int count=0;
	if((count = scandir("/proc",&dirlist,*digit_filter,alphasort)) == -1){ //????????? ?????????
		fprintf(stderr,"/proc Directory Scacn Error : %s\n",strerror(errno));
		exit(1);
	}
	proc_count = count;
	memset(proc_struct,0,sizeof(proc_struct));

	for(int i=0; i<count; i++){ // ????????? ??? dir ????????? ????????? ??????
		proc_struct[i].PID = atoi(dirlist[i]->d_name);
	}
	Proc tmp;
	for(int i=0; i<count; i++){ //PID?????? ??????
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

		if(stat(statpath, &statbuf)==-1){ //pid??? uid????????????
			fprintf(stderr,"%d stat error\n",proc_struct[i].PID);
			exit(1);
		}

		if((pwd=getpwuid(statbuf.st_uid))==NULL){ //uid??? user????????????
			fprintf(stderr,"%d getpwuid error\n",proc_struct[i].PID);
			exit(1);
		}
		//USER?????? ??????
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

		//stat ?????? ????????????
		int process_fd;
		char newstatpath[1024];
		memset(newstatpath,'\0',1024);
		sprintf(newstatpath,"%s/stat",statpath);
		if((process_fd=open(newstatpath,O_RDONLY)) < 0){
			fprintf(stderr,"open error for %s\n",newstatpath);
			exit(1);
		}

		//??????????????? stat??? ??????
		char process_stat_info[1024];
		memset(process_stat_info,'\0',sizeof(process_stat_info));
		read(process_fd,process_stat_info,1024);
		close(process_fd);

		
		//???????????? stat??? ??????????????? ???????????? ??????
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

			
		//VSZ ?????????
		proc_struct[i].VSZ= getVmSize(proc_struct[i].PID);

		//RSS ?????????
		long long int rss = getRSS(proc_struct[i].PID);
		proc_struct[i].RSS = rss;

				
		//cpu ????????? ?????????
		long long int utime = atoll(token[13]);
		long long int stime = atoll(token[14]);
		long long int startTime = atoll(token[21]);
		long long int uptime  = getUptime();	

		long long int totalTime = utime+stime;
		double totalTime_tic = (double)totalTime/sysconf(_SC_CLK_TCK);
		proc_struct[i].CPU = (double)totalTime_tic/uptime*100;

		//????????? ????????? ?????????
		long long int memTotal = getMemTotal();
		proc_struct[i].MEM = (double)rss/(double)memTotal *100 * 1.024;
		
		
		//TIME ?????????
		int secTime = (int)totalTime_tic;
		int minTime = secTime/60;
		memset(proc_struct[i].TIME,'\0',sizeof(proc_struct[i].TIME));
		sprintf(proc_struct[i].TIME,"%d:%02d",minTime,secTime-minTime*60);

		//START ?????????
		time_t fulltime;
		struct tm *lt;

		fulltime = time(NULL);
		time_t start_time = fulltime-uptime+(atoi(token[21])/sysconf(_SC_CLK_TCK));
		memset(proc_struct[i].START,'\0',sizeof(proc_struct[i].START));
		lt = localtime(&start_time);
		sprintf(proc_struct[i].START,"%02d:%02d",lt->tm_hour,lt->tm_min);

		
		//STAT ?????????
		memset(proc_struct[i].STAT,'\0',1024);
		char stat_tmp[1024];
		memset(stat_tmp,'\0',1024);

		strcat(stat_tmp, token[2]);

		if(proc_struct[i].PID == atoi(token[5])){
			strcat(stat_tmp, "s");
		}
		
		if(atoi(token[18]) > 0){
			strcat(stat_tmp, "N");
		}
		else if(atoi(token[18])<0){
			strcat(stat_tmp, "<");
		}
		if(getVmLck(proc_struct[i].PID) != 0){
			strcat(stat_tmp,"L");
		}
		if(atoi(token[19]) > 1){
			strcat(stat_tmp,"l");
		}
		if(atoi(token[7]) != -1){
			strcat(stat_tmp, "+");
		}
		strcpy(proc_struct[i].STAT,stat_tmp);
		
		//TTY ?????????
		memset(proc_struct[i].TTY, '\0', 1024);
		getTTY(statpath,proc_struct[i].TTY);
		
		
		//COMMAND????????????
		char cmdline_path[1024];
		memset(cmdline_path, '\0', 1024);
		sprintf(cmdline_path, "/proc/%d/cmdline", proc_struct[i].PID);
		FILE * cmdline_fp = fopen(cmdline_path, "r");

		char command[1024];
		memset(command,  '\0',1024);
		
		if(NULL!=fgets(command, 1024, cmdline_fp)){
			for(int i=0; i<sizeof(command); i++){
				if((command[i]=='\0')&&(command[i+1]!='\0')){
					command[i]=' ';
				}
			}
			command[strlen(command)]='\0';
			memset(proc_struct[i].COMMAND, '\0', 1024);
			strcpy(proc_struct[i].COMMAND, command);
			fclose(cmdline_fp);
		}
		else{//comm????????? ????????????
			char comm_path[1024];
			memset(comm_path,'\0', 1024);
			sprintf(comm_path, "/proc/%d/comm", proc_struct[i].PID);
			FILE * comm_fp = fopen(comm_path, "r");
			if(NULL==fgets(command, 1024, comm_fp)){
				fprintf(stderr, "no path\n");
				exit(1);
			}
			else{
				for(int i=0; i<strlen(command); i++){
					if(command[i]=='\n'){
						command[i]='\0';
					}
				}

				memset(proc_struct[i].COMMAND,  '\0' , 1024);
				sprintf(proc_struct[i].COMMAND, "[%s]", command);

				fclose(comm_fp);
			}
			memset(command, '\0', 1024);
		}

	}

}

void setSize(){
	if(ioctl(0,TIOCGWINSZ, (char*)&size)<0){
		fprintf(stderr,"ioctl error\n");
		exit(1);
	}
	w_col = size.ws_col;
}

void print_option_a(){
	snprintf(print[0],w_col,"%5s %-9s%-5s%7s %-8s","PID", "TTY","STAT", "TIME", "COMMAND");
	for(int i=0; i<proc_count; i++){
		if(strcmp(proc_struct[i].TTY, "?")!=0){
			snprintf(print[p++],w_col,"%5d %-9s%-5s%7s %-8s",  
				proc_struct[i].PID, 
				proc_struct[i].TTY,
				proc_struct[i].STAT,
				proc_struct[i].TIME,
				proc_struct[i].COMMAND);
		}
	}
}
void print_option_u(){
	snprintf(print[0],w_col,"%-10s%5s%5s%5s%8s%7s %-9s%-5s%7s%7s %-8s", "USER", "PID", "%CPU", "%MEM" ,"VSZ", "RSS","TTY","STAT", "START", "TIME", "COMMAND");
	for(int i=0; i<proc_count; i++){
		memset(statpath, '\0', 1024);
		sprintf(statpath,"/proc/%d",proc_struct[i].PID);

		if(stat(statpath, &statbuf)==-1){
			fprintf(stderr,"%d stas error\n",proc_struct[i].PID);
			exit(1);
		}
		if((pwd=getpwuid(statbuf.st_uid))==NULL){
			fprintf(stderr,"%d getpwuid error\n",proc_struct[i].PID);
			exit(1);
		}
	}
	for(int i=0; i<proc_count; i++){
		if(strcmp(proc_struct[i].USER, pwd->pw_name)==0){
			if(strcmp(proc_struct[i].TTY, "?")!=0){
				snprintf(print[p++],w_col,"%-10s%5d%5.1lf%5.1lf%8lld%7lld %-9s%-5s%7s%7s %-8s", 
					proc_struct[i].USER, 
					proc_struct[i].PID, 
					proc_struct[i].CPU, 
					proc_struct[i].MEM,
					proc_struct[i].VSZ,
					proc_struct[i].RSS,
					proc_struct[i].TTY,
					proc_struct[i].STAT,
					proc_struct[i].START,
					proc_struct[i].TIME,
					proc_struct[i].COMMAND);
			}
		}
	}
}
void print_option_x(){
	snprintf(print[0],w_col,"%5s %-9s%-5s%7s %-8s","PID", "TTY","STAT", "TIME", "COMMAND");
	for(int i=0; i<proc_count; i++){
		memset(statpath, '\0', 1024);
		sprintf(statpath,"/proc/%d",proc_struct[i].PID);

		if(stat(statpath, &statbuf)==-1){
			fprintf(stderr,"%d stas error\n",proc_struct[i].PID);
			exit(1);
		}
		if((pwd=getpwuid(statbuf.st_uid))==NULL){
			fprintf(stderr,"%d getpwuid error\n",proc_struct[i].PID);
			exit(1);
		}
	}

	for(int i=0; i<proc_count; i++){
		if(strcmp(proc_struct[i].USER, pwd->pw_name)==0){
			snprintf(print[p++],w_col,"%5d %-9s%-5s%7s %-8s",  
				proc_struct[i].PID, 
				proc_struct[i].TTY,
				proc_struct[i].STAT,
				proc_struct[i].TIME,
				proc_struct[i].COMMAND);
		}
	}
}
void print_option_ux(){
	snprintf(print[0],w_col,"%-10s%5s%5s%5s%8s%7s %-9s%-5s%7s%7s %-8s", "USER", "PID", "%CPU", "%MEM" ,"VSZ", "RSS","TTY","STAT", "START", "TIME", "COMMAND");	
	for(int i=0; i<proc_count; i++){
		memset(statpath, '\0', 1024);
		sprintf(statpath,"/proc/%d",proc_struct[i].PID);

		if(stat(statpath, &statbuf)==-1){
			fprintf(stderr,"%d stas error\n",proc_struct[i].PID);
			exit(1);
		}
		if((pwd=getpwuid(statbuf.st_uid))==NULL){
			fprintf(stderr,"%d getpwuid error\n",proc_struct[i].PID);
			exit(1);
		}
	}

	for(int i=0; i<proc_count; i++){
		if(strcmp(proc_struct[i].USER, pwd->pw_name)==0){
			snprintf(print[p++],w_col,"%-10s%5d%5.1lf%5.1lf%8lld%7lld %-9s%-5s%7s%7s %-8s", 
				proc_struct[i].USER, 
				proc_struct[i].PID, 
				proc_struct[i].CPU, 
				proc_struct[i].MEM,
				proc_struct[i].VSZ,
				proc_struct[i].RSS,
				proc_struct[i].TTY,
				proc_struct[i].STAT,
				proc_struct[i].START,
				proc_struct[i].TIME,
				proc_struct[i].COMMAND);
		}
	}
}
void print_option_ax(){
	snprintf(print[0],w_col,"%5s %-9s%-5s%7s %-8s","PID", "TTY","STAT", "TIME", "COMMAND");
	for(int i=0; i<proc_count; i++){
			snprintf(print[p++],w_col,"%5d %-9s%-5s%7s %-8s",  
				proc_struct[i].PID, 
				proc_struct[i].TTY,
				proc_struct[i].STAT,
				proc_struct[i].TIME,
				proc_struct[i].COMMAND);
	}
}
void print_option_au(){
	snprintf(print[0],w_col,"%-10s%5s%5s%5s%8s%7s %-9s%-5s%7s%7s %-8s", "USER", "PID", "%CPU", "%MEM" ,"VSZ", "RSS","TTY","STAT", "START", "TIME", "COMMAND");	
	for(int i=0; i<proc_count; i++){
		if(strcmp(proc_struct[i].TTY, "?")!=0){
			snprintf(print[p++],w_col,"%-10s%5d%5.1lf%5.1lf%8lld%7lld %-9s%-5s%7s%7s %-8s", 
			proc_struct[i].USER, 
			proc_struct[i].PID, 
			proc_struct[i].CPU, 
			proc_struct[i].MEM,
			proc_struct[i].VSZ,
			proc_struct[i].RSS,
			proc_struct[i].TTY,
			proc_struct[i].STAT,
			proc_struct[i].START,
			proc_struct[i].TIME,
			proc_struct[i].COMMAND);
		}
	}
}
void print_option_aux(){
	snprintf(print[0],w_col,"%-10s%5s%5s%5s%8s%7s %-9s%-5s%7s%7s %-8s", "USER", "PID", "%CPU", "%MEM" ,"VSZ", "RSS","TTY","STAT", "START", "TIME", "COMMAND");
	for(int i=0; i<proc_count; i++){
		snprintf(print[p++],w_col,"%-10s%5d%5.1lf%5.1lf%8lld%7lld %-9s%-5s%7s%7s %-8s", 
			proc_struct[i].USER, 
			proc_struct[i].PID, 
			proc_struct[i].CPU, 
			proc_struct[i].MEM,
			proc_struct[i].VSZ,
			proc_struct[i].RSS,
			proc_struct[i].TTY,
			proc_struct[i].STAT,
			proc_struct[i].START,
			proc_struct[i].TIME,
			proc_struct[i].COMMAND);
	}
}
void print_none_option(){
	memset(print[0], 0, sizeof(print[0]));
	snprintf(print[0],w_col,"%5s %-9s%9s %-8s","PID", "TTY","TIME", "COMMAND");
	char *ret, tty[1024];
	memset(tty, '\0', 1024);
	if((ret = ttyname(STDERR_FILENO))==NULL){
		fprintf(stderr, "ttyname() error\n");
		exit(1);
	}
	strcpy(tty, ret+5);
	for(int i=0; i<proc_count; i++){
		if(strcmp(tty, proc_struct[i].TTY)==0){
			char timebuf[1024];
			memset(timebuf, 0, 1024);
			strcpy(timebuf, proc_struct[i].TIME);
			if(atoi(timebuf)>=60){
				int h = atoi(timebuf)/60;
				int m = atoi(timebuf)-h*60;
				for(int k=0; k<strlen(timebuf); k++){
					if(timebuf[k]==':'){
						sprintf(proc_struct[i].TIME, "%02d:%02d%s", h, m, &timebuf[k]);
						break;
					}
				}
			}
			else{
				if(atoi(timebuf)<10){
					sprintf(proc_struct[i].TIME, "00:0%s", timebuf);
				}
				else{
					sprintf(proc_struct[i].TIME, "00:%s", timebuf);
				}
			}
			sprintf(print[p++],"%5d %-9s%9s %-8s",  
				proc_struct[i].PID, 
				proc_struct[i].TTY,
				proc_struct[i].TIME,
				proc_struct[i].COMMAND);
		}
	}
}


int main(int argc, char *argv[]){
	fflush(stdout);
	for(int i=0; i<1024; i++){
		memset(print[i],'\0',1024);
	}
	setSize();
	loadData();
	if(argc<1 || argc>2){
		fprintf(stderr,"usage : pps [option]\n");
		exit(1);
	}
	else{
		if(argc==2){
			if(strcmp(argv[1],"a")==0){
				print_option_a();
			}
			else if(strcmp(argv[1],"u")==0){
				print_option_u();
			}
			else if(strcmp(argv[1],"x")==0){
				print_option_x();
			}
			else if(strcmp(argv[1],"au")==0 || strcmp(argv[1],"ua")==0){	
				print_option_au();
			}
			else if(strcmp(argv[1],"ax")==0 || strcmp(argv[1],"xa")==0){	
				print_option_ax();
			}
			else if(strcmp(argv[1],"xu")==0 || strcmp(argv[1],"ux")==0){
				print_option_ux();
			}
			else if(strcmp(argv[1],"aux")==0 || strcmp(argv[1],"axu")==0 || strcmp(argv[1],"uax")==0 || strcmp(argv[1],"uxa")==0 || strcmp(argv[1],"xau")==0 || strcmp(argv[1],"xua")==0){
				print_option_aux();
			}
			else{
				fprintf(stderr,"please check option\n");
				exit(1);
			}

		}else{
			print_none_option();
		}
	}

	for(int i=0; i<p; i++){	
		printf("%s\n", print[i]);	
	}

	return 0;
}
