// This program reads a list of raw audio files, splits them into 128-byte
// blocks, and produces C source code and header.
//
// USAGE:  'raw2c file1.raw file2.raw ... filen.raw'
// OUTPUT: sounds.c and sounds.h
//

// The input files are assumed to be raw monophonic (single channel)
// signed 16-bit audio.  You can load a .wav or other audio file into
// audacity, convert it to the depth/sample rate that you want, and
// output the raw audio from there.
// 1) audacity <input file>
// 2) tracks->mix->mix stereo down to mono
// 3) select->all
// 4) tracks->resample
//    enter 8000 as new sample rate
// 5) file->export audio
//    filename: whatever.raw
//    Folder: change as needed
//    Format: Other Uncompressed Files
//    Header: RAW (header-less)
//    Encoding: Signed 16-bit PCM
//    Export Range: Entire Project
// 

#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>

// define the size of an audio buffer.
#define BUF_SIZE 128

void process(char *fname, FILE *header, FILE *cfile)
{
  FILE * input;
  char *ename;
  long nbytes;
  int nbuffers;
  int i,j;
  int first,firstonline;
  int8_t c;
  int16_t indat;
  
  // Copy the input file name
  ename = strdup(fname);
  // Strip off ".raw"
  ename[strlen(ename)-4] = 0;

  input = fopen(fname,"r");
  // find out how many blocks we need.
  fseek(input,0, SEEK_END);
  nbytes = ftell(input);
  printf("The file has %ld bytes\n",nbytes);

  nbuffers = nbytes/BUF_SIZE;
  if(nbytes % BUF_SIZE)
    nbuffers++;
    
  printf("Processing %s\n",fname);
  printf("It will take %d buffers\n",nbuffers);

  fprintf(header,"#define NUM_%s_BUFFERS \t%d\n",ename,nbuffers);
  fprintf(header,"extern effect_buffer %s[NUM_%s_BUFFERS];\n\n",
	  ename,ename);
  
  // go back to beginning of file.
  fseek(input,0, SEEK_SET);

  fprintf(cfile,"effect_buffer %s[NUM_%s_BUFFERS] = {\n",ename,ename);

  for(i=0;i<nbuffers;i++)
    {
      fprintf(cfile,"  {\n");
      fprintf(cfile,"  {\n");
      first = 1;
      firstonline = 1;
      for(j = 0; j < BUF_SIZE; j++)
	{
	  if(!fread(&indat,2,1,input))
	    c = 0;
	  else
	    c = (indat + 0x128) >>  8;
	  if(first)
	    {
	      fprintf(cfile,"    0x%02X",(c&0xFF));
	      first=0;
	    }
	  else
	    fprintf(cfile,", 0x%02X",(c&0xFF));
	  // 	  printf("\t0x%02X",(c&0xFF));
	  if(!  ((j+1)%12))
	    {
	      fprintf(cfile,",\n");
	      first = 1;
	    }
	}
      fprintf(cfile,"\t}\n");
      if(i < (nbuffers-1))
	fprintf(cfile,"\t},\n");
      else
	fprintf(cfile,"\t}\n");
    }
  fprintf(cfile,"};\n\n");

  free(ename);
}

int main(int argc, char **argv)
{
  int8_t c;
  int count = -1;
  int first = 1;
  int total = 0;
  int i;
  char *outcname, *outhname;
    
  if(argc < 3)
    {
      printf("\nUsage: %s <output-base> <inputfile1> [<inputfile2>] [<inputfile3>]... \n\n",argv[0]);
      exit(1);
    }

  outhname = malloc(strlen(argv[1])+4);
  outcname = malloc(strlen(argv[1])+4);
  strcpy(outhname,argv[1]);
  strcpy(outcname,argv[1]);
  strcat(outhname,".h");
  strcat(outcname,".c");
  // open the header file and C file for output
  FILE *headerfile = fopen(outhname,"w");
  FILE *cfile = fopen(outcname,"w");
  // Write the beginning of the header file
  fprintf(headerfile, "#ifndef SOUNDS_H\n");
  fprintf(headerfile, "#define SOUNDS_H\n\n");
  fprintf(headerfile, "#include <stdint.h>\n\n");
  fprintf(headerfile,"// Define a struct that holds some samples of audio\n");
  fprintf(headerfile,"// data.  Doing it this way simplifies other parts of\n");
  fprintf(headerfile,"// the code. A sound effect can be defined as an array\n");
  fprintf(headerfile,"// of effect buffers\n");
  fprintf(headerfile,"#define EFFECT_BUFFER_SIZE %d\n",BUF_SIZE);
  fprintf(headerfile,"typedef struct{\n");
  fprintf(headerfile,"  uint8_t data[EFFECT_BUFFER_SIZE];\n");
  fprintf(headerfile,"}effect_buffer;\n\n");
  fprintf(headerfile,"// Now define all of the sound effects;\n\n");
  // Write the beginning of the C file
  fprintf(cfile, "#include<sounds.h>\n\n");
  // Read all of the raw files, and output to the header and C file
  for(i=2;i<argc;i++)
    {
      process(argv[i],headerfile,cfile);
    }
  // Finish the header file
  fprintf(headerfile, "#endif\n");
  // close the output files
  fclose(cfile);
  fclose(headerfile);
  return 0;
}
