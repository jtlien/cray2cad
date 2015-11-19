#include "stdio.h"
#include "string.h"
#include "ctype.h"

#define LPAREN 40
#define lparen 40
#define TRUE 1
#define FALSE 0
#define rparen 41
#define quotchar 34
#define EOLN '\n'
#define RBRACK  '}'
#define NULLCHAR 0

FILE *file1,*file2,*file3;
FILE *fopen();


// Read in the dups file created by dodups
//   and get all the terms that are copies
//   create a file of changes from to
//     so the original boolean file can be updated to eliminate the copies

char this_line[256];
char prev_line[256];
char in_line[256];

 int ii;
 int endoffile;
 int linechar;
int debug;

char first_lterm[120];
char this_lterm[120];

char first_term[120];
char this_term[120];
char new_line[256];
char this_line[256];

void skipblanks()
{

  while((in_line[linechar] == ' ') && ( in_line[linechar] != NULLCHAR))
    {
      linechar += 1;
    }

}

void  get_first_term()
  {
    int i;

    linechar = 4;
    skipblanks();
    for (i = 0; i < 3; i += 1)
      {
	first_term[i] = in_line[linechar];
        linechar += 1;
      }

    first_term[4] = NULLCHAR;

    if ( isupper(first_term[0]))
     {
       first_lterm[0] = tolower(first_term[0]);
       first_lterm[1] = tolower(first_term[1]);
       first_lterm[2] = tolower(first_term[2]);
       first_lterm[3] = NULLCHAR;
      }
     else
      {
       first_lterm[0] = toupper(first_term[0]);
       first_lterm[1] = toupper(first_term[1]);
       first_lterm[2] = toupper(first_term[2]);
       first_lterm[3] = NULLCHAR;
      }
     
  }

void getaline(aline,infile)
char *aline[120];
FILE *infile;

{
int i;
char *fgets();
char *fstat;
char *lineptr;


i = 1;
fstat = fgets(aline,120,infile);
if (fstat==NULL)
 {
  endoffile=TRUE;
 }
 i = 0;
 
 // printf("line in = %s \n",aline);
}       /* end of getline */

void shorten()
{
  int jj;

  for (jj=0; jj < strlen(in_line); jj += 1)
    {
      if (in_line[jj] == '.')
	{
          in_line[jj] = NULLCHAR;
        }
    }

}

void get_line_new()
{
  int ii;

  strcpy(new_line,in_line);

  this_term[0] = in_line[6];
  this_term[1] = in_line[7];
  this_term[2] = in_line[8];
  this_term[3] = NULLCHAR;

  if ( isalpha ( this_term[0]) == FALSE)
    {
      printf("Source has bad character = %c \n",this_term[0]);
    }
  if ( isalpha ( this_term[1]) == FALSE)
    {
      printf("Source has bad character = %c \n",this_term[1]);
    }
  if ( isalpha ( this_term[2]) == FALSE)
    {
      printf("Source has bad character = %c \n",this_term[2]);
    }

  if (isupper(this_term[0]))
    {
       this_lterm[0] = tolower(this_term[0]);
       this_lterm[1] = tolower(this_term[1]);
       this_lterm[2] = tolower(this_term[2]);
       this_lterm[3] = NULLCHAR;
      }
     else
      {
       this_lterm[0] = toupper(this_term[0]);
       this_lterm[1] = toupper(this_term[1]);
       this_lterm[2] = toupper(this_term[2]);
       this_lterm[3] = NULLCHAR;
      }


  for (ii = 0; ii < 9; ii += 1)
    {
      new_line[ii] = ' ';
    }
  

}

 main (argc,argv)
 int argc;
 char *argv[];
{

  debug = 0;
  // printf("about to open = %s \n",argv[1]);

   file1 = fopen(argv[1],"r");
   if (file1 == NULL)
    {
     printf("Can't open the input  file %s\n",argv[1]);
     exit(1);
    }
   
   endoffile = FALSE;

   getaline(in_line,file1);   
   shorten();

   // printf("line in = %s \n",in_line);

   get_first_term();

   strcpy( prev_line, in_line);
   for (ii = 0; ii < 9; ii += 1)
     {
       prev_line[ii] = ' ';
     }
   
   if (debug)
     {
   printf("first_term = %s \n",first_term);
   printf("prev_line = %s \n",prev_line);
     }
   getaline(in_line,file1);
   shorten();
   get_line_new();

  
  while(endoffile == FALSE)
    {

      while (( strcmp( new_line , prev_line ) == 0) && (endoffile == FALSE))
       {
         printf("s/ %s / %s /g\n",this_term,first_term);
         printf("s/ %s / %s /g\n",this_lterm,first_lterm);
	 //  printf("In loop - new_line  = %s \n",new_line);
	 //  printf("In loop - prev_line = %s \n",prev_line);
         getaline(in_line,file1);
         shorten();
         get_line_new();

      }

      if ( endoffile == FALSE)   // not end of file yet...
	{
	  
         shorten();
         get_first_term();

         strcpy( prev_line, in_line);
         for (ii = 0; ii < 9; ii += 1)
          {
           prev_line[ii] = ' ';
          }
        
         getaline(in_line,file1);
	 if ( endoffile == FALSE)
           {
            shorten();
            get_line_new();
           }
        }

    }
  fclose(file1);

}


