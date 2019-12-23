/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Akshay Shinde (ashinde3@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  cpu->stage[EX1].insflush = 0;

  for(int i=0; i<32; i++)
  {
    cpu->regs_valid[i] = 1;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "rs3", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].rs3,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0)
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if(strcmp(stage->opcode, "STR") == 0)
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rs1, stage->rs2, stage->rs3);
  }

  if(strcmp(stage->opcode, "LOAD") ==0 )
  {
    printf("%s,R%d,R%d,#%d", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if(strcmp(stage->opcode, "LDR") ==0 )
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if(strcmp(stage->opcode, "ADD") ==0 )
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if(strcmp(stage->opcode, "ADDL") ==0 )
  {
    printf("%s,R%d,R%d,#%d", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if(strcmp(stage->opcode, "SUB") ==0 )
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if(strcmp(stage->opcode, "SUBL") ==0 )
  {
    printf("%s,R%d,R%d,#%d", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if(strcmp(stage->opcode, "MUL") ==0 )
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "MOVC") == 0)
  {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if(strcmp(stage->opcode, "AND") ==0 )
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if(strcmp(stage->opcode, "OR") ==0 )
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if(strcmp(stage->opcode, "EXOR") ==0 )
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if(strcmp(stage->opcode, "BZ") == 0)
  {
    printf("%s,#%d", stage->opcode, stage->imm);
  }

  if(strcmp(stage->opcode, "BNZ") == 0)
  {
    printf("%s,#%d", stage->opcode, stage->imm);
  }

  if(strcmp(stage->opcode, "JUMP") == 0)
  {
    printf("%s,R%d,#%d", stage->opcode, stage->rs1, stage->imm);
  }

  if(strcmp(stage->opcode, "HALT") == 0)
  {
    printf("%s", stage->opcode);
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled)
  {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    if(!cpu->stage[DRF].stalled)
    {
      /* Update PC for next instruction */
      cpu->pc += 4;

      /* Copy data from fetch latch to decode latch*/
      cpu->stage[DRF] = cpu->stage[F];
    }


    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }

  else if(cpu->stage[EX1].insflush == 1)
  {
    strcpy(cpu->stage[F].opcode, "");
    printf("Fetch         : EMPTY\n");
  }

  else if(stage->stalled==1 || stage->busy==1)    // To stall fetch when MUL enters Execute stage
  {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

   if(ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  if(stage->stalled)
  {
    stage->stalled = 0;
  }

  if (!stage->busy && !stage->stalled)
  {

    // STORE
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      stage->arithminstr = 0;
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    // STR
    if (strcmp(stage->opcode, "STR") == 0)
    {
      stage->arithminstr = 0;
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2] && cpu->regs_valid[stage->rs3])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        stage->rs3_value = cpu->regs[stage->rs3];
      }

      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    // LOAD
    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      stage->arithminstr = 0;
      if(cpu->regs_valid[stage->rs1])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->regs_valid[stage->rd]--;
      }

      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }
    }

    // LDR
    if (strcmp(stage->opcode, "LDR") == 0)
    {
      stage->arithminstr = 0;
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd]--;
      }

      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }
    }

    // ADD-SUB-MUL
    if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0)
    {
      stage->arithminstr = 1;
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd]--;
      }

      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }
    }

    // ADDL-SUBL
    if (strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0)
    {
      stage->arithminstr = 1;
      if(cpu->regs_valid[stage->rs1])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->regs_valid[stage->rd]--;
      }

      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      stage->arithminstr = 0;
      cpu->regs_valid[stage->rd]--;
    }

    // AND-OR-EXOR
    if (strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EXOR") == 0)
    {
      stage->arithminstr = 0;
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd]--;
      }

      else
      {
        cpu->stage[F].stalled=1;
        cpu->stage[DRF].stalled=1;
      }
    }

    // BZ-BNZ
    if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0)
    {
      stage->arithminstr = 0;
      if((cpu->stage[WB].arithminstr == 1) || (cpu->stage[MEM2].arithminstr == 1))
      {
        stage->stalled = 1;
      }
      else
      {
        stage->stalled = 0;
      }
    }

    // JUMPs
    if (strcmp(stage->opcode, "JUMP") == 0)
    {
      stage->arithminstr = 0;
      stage->rs1_value = cpu->regs[stage->rs1];
    }

    // HALT
    if(strcmp(stage->opcode, "HALT") == 0)
    {
      stage->arithminstr = 0;
      cpu->stage[F].stalled = 1;
      cpu->stage[F].pc = 0;
      strcpy(cpu->stage[F].opcode, "");
      cpu->ex_halt = 1;
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX1] = cpu->stage[DRF];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Decode/RF", stage);
    }
  }

  else if(cpu->stage[EX1].insflush == 1)
  {
     strcpy(cpu->stage[F].opcode, "");
     printf("Decode        : EMPTY\n");
  }

  else
  {
    if(ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Decode/RF", stage);
    }
  }




  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX1];
  if (!stage->busy && (stage->stalled == 0)) {

    if (strcmp(stage->opcode, "STORE") == 0)
    {
      stage->mem_address = stage->rs2_value + stage->imm;
    }

    if (strcmp(stage->opcode, "STR") == 0)
    {
      stage->mem_address = stage->rs2_value + stage->rs3_value;
    }

    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      stage->mem_address = stage->rs1_value + stage->imm;
    }

    if (strcmp(stage->opcode, "LDR") == 0)
    {
      stage->mem_address = stage->rs1_value + stage->rs2_value;
    }

    if (strcmp(stage->opcode, "ADD") == 0)
    {
      stage->buffer = stage->rs1_value + stage->rs2_value;
      if(stage->buffer == 0)
          cpu->zeroFlag = 1;
      else
          cpu->zeroFlag = 0;
    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {
      stage->buffer = stage->rs1_value + stage->imm;
      if(stage->buffer == 0)
          cpu->zeroFlag = 1;
      else
          cpu->zeroFlag = 0;
    }

    if (strcmp(stage->opcode, "SUB") == 0)
    {
      stage->buffer = stage->rs1_value - stage->rs2_value;
      if(stage->buffer == 0)
          cpu->zeroFlag = 1;
      else
          cpu->zeroFlag = 0;
    }

    if (strcmp(stage->opcode, "SUBL") == 0)
    {
      stage->buffer = stage->rs1_value - stage->imm;
      if(stage->buffer == 0)
          cpu->zeroFlag = 1;
      else
          cpu->zeroFlag = 0;
    }

    if (strcmp(stage->opcode, "MUL") == 0)
    {
      if(stage->mulFlag == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].busy = 1;
        cpu->stage[DRF].busy = 1;
        stage->nop = 1;
      }

      if(stage->mulFlag == 1)
      {
        stage->buffer = stage->rs1_value * stage->rs2_value;
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        cpu->stage[F].busy=0;
        cpu->stage[DRF].busy=0;
        stage->nop=0;
      }

      stage->mulFlag = 1;
      if(stage->buffer == 0)
          cpu->zeroFlag = 1;
      else
          cpu->zeroFlag = 0;
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      stage->buffer = stage->imm;
    }

    if (strcmp(stage->opcode, "AND") == 0)
    {
      stage->buffer = stage->rs1_value & stage->rs2_value;
    }

    if (strcmp(stage->opcode, "OR") == 0)
    {
      stage->buffer = stage->rs1_value | stage->rs2_value;
    }

    if (strcmp(stage->opcode, "EXOR") == 0)
    {
      stage->buffer = stage->rs1_value ^ stage->rs2_value;
    }

    if (strcmp(stage->opcode, "BNZ") == 0)
    {
      if(!cpu->zeroFlag)
      {
        stage->mem_address = stage->pc + stage->imm;

      }
      else
      {
        stage->insflush = 1;
        stage->mem_address = 0;
      }
    }

    if (strcmp(stage->opcode, "BZ") == 0)
    {
      if(cpu->zeroFlag == 1)
      {
        stage->mem_address = stage->pc + stage->imm;
        cpu->zeroFlag = 0;
      }
      else
      {
        stage->insflush = 1;
        stage->mem_address = 0;
      }
    }

    if (strcmp(stage->opcode, "JUMP") == 0)
    {
      cpu->pc = stage->rs1_value + stage->imm;
    }

    if(strcmp(stage->opcode, "HALT") == 0)
    {
      stage->insflush = 1;
      cpu->stage[DRF].pc = 0;
      strcpy(cpu->stage[DRF].opcode, "");
      cpu->stage[DRF]. stalled = 1;
      cpu->stage[F].stalled = 1;
      strcpy(cpu->stage[F].opcode, "");
      cpu->stage[F].pc = 0;
      cpu->ex_halt=1;
    }


    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[EX2] = cpu->stage[EX1];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute 1", stage);
    }
  }

  else
  {
    cpu->stage[EX2] = cpu->stage[EX1];
    if(ENABLE_DEBUG_MESSAGES)
    {
      printf("Execute 1        : EMPTY\n");
    }
  }
  return 0;
}

