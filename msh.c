// The MIT License (MIT)
// 
// Copyright (c) 2020 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_NUM_ARGUMENTS 3

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

char * token[MAX_NUM_ARGUMENTS];

struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t unused[8];
    uint16_t ClusterHigh;
    uint8_t unused2[4];
    uint16_t ClusterLow;
    uint32_t size;
};
struct DirectoryEntry dir[16];

  //char BS_OEName[8];
  int16_t BPB_BytesPerSec;
  int8_t BPB_SecPerClus;
  int16_t BPB_RsvdSecCnt;
  int8_t BPB_NumFATs;
  int32_t BPB_FATSz32;
  //int16_t BPB_RootEntCnt;
  //char BS_VOlLab[11];
  //int32_t BPB_RootClus;

FILE * fp;

int LBAToOffset(int32_t sector)
{
    return ((sector - 2) * BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}

int16_t NextLB(uint32_t sector)
{
    uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt) + (sector * 4);
    int16_t val;
    fseek(fp, FATAddress, SEEK_SET);
    fread(&val, 2, 1, fp);
    return val;
}

int main()
{

  int32_t RootDirSectors = 0;
  int32_t FirstDataSector = 0;
  int32_t FirstSectorCluster = 0;
  


  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your FAT32 functionality

    int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      printf("token[%d] = %s\n", token_index, token[token_index] );  
    }

    free( working_root );

  }

  if(!strcmp("open", token[0]))
  {
      fp = fopen(token[1], "r");
      fseek(fp, 11, SEEK_SET);
      fread(&BPB_BytesPerSec, 2, 1, fp);
      //printf("BPB_BytesPerSec: %d\n", BPB_BytesPerSec);

      fseek(fp, 13, SEEK_SET);
      fread(&BPB_SecPerClus, 1, 1, fp);
      //printf("BPB_SecPerClus: %d\n", BPB_SecPerClus);

      fseek(fp, 0x100400, SEEK_SET);
      fread(&dir[0], sizeof(struct DirectoryEntry), 16, fp);
      for(int i = 0; i < 16; i++)
      {
          printf("Filename: %s\n", dir[i].DIR_Name);
      }
      
  }
  /*
  else if(strcmp(token[0], "close") == 0)
  {
      if(fp == NULL)
      {
          printf("File FAT32 not open");
          return;
      }
      fclose(fp);
      fp = NULL;
  }
  */
  else if(strcmp(token[0], "info") == 0)
  {
      printf("BPB_BytesPerSec: %d \n", BPB_BytesPerSec);
      printf("BPB_SecPerClus: %d \n", BPB_SecPerClus);
      printf("BPB_RsvdSecCnt: %d \n", BPB_RsvdSecCnt);
      printf("BPB_NumFATs: %d \n", BPB_NumFATs);
      printf("BPB_FATSz32: %d \n", BPB_FATSz32);
  }
  else if(strcmp(token[0], "stat") == 0)
  {
      
  }


  return 0;
}

