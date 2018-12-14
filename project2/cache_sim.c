#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

//some definitions
#define FALSE 0
#define TRUE 1
#define ADDR long long
#define BOOL char

#define ADDRESSBITS 64
#define WORDBITS 32

typedef struct _MEMACCESS {
    ADDR addr;
    BOOL is_read;
} MEMACCESS;

typedef enum _RPL { LRU = 0, RAND = 1 } RPL;


int ** address_store, *** data_store;

int CACHE_size;
int ASSOCIATIVITY;
int BLOCK_size;
int num_BLOCK = 0;
int hit_count = 0;
int miss_count = 0;
RPL REPLACEMENT_policy;

int index, wordoffset, byteoffset, tag; //index size, word offset size, byte offset size, tag size
int address[ADDRESSBITS];
//misc. function
FILE* fp = 0;
char trace_file[100] = "memtrace.trc";
BOOL read_new_memaccess(MEMACCESS*);  //read new memory access from the memory trace file (already implemented)


//configure the cache
void init_cache(int cache_size, int block_size, int assoc, RPL repl_policy);

//check if the memory access hits on the cache
BOOL isHit(ADDR addr);

//insert a cache block for a memory access
ADDR insert_to_cache(ADDR addr);

//print the simulation statistics
print_stat();

//other functions
void make_decimal(int *decimal, int start, int size, int* address);


//main
int main(int argc, char*argv[])
{
    int i = 0;
    int cache_size = 32768;
    int assoc = 8;
    int block_size = 32;
    RPL repl_policy = LRU;
    
    /*
     *  Read through command-line arguments for options.
     */
    /*
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
     }*/
    
    /*
     * main body of cache simulator
     */
    
    init_cache(cache_size, block_size, assoc, repl_policy);//my
    //configure the cache with the cache parameters specified in the input arguments
    
    while (1)
    {
        MEMACCESS new_access;
        
        BOOL success = read_new_memaccess(&new_access);  //read new memory access from the memory trace file
        
        if (success != TRUE)   //check the end of the trace file
            break;
        
        
        printf("%d\n", isHit(new_access.addr));
        
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
    
    if (fp == NULL)
    {
        fp = fopen(trace_file, "r");
        if (fp == NULL)
        {
            fprintf(stderr, "error opening file");
            exit(2);
            
        }
    }
    
    if (mem_access == NULL)
    {
        fprintf(stderr, "MEMACCESS pointer is null!");
        exit(2);
    }
    
    if (fscanf(fp, "%llx %s", &access_addr, &access_type) != EOF)
    {
        mem_access->addr = access_addr;
        if (strcmp(access_type, "RD") == 0)
            mem_access->is_read = TRUE;
        else
            mem_access->is_read = FALSE;
        
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
    REPLACEMENT_policy = repl_policy;
    num_BLOCK = CACHE_size / BLOCK_size;
    
    byteoffset = 2;
    wordoffset = log(BLOCK_size) / log(2) - 2;
    index = log(CACHE_size / BLOCK_size) / log(2);
    tag = ADDRESSBITS - byteoffset - wordoffset - index;
    
    address_store = (int**)calloc((int)pow(2, index), sizeof(int*)); //address store
    
    data_store = (int***)calloc((int)pow(2, index), sizeof(int**));
    
    for (int i = 0; i < (int)pow(2, index); i++)
    {
        address_store = (int*)calloc(ADDRESSBITS, sizeof(int));
        data_store[i] = (int**)calloc(ASSOCIATIVITY + 1, sizeof(int*));
        
        data_store[i][0] = (int*)calloc(1, sizeof(int)); //valid bit
        for (int j = 1; j <= ASSOCIATIVITY; j++)
        {
            data_store[i][j] = (int*)calloc((2 + tag), sizeof(int)); //dirty tag 데이터 블록은 뺐습니다.
        }
    }
    
    //initiallize cache
}

BOOL isHit(ADDR addr) //index로 찾아가서 valid 확인하고 tag비교
{
    int index_num = 0;
    
    //주소 배열에 저장
    for (int i = ADDRESSBITS - 1; i >= 0; i--) //instruction[63] = LSB, instruction[0] = MSB  tag, index, offset
    {
        address[i] = addr % 2;
        addr = addr >> 1;
    }
    
    make_decimal(&index_num, tag, index, address); //index 얻음
    
    for (int i = 1; i <= ASSOCIATIVITY; i++) //set 돌아다니면서 일치하는 tag 찾기
        if (data_store[index_num][0][1]) //valid data block
            if (strncmp(data_store[index_num][0][i] + 2, address, tag) == 0) //data 블록의 두번째 칸부터 tag크기 만큼과 address의 tag 크기 만큼 비교하기
            {
                ++hit_count;
                return TRUE;
            }
    
    ++miss_count;
    return FALSE; //invalid block 이거나 일치하는 tag 없을 때
}

ADDR insert_to_cache(ADDR addr)
{
    int check = -1, index_num = 0, maxLRU = 0, random = 0, LRUcheck = 0;
    
    make_decimal(&index_num, tag, index, address); //index 얻음
    
    for (int i = 1; i <= ASSOCIATIVITY; i++) //set 돌아다니면서 빈 캐시가 존재할 시 그 곳에 저장
        if (data_store[index_num][i][1] == 0)
        {
            check = i;
            LRUcheck = data_store[index_num][i][0];
            data_store[index_num][i][0] = ASSOCIATIVITY - 1;
            
            if (REPLACEMENT_policy == 0)
            {
                for (int j = 1; j <= ASSOCIATIVITY; j++)
                    if (data_store[index_num][j][0] > LRUcheck)
                        data_store[index_num][j][0]--;
            }
            //LRU 방식일 시 값 갱신
            break;
        }
    
    if (check == -1)
    {
        if (REPLACEMENT_policy == 0)
        {
            for (int j = 1; j <= ASSOCIATIVITY; j++)
            {
                if (maxLRU < data_store[index_num][j][0])
                    maxLRU = j;
            }
            
            for (int i = 0; i < ADDRESSBITS; i++)
                data_store[index_num][maxLRU][i] = address[i];
            
        }
        
        else {
            srand(time(NULL));
            random = rand() % ASSOCIATIVITY + 1;
            for (int i = 0; i < ADDRESSBITS; i++)
                data_store[index_num][random][i] = address[i];
        }
    }
    
}

void make_decimal(int *decimal, int start, int size, int* address)
{
    int i;
    
    for (i = start; i < start + size; i++)
    {
        if (address[i])
        {
            (*decimal)++;
        }
        (*decimal) = (*decimal) << 1;
    }
    
    (*decimal) = (*decimal) >> 1;
    
    return;
}

void print_stat()
{
    printf("\ncache_size: %dB \n",CACHE_size);
    printf("block_size: %dB \n",BLOCK_size);
    printf("associativity: %d \n",ASSOCIATIVITY);
    printf("replacement policy: %s \n",REPLACEMENT_policy);
    printf("cache accesses: %d \n",hit_count+miss_count);
    printf("cache_hits: %d \n",hit_count);
    printf("cache_misses: %d \n",miss_count);
    printf("cache_miss_rate: %3.1f \n",miss_count/(hit_count+miss_count)*100);
}