int
execute2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  //if (!stage->busy && !stage->stalled) {
  if (!stage->busy && !stage->stalled) {
    if (strcmp(stage->opcode, "STORE") == 0)
    {
    }

    if (strcmp(stage->opcode, "STR") == 0)
    {
    }

    if (strcmp(stage->opcode, "LOAD") == 0)
    {
    }

    if (strcmp(stage->opcode, "LDR") == 0)
    {
    }

    if (strcmp(stage->opcode, "ADD") == 0)
    {
    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {
    }

    if (strcmp(stage->opcode, "SUB") == 0)
    {
    }

    if (strcmp(stage->opcode, "SUBL") == 0)
    {
    }

    if (strcmp(stage->opcode, "MUL") == 0)
    {
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
    }

    if (strcmp(stage->opcode, "AND") == 0)
    {
    }

    if (strcmp(stage->opcode, "OR") == 0)
    {
    }

    if (strcmp(stage->opcode, "EXOR") == 0)
    {
    }

    if (strcmp(stage->opcode, "BZ") == 0)
    {
    }

    if (strcmp(stage->opcode, "BNZ") == 0)
    {
    }

    if (strcmp(stage->opcode, "JUMP") == 0)
    {
    }

    if(strcmp(stage->opcode, "HALT") == 0)
    {
    }


    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM1] = cpu->stage[EX2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute 2", stage);
    }
  }

  else
  {
    cpu->stage[MEM1] = cpu->stage[EX2];

    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Execute 2        : EMPTY\n");
    }
  }
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM1];
  if (!stage->busy && !stage->stalled) {

    if (strcmp(stage->opcode, "STORE") == 0)
    {
    }

    if (strcmp(stage->opcode, "STR") == 0)
    {
    }

    if (strcmp(stage->opcode, "LOAD") == 0)
    {
    }

    if (strcmp(stage->opcode, "LDR") == 0)
    {
    }

    if (strcmp(stage->opcode, "ADD") == 0)
    {
    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {
    }

    if (strcmp(stage->opcode, "SUB") == 0)
    {
    }

    if (strcmp(stage->opcode, "SUBL") == 0)
    {
    }

    if (strcmp(stage->opcode, "MUL") == 0)
    {
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
    }

    if (strcmp(stage->opcode, "AND") == 0)
    {
    }

    if (strcmp(stage->opcode, "OR") == 0)
    {
    }

    if (strcmp(stage->opcode, "EXOR") == 0)
    {
    }

    if (strcmp(stage->opcode, "BZ") == 0)
    {
    }

    if (strcmp(stage->opcode, "BNZ") == 0)
    {
    }

    if (strcmp(stage->opcode, "JUMP") == 0)
    {
    }

    if(strcmp(stage->opcode, "HALT") == 0)
    {
    }


    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory 1", stage);
    }
  }

  else
  {
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Memory 1         : EMPTY\n");
    }
  }
  return 0;
}

