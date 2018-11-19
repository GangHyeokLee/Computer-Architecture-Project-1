#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//some definitions
#define FALSE 0
#define TRUE 1
//clock cycles
long long cycles;

// registers
int regs[32];

int Read_register1, Read_register2, Write_register; //for decode

int shamt, funct; //for R-format
int address; //for I-format
int jump_address; //for J-format

           //ALU Control
int ALU_control_instrution;
int flag;

//MUX
int read1, read2;

//exe ALU result
int ALU_result;

//data from mem to wb
int data_toWB_frommem = 0;

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
void exe();
void mem();
void wb();


//sub_function
void make_decimal(int* decimal, int start, int size); //decode instruction to decimal number
void alu();
void alu_control();
void MUX_for_ALU();
void print_cycles();
void print_pc();
void print_reg();
void jump_mux();

//main
int main(int count, char *args[])
{
   char done = FALSE;


   if (count != 2)
   {
      printf("Wrong type\n");
      return 0;
   }

   if (strcmp(args[1], "0") == 0)
      printf("Debug Mode\n");
   else if (strcmp(args[1], "1") == 0)
      printf("Run Mode\n");
   else
   {
      printf("Wrong type\n");
      return 0;
   }

   init();
   while (!done)
   {
      flag = 0;//initialze flag
      fetch();     //fetch an instruction from a instruction memory
      decode();    //decode the instruction and read data from register file
      exe();       //perform the appropriate operation
      mem();       //access the data memory
      wb();        //write result of arithmetic operation or data read from the data memory if required

      cycles++;    //increase clock cycle

      if (strcmp(args[1], "0") == 0)
      {
         print_cycles();  //print clock cycles
         print_pc();       //print pc
         print_reg();    //print registers
      }

      Read_register1 = Read_register2 = Write_register = shamt = funct = address = jump_address = data_toWB_frommem = 0;//reset

                                                                       // check the exit condition
      if (regs[9] == 10)  //if value in $t1 is 10, finish the simulation
         done = TRUE;

      //if debug mode, print clock cycle, pc, reg
   }

   if (strcmp(args[1], "1") == 0)
   {
      print_cycles();  //print clock cycles
      print_pc();       //print pc
      print_reg();    //print registers
   }

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
      control.EX.ALUOp1 = 1;
      control.EX.ALUOp0 = 1;
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
      flag = 1;
      make_decimal(&jump_address, 6, 26); //decode jump address
      pc_for_concat = (pc & 4026531840); //copy MSB 4bit
      jump_address += pc_for_concat; //concatenate
   }
   else
   {
      make_decimal(&Read_register1, 6, 5); //decode Read register1

      if (control.EX.Regdst == 1) //RegDst MUX
      {
         make_decimal(&Read_register2, 11, 5); //decode Read register2
      }
      else
      {
         make_decimal(&Write_register, 11, 5); //decode Read Write register
      }


      //decode rest part
      if (control.EX.Regdst == 1) //for R-format
      {
         make_decimal(&Write_register, 16, 5); //decode Write register
         make_decimal(&shamt, 21, 5); //decode shift amount
         make_decimal(&funct, 26, 6); //decode function
      }
      else //for I-format
      {
         if (instruction[16]) //negative number, sign-extend
         {
            for (int i = 16; i <= 31; i++) //make positive number - 1
            {
               instruction[i] = !instruction[i];
            }
            make_decimal(&address, 16, 16); //decode address
            address++; //recover lost 1
            address *= -1; //make negative number
         }
         else //positive number
         {
            make_decimal(&address, 16, 16); //decode address
         }
      }
   }

   return;
}

void exe()
{
   alu_control();
   alu();
}

void mem()
{
   if (flag == 1)//PASS
      return;

   if (control.M.MemWrite == 1)
   {
      data_mem[ALU_result] = regs[Write_register];
   }

   if (control.M.MemRead == 0)
      return;


   data_toWB_frommem = data_mem[ALU_result];

   return;
}

void wb()
{
   if (control.link == 1) //jal에서 link를 걸어줍니다.
   {
      regs[31] = pc;
   }
   
   if (control.jump == 1)//jump 판단하는 MUX입니다.
   {
      pc = jump_address;
   }

   if (flag == 1)//PASS
      return;

   if (control.WB.RegWrite == 0)
      return;

   if (control.WB.MemtoReg == 1)
      regs[Write_register] = data_toWB_frommem;
   else regs[Write_register] = ALU_result;

   return;
}

