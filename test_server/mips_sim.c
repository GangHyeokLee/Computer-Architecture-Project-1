#include <stdio.h>
#include <stdlib.h>

//some definitions
#define FALSE 0
#define TRUE 1
//clock cycles
long long cycles;

// registers
int regs[32];

// program counter
int pc;

// memory
#define INST_MEM_SIZE 32*1024
#define DATA_MEM_SIZE 32*1024
int inst_mem[INST_MEM_SIZE]; //instruction memory
int data_mem[DATA_MEM_SIZE]; //data memory

//misc. function
void init();
void fetch(); //regs[31] = LSB, regs[0] = MSB

//main
int main()
{
	char done=FALSE;

	char control_unit[9]; //regdst, ALUsrc, Memto-Reg, Reg-Write, Mem-Read, Branch, ALUOp1, ALUOp0

	init();
	while(!done)
	{
		fetch();     //fetch an instruction from a instruction memory
		decode();    //decode the instruction and read data from register file
		/*exe();       //perform the appropriate operation 
		mem();       //access the data memory
		wb();        //write result of arithmetic operation or data read from the data memory if required
		*/
		cycles++;    //increase clock cycle
		
		// check the exit condition 
		if(regs[9]==10)  //if value in $t1 is 10, finish the simulation
			done=TRUE;

		//if debug mode, print clock cycle, pc, reg 
	}

	/*print_cycles();  //print clock cycles
	print_pc();		 //print pc
	print_reg();	 //print registers
	*/
	return 0;
}


//initialize all datapat elements
//fill the instruction and data memory
//reset the registers

void init()
{
	FILE* fp = fopen("runme.hex","r");
	int i;
	long inst;

	if(fp == NULL)
	{
		fprintf(stderr,"Error opening file.\n");
		exit(2);
	}

	/* fill instruction memory */
	i=0;
	while(fscanf(fp, "%x", &inst)==1)
	{
		inst_mem[i++]=inst;
	}
	
	
	/*reset the registers*/
	for(i=0;i<32;i++)
	{
		regs[i]=0;
	}

	/*reset pc*/
	pc=0;
}




//functions
void fetch() //read address and make it bit
{
	int command = inst_mem[pc]; //read address and save command

	for (int i = 31; i >= 0; i--) //regs[31] = LSB, regs[0] = MSB
	{
		regs[i] = command % 2;
		command = command >> 1;
	}

	pc++; //pc+4

	return;
}

void decode()
{
	int * opcode = NULL;
	opcode = (int*)calloc(7, sizeof(int));

	for (int i = 0; i < 7; i++) //copy opcode
	{
		opcode[i] = regs[i];
	}


}