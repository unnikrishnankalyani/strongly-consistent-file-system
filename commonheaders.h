#define MAX_PATH_LENGTH 1000
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
//#include <unistd.h>

#define HTABLESIZE 100

long get_time(){   
    struct timespec* ts;
    clock_gettime(CLOCK_MONOTONIC, ts);
    return ts->tv_nsec;
}

void getServerPath(const char* clientpath, const char*  root_path, char* server_path)
{
    server_path[0] = '\0';
    strncat(server_path, root_path, MAX_PATH_LENGTH);
    strncat(server_path, clientpath, MAX_PATH_LENGTH);
}

void getLocalPath(const char* file_path, const char*  cache_path, char* client_path)
{
    // char cacheFileName[80];
    // snprintf(cacheFileName, 80, "%lu", hash((const char* )file_path));
    client_path[0] = '\0';
    strncat(client_path, cache_path, MAX_PATH_LENGTH);
    // strncat(client_path, cacheFileName, MAX_PATH_LENGTH);
    strncat(client_path, file_path, MAX_PATH_LENGTH);

}
void getLocalTmpPath(const char* file_path, const char*  cache_path, char* client_path)
{
    // char cacheFileName[80];
    // snprintf(cacheFileName, 80, "%lu", hash((const char* )file_path));
    client_path[0] = '\0';
    strncat(client_path, cache_path, MAX_PATH_LENGTH);
    // strncat(client_path, cacheFileName, MAX_PATH_LENGTH);
    strncat(client_path, file_path, MAX_PATH_LENGTH);
    strncat(client_path, "_tmp", MAX_PATH_LENGTH);

}
void getServerTmpPath(const char* clientpath, const char*  root_path, char* server_path)
{
    server_path[0] = '\0';
    strncat(server_path, root_path, MAX_PATH_LENGTH);
    strncat(server_path, clientpath, MAX_PATH_LENGTH);
    strncat(server_path, "_tmp", MAX_PATH_LENGTH);
}


unsigned long hashfilename(const char* str)
{   
    unsigned long hash = 5381;
    int c;
    
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}

unsigned int hashtableindex(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    //printf("%d: x before\n", x);
    x = x % HTABLESIZE;
    //printf("%d: x after\n", x);
    return x ;
}

typedef struct hash_node{
  long key;
  long value;
  struct hash_node *next;
}hash_node;

hash_node *ht[HTABLESIZE];

void init_ht(){
  int i;
  for (i=0; i<HTABLESIZE; i++){
    ht[i] = 0;
  }
}

hash_node *create_hash_node(long key, long value){
  //printf("----trying to insert %d %s------\n", key, value);
  hash_node *new_hn = (hash_node *) malloc(sizeof(hash_node));
  new_hn->key = key;
  new_hn->value = value;
  new_hn->next = NULL; //set this later if needed
  return new_hn;
}

long get(const char* filename){
 
    long hashfile = hashfilename(filename);
    long hash_index = hashtableindex(hashfile);

  hash_node *temp_hn = ht[hash_index];

  while (temp_hn!=NULL){
    if (hashfile == temp_hn->key){
        return temp_hn->value;
    }
    temp_hn = temp_hn->next;
  }

  return -1; //not found
}

void put(long hashfilename, long timestamp){
  
//   long hashfile = hashfilename(filename);
  long hash_key = hashtableindex(hashfilename);
  
  hash_node *h = ht[hash_key];
  //printf("hash_key = %d\n", hash_key);
  //if no entry, store and return
  if ( h == NULL){
    ht[hash_key] = create_hash_node(hashfilename,timestamp);
    //printf("created new key: %d, %s\n", x, v);
    return;
  } 
  
  //check if key exists already
  hash_node *temp_hn = ht[hash_key];
  hash_node *prev = NULL;
  while (temp_hn!=NULL){
    if (hashfilename == temp_hn->key){
        temp_hn->value = timestamp;
        return;
    }
    prev = temp_hn;
    temp_hn = temp_hn->next;
  }
  //key does not exist, append to end of list (temp_hn is NULL, use prev)
  prev->next = create_hash_node(hashfilename,timestamp);
  //printf("created new key: %d, %s\n", x, v);
  
  return;
}

void read_from_database(char * cache_path){
  FILE *fptr;
  char client_path[MAX_PATH_LENGTH];
    char * path = "/database.txt";
    getLocalPath(path, cache_path, client_path);
  if(fptr = fopen(client_path,"r")){
    char *line;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fptr)) != -1) {
        char *tempstr = strdup(line);
        char *k  = strsep(&tempstr, ",");
        char *v = strsep(&tempstr, ",");

        free(tempstr);
        put(atol(k), atol(v));
    }
    if(line)
      free(line);
    fclose(fptr);
  }
  return;
}

void dump(char* cache_path){
char client_path[MAX_PATH_LENGTH];
char * path = "/database.txt";
getLocalPath(path, cache_path, client_path);

printf("path: %s\n", client_path);
  FILE *fptr; 
  printf("dumping... \n");
  fptr = fopen(client_path,"w"); //do I need a backup?


  int i;    

  for (i=0; i<HTABLESIZE; i++){
    if(ht[i] != NULL){
      hash_node *temp_hn = ht[i];
      while (temp_hn!=NULL){
        printf("%ld,%ld\n", temp_hn->key, temp_hn->value);
        fprintf(fptr,"%ld,%ld\n", temp_hn->key, temp_hn->value);
        temp_hn = temp_hn->next;
      }
    }
  }
  fclose(fptr);
  return;
}

// int main(int argc, char *argv[]){
//   //initialize the hash table
//   init_ht();

//   //read stored k,v
//   read_from_database();

//   int i;

//   //dump into file
//   dump();
//   return 0;
// }