//decode instruction to decimal number
void make_decimal(int *decimal, int start, int size)
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

   (*decimal) = (*decimal) >> 1;

   return;
}

void alu_control()
{
   if (control.EX.ALUOp1 == 1 && control.EX.ALUOp0 == 0) {//R-Format
      switch (funct) {
      case 8://jr
         flag = 1;
         pc = regs[31]; //update pc
         break;
      case 32:
         ALU_control_instrution = 0010;//add
         break;
      case 34:
         ALU_control_instrution = 0110;//subtract
         break;
      case 36:
         ALU_control_instrution = 0000;//AND
         break;
      case 35:
         ALU_control_instrution = 0001;//OR
         break;
      }
   }
   else if (control.EX.ALUOp1 == 0 && control.EX.ALUOp0 == 0)//lw, sw
      ALU_control_instrution = 0010;
   else if (control.EX.ALUOp1 == 0 && control.EX.ALUOp0 == 1)//beq
      ALU_control_instrution = 0110;
   else if (control.EX.ALUOp1 == 1 && control.EX.ALUOp0 == 1)//slti
      ALU_control_instrution = 1011;
}

void MUX_for_ALU()
{
   read1 = regs[Read_register1];
   if (control.EX.ALUsrc == 0) {
      read2 = Read_register2;
   }
   else if (control.EX.ALUsrc == 1) {
      read2 = address;
   }
}

void alu()
{
   MUX_for_ALU();

   switch (ALU_control_instrution)
   {
   case 0010://add
      ALU_result = read1 + read2;
      break;
   case 0110://subtract
      ALU_result = read1 - read2;

      if (control.M.Branch && !(ALU_result)) //MUX for Branch
      {
         pc += address;
      }

      break;
   case 0000://and
      ALU_result = read1 & read2;
      break;
   case 0001://OR
      ALU_result = read1 | read2;
      break;
   case 1011:
      ALU_result = read1 < read2;

   }
}

void print_cycles()
{
   printf("Clock Cycles: %d\n", cycles);

   return;
}

void print_pc()
{
   printf("Current pc number: %d\n", pc);

   return;
}

void print_reg()
{
   printf("R0    [r0] = %x\n", regs[0]);
   printf("R1    [at] = %x\n", regs[1]);
   printf("R2    [v0] = %x\n", regs[2]);
   printf("R3    [v1] = %x\n", regs[3]);
   printf("R4    [r0] = %x\n", regs[4]);
   printf("R5    [a1] = %x\n", regs[5]);
   printf("R6    [a2] = %x\n", regs[6]);
   printf("R7    [a3] = %x\n", regs[7]);
   printf("R8    [t0] = %x\n", regs[8]);
   printf("R9    [t1] = %x\n", regs[9]);
   printf("R10   [t2] = %x\n", regs[0]);
   printf("R11   [t3] = %x\n", regs[11]);
   printf("R12   [t4] = %x\n", regs[12]);
   printf("R13   [t5] = %x\n", regs[13]);
   printf("R14   [t6] = %x\n", regs[14]);
   printf("R15   [t7] = %x\n", regs[15]);
   printf("R16   [s0] = %x\n", regs[16]);
   printf("R17   [s1] = %x\n", regs[17]);
   printf("R18   [s2] = %x\n", regs[18]);
   printf("R19   [s3] = %x\n", regs[19]);
   printf("R20   [s4] = %x\n", regs[10]);
   printf("R21   [s5] = %x\n", regs[21]);
   printf("R22   [s6] = %x\n", regs[22]);
   printf("R23   [s7] = %x\n", regs[23]);
   printf("R24   [t8] = %x\n", regs[24]);
   printf("R25   [t9] = %x\n", regs[25]);
   printf("R26   [k0] = %x\n", regs[26]);
   printf("R27   [k1] = %x\n", regs[27]);
   printf("R28   [gp] = %x\n", regs[28]);
   printf("R29   [sp] = %x\n", regs[29]);
   printf("R30   [s8] = %x\n", regs[30]);
   printf("R31   [ra] = %x\n\n", regs[31]);

   return;
}
