#include "stdio.h"
#include "string.h"

#define EOLN '\n'
#define MAXCIRC 2024
#define MAXBOOL 4000
#define TRUE 1
#define FALSE 0

FILE *file1,*file2,*file3,*file4,*file5,*fopen();
FILE *file6;

char flipterm[20];

/*******************************************************************/
/* This program takes an Cray 2 package file ??pkg                 */
/*    and produces an output file that has loc,pkgtype,pkgpin,bool */
/*                    loc = 2 charactor location                   */
/*                    pkgtype = type of chip                       */
/*                    pkgpin = package output pin                  */
/*                    bool = boolean term                          */
/*                                                                 */
/*******************************************************************/


int endofterms;

 int argcnt;
int alltermcnt;

char locstr[10];
char chip_type;
int termcnt;

char output_pin;
char output_pinm;

int *inptr[10];
int rdindex;          /* read index */
int lbindex;           /* least boolean index */
int bufflen;          /* length of buffer */
char boolbuf;
char linein[120];
char inbuff[MAXCIRC];          /* holds an entire circuit  */
int buffindex;
int linenum;
int endoffile;
int linechar;
int blankline;
int debug;
int dotfound;

int i,j,k,l,m,n;
int infile;
char curfile[20];
char curfilex[20];
char curboard[20];

int lineindex;

struct termstuff
 {
   char chiptype;
   char term[10];
   char locstr[10];
   char output_pin;
   char output_pinm;
} termarray[5000];

int locusedarray[2000];
int boolusedarray[100000];

char thisterm[10];

void exitnow(int exitval)
{
  printf("Would exit here \n");

}

void skipblanks()
 {
  while ((linein[lineindex] == ' ') && (lineindex <120))
    {
     lineindex += 1;
    }
 }  /* end of skip blanks  */


//
//  get a line from current input file
//

void getaline(char *linein,FILE *infile)

{
int i;
char *fstat;
char *lineptr;

i = 1;
fstat = fgets(linein,120,infile);
 lineptr = &linein[0];
if (*lineptr == 12)
 {
  fstat = fgets(linein,120,infile);
  lineptr = &linein[0];
 }
if ((fstat==NULL) || ( *lineptr == 3) || (*lineptr == 26))
 {
  endoffile=TRUE;
  return;
 }
 i = 0;
 while (*lineptr != '\n')
  {
   if ((*lineptr >127) || (*lineptr < ' '))
     {
      *lineptr = ' ';
     }
   lineptr++;
  }
}       /* end of getaline */

//
//
//

void readuptodot()


{
int dotfound;
int jj,k,j;
 dotfound = FALSE;
 k=0;
 while (( endoffile == FALSE) && ( dotfound == FALSE))
  {
   if (debug) { printf("%s\n",linein); }
   linechar = 0;
   while(linein[linechar] != '\n')
    {
     if (k < MAXCIRC)
      {
       inbuff[k] = linein[linechar];
       k += 1;
       if (linein[linechar] == '.')
        {
         dotfound = TRUE;
        }
      }
      linechar += 1;
    }
   inbuff[k] = '\n';
   k += 1;
   getaline(linein,file1);
  }
  inbuff[k] = 0;
 bufflen = k;
}    /* end read up to dot */

