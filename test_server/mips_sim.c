#include <stdio.h>
#include <stdlib.h>

//some definitions
#define FALSE 0
#define TRUE 1
//clock cycles
long long cycles;

// registers
int regs[32];
int * return_address;

// program counter
int pc;

// memory
#define INST_MEM_SIZE 32*1024
#define DATA_MEM_SIZE 32*1024
int inst_mem[INST_MEM_SIZE]; //instruction memory
int data_mem[DATA_MEM_SIZE]; //data memory

//Control Unit
typedef struct Execution{
	char Regdst;
	char ALUOp1;
	char ALUOp0;
	char ALUsrc;
} Execution;

typedef struct Memory {
	char Branch;
	char MemRead;
	char MemWrite;
} Memory;

typedef struct WriteBack{
	char RegWrite;
	char MemtoReg;
} WriteBack;

typedef struct Control_Unit{
	Execution EX;
	Memory M;
	WriteBack WB;
	char jump;
	char link;
} Control_Unit;

Control_Unit control;


//misc. function
void init();
void fetch(); //regs[31] = LSB, regs[0] = MSB
void decode();


//sub_function
void make_decimal(int * decimal, int start, int size); //decode instruction to decimal number

//main
int main()
{
	char done=FALSE;

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
	int opcode = 0, jump_address = 0;

	make_decimal(&opcode, 0, 6); //decode opcode

	switch (opcode)
	{
	case 0: //R-format
		control.EX.Regdst = 1;
		control.EX.ALUOp1 = 1;
		control.EX.ALUOp0 = 0;
		control.EX.ALUsrc = 0;
		control.M.Branch = 0;
		control.M.MemRead = 1;
		control.M.MemWrite = 0;
		control.WB.RegWrite = 1;
		control.WB.MemtoReg = 0;
		control.jump = 0;
		control.link = 0;
		break;
	case 2: //Jump
		control.EX.Regdst = 'x';
		control.EX.ALUOp1 = 'x';
		control.EX.ALUOp0 = 'x';
		control.EX.ALUsrc = 'x';
		control.M.Branch = 0;
		control.M.MemRead = 0;
		control.M.MemWrite = 0;
		control.WB.RegWrite = 0;
		control.WB.MemtoReg = 'x';
		control.jump = 1;
		control.link = 0;
		break;
	case 3: //Jal
		control.EX.Regdst = 'x';
		control.EX.ALUOp1 = 'x';
		control.EX.ALUOp0 = 'x';
		control.EX.ALUsrc = 'x';
		control.M.Branch = 0;
		control.M.MemRead = 0;
		control.M.MemWrite = 0;
		control.WB.RegWrite = 1;
		control.WB.MemtoReg = 'x';
		control.jump = 1;
		control.link = 1;
		break;
	case 4: //beq
		control.EX.Regdst = 'x';
		control.EX.ALUOp1 = 0;
		control.EX.ALUOp0 = 1;
		control.EX.ALUsrc = 0;
		control.M.Branch = 1;
		control.M.MemRead = 0;
		control.M.MemWrite = 0;
		control.WB.RegWrite = 0;
		control.WB.MemtoReg = 'x';
		control.jump = 0;
		control.link = 0;
		break;
	case 8: //add immediate
		control.EX.Regdst = 0;
		control.EX.ALUOp1 = 0;
		control.EX.ALUOp0 = 0;
		control.EX.ALUsrc = 1;
		control.M.Branch = 0;
		control.M.MemRead = 0;
		control.M.MemWrite = 0;
		control.WB.RegWrite = 1;
		control.WB.MemtoReg = 0;
		control.jump = 0;
		control.link = 0;
		break;
	case 10: //set less than immediate
		control.EX.Regdst = 0;
		control.EX.ALUOp1 = 0;
		control.EX.ALUOp0 = 0;
		control.EX.ALUsrc = 1;
		control.M.Branch = 0;
		control.M.MemRead = 0;
		control.M.MemWrite = 0;
		control.WB.RegWrite = 1;
		control.WB.MemtoReg = 0;
		control.jump = 0;
		control.link = 0;
		break;
	case 35: //load word
		control.EX.Regdst = 0;
		control.EX.ALUOp1 = 0;
		control.EX.ALUOp0 = 0;
		control.EX.ALUsrc = 1;
		control.M.Branch = 0;
		control.M.MemRead = 1;
		control.M.MemWrite = 0;
		control.WB.RegWrite = 1;
		control.WB.MemtoReg = 1;
		control.jump = 0;
		control.link = 0;
		break;
	case 43: //store word
		control.EX.Regdst = 'x';
		control.EX.ALUOp1 = 0;
		control.EX.ALUOp0 = 0;
		control.EX.ALUsrc = 1;
		control.M.Branch = 0;
		control.M.MemRead = 0;
		control.M.MemWrite = 1;
		control.WB.RegWrite = 0;
		control.WB.MemtoReg = 'x';
		control.jump = 0;
		control.link = 0;
		break;
	}

	if (control.jump && !(control.link)) //jump execution
	{
		make_decimal(&jump_address, 6, 26); //decode jump address
		jump_address << 2;
	}


}

void make_decimal(int * decimal, int start, int size) //decode instruction to decimal number
{
	int i;

	for (i = start; i < size; i++)
	{
		for (int i = 0; i < 6; i++)
		{
			if (regs[i])
			{
				(*decimal)++;
			}
			(*decimal) *= 2;
		}

	}

	return;
}