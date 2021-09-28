#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>


char print[1024][1024];
struct winsize size;
int w_col;

char CPU_mode[1024];
int CPU_core;
char online_CPU_list[1024];
int threads_per_core;
int cores_per_socket;
char vendor_ID[1024];
int model;
char model_name[1024];
double CPU_speed;

char L1d_cache[100];
char L1i_cache[100];
char L2_cache[100];
char L3_cache[100];

char NUMA_node0_CPU[1024];

char vul_itlb_multihit[1024];
char vul_l1tf[1024];
char vul_mds[1024];
char vul_meltdown[1024];
char vul_spec_store_bypass[1024];
char vul_spectre_v1[1024];
char vul_spectre_v2[1024];
char vul_srbds[1024];
char vul_tsx_async_abort[1024];
char flag[1024];

char path[20] = "/proc/cpuinfo";

void setSize(){
	if(ioctl(0,TIOCGWINSZ, (char*)&size)<0){
		fprintf(stderr,"ioctl error\n");
		exit(1);
	}
	w_col = size.ws_col;
}

void get_Address_size(){
	FILE *fp;
    memset(CPU_mode,'\0',1024);
	if((fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,fp)){
		if(strncmp(line,"address sizes",13)==0){
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
			if(line[i] == ':'){
				fclose(fp);
				strcpy(CPU_mode,&line[i+2]);
				CPU_mode[strlen(CPU_mode)-1] = '\0';
			}
		}
	}
	else{
		fclose(fp);
	}

}

void get_CPU_core(){
	FILE *fp;
	if((fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,fp)){
		if(strncmp(line,"cpu cores",9)==0){
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
			if(line[i] == ':'){
				fclose(fp);
				CPU_core = atoi(&line[i+2]);
			}
		}
	}
	else{
		fclose(fp);
	}

}

void get_vendor_ID(){
	FILE *fp;
    memset(vendor_ID,'\0',1024);
	if((fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,fp)){
		if(strncmp(line,"vendor_id",9)==0){
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
			if(line[i] == ':'){
				fclose(fp);
				strcpy(vendor_ID,&line[i+2]);
				vendor_ID[strlen(vendor_ID)-1] = '\0';
			}
		}
	}
	else{
		fclose(fp);
	}

}

void get_model(){
	FILE *fp;
	if((fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,fp)){
		if(strncmp(line,"model",5)==0){
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
			if(line[i] == ':'){
				fclose(fp);
				model = atoi(&line[i+2]);
			}
		}
	}
	else{
		fclose(fp);
	}

}

void get_model_name(){
	FILE *fp;
    memset(model_name,'\0',1024);
	if((fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,fp)){
		if(strncmp(line,"model name",10)==0){
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
			if(line[i] == ':'){
				fclose(fp);
				strcpy(model_name,&line[i+2]);
				model_name[strlen(model_name)-1] = '\0';
			}
		}
	}
	else{
		fclose(fp);
	}

}

void get_CPU_speed(){
	FILE *fp;
	if((fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,fp)){
		if(strncmp(line,"cpu MHz",7)==0){
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
			if(line[i] == ':'){
				fclose(fp);
				CPU_speed = atof(&line[i+2]);
			}
		}
	}
	else{
		fclose(fp);
	}

}

void get_flag(){
	FILE *fp;
    memset(flag,'\0',1024);
	if((fp=fopen(path,"r"))==NULL){
		fprintf(stderr,"open error for %s\n",path);
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);
	while(NULL != fgets(line,1024,fp)){
		if(strncmp(line,"flags",5)==0){
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
			if(line[i] == ':'){
				fclose(fp);
				strcpy(flag,&line[i+2]);
			}
		}
	}
	else{
		fclose(fp);
	}

}

void get_vul_itlb_multihit(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/itlb_multihit","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/itlb_multihit \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_itlb_multihit,line);
	vul_itlb_multihit[strlen(vul_itlb_multihit)-1] = '\0';
	fclose(fp);
}

void get_vul_L1tf(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/l1tf","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/l1tf \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_l1tf,line);
	vul_l1tf[strlen(vul_l1tf)-1] = '\0';

	fclose(fp);
}