int
memory2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
  if (!stage->busy && !stage->stalled && stage->nop == 0)
  {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }

    if (strcmp(stage->opcode, "STR") == 0)
    {
      cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }

    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      stage->buffer= cpu->data_memory[stage->mem_address];
    }

    if (strcmp(stage->opcode, "LDR") == 0)
    {
      stage->buffer= cpu->data_memory[stage->mem_address];
    }

    /*if (strcmp(stage->opcode, "ADD") == 0)
    {
    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {
    }

    if (strcmp(stage->opcode, "SUB") == 0)
    {
    }

    if (strcmp(stage->opcode, "SUBL") == 0)
    {
    }

    if (strcmp(stage->opcode, "MUL") == 0)
    {
    }*/

    /* No Register file read needed for MOVC */
    /*if (strcmp(stage->opcode, "MOVC") == 0)
    {
    }

    if (strcmp(stage->opcode, "AND") == 0)
    {
    }

    if (strcmp(stage->opcode, "OR") == 0)
    {
    }

    if (strcmp(stage->opcode, "EXOR") == 0)
    {
    }*/

    if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0)
    {
      if(stage->mem_address != 0)
      {
        cpu->pc =stage->mem_address;

      //printf("\nTesting: %d: %d\n",cpu->stage[EX].rd,cpu->regs_valid[cpu->stage[EX].rd]);
        if((strcmp(cpu->stage[MEM1].opcode, "ADD") == 0) || (strcmp(cpu->stage[MEM1].opcode, "SUB") == 0) || (strcmp(cpu->stage[MEM1].opcode, "MUL") == 0) || (strcmp(cpu->stage[MEM1].opcode, "AND") == 0) || (strcmp(cpu->stage[MEM1].opcode, "OR") == 0) || (strcmp(cpu->stage[MEM1].opcode, "EXOR") == 0) || (strcmp(cpu->stage[MEM1].opcode, "MOVC") == 0) || (strcmp(cpu->stage[MEM1].opcode, "LOAD") == 0) || (strcmp(cpu->stage[MEM1].opcode, "LDR") == 0) || (strcmp(cpu->stage[MEM1].opcode, "ADDL") == 0) || (strcmp(cpu->stage[MEM1].opcode, "SUBL") == 0))
        {
          cpu->regs_valid[cpu->stage[MEM1].rd]++;
        }

          //stage->flush=1;
        cpu->stage[DRF].pc = 0;
         strcpy(cpu->stage[DRF].opcode, "");
         //cpu->stage[DRF].stalled = 1;
        //cpu->stage[EX].stalled = 1;
         strcpy(cpu->stage[MEM1].opcode, "");
        cpu->stage[MEM1].pc = 0;

        if(stage->imm < 0)
        {
          cpu->ins_completed = (cpu->ins_completed + (stage->imm/4))-1;
        }

        else
        {
          cpu->ins_completed = (cpu->ins_completed - (stage->imm/4));
        }
        if(cpu->ex_halt)
        {
          cpu->ex_halt = 0;
          cpu->stage[F].stalled =0;
        }
      }
    }

    /*if (strcmp(stage->opcode, "BNZ") == 0)
    {
    }*/

    if (strcmp(stage->opcode, "JUMP") == 0)
    {
    }

    if(strcmp(stage->opcode, "HALT") == 0)
    {
      cpu->stage[MEM1].pc = 0;
      strcpy(cpu->stage[MEM1].opcode, "");
      cpu->stage[DRF].pc = 0;
      strcpy(cpu->stage[DRF].opcode, "");
      cpu->stage[MEM1].stalled = 1;
      cpu->stage[DRF].stalled = 1;
       strcpy(cpu->stage[F].opcode, "");
      cpu->stage[F].stalled = 1;
      cpu->stage[F].pc = 0;
      cpu->ex_halt=1;
    }


    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory 2", stage);
    }
  }

  else
  {
    cpu->stage[WB] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Memory 2         : EMPTY\n");
    }
  }
  return 0;
}
/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled && stage->nop == 0 && (strcmp(stage->opcode,"")!=0))
  {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "LDR") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "ADD") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "SUB") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "SUBL") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "MUL") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "AND") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "OR") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "EXOR") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;
    }

    if (strcmp(stage->opcode, "HALT") == 0)
    {
      //cpu->regs[stage->rd] = stage->buffer;
       cpu->ins_completed = cpu->code_memory_size - 1;
       cpu->stage[EX2].pc = 0;
       strcpy(cpu->stage[EX2].opcode, "");
       cpu->stage[DRF].pc = 0;
       strcpy(cpu->stage[DRF].opcode, "");
       cpu->stage[EX2].stalled = 1;
       cpu->stage[DRF].stalled = 1;
       cpu->stage[F].stalled = 1;
       strcpy(cpu->stage[F].opcode, "");
       cpu->stage[F].pc = 0;
       cpu->stage[MEM2].pc = 0;
       strcpy(cpu->stage[MEM2].opcode, "");
       cpu->stage[MEM2].stalled = 1;
       cpu->ex_halt=1;
    }

    cpu->ins_completed++;

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Writeback", stage);
    }
  }

  else
  {
    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Writeback      : EMPTY\n");
    }
  }
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{
  while (1)
  {

    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size) {
      printf("(apex) >> Simulation Complete");
      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    writeback(cpu);
    memory2(cpu);
    memory1(cpu);
    execute2(cpu);
    execute1(cpu);
    decode(cpu);
    fetch(cpu);

    cpu->clock++;
  }
    printf("\n");
    printf("========ARCHITECTURAL REGISTER VALUES========\n");
    for(int j=0;j<=15;j++)
    {
      //printf("\n");
      printf(" | Reg[%d] | Value = %d | Status = %s | \n",j,cpu->regs[j], (cpu->regs_valid[j])?"Valid" : "Invalid");
    }
    //printf("\n\n");
    printf("======DATA MEMORY======\n");
    for(int k=0;k<=99;k++)
    {
      printf(" | MEM[%d] | Value=%d | \n", k,cpu->data_memory[k]);
    }


  return 0;
}