int checkloc( char *inlocstr)
{
  int locindex;
  int baseval;
  int lowval;

  if (islower (inlocstr[0] ))
    {
      printf("Error: location string has lower case letter - %s \n",inlocstr);
    }
  else
    {
      baseval = inlocstr[0] - 'A';
    }

  if (isalpha(inlocstr[1]))
    {
      if (islower(inlocstr[1]))
	{
          printf("Error: location string has lower case letter - %s \n",inlocstr);
	}
      else
	{
	  lowval = inlocstr[1]-'A';
	}
    }
  else
    {
      if (isdigit ( inlocstr[1])&& (inlocstr[1] != '9') && (inlocstr[1] != '0'))
	{
         lowval = inlocstr[1]-'0'+23;
        }
      else
	{
          printf("Unexpected character in loc = %s \n",inlocstr);
        }
    }

  locindex = (32 * baseval) + lowval;

  if (locindex < 2000 )
    {
     if (locusedarray[locindex]==1)
      {
       return(1);
      }
     else
     {
      locusedarray[locindex] = 1;

      return(0);
     }
    }
  else
    {
      printf("Internal error in checkloc \n");
    }

}   
//
// Check a boolean term to see if it is already used
//
int checkbool( char *inboolstr)
{
  int baseval1;
  int baseval2;
  int lowval;
  int boolindex;

  if (islower (inboolstr[0] ))
    {
      baseval1 = toupper(inboolstr[0]) - 'A';
    }
  else
    {
      baseval1 = inboolstr[0] - 'A';
    }

  if (islower (inboolstr[1] ))
    {
      baseval2 = toupper(inboolstr[1]) - 'A';
    }
  else
    {
      baseval2 = inboolstr[1] - 'A';
    }

  if (islower (inboolstr[2] ))
    {
      lowval = toupper(inboolstr[2]) - 'A';
    }
  else
    {
      lowval = inboolstr[2] - 'A';
    }

  // 26 * 26 = 625 + 50 + 1

  boolindex = (baseval1 * 676) + (baseval2 * 26) + lowval;

  if (boolindex < 100000)
    {
      if (boolusedarray[boolindex] == 0)
	{
          boolusedarray[boolindex] = 1;
          return(0);
        }
      else
        {
          if (chip_type != 'Z')
	    {
              if (strncmp(inboolstr,"zzo",10)==0)
		{
		  return(0);
                }
	      if (strncmp(inboolstr,"ZZO",10)==0)
		{
		  return(0);
                }

	      printf("Boolean output term already used - %s  chip_type = %c \n",inboolstr,chip_type);
             return(1);
	    }
	}
    }
  else
    {
      printf("Internal error in pkgmap routine checkbool \n");
    }
}   

//
//
//
void getloc()


{

lineindex =0;
 locstr[0] = linein[lineindex];
lineindex+= 1;
 locstr[1] = linein[lineindex];
lineindex+= 1;
 locstr[2] = 0;

 if (checkloc(locstr) == 1)
   {
     printf("More than one entry for location = %s \n",locstr);
     exitnow(0);
   }
} // end getloc

//
//
//
void getchiptype()


{


lineindex =2;
 chip_type= linein[lineindex];
lineindex+= 1;    // chip type
lineindex+= 1;
lineindex+= 1;
lineindex+= 1;

 
}

isalphanum(c)
char *c;
{
 if (isalpha(c) || isdigit(c))
   {
    return(1);
   }
 else
   {
   return(0);
   }
}  /* end is alphanum */

void getterm()
{

  thisterm[0] = linein[lineindex];
  lineindex += 1;

  thisterm[1] = linein[lineindex];
  lineindex += 1;

  thisterm[2] = linein[lineindex];
  lineindex += 1;

  thisterm[3] = 0;

  skipblanks();
  if (linein[lineindex] == '.')
    {
      endofterms = TRUE;
    }

  if (thisterm[0] != '*')
    {
     if (checkbool(thisterm) == 1)
      {
       printf("Boolean term was already used = %s \n", thisterm);
       }
    }

}   

//
// Hangle a single term
//

