#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct parameters{
    char filename[1000];
    int filesize;
    char filetype;
    char fileperm[100];
    int linksize;
};

int traverse_directory_recursive(char * path,struct parameters parameter,char * string);
int regexp_compare(char * str1,char * str2);
void sigint_handler(int signo);

int main(int argc, char ** argv){

    int i=0,h=0;
    int inputfile=0;
    int outputfile=0;
    int parser = 0;
    int parser_number=0;
    int inputcheck[6]={0,0,0,0,0,0};
    char path[1000];
    struct parameters param;
    param.filesize=-1;
    param.filetype=-1;
    param.linksize=-1;
    param.fileperm[0]=-1;
    param.filename[0]=-1;
    char printformat[1000000];

    while((parser = getopt(argc, argv, "w:f:b:t:p:l:")) != -1)   
    {  
        switch(parser)  
        {  
            case 'w':
            {
                if(inputcheck[0]>=1){
                    fprintf(stderr,"Same input twice.\n");
                    return -1;
                }                
                sprintf(path,"%s",optarg);
                inputcheck[0]++;
                break;
            }
            case 'f':
            {
                if(inputcheck[1]>=1){
                    fprintf(stderr,"Same input twice.\n");
                    return -1;
                }

                sprintf(param.filename,"%s",optarg);
                inputcheck[1]++;
                break;
            }
            case 'b':
            {
                if(inputcheck[2]>=1){
                    fprintf(stderr,"Same input twice.\n");
                    return -1;
                }
                param.filesize=0;
                int length=0;
                while(optarg[length]!='\0'){
                    length++;
                }
                i=length-1; 
                int multiplier=1; 
                while(i>=0){
                    if(optarg[i]>='0' && optarg[i]<='9'){
                        param.filesize+=(int)(optarg[i] - '0') * multiplier;
                        multiplier *= 10;
                        i--;
                    }
                    else{
                        fprintf(stderr,"Invalid Value");
                        return -1;
                    }
                }
                inputcheck[2]++;
                break;
            }  
            case 't':
            {
                if(inputcheck[3]>=1){
                    fprintf(stderr,"Same input twice.\n");
                    return -1;
                }

                param.filetype=optarg[0];

                inputcheck[3]++;
                break;
            }
            case 'p':
            {
                if(inputcheck[4]>=1){
                    fprintf(stderr,"Same input twice.\n");
                    return -1;
                }
                
                sprintf(param.fileperm,"%s",optarg);
                inputcheck[4]++;
                break;
            }
            case 'l':
            {
                if(inputcheck[5]>=1){
                    fprintf(stderr,"Same input twice.\n");
                    return -1;
                }
                param.linksize=0;
                int length=0;
                while(optarg[length]!='\0'){
                    length++;
                }
                i=length-1; 
                int multiplier=1; 
                while(i>=0){
                    if(optarg[i]>='0' && optarg[i]<='9'){
                        param.linksize+=(int)(optarg[i] - '0') * multiplier;
                        multiplier *= 10;
                        i--;
                    }
                    else{
                        fprintf(stderr,"Error3: Invalid Value");
                        return -1;
                    }
                }

                inputcheck[5]++;
                break;
            }
            default:
            {
                fprintf(stderr,"The command line arguments are missing/invalid.\n");
                return -1;
            }
            break;
        }
        parser_number++;  
    }
    if(parser_number>6 || parser_number<2){
        fprintf(stderr,"Too many/less arguments.\n");
        return -1;
    }
    if(inputcheck[0]<1){
        fprintf(stderr,"There are no path for search.(-w parameter)\n");
        return -1;
    }

    signal(SIGINT,sigint_handler);

    int returntest=0;
    returntest=traverse_directory_recursive(path,param,&printformat[0]);
    if(returntest==0){
        fprintf(stderr,"No file found.\n");
        return -1;
    }
    //printf("Return: %d\n",returntest);
    //printf("The path is %s\n",printformat);

    //char temp[1000];
    int p=0,m=0,t=0;
    int pathlength=0;
    int index=0;
    pathlength=strlen(path);
    p=pathlength;
    printf("%s",path);
    for(h=0;h<returntest;h++){
        while(printformat[p]!='+'){
            if(printformat[p]=='/'){
                m++;
                printf("\n|");
                for(t=0;t<m;t++){
                    printf("--");
                }
                p++;
            }
            printf("%c",printformat[p]);
            p++;
        }
        printf("\n");
        p+=pathlength+1;
        m=0;
    }

    return 0;
}