void get_vul_mds(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/mds","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/mds \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_mds,line);
	vul_mds[strlen(vul_mds)-1] = '\0';

	fclose(fp);
}

void get_vul_meltdown(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/meltdown","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/meltdown \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_meltdown,line);
	vul_meltdown[strlen(vul_meltdown)-1] = '\0';

	fclose(fp);
}

void get_vul_spec_store_bypass(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/spec_store_bypass","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/spec_store_bypass \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_spec_store_bypass,line);
	vul_spec_store_bypass[strlen(vul_spec_store_bypass)-1] = '\0';

	fclose(fp);
}

void get_vul_spectre_v1(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/spectre_v1","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/spectre_v1 \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_spectre_v1,line);
	vul_spectre_v1[strlen(vul_spectre_v1)-1] = '\0';

	fclose(fp);
}

void get_vul_spectre_v2(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/spectre_v2","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/spectre_v2 \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_spectre_v2,line);
	vul_spectre_v2[strlen(vul_spectre_v2)-1] = '\0';

	fclose(fp);
}

void get_vul_srbds(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/srbds","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/srbds \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_srbds,line);
	vul_srbds[strlen(vul_srbds)-1] = '\0';

	fclose(fp);
}

void get_vul_tsx_async_abort(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/vulnerabilities/tsx_async_abort","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/vulnerabilities/tsx_async_abort \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(vul_tsx_async_abort,line);
	vul_tsx_async_abort[strlen(vul_tsx_async_abort)-1] = '\0';

	fclose(fp);
}

void get_L1d_cache(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/cpu0/cache/index0/size","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/cpu*/cache/index0/size\n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(L1d_cache,line);	
	L1d_cache[strlen(L1d_cache)-2]='\0';
	strcat(L1d_cache," KiB");

	fclose(fp);
}