void handle_term(int itermcnt)
{
  if (chip_type == 'A')
    {
      if ( itermcnt == 1)  // D = KLMN
	{
          output_pin = 'D';
          output_pinm = 'E';
        }
      if ( itermcnt == 2)
	{
          output_pin = 'G';  // HIJ
          output_pinm = 'F';
        }

      if ( itermcnt > 2)
	{
          printf("Error: Too many output for CHIP type = 'A'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
    }


  if (chip_type == 'B')
    {
      if ( itermcnt == 1)
	{
          output_pin = 'I';   // I = JK
          output_pinm = 'H';
        }
      if ( itermcnt == 2)
	{
          output_pin = 'F';  // F = LM
          output_pinm = 'G';
        }

      if ( itermcnt == 3)
	{
          output_pin = 'E';   // E = NA
          output_pinm = 'D';
        }

      if ( itermcnt > 3)
	{
          printf("Error: Too many output for CHIP type = 'B'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

  if (chip_type == 'C')
    {
      if ( itermcnt == 1)
	{
          output_pin = 'F'; // F = HI+JK+LM+NA+BC
          output_pinm = 'G';
        }

      if ( itermcnt > 1)
	{
          printf("Error: Too many output for CHIP type = 'C'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

  if (chip_type == 'D')   //
    {
      if ( itermcnt == 1)
	{
          output_pin = 'F';
          output_pinm = 'G';
        }

      if ( itermcnt > 1)
	{
          printf("Error: Too many output for CHIP type = 'D'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }


 if (chip_type == 'E')
    {
      if ( itermcnt == 1)
	{
          output_pin = 'D';  // D = CJ
          output_pinm = 'E';
        }

      if ( itermcnt == 2)
	{
          output_pin = 'G';  // G = CJ
          output_pinm = 'F';
        }

      if ( itermcnt == 3)
	{
          output_pin = 'H';    // H = CJ
          output_pinm = 'I';
        }

      if ( itermcnt == 4)
	{
          output_pin = 'L';    // L = CJ
          output_pinm = 'K';
        }

      if ( itermcnt == 5)   // M = CJ
	{
          output_pin = 'M';
          output_pinm = 'N';
        }

      if ( itermcnt == 6)   // A = CJ
 	{
          output_pin = 'A';
          output_pinm = 'B';
        }

      if ( itermcnt > 6)
	{
          printf("Error: Too many output for CHIP type = 'E'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'F')
    {
      if ( itermcnt == 1)
	{
          output_pin = 'L';
          output_pinm = '*';
        }

      if ( itermcnt == 2)
	{
          output_pin = 'M';
          output_pinm = '*';
        }

      if ( itermcnt == 3)
	{
          output_pin = 'N';
          output_pinm = '*';
        }

      if ( itermcnt == 4)
	{
          output_pin = 'A';
          output_pinm = '*';
        }

      if ( itermcnt == 5)
	{
          output_pin = 'F';
          output_pinm = '*';
        }

      if ( itermcnt == 6)
	{
          output_pin = 'G';
          output_pinm = '*';
        }

      if ( itermcnt == 7)
	{
          output_pin = 'H';
          output_pinm = '*';
        }

      if ( itermcnt == 8)
	{
          output_pin = 'I';
          output_pinm = '*';
        }

      if ( itermcnt == 9)
	{
          output_pin = 'E';
          output_pinm = '*';
        }

      if ( itermcnt > 9)
	{
          printf("Error: Too many outputs for CHIP type = 'F'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 // used for add/carry logic
 //
 if (chip_type == 'G')
    {
      if ( itermcnt == 1)  // jklcba + klcba + lcba
	{
          output_pin = 'I';
          output_pinm = 'H';
        }

      if ( itermcnt == 2)   // lcba
	{
          output_pin = 'F';
          output_pinm = 'G';
        }

      if ( itermcnt == 3)  // jkl
	{
          output_pin = 'E';
          output_pinm = 'D';
        }

      if ( itermcnt > 3)
	{
          printf("Error: Too many outputs for CHIP type = 'G'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }
 //
 //
 if (chip_type == 'H')
    {
      if ( itermcnt == 1)  // edcban
	{
          output_pin = 'G';
          output_pinm = 'F';
        }

      if ( itermcnt == 2)   // jklm
	{
          output_pin = 'H';
          output_pinm = 'I';
        }

      if ( itermcnt > 2)
	{
          printf("Error: Too many outputs for CHIP type = 'H'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'I')
    {
      if ( itermcnt == 1)  // jkl
	{
          output_pin = 'I';
          output_pinm = 'H';
        }

      if ( itermcnt == 2)   // cba
	{
          output_pin = 'E';
          output_pinm = 'D';
        }

      if ( itermcnt == 3)   // mn
	{
          output_pin = 'G';
          output_pinm = 'F';
        }

      if ( itermcnt > 3)
	{
          printf("Error: Too many outputs for CHIP type = 'I'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'J')
    {
      if ( itermcnt == 1)  // ed
	{
          output_pin = 'G';
          output_pinm = 'F';
        }

      if ( itermcnt == 2)   // jd
	{
          output_pin = 'H';
          output_pinm = 'I';
        }

      if ( itermcnt == 3)   // lk
	{
          output_pin = 'N';
          output_pinm = 'M';
        }

      if ( itermcnt == 4)   // ck
	{
          output_pin = 'A';
          output_pinm = 'B';
        }

      if ( itermcnt > 4)
	{
          printf("Error: Too many outputs for CHIP type = 'J'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'K')
    {
      if ( itermcnt == 1)  // klj + mnc
	{
          output_pin = 'H';
          output_pinm = 'I';
        }

      if ( itermcnt == 2)   // abj + dec
	{
          output_pin = 'F';
          output_pinm = 'G';
        }

      if ( itermcnt > 2)
	{
          printf("Error: Too many outputs for CHIP type = 'K'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }


 if (chip_type == 'L')
    {

      if ( itermcnt == 1)  // ED
	{
          output_pin = 'G';
          output_pinm = 'F';
        }

      if ( itermcnt == 2)   //JD
	{
          output_pin = 'H';
          output_pinm = 'I';
        }


      if ( itermcnt == 3)  // LD
	{
          output_pin = 'N';
          output_pinm = 'M';
        }

      if ( itermcnt == 4)   // CD
	{
          output_pin = 'A';
          output_pinm = 'B';
        }

      if ( itermcnt > 4)
	{
          printf("Error: Too many outputs for CHIP type = 'L'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'M')
    {

      if ( itermcnt == 1)  // MNA+JL+BC clock K
	{
          output_pin = 'E';
          output_pinm = 'D';
        }

      if ( itermcnt == 2)   //
	{
          output_pin = 'F';
          output_pinm = 'G';
        }


      if ( itermcnt == 3)  // 
	{
          output_pin = 'I';
          output_pinm = 'H';
        }

      if ( itermcnt > 3)
	{
          printf("Error: Too many outputs for CHIP type = 'M'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'N')
    {

      if ( itermcnt == 1)  // GFEAJ+DEAJ+NAJ+MLJ+CB clock K
	{
          output_pin = 'I';
          output_pinm = 'H';
        }

      if ( itermcnt > 1)
	{
          printf("Error: Too many outputs for CHIP type = 'N'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'O')
    {

      if ( itermcnt == 1)  // LD+Ad; k clock
	{
          output_pin = 'M';
          output_pinm = 'N';
        }

      if ( itermcnt == 2)  //   JD + Gd
	{
          output_pin = 'I';
          output_pinm = 'H';
        }

      if ( itermcnt == 3)  // BD + Cd
	{
          output_pin = 'F';
          output_pinm = 'E';
        }

      if ( itermcnt > 3)
	{
          printf("Error: Too many outputs for CHIP type = 'O'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'P')
    {

      if ( itermcnt == 1)  // 
	{
          output_pin = 'B';
          output_pinm = 'C';
        }

      if ( itermcnt == 2)  // 
	{
          output_pin = 'D';
          output_pinm = 'E';
        }

      if ( itermcnt > 2)
	{
          printf("Error: Too many outputs for CHIP type = 'P'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'Q')
    {

      if ( itermcnt == 1)  // LBA+CMA+CBN
	{
          output_pin = 'F';
          output_pinm = 'G';
        }

      if ( itermcnt == 2)  // LBA+CMA+CBN
	{
          output_pin = 'E';
          output_pinm = 'D';
        }

      if ( itermcnt == 3)  // JK
	{
          output_pin = 'I';
          output_pinm = 'H';
        }

      if ( itermcnt > 3)
	{
          printf("Error: Too many outputs for CHIP type = 'Q'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }


 if (chip_type == 'R')
    {

      if ( itermcnt == 1)  // 
	{
          output_pin = 'B';
          output_pinm = '*';
        }

      if ( itermcnt == 2)  // 
	{
          output_pin = 'A';
          output_pinm = '*';
        }

      if ( itermcnt == 3)  // 
	{
          output_pin = 'N';
          output_pinm = '*';
        }

      if ( itermcnt == 4)  // 
	{
          output_pin = 'M';
          output_pinm = '*';
        }

      if ( itermcnt > 4)
	{
          printf("Error: Too many outputs for CHIP type = 'R'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'S')
    {

      if ( itermcnt == 1)  // 
	{
          output_pin = 'A';
          output_pinm = 'Z';
        }

      if ( itermcnt > 1)
	{
          printf("Error: Too many outputs for CHIP type = 'S'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'T')
    {

      if ( itermcnt == 1)  // GLE+DFE+DLC+GFC  ; K clock
	{
          output_pin = 'B';
          output_pinm = 'A';
        }

      if ( itermcnt == 2)  // GLE+DFE+DLC+GFC
	{
          output_pin = 'M';
          output_pinm = 'N';
        }

      if ( itermcnt == 3)  // J
	{
          output_pin = 'H';
          output_pinm = 'I';
        }

      if ( itermcnt > 3)
	{
          printf("Error: Too many outputs for CHIP type = 'T'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'U')
    {

      if ( itermcnt == 1)  // 
	{
          output_pin = 'F';
          output_pinm = 'E';
        }

      if ( itermcnt == 2)  // 
	{
          output_pin = 'I';
          output_pinm = 'H';
        }


      if ( itermcnt > 2)
	{
          printf("Error: Too many outputs for CHIP type = 'U'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'V')
    {

      if ( itermcnt == 1)  // C
	{
          output_pin = 'B';
          output_pinm = 'A';
        }

      if ( itermcnt == 2)  // J
	{
          output_pin = 'I';
          output_pinm = 'H';
        }

      if ( itermcnt == 3)  // D/E
	{
          output_pin = 'F';
          output_pinm = 'G';
        }

      if ( itermcnt == 4)  // K/L
	{
          output_pin = 'M';
          output_pinm = 'N';
        }

      if ( itermcnt > 4)
	{
          printf("Error: Too many outputs for CHIP type = 'V'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'W')
    {

      if ( itermcnt == 1)  // 
	{
          output_pin = 'D';
          output_pinm = 'E';
        }

      if ( itermcnt == 2)  // 
	{
          output_pin = 'G';
          output_pinm = 'F';
        }

      if ( itermcnt == 3)  // 
	{
          output_pin = 'H';
          output_pinm = 'I';
        }

      if ( itermcnt == 4)  // 
	{
          output_pin = 'L';
          output_pinm = 'K';
        }

      if ( itermcnt == 5)  // 
	{
          output_pin = 'M';
          output_pinm = 'N';
        }

      if ( itermcnt == 6)  // 
	{
          output_pin = 'A';
          output_pinm = 'B';
        }

      if ( itermcnt > 6)
	{
          printf("Error: Too many outputs for CHIP type = 'W'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'X')
    {

      if ( itermcnt == 1)  // 
	{
          output_pin = 'E';
          output_pinm = 'D';
        }

      if ( itermcnt == 2)  // 
	{
          output_pin = 'F';
          output_pinm = 'G';
        }


      if ( itermcnt > 2)
	{
          printf("Error: Too many outputs for CHIP type = 'X'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'Y')
    {

      if ( itermcnt == 1)  // 
	{
          output_pin = 'E';
          output_pinm = 'G';
        }

      if ( itermcnt == 2)  // 
	{
          output_pin = 'D';
          output_pinm = 'F';
        }


      if ( itermcnt == 3)  // 
	{
          output_pin = 'C';
          output_pinm = 'N';
        }

      if ( itermcnt == 4)  // 
	{
          output_pin = 'B';
          output_pinm = 'A';
        }


      if ( itermcnt > 4)
	{
          printf("Error: Too many outputs for CHIP type = 'Y'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if (chip_type == 'Z')
    {

      if ( itermcnt == 1)  // 
	{
          output_pin = 'G';
          output_pinm = '*';   // do not put out
        }

      if ( itermcnt == 2)  // 
	{
          output_pin = 'H';
          output_pinm = '*';
        }

      if ( itermcnt == 3)  // 
	{
          output_pin = 'N';
          output_pinm = '*';
        }

      if ( itermcnt == 4)  // 
	{
          output_pin = 'A';
          output_pinm = '*';
        }


      if ( itermcnt > 4)

	{
          printf("Error: Too many outputs for CHIP type = 'Z'\n");
          printf("For line = %s \n", linein);
          exitnow(0);
        }
      
    }

 if ( alltermcnt < 5000 )
   {
     termarray[alltermcnt].chiptype = chip_type;

     strncpy(termarray[alltermcnt].locstr,locstr,10);
     strncpy(termarray[alltermcnt].term,thisterm,10);
     termarray[alltermcnt].output_pin = output_pin;
     termarray[alltermcnt].output_pinm = output_pinm;
     alltermcnt += 1;
   }

}


//
//
//
void handle_term_list()
{
  termcnt = 1;
  while(endofterms == FALSE)
    {
      getterm();
      if ((thisterm[0] == '*') && (thisterm[1] == '*') && (thisterm[2]=='*'))
	{
	  // termcnt +=1;
        }
      else
	{
         handle_term(termcnt);
        }
      termcnt += 1;
    }

}

//
//
//
rdinuptodot()


{
int dotfound;
int dotind;
int jj,k,j;
char *tempptr;

 dotfound = FALSE;
 k=0;
 while (( endoffile == FALSE) && ( dotfound == FALSE))
  {
   getaline(linein,file1);
   if (endoffile == FALSE)
    {
     if (debug) { printf("%s\n",linein); }
     linechar = 0;
     while(linein[linechar] != '\n')

      {
       if (k < MAXCIRC)
        {
         inbuff[k] = linein[linechar];
         k += 1;
         if (linein[linechar] == '.')
          {
           dotfound = TRUE;
           dotind = linechar +1;
          }
        }
        linechar += 1;
      }
     inbuff[k] = '\n';
     k += 1;
    }   /* not end of file */
   }


}    /* end read up to dot */


int main (int argc,char *argv[])

 {

   // printf("PKGMAP -  \n");

  file1 = fopen(argv[1],"r");
  if (file1 == NULL)
   {
    printf("Can't open the input board package file %s\n",argv[1]);
    exitnow(1);
   }

  file2 = fopen(argv[2],"w");

  if (file2 == NULL)
   {
    printf("Can't open the output file %s \n",argv[2]);
    exitnow(1);
   }

  for (i =0; i < 2000; i+= 1)
    {
      locusedarray[i]=0;
    }

  for (i =0; i < 70000; i+= 1)
    {
      boolusedarray[i]=0;
    }

  if (debug) { printf("File open ok %u \n",file1); }
  endoffile = FALSE;
  linenum = 1;
  linein[0] = 0;
  while(isalpha(linein[0]) == FALSE)  /* skip header */
   {
    getaline(linein,file1);
   }

  alltermcnt=0;

  while (endoffile==FALSE)
    {
     
      if (endoffile == FALSE)
       {
        getloc();
        getchiptype();

        endofterms=FALSE;

        handle_term_list();

       }

      readuptodot();
     }    /* while not end of file */


   fclose(file1);

   for(j=0; j < alltermcnt; j +=1 )
     {

       fprintf(file2,"%s%c%c %s \n", termarray[j].locstr,termarray[j].chiptype,
	       termarray[j].output_pin,termarray[j].term);


       if (isupper( termarray[j].term[0] ) )
	 {
	   flipterm[0] = tolower( termarray[j].term[0]);
           flipterm[1] = tolower( termarray[j].term[1]);
           flipterm[2] = tolower( termarray[j].term[2]);
           flipterm[3] = 0;
         }
       else
	 {
	   flipterm[0] = toupper( termarray[j].term[0]);
           flipterm[1] = toupper( termarray[j].term[1]);
           flipterm[2] = toupper( termarray[j].term[2]);
           flipterm[3] = 0;
         }

       if (termarray[j].output_pinm != '*')  // indicates no complement
	 {
       fprintf(file2,"%s%c%c %s \n", termarray[j].locstr,termarray[j].chiptype,
	       termarray[j].output_pinm,flipterm);
         }
     }
     
   fclose(file2);

 }  /* for all eight boards */