int traverse_directory_recursive(char * path,struct parameters parameter,char * string){
    int ret=0;
    int n=0;
    char pathname[1000];
    DIR * directory;
    struct dirent * dir_en;
    int check[5]={1,1,1,1,1};
    strcpy(pathname,path);
    directory = opendir(path);
    if(directory == NULL){
        fprintf(stderr,"Failed to open path.\n");
        return -1;
    }
    while( dir_en = readdir(directory) ){
        strcpy(pathname,path);

        int cmp1=strcmp(dir_en->d_name,".");
        int cmp2=strcmp(dir_en->d_name,"..");

        int dfd= dirfd(directory);
        struct stat file_status;
        if(fstatat(dfd,dir_en->d_name,&file_status,0)<0){
            perror("Error");
        }

        if(cmp1!=0 && cmp2!=0){
            if(parameter.filename[0]>=0){
                check[0]=0;
                int compare=regexp_compare(parameter.filename,dir_en->d_name);

                if(compare==0){
                    check[0]=1;
                }
            }
            if(parameter.fileperm[0]>=0){
                char perm[1000];
                int p[3];
                int input_perm[3]={0,0,0};
                int val=0,c=0;

                check[1]=0;
                sprintf(perm,"%o",file_status.st_mode);
                val=strlen(perm)-3;
                for(c=0;c<3;c++){
                    p[c]=(int)perm[val]-'0';
                    val++;
                }
                if(parameter.fileperm[0]=='r'){
                    input_perm[0]+=4;
                }
                if(parameter.fileperm[1]=='w'){
                    input_perm[0]+=2;
                }
                if(parameter.fileperm[2]=='x'){
                    input_perm[0]+=1;
                }
                if(parameter.fileperm[3]=='r'){
                    input_perm[1]+=4;
                }
                if(parameter.fileperm[4]=='w'){
                    input_perm[1]+=2;
                }
                if(parameter.fileperm[5]=='x'){
                    input_perm[1]+=1;
                }
                if(parameter.fileperm[6]=='r'){
                    input_perm[2]+=4;
                }
                if(parameter.fileperm[7]=='w'){
                    input_perm[2]+=2;
                }
                if(parameter.fileperm[8]=='x'){
                    input_perm[2]+=1;
                }
                if(input_perm[0]==p[0] && input_perm[1]==p[1] && input_perm[2]==p[2]){
                    check[1]=1;
                }

            }
            if(parameter.filesize>=0){
                check[2]=0;
                if((int)file_status.st_size==parameter.filesize){
                    check[2]=1;
                }
            }
            if(parameter.filetype>=0){
                check[3]=0;
                if((file_status.st_mode & S_IFMT)==S_IFBLK && parameter.filetype=='b'){
                    check[3]=1;
                }
                if((file_status.st_mode & S_IFMT)==S_IFCHR && parameter.filetype=='c'){
                    check[3]=1;
                }
                if((file_status.st_mode & S_IFMT)==S_IFDIR && parameter.filetype=='d'){
                    check[3]=1;
                }
                if((file_status.st_mode & S_IFMT)==S_IFLNK && parameter.filetype=='l'){
                    check[3]=1;
                }
                if((file_status.st_mode & S_IFMT)==S_IFIFO && parameter.filetype=='p'){
                    check[3]=1;
                }
                if((file_status.st_mode & S_IFMT)==S_IFSOCK && parameter.filetype=='s'){
                    check[3]=1;
                }
                if((file_status.st_mode & S_IFMT)==S_IFREG && parameter.filetype=='f'){
                    check[3]=1;
                }
            }
            if(parameter.linksize>=0){
                check[4]=0;
                if(file_status.st_nlink==parameter.linksize){
                    check[4]=1;
                }
            }            
            strcat(pathname,"/");
            strcat(pathname,dir_en->d_name);
            if(check[0]==1 && check[1]==1 && check[2]==1 && check[3]==1 && check[4]==1){
                //printf("Filename is: %s \n",dir_en->d_name);
                //printf("pathname:%s\n",pathname);
                ret=1;
                strcat(string,pathname);
                strcat(string,"+");
            }

            if((file_status.st_mode & S_IFMT)==S_IFDIR){
                ret+=traverse_directory_recursive(pathname,parameter,string);
            }      
        }        
    }
    closedir(directory);
    return ret;

}

int regexp_compare(char * str1,char * str2){
    int i=0;
    int j=0;
    int end_of_symbol=0;

    for(int k = 0; str1[k]; k++){
        str1[k] = tolower(str1[k]);
    }
    for(int k = 0; str2[k]; k++){
        str2[k] = tolower(str2[k]);
    }

    while(i<strlen(str1)){
        if(str1[i]=='+'){
            i--;
            end_of_symbol=1;
        }
        else if(str1[i]==str2[j]){
            i++;
            j++;
        }
        else if(end_of_symbol==1){
            i++;
            while(str1[i]=='+'){
                i++;
            }
            end_of_symbol=0;
        }
        else{
            return -1;
        }
    }
    return 0;
}

void sigint_handler(int signo){
    printf("EXITING.\n");
    _exit(-1);
}