void get_L1i_cache(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/cpu0/cache/index1/size","r"))==NULL){
		fprintf(stderr,"open error for/sys/devices/system/cpu/cpu*/cache/index1/size\n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(L1i_cache,line);
	L1i_cache[strlen(L1i_cache)-2]='\0';
	strcat(L1i_cache," KiB");

	fclose(fp);
}

void get_L2_cache(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/cpu0/cache/index2/size","r"))==NULL){
		fprintf(stderr,"open error for/sys/devices/system/cpu/cpu*/cache/index2/size\n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(L2_cache,line);
	L2_cache[strlen(L2_cache)-2]='\0';
	strcat(L2_cache," KiB");

	fclose(fp);
}

void get_L3_cache(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/cpu0/cache/index3/size","r"))==NULL){
		fprintf(stderr,"open error for/sys/devices/system/cpu/cpu*/cache/index3/size\n");
		exit(1);
	}

	char line[1024];
	int check=0;
	int tmp;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(L3_cache,line);
	memset(line,'\0',1024);

	tmp = atoi(L3_cache)/1024;
	sprintf(line,"%d MiB",tmp);
	memset(L3_cache,'\0',100);
	strcpy(L3_cache,line);

	fclose(fp);
}

void get_NUMA_node0_CPU(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/cpu0/node0/cpulist","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/cpu0/node0/cpulist \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(NUMA_node0_CPU,line);
	NUMA_node0_CPU[strlen(NUMA_node0_CPU)-1] = '\0';

	fclose(fp);
}

void get_online_CPU_list(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/online","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/online \n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	strcpy(online_CPU_list,line);
	online_CPU_list[strlen(online_CPU_list)-1] = '\0';

	fclose(fp);
}

void get_threads_per_core(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/cpu0/topology/thread_siblings","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/cpu0/topology/thread_siblings\n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	threads_per_core = atoi(line);

	fclose(fp);
}

void get_cores_per_socket(){
	FILE *fp;
	if((fp=fopen("/sys/devices/system/cpu/cpu0/topology/core_siblings","r"))==NULL){
		fprintf(stderr,"open error for /sys/devices/system/cpu/cpu0/topology/core_siblings\n");
		exit(1);
	}

	char line[1024];
	int check=0;
	memset(line,'\0',1024);

	fgets(line,1024,fp);
	cores_per_socket = atoi(line);

	fclose(fp);
}



int main(){
    setSize();
    get_Address_size();
    get_CPU_core();
	get_online_CPU_list();
	get_threads_per_core();
	get_cores_per_socket();
    get_vendor_ID();
    get_model();
    get_model_name();
    get_CPU_speed();
	get_L1d_cache();
	get_L1i_cache();
	get_L2_cache();
	get_L3_cache();
	get_NUMA_node0_CPU();
	get_vul_itlb_multihit();
	get_vul_L1tf();
	get_vul_mds();
	get_vul_meltdown();
	get_vul_spec_store_bypass();
	get_vul_spectre_v1();
	get_vul_spectre_v2();
	get_vul_srbds();
	get_vul_tsx_async_abort();
    get_flag();

	char flag_string[7] = "Flags:";
    snprintf(print[0],w_col,"%-32s %-8s","Address sizes:",CPU_mode);
    snprintf(print[1],w_col,"%-32s %-8d","CPU(s):",CPU_core);
	snprintf(print[2],w_col,"%-32s %-8s","on-line CPU(s) list:",online_CPU_list);
    snprintf(print[3],w_col,"%-32s %-8d","Thread(s) per core:",threads_per_core);
  	snprintf(print[4],w_col,"%-32s %-8d","Core(s) per socket:",cores_per_socket);	
	snprintf(print[5],w_col,"%-32s %-8s","Vendor ID:",vendor_ID);
    snprintf(print[6],w_col,"%-32s %-8d","Model",model);
    snprintf(print[7],w_col,"%-32s %-8s","Model name:",model_name);
    snprintf(print[8],w_col,"%-32s %-8.3lf","CPU MHz:",CPU_speed);
	snprintf(print[9],w_col,"%-32s %-8s","L1d cache:",L1d_cache);
    snprintf(print[10],w_col,"%-32s %-8s","L1i cache:",L1i_cache);
	snprintf(print[11],w_col,"%-32s %-8s","L2 cache:",L2_cache);
	snprintf(print[12],w_col,"%-32s %-8s","L3 cache:",L3_cache);
	snprintf(print[13],w_col,"%-32s %-8s","NUMA node0 CPU(s):",NUMA_node0_CPU);
	snprintf(print[14],w_col,"%-32s %-8s","Vulnerability Itlb multihit:",vul_itlb_multihit);
	snprintf(print[15],w_col,"%-32s %-8s","Vulnerability L1tf:",vul_l1tf);
	snprintf(print[16],w_col,"%-32s %-8s","Vulnerability Mds:",vul_mds);
	snprintf(print[17],w_col,"%-32s %-8s","Vulnerability Meltdown:",vul_meltdown);
	snprintf(print[18],w_col,"%-32s %-8s","Vulnerability Spec store bypass:",vul_spec_store_bypass);
	snprintf(print[19],w_col,"%-32s %-8s","Vulnerability spectre v1:",vul_spectre_v1);
	snprintf(print[20],w_col,"%-32s %-8s","Vulnerability spectre v2:",vul_spectre_v2);
	snprintf(print[21],w_col,"%-32s %-8s","Vulnerability Srbds:",vul_srbds);
	snprintf(print[22],w_col,"%-32s %-8s","Vulnerability Tsx async abort:",vul_tsx_async_abort);
	//snprintf(print[23],w_col,"%-32s %-8s","Flags:",flag);

    for(int i=0; i<23; i++){
        printf("%s\n",print[i]);
    }

	int idx=0, row=0, check=0;
	char print_flag[1024][1024];
	//memset(print_flag,'\0',sizeof(print_flag));
	
	for(int i=0; i<1024; i++){
		if(check){
			row = i;
			break;
		}
		for(int j=0; j<w_col; j++){
			if(flag[idx] == '\0'){
				check=1;
				break;
			}
			if(j<33){
				print_flag[i][j]=' ';
			}
			else{
				print_flag[i][j]=flag[idx++];
				if(j==w_col-1){
					print_flag[i][j+1] = '\0';
				}
			}
		}
	}
	for(int i=0; i<6; i++){
		print_flag[0][i] = flag_string[i];
	}
	//printf("row:%d\n",row);
	for(int i=0; i<row; i++){
		printf("%s\n",print_flag[i]);
	}
//	printf("\nflags                           :%s\n",flag);
}
