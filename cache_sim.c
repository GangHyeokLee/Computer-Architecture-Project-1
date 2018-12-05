#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//some definitions
#define FALSE 0
#define TRUE 1
#define ADDR long long
#define BOOL char

typedef struct _MEMACCESS{
    ADDR addr;
    BOOL is_read;
} MEMACCESS;

typedef enum _RPL{LRU=0, RAND=1} RPL;


int CACHE_size;
int ASSOCIATIVITY;
int BLOCK_size;
RPL REPLACEMENT_policy;

//misc. function
FILE* fp = 0;
char trace_file[100]="memtrace.trc";
BOOL read_new_memaccess(MEMACCESS*);  //read new memory access from the memory trace file (already implemented)


//configure the cache
void init_cache(int cache_size, int block_size, int assoc, RPL repl_policy);

//check if the memory access hits on the cache
BOOL isHit(ADDR addr);

//insert a cache block for a memory access
ADDR insert_to_cache(ADDR addr);

//print the simulation statistics
print_stat();


//main
int main(int argc, char*argv[])  
{
    int i=0;
    int cache_size=32768;
    int assoc=8;
    int block_size=32;
    RPL repl_policy=LRU;

	/*
    *  Read through command-line arguments for options.
    */
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 's') 
                cache_size=atoi(argv[i+1]);
            
            if (argv[i][1] == 'b')
                block_size=atoi(argv[i+1]);
            
            if (argv[i][1] == 'a')
                assoc=atoi(argv[i+1]);
            
            if (argv[i][1] == 'f')
                strcpy(trace_file,argv[i+1]);


            if (argv[i][1] == 'r')
            {
                if(strcmp(argv[i+1],"lru")==0)
                    repl_policy=LRU;
                else if(strcmp(argv[i+1],"rand")==0)
                    repl_policy=RAND;
                else
                {
                    printf("unsupported replacement policy:%s\n",argv[i+1]);
                    return -1;
                }           
            }
        }
    }
    
    /*
     * main body of cache simulator
    */
    
    init_cache(cache_size, block_size, assoc, repl_policy);//my
    //configure the cache with the cache parameters specified in the input arguments
    
    while(1)
	{
        MEMACCESS new_access;
        
        BOOL success=read_new_memaccess(&new_access);  //read new memory access from the memory trace file
        
        if(success!=TRUE)   //check the end of the trace file
            break;

    /*
        if(isHit(new_access.addr)==FALSE)   //check if the new memory access hit on the cache
        {
            insert_to_cache(new_access.addr);  //if miss, insert a cache block for the memory access to the cache
        }
    */

	}
    
    // print statistics here

	return 0;
}


/*
 * read a new memory access from the memory trace file
 */
BOOL read_new_memaccess(MEMACCESS* mem_access)
{
    ADDR access_addr;
    char access_type[10];
    /*
     * open the mem trace file
     */

    if(fp==NULL)
    {
        fp=fopen(trace_file,"r");
        if(fp==NULL)
        {
            fprintf(stderr,"error opening file");
            exit(2);

        }   
    }

    if(mem_access==NULL)
    {
        fprintf(stderr,"MEMACCESS pointer is null!");
        exit(2);
    }

    if(fscanf(fp,"%llx %s", &access_addr, &access_type)!=EOF)
    {
        mem_access->addr=access_addr;
        if(strcmp(access_type,"RD")==0)
            mem_access->is_read=TRUE;
        else
            mem_access->is_read=FALSE;
        
        return TRUE;
    }       
    else
        return FALSE;

}

void init_cache(int cache_size, int block_size, int assoc, RPL repl_policy)
{
    CACHE_size = cache_size;
    BLOCK_size = block_size;
    ASSOCIATIVITY = assoc;
    REPL_policy = repl_policy;
    //initiallize cache
}
