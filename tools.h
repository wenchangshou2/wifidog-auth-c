#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
static int is_dir_exist(const char *dir_path)
{
  if(dir_path==NULL){
    return -1;
  }
  if(opendir(dir_path)==NULL)
    return -1;
  return 0;
}
static int is_file_exist(const char *file_path){
  if(file_path==NULL){
    return -1;
  }
  if (access(file_path, F_OK) == 0)
  {
    return 0;
  }
  return -1;
}
int a2i(const char *s)
{
 int sign=1;
 if(*s == '-')
        sign = -1;
 s++;
 int num=0;
 while(*s)
  {
    num=((*s)-'0')+num*10;
    s++;
  }
 return num*sign;
}
