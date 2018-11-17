#include <stdio.h>
#include <stdlib.h>

//some definitions
#define FALSE 0
#define TRUE 1
//clock cycles
long long cycles;

// registers
int regs[32];
int rs, rt;

int rd, shamt, funct; //for R-format
int add; //for I-format
int jump_address; //for J-format

// program counter
int pc;

// memory
#define INST_MEM_SIZE 32*1024
#define DATA_MEM_SIZE 32*1024
int inst_mem[INST_MEM_SIZE]; //instruction memory
int data_mem[DATA_MEM_SIZE]; //data memory

//instruction 32bit version
char instruction[32];


//Control Unit
typedef struct Execution {
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

typedef struct WriteBack {
	char RegWrite;
	char MemtoReg;
} WriteBack;

typedef struct Control_Unit {
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
void make_decimal(int* decimal, int start, int size); //decode instruction to decimal number

//main
int main()
{
	char done = FALSE;

	init();
	while (!done)
	{
		fetch();     //fetch an instruction from a instruction memory
		decode();    //decode the instruction and read data from register file
		/*exe();       //perform the appropriate operation
		mem();       //access the data memory
		wb();        //write result of arithmetic operation or data read from the data memory if required
		*/
		cycles++;    //increase clock cycle

		rs = rt = rd = shamt = funct = add = jump_address = 0; //reset every registers

		// check the exit condition 
		if (regs[9] == 10)  //if value in $t1 is 10, finish the simulation
			done = TRUE;

		//if debug mode, print clock cycle, pc, reg 
	}

	/*print_cycles();  //print clock cycles
	print_pc();       //print pc
	print_reg();    //print registers
	*/
	return 0;
}


//initialize all datapat elements
//fill the instruction and data memory
//reset the registers

void init()
{
	FILE* fp = fopen("runme.hex", "r");
	int i;
	long inst;

	if (fp == NULL)
	{
		fprintf(stderr, "Error opening file.\n");
		exit(2);
	}

	/* fill instruction memory */
	i = 0;
	while (fscanf(fp, "%x", &inst) == 1)
	{
		inst_mem[i++] = inst;
	}


	/*reset the registers*/
	for (i = 0; i < 32; i++)
	{
		regs[i] = 0;
	}

	/*reset pc*/
	pc = 0;
}


//functions
void fetch() //read address and make it bit
{
	int command = inst_mem[pc]; //read address and save command

	for (int i = 31; i >= 0; i--) //instruction[31] = LSB, instruction[0] = MSB
	{
		instruction[i] = command % 2;
		command = command >> 1;
	}

	pc++; //pc+4

	return;
}

void decode()
{
	int opcode = 0, pc_for_concat;

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
	case 2: //Jump, j
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
	case 3: //Jal, j
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
	case 4: //beq, i
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
	case 8: //add immediate, i
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
	case 10: //set less than immediate, i
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
	case 35: //load word, i
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
	case 43: //store word, i
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

	/*calculating address*/
	if (control.jump == 1) //for J-format
	{
		make_decimal(&jump_address, 6, 26); //decode jump address
		pc_for_concat = (pc & 4026531840); //copy MSB 4bit
		jump_address += pc_for_concat; //concatenate
	}
	else
	{
		make_decimal(&rs, 6, 5); //decode rs
		make_decimal(&rt, 11, 5); //decode rt

		if (control.EX.Regdst == 1) //for R-format
		{
			make_decimal(&rd, 16, 5); //decode rd
			make_decimal(&shamt, 21, 5); //decode shift amount
			make_decimal(&funct, 26, 6); //decode function
		}
		else
		{
			if (instruction[16]) //negative number, sign-extend
			{
				for (int i = 16; i <= 31; i++) //make positive number - 1
				{
					instruction[i] = !instruction[i];
				}
				make_decimal(&add, 16, 16); //decode address
				add++; //recover lost 1
				add *= -1; //make negative number
			}
			else //positive number
			{
				make_decimal(&add, 16, 16); //decode address
			}
			
		}
	}

	return;
}

void make_decimal(int *decimal, int start, int size) //decode instruction to decimal number
{
	int i;

	for (i = start; i < start + size; i++)
	{
		if (instruction[i])
		{
			(*decimal)++;
		}
		(*decimal) = (*decimal) << 1;
	}

	(*decimal)  = (*decimal) >> 1;

	return;
}

void add(int t0, int s1, int s2){
    t0 = s1 + s2;
}

void addi(int t0, int s1, int num){
    t0 = s1 + num;
}

void jal(){
    
}

void j(){
    if( control.jump == 1)
        pc = jump_address;
    else
        return;
}

void jr(){
    if( control.jump == 1)
        regs[31] = pc + 4;
    else
        return;
}

void lw(){
    rt = rs + address;
}

void sw(){
    
}

void slti(int t0, int s1, int num){
    if(s1 < num)
        t0 = 1;
}

void beq(int rs, int rt){
    if(rs == rt)
        pc += address;
}
