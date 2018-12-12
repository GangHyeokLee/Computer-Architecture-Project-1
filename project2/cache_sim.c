#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//some definitions
#define FALSE 0
#define TRUE 1
#define ADDR long long
#define BOOL char

#define ADDRESSBITS 64
#define WORDBITS 32

typedef struct _MEMACCESS{
    ADDR addr;
    BOOL is_read;
} MEMACCESS;

typedef enum _RPL{LRU=0, RAND=1} RPL;


int ** address_store, *** data_store;

int CACHE_size;
int ASSOCIATIVITY;
int BLOCK_size;
int num_BLOCK = 0;
RPL REPLACEMENT_policy;

int index, wordoffset, byteoffset, tag; //index size, word offset size, byte offset size, tag size

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

//other functions
void make_decimal(int *decimal, int start, int size, int* address);


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
    
    while(1)
	{
        MEMACCESS new_access;
        
        BOOL success=read_new_memaccess(&new_access);  //read new memory access from the memory trace file
        
        if(success!=TRUE)   //check the end of the trace file
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
    REPLACEMENT_policy = repl_policy;
    num_BLOCK = CACHE_size / BLOCK_size;
    
	byteoffset = 2;
	wordoffset = log(BLOCK_size) / log(2) - 2;
	index = log(CACHE_size / BLOCK_size) / log(2);
	tag = ADDRESSBITS - byteoffset - wordoffset - index;

	address_store = (int**)calloc((int)pow(2, index), sizeof(int*)); //address store

	data_store = (int***)calloc((int)pow(2, index), sizeof(int**));

	for (int i = 0; i < (int)pow(2,index); i++)
	{
		address_store = (int*)calloc(ADDRESSBITS, sizeof(int));
		data_store[i] = (int**)calloc(ASSOCIATIVITY+1, sizeof(int*));

		data_store[i][0] = (int*)calloc(1, sizeof(int)); //valid bit
		for (int j = 1; j <= ASSOCIATIVITY; j++)
		{
			data_store[i][j] = (int*)calloc((1 + tag), sizeof(int)); //dirty tag ������ ����� �����ϴ�.
		}
	}	

    //initiallize cache
}

BOOL isHit(ADDR addr) //index�� ã�ư��� valid Ȯ���ϰ� tag��
{
	int address[ADDRESSBITS];
	int index_num = 0;
	 
	//�ּ� �迭�� ����
	for (int i = ADDRESSBITS - 1; i >= 0; i--) //instruction[63] = LSB, instruction[0] = MSB  tag, index, offset
	{
		address[i] = addr % 2;
		addr = addr >> 1;
	}

	make_decimal(&index_num, tag, index, address); //index ����

	if (data_store[index_num][0][0]) //valid data block
	{
		for (int i = 1; i <= ASSOCIATIVITY; i++) //set ���ƴٴϸ鼭 ��ġ�ϴ� tag ã��
		{
			if (strncmp(data_store[index_num][i]+1, address, tag) == 0) //data ����� �ι�° ĭ���� tagũ�� ��ŭ�� address�� tag ũ�� ��ŭ ���ϱ�
			{
				return TRUE;
			}
		}
	}
	
	return FALSE; //invalid block �̰ų� ��ġ�ϴ� tag ���� ��
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