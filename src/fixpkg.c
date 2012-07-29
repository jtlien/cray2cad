#include "stdio.h"
#include "string.h"
#include "stdlib.h"

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
char boolterm[100];

int termcnt;
char thisterm[100];

int lineindex;

char termlist[1000][10];
char prevterm[10];
int firstin;

void skipblanks()
 {
  while ((linein[lineindex] == ' ') && (lineindex <120))
    {
     lineindex += 1;
    }
 }  /* end of kip blanks  */


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

   firstin = FALSE;

   // printf("PKGMAP -  \n");

  file1 = fopen(argv[1],"r");
  if (file1 == NULL)
   {
    printf("Can't open the input board package file %s\n",argv[1]);
    exit(1);
   }

  file2 = fopen(argv[2],"w");

  if (file2 == NULL)
   {
    printf("Can't open the output file %s \n",argv[2]);
    exit(1);
   }

  termcnt = 0;
  prevterm[0] = 0;


  endoffile = FALSE;
  linenum = 1;
  linein[0] = 0;

  while(endoffile==FALSE)  /* skip header */
   {
    getaline(linein,file1);
    lineindex= 0;
    getterm();

    i = 0;
    while((linein[lineindex] != ' ' ) && (linein[lineindex] != 26  ))
      {
        boolterm[i] = linein[lineindex];
        
        lineindex+=1;
        i+=1;
      }

    boolterm[i] = 0;

    // printf("Thisterm = %s boolterm = %s \n",thisterm, boolterm);
    skipblanks();


    if (strncmp(thisterm,prevterm,10)==0)
      {
       strncpy(termlist[termcnt],boolterm,10);
        termcnt += 1;
      }
    else
      {

	if (firstin == TRUE)
	  {
           fprintf(file2,"%s - ", prevterm);
           for(j=0; j < termcnt; j += 1)
	    {
	     fprintf(file2,"%s ",termlist[j]);
             }
           fprintf(file2,".\n");
          }
	else
          {
            firstin=TRUE;
          }

        termcnt = 0;
        strncpy(termlist[termcnt],boolterm,10);
        termcnt = 1;
      }
    strncpy(prevterm,thisterm,10);

   }

   fprintf(file2,"%s - ", prevterm);
   for(j=0; j < termcnt; j += 1)
      {
	    fprintf(file2,"%s ",termlist[j]);
      }
    fprintf(file2," . \n");
       
   fclose(file1);
   fclose(file2);

 }  /* for all eight boards */
