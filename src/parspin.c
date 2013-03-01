
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define EOLN '\n'
#define CHARSPER 300
#define MAXCIRC 2024
#define MAXBOOL 8000
#define MAXPKGS 8000
#define MAXLOADS 24  // loads on each boolean
#define MAXRJMPS  5000   // at most 96 * 12 * 4  raw jumpers
#define MAXBRDJMPS 1000
#define MAXJMPS  100      // 96 
#define MAXPINPERMOD 20
#define MAXBOARD 8
#define CHIPHSPACE  600   // mils between chip columns
#define CHIPVSPACE  500   // mils between chip rows

#define TRUE 1
#define FALSE 0

FILE *file1,*file2,*file3,*file4,*file5,*fopen();
FILE *file6;
FILE *outfile;
FILE *outfile1,*outfile2,*outfile3,*outfile4,*outfile5,*outfile6;
FILE *outfile7,*outfile8;
FILE *outfilea,*outfileb;
FILE *outfilec;



/*******************************************************************/
/*                                                                 */
/*                    pkgpin = package output pin                  */
/*                    bool = boolean term                          */
/*                                                                 */
/*******************************************************************/

int jmpxval;
int jmpyval;
int jumpernum;

int groupcnt;
int endofterms;

int rchipcnt;
int schipcnt;
int argcnt;
int alltermcnt;
char outstr[20];
char locstr[10];
char chip_type;
int termcnt;

int output_index;   // index into output_array
char output_pin;
char output_pinm;
int pkgindex;

int locindex;
int *inptr[10];
int rdindex;          /* read index */
char boolbuf;
char linein[400];
char inbuff[MAXCIRC];          /* holds an entire circuit  */
int buffindex;
int linenum;
int endoffile;
int linechar;
int blankline;
int debug;
int dotfound;
int outputcnt;  // count of all boolean outputs found in xxpkg file
int bufflen;

char srcpin;

int i,j,k,l,m,n;
int infile;
int pkgcnt;

int lineindex;

char firstbool[20];

char pinlist[40][10];   // list of pins on a net

char* outterms_srt[1000];

char* interms_srt[1000];

//
//  Current read in boolean equation groups of terms
//
struct termstuff
{
  char bool[10];
  char pin;
  char pinnum;

} term_array[40];

//
// package info
//
struct pkgstuff
{
  char locstr[10];
  char chiptype;
  int xloc;
  int yloc;
  int boardnum;
  char pinterms[MAXPINPERMOD][10];
  int pinisout[MAXPINPERMOD];          // flag to indicate output pin
  int pinio[MAXPINPERMOD];             // used to indicate input or output
}  pkgarray[MAXPKGS];

struct loadtype
{
  char locstr[5];
  char chiptype;
  char srcpin;
  int pinx;
  int piny;
  int pinnum;
  int brdnum;

} loadexamp;

//
//  Nets from each output
//
struct outputstuff
{
  char bool[10];
  char loc[10];       // location string
  char srcpin;
  int srcbrd;        // board the source is on
  int isinput;
  char chip_type;   // the chip output type for this output
  int pkgindex;
  int pinnum;
  char pinstr[10];
  int foundinbool;
  int maxbrd;
  int minbrd;
  int optx;           // best location for jumper
  int opty;  
  int jmpneeded;
  int loadcnt;
  struct loadtype loadlist[MAXLOADS];  
           // 16*pkgindex + loadpin or jmppin or if > 20000
                      //  24 * connector + con pin  + 20000
} output_array[MAXBOOL];

int brd1jmpcnt;
int brd2jmpcnt;
int brd3jmpcnt;
int brd4jmpcnt;
int brd5jmpcnt;
int brd6jmpcnt;
int brd7jmpcnt;
int brd8jmpcnt;

struct brdjmptype
{
  char bool[10];
  int jmpnum;
} brdjmpexample;

// arrays to hold bool terms and their corresponding jumpers
//
struct brdjmptype board1_jmps[MAXBRDJMPS];
struct brdjmptype board2_jmps[MAXBRDJMPS];
struct brdjmptype board3_jmps[MAXBRDJMPS];
struct brdjmptype board4_jmps[MAXBRDJMPS];
struct brdjmptype board5_jmps[MAXBRDJMPS];
struct brdjmptype board6_jmps[MAXBRDJMPS];
struct brdjmptype board7_jmps[MAXBRDJMPS];
struct brdjmptype board8_jmps[MAXBRDJMPS];

char inlistarray[1000][10];
char outlistarray[1000][10];

int vincnt;
int voutcnt;
char tstr[10];

//
// inter board jumper data  from read in file
//               file has  [A,B,C][A-Z,1-8][O--W] integer[0-255] term
//              example    ACO 7 SCO  whould have boolean term
//                           SCO jumping from board 6 to board 8 at
//                               location AC and pin O

struct jumperstuff
{
  char locstr[10];
  char pinchar;    // pins o,p,q,r,s,t,u,v,w indicate pin
  int  pkgind;
  int xloc;
  int yloc;
  char term[10];   // boolean term
  int usedmask;      // 11000000 indicates boards 1 and 2 connected
} jmpdata[20000];

struct jumplocstuff
{
  char locstr[10];
  int xloc;
  int yloc;
  int availmask;  
} jmplocs[5000];


struct conlocstuff
{
  char locstr[10];  // ZA..ZX
  int brdnum;       // 1..8
  int xloc;
  int yloc;
  char boolterms[26][26];
} conlocs[30];             // needed for netlists for 8 x 3 = 24 connectors

int jmpgoodcnt;       // count of good jumpers found

int jmpgood[1200];  // used to mark which jumper positions are still good

//
// Jumpers arranged by associated chip packages
//
struct pkgjumpstuff
{
  char locstr[10];
  int  jmpcnts[12];        // number of jumpers at each pin
  int  layermasks[12];   // layermasks for each pin [O..Z]
  int  layerused[12][4];  // shows up to four jumpers at each pin
  char boolterms[12][4][10];  // corresponding boolean terms 
} pkgjmparray[MAXPKGS];

int locusedarray[2000];

int pinx;
int piny;
int conpinx;
int conpiny;

int pinnum;
char plocstr[10];    // used for optimal jumper placement
int ppinnum;

int xtotal;
int ytotal;
int dist;
int bestjmp;
int bestdist;
char bjmplocstr[10];   // location string for best jumper

void exitnow(int exitval)
{
  printf("Would exit here \n");
  exit(exitval);
}

//
//  get a line from current input file
//

void getaline(char *linein,FILE *infile)

{
int i;
char *fstat;
char *lineptr;

i = 1;
fstat = fgets(linein,CHARSPER,infile);
 lineptr = &linein[0];
 if (*lineptr == 12)  // if null line
 {
  fstat = fgets(linein,CHARSPER,infile);
  lineptr = &linein[0];
 }
if ((fstat==NULL) || ( *lineptr == 3) || (*lineptr == 26))
 {
   
  endoffile=TRUE;
  return;
 }
 i = 0;
 //
 // detab
 //
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

void readuptodot(FILE *infile)


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
   getaline(linein,infile);
  }
  inbuff[k] = 0;
 bufflen = k;
}    /* end read up to dot */

void skippasteq()
{

  while((linein[lineindex] != '=') && (lineindex < CHARSPER))
    {
      lineindex += 1;
    }
  if(linein[lineindex]== '=')
    {
    }
  else
    {
      printf("Expect '=' \n");
    }
    lineindex += 1;
   
}

void skippastplus()
{

  while((linein[lineindex] != '+') && (lineindex < CHARSPER))
    {
      lineindex += 1;
    }
  if(linein[lineindex]== '+')
    {
    }
  else
    {
      printf("Expect '+' \n");
    }
    lineindex += 1;
   
}

void skipblanks()
 {
  while ((linein[lineindex] == ' ') && (lineindex <CHARSPER))
    {
     lineindex += 1;
    }
 }  /* end of skip blanks  */

void skipblanksandslash()
 {
   while (((linein[lineindex] == ' ') || (linein[lineindex] == '/' )) && (lineindex <CHARSPER))
    {
     lineindex += 1;
    }

 }  /* end of skip blanks and dash  */

void skipdashes()
 {
 
  while ((linein[lineindex] == '-') && (lineindex <CHARSPER))
    {
     lineindex += 1;
    }
 }  /* end of skip dashes  */

//
//
//
char *flipcase(char *instr)
{
  
  int k;

  strncpy(outstr,instr,20);

  if (isupper(instr[0] ) )  // if upper
    {
     for(k=0; k < strlen(instr); k +=1)
       {
	 outstr[k] = tolower(instr[k] );  // if upper, to lower
       }
     }
    else
     {
      for(k=0; k < strlen(instr); k +=1)   
       {
         outstr[k] = toupper( instr[k] );   // else if lower, toupper
        }
     }	
    return(outstr);
	
} // flipcase
		   
//
//   select output file by board number
//
void select_brd_file( int brdnum)
{

  if(brdnum == 1 )
    {
      outfile = outfile1;
    }

  if(brdnum == 2 )
    {
      outfile = outfile2;
    }

  if(brdnum == 3 )
    {
      outfile = outfile3;
    }

  if(brdnum == 4 )
    {
      outfile = outfile4;
    }

  if(brdnum == 5 )
    {
      outfile = outfile5;
    }

  if(brdnum == 6 )
    {
      outfile = outfile6;
    }

  if(brdnum == 7 )
    {
      outfile = outfile7;
    }

  if(brdnum == 8 )
    {
      outfile = outfile8;
    }
   
}  // select_brd_file

//
//    put out a jumper in the kicad netlist format
//
void out_jumper_kicadnet( char *boolterm, int jmpnum)
{


  fprintf(outfile," ( /4AC2PCA3 VPIN J%d VPIN  \n",jmpnum);
  fprintf(outfile,"  ( 1  %s ) \n", boolterm);
  fprintf(outfile," ) \n");

}

//
//
//
void out_all_kicadjmps()
{
  int i;

  //printf("In out_all_kicadjmps \n");

  outfile = fopen("abcjmps","w");
  if (outfile == NULL)
   {
    printf("Can't open the abc board jumpers  file %s\n","abcjmps");
    exitnow(1);
   }
  for(i=0; i < brd1jmpcnt; i +=1)
    {
      out_jumper_kicadnet( board1_jmps[i].bool, board1_jmps[i].jmpnum);
    }

  fclose(outfile);

  outfile = fopen("defjmps","w");
  if (outfile == NULL)
   {
    printf("Can't open the def board jumpers  file %s\n","defjmps");
    exitnow(1);
   }
  for(i=0; i < brd2jmpcnt; i +=1)
    {
      out_jumper_kicadnet( board2_jmps[i].bool, board2_jmps[i].jmpnum);
    }

  fclose(outfile);

  outfile = fopen("ghijmps","w");
  if (outfile == NULL)
   {
    printf("Can't open the ghi board jumpers  file %s\n","ghijmps");
    exitnow(1);
   }
  for(i=0; i < brd3jmpcnt; i +=1)
    {
      out_jumper_kicadnet( board3_jmps[i].bool, board3_jmps[i].jmpnum);
    }

  fclose(outfile);

  outfile = fopen("jkljmps","w");
  if (outfile == NULL)
   {
    printf("Can't open the jkl board jumpers  file %s\n","jkljmps");
    exitnow(1);
   }
  for(i=0; i < brd4jmpcnt; i +=1)
    {
      out_jumper_kicadnet( board4_jmps[i].bool, board4_jmps[i].jmpnum);
    }

  fclose(outfile);

  outfile = fopen("mnojmps","w");
  if (outfile == NULL)
   {
    printf("Can't open the mno board jumpers  file %s\n","mnojmps");
    exitnow(1);
   }
  for(i=0; i < brd5jmpcnt; i +=1)
    {
      out_jumper_kicadnet( board5_jmps[i].bool, board5_jmps[i].jmpnum);
    }

  fclose(outfile);

  outfile = fopen("pqrjmps","w");
  if (outfile == NULL)
   {
    printf("Can't open the pqr board jumpers  file %s\n","pqrjmps");
    exitnow(1);
   }
  for(i=0; i < brd6jmpcnt; i +=1)
    {
      out_jumper_kicadnet( board6_jmps[i].bool, board6_jmps[i].jmpnum);
    }

  fclose(outfile);

  outfile = fopen("stujmps","w");
  if (outfile == NULL)
   {
    printf("Can't open the stu board jumpers  file %s\n","stujmps");
    exitnow(1);
   }
  for(i=0; i < brd7jmpcnt; i +=1)
    {
      out_jumper_kicadnet( board7_jmps[i].bool, board7_jmps[i].jmpnum);
    }

  fclose(outfile);

  outfile = fopen("vwxjmps","w");
  if (outfile == NULL)
   {
    printf("Can't open the vwx board jumpers  file %s\n","vwxjmps");
    exitnow(1);
   }
  for(i=0; i < brd8jmpcnt; i +=1)
    {
      out_jumper_kicadnet( board8_jmps[i].bool, board8_jmps[i].jmpnum);
    }

  fclose(outfile);


}

void out_acon_kicad( int connum, FILE *outfile)
{
  int j;

  fprintf(outfile," ( /4AC2PCA3 C24PIN P%d C24PIN  \n",(connum%3)+1);

  for (j = 1; j < 25; j +=1)
    {
      if (conlocs[connum].boolterms[j][0] != 0 )
        {
	  fprintf(outfile,"   ( %d %s ) \n",j,conlocs[connum].boolterms[j]);
	}
      else
        {
          fprintf(outfile,"   ( %d ? ) \n",j);   // no connection
        }
    }
  fprintf(outfile," ) \n");
}

void out_cons_kicad( int brdnum, FILE *outfile)
{
  int i;
  
  i = 3*brdnum;
  out_acon_kicad( i, outfile );
  i += 1;
  out_acon_kicad( i, outfile);
  i += 1;
  out_acon_kicad( i, outfile);
}

//
//  Output a jumper module for kicad
//
void out_jumper_kicad( int brdnum, int jmpxval,int jmpyval, int jnum)
{
char quotchar;
 FILE *outfile;

 if (brdnum == 0)
   {
     outfile = outfile1;
   }

 if (brdnum == 1)
   {
     outfile = outfile2;
   }

 if (brdnum == 2)
   {
     outfile = outfile3;
   }

 if (brdnum == 3)
   {
     outfile = outfile4;
   }

 if (brdnum == 4)
   {
     outfile = outfile5;
   }

 if (brdnum == 5)
   {
     outfile = outfile6;
   }

 if (brdnum == 6)
   {
     outfile = outfile7;
   }

 if (brdnum == 7)
   {
     outfile = outfile8;
   }

 if (brdnum == 8)
   {
     printf("Board assigned to 8 \n");
   }

quotchar = '"';

fprintf(outfile,"$MODULE c-jmp\n");
fprintf(outfile,"Po %d0 %d0 0 15 4FDFA209 4FDFA255 ~~\n",jmpxval,jmpyval);
fprintf(outfile,"Li c-jmp\n");
fprintf(outfile,"Cd module 1 pin (ou trou mecanique de percage)\n");
fprintf(outfile,"Kw DEV\n");
fprintf(outfile,"Sc 4FDFA255\n");
fprintf(outfile,"AR c-jmp\n");
fprintf(outfile,"Op 0 0 0\n");
fprintf(outfile,"T0 -500 -450 400 100 0 20 N V 21 N %cc-jmp%c \n",quotchar,
                  quotchar);
fprintf(outfile,"T1 0 600 400 200 0 40 N I 21 N %cj%d%c\n",quotchar,jnum,quotchar);
fprintf(outfile,"DC 0 0 200 -100 80 21\n");
fprintf(outfile,"$PAD\n");
fprintf(outfile,"Sh %c1%c C 400 400 0 0 0\n",quotchar,quotchar);
fprintf(outfile,"Dr 200 0 0\n");
fprintf(outfile,"At STD N 00E0FFFF\n");
fprintf(outfile,"Ne 0 %c%c\n",quotchar,quotchar);
fprintf(outfile,"Po 0 0\n");
fprintf(outfile,"$EndPAD\n");
fprintf(outfile,"$EndMODULE  c-jmp\n");

}  // end out_jumper_kicad

//
//  Translate pkg pin label to package pin number for s chip
//
int pinxlate_s( char inchar)
{
  int retval;

  retval = 0;

  if (inchar == 'A')
    {
      retval = 1;
    }

  if (inchar == 'B')
    {
      retval = 2;
    }

  if (inchar == 'C')
    {
      retval = 3;
    }

  if (inchar == 'D')
    {
      retval = 5;
    }

  if (inchar == 'E')
    {
      retval = 6;
    }

  if (inchar == 'F')
    {
      retval = 7;
    }

  if (inchar == 'G')
    {
      retval = 8;
    }
  if (inchar == 'H')
    {
      retval = 9;
    }

  if (inchar == 'I')
    {
      retval = 10;
    }

  if (inchar == 'J')
    {
      retval = 11;
    }

  if (inchar == 'K')
    {
      retval = 13;
    }

  if (inchar == 'L')
    {
      retval = 14;
    }

  if (inchar == 'M')
    {
      retval = 15;
    }

  if (inchar == 'N')
    {
      retval = 16;
    }
  return(retval);

} 

//
//  Put out a group of pins in verilog format
//
void print_veri_pingroupa( int length,int *pin_list, int ipkgind )
{
  int i;
  int firstin;
  char  pins[20][10];
 

   for(i=1;i<17;i++)
    {
      strncpy(pins[i],pkgarray[ipkgind].pinterms[i],10);
    }

   i =0;
  firstin=1;

  while( i < length)
    {
      if (pins[pin_list[i]][0] != 0 )
        {
	  if (firstin)
	    {
	      fprintf(outfilea," %s ",pins[pin_list[i]]);
              firstin=0;
            }
          else
	    {
	      fprintf(outfilea,"& %s ",pins[pin_list[i]]);
            }
        }
     i += 1;
    }     

}  // print_veri_pingroupa

//
//  Put out a group of pins in verilog format
//
void print_veri_pingroupb( int length,int *pin_list, int ipkgind )
{
  int i;
  int firstin;
  char  pins[20][10];
 

   for(i=1;i<17;i++)
    {
      strncpy(pins[i],pkgarray[ipkgind].pinterms[i],10);
    }

   i =0;
  firstin=1;

  while( i < length)
    {
      if (pins[pin_list[i]][0] != 0 )
        {
	  if (firstin)
	    {
	      fprintf(outfileb," %s ",pins[pin_list[i]]);
              firstin=0;
            }
          else
	    {
	      fprintf(outfileb,"& %s ",pins[pin_list[i]]);
            }
        }
     i += 1;
    }     

}  // print_veri_pingroupb

//
//
//
int distance_m(int x1, int y1, int x2, int y2)
{
  int dist;

  dist = (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
  
}

//
//  Translate pkg pin label to package pin number
//
int pinxlate( char inchar)
{
  int retval;

  retval = 0;

  if (inchar == 'A')
    {
      retval = 1;
    }

  if (inchar == 'B')
    {
      retval = 2;
    }

  if (inchar == 'C')
    {
      retval = 3;
    }

  if (inchar == 'D')
    {
      retval = 5;
    }

  if (inchar == 'E')
    {
      retval = 6;
    }

  if (inchar == 'F')
    {
      retval = 7;
    }

  if (inchar == 'G')
    {
      retval = 8;
    }
  if (inchar == 'H')
    {
      retval = 9;
    }

  if (inchar == 'I')
    {
      retval = 10;
    }

  if (inchar == 'J')
    {
      retval = 11;
    }

  if (inchar == 'K')
    {
      retval = 13;
    }

  if (inchar == 'L')
    {
      retval = 14;
    }

  if (inchar == 'M')
    {
      retval = 15;
    }

  if (inchar == 'N')
    {
      retval = 16;
    }

  if (inchar == 'O')
    {
      retval = 17;
    }

  if (inchar == 'P')
    {
      retval = 18;
    }
  return(retval);

} 

//
//  Translate pin number to pin char
//
char lookuppin( int inval)
{
  char retchar;

  retchar = '-';

  if (inval == 1)
    {
      retchar = 'A';
    }
 
  if (inval == 2)
    {
      retchar = 'B';
    }
 
  if (inval == 3)
    {
      retchar = 'C';
    }
 
  if (inval == 4)
    {
      retchar = '-';
    }
 
  if (inval == 5)
    {
      retchar = 'D';
    }
 
  if (inval == 6)
    {
      retchar = 'E';
    }
 
   if (inval == 7)
    {
      retchar = 'F';
    }
 
  if (inval == 8)
    {
      retchar = 'G';
    }
 
  if (inval == 9)
    {
      retchar = 'H';
    }
 
  if (inval == 10)
    {
      retchar = 'I';
    }
 
  if (inval == 11)
    {
      retchar = 'J';
    }
 
  if (inval == 12)
    {
      retchar = 'K';
    }
 
  if (inval == 13)
    {
      retchar = '-';
    }
 
  if (inval == 14)
    {
      retchar = 'L';
    }
 
  if (inval == 15)
    {
      retchar = 'M';
    }
 
  if (inval == 16)
    {
      retchar = 'N';
    }
  return(retchar);

} 

//
//   Chip pin to x,y location  clocstr = loc[0..1],pin[0]
//       good for chip pins and jumpers
//
void cpin_to_xy( char *clocstr)
{
  int xbase;
  int ybase;
  int cgroup;  // group of 32 chips, 0 .. 23
  int cbgroup;    // group of chips on board, 0..2
  int row;
  int rowcount;
  char pinchar;

  cgroup = clocstr[0] - 'A';

  cbgroup = cgroup % 3;

  xbase = 2200;
  ybase = 2150;

  if (cbgroup == 1)
    {
      xbase = xbase + (4 *  CHIPHSPACE);
    }

  if (cbgroup == 2)
    {
      xbase = xbase + (8 *  CHIPHSPACE);
    }

  if (isalpha(clocstr[1]))
    {
      rowcount = clocstr[1] - 'A';
      xbase = xbase + (rowcount % 4) * CHIPHSPACE;
      row = rowcount / 4;
      ybase = ybase + row * CHIPVSPACE;
     }
  else
    {
      rowcount = clocstr[1] - '1' + 24;
      xbase = xbase + (rowcount % 4) * CHIPHSPACE;
      row = rowcount / 4;
      ybase = ybase + (row * CHIPVSPACE);
     
    }

  pinchar = clocstr[2];
  
  if (pinchar == 'A')
    {
      pinx = xbase;
      piny = ybase;
    }
  
  if (pinchar == 'B')
    {
      pinx = xbase-50;
      piny = ybase+50;
    }

  if (pinchar == 'C')
    {
      pinx = xbase;
      piny = ybase+100;
    }

  if (pinchar == 'D')
    {
      pinx = xbase;
      piny = ybase+200;
    }

  if (pinchar == 'E')
    {
      pinx = xbase-50;
      piny = ybase+250;
    }

  if (pinchar == 'F')
    {
      pinx = xbase;
      piny = ybase+300;
    }

  if (pinchar == 'G')
    {
      pinx = xbase-50;
      piny = ybase+350;
    }

  if (pinchar == 'H')
    {
      pinx = xbase+350;
      piny = ybase+350;
    }
  
  if (pinchar == 'I')
    {
      pinx = xbase+400;
      piny = ybase+300;
    }

  if (pinchar == 'J')
    {
      pinx = xbase+350;
      piny = ybase-250;
    }

  if (pinchar == 'K')
    {
      pinx = xbase+350;
      piny = ybase+150;
    }

  if (pinchar == 'L')
    {
      pinx = xbase+400;
      piny = ybase+100;
    }

  if (pinchar == 'M')
    {
      pinx = xbase+350;
      piny = ybase+50;
    }

  if (pinchar == 'N')
    {
      pinx = xbase+400;
      piny = ybase;
    }

  if (pinchar == 'O')   // jumpers
    {
      pinx = xbase+500;
      piny = ybase;
    }

  if (pinchar == 'P')
    {
      pinx = xbase+600;
      piny = ybase;
    }

  if (pinchar == 'Q')
    {
      pinx = xbase+550;
      piny = ybase+50;
    }

  if (pinchar == 'R')
    {
      pinx = xbase+500;
      piny = ybase+100;
    }

  if (pinchar == 'S')
    {
      pinx = xbase+600;
      piny = ybase+100;
    }

  if (pinchar == 'T')
    {
      pinx = xbase+550;
      piny = ybase+150;
    }

  if (pinchar == 'U')
    {
      pinx = xbase+500;
      piny = ybase+200;
    }

  if (pinchar == 'V')
    {
      pinx = xbase+600;
      piny = ybase+200;
    }

  if (pinchar == 'W')
    {
      pinx = xbase+550;
      piny = ybase+250;
    }

  if (pinchar == 'X')
    {
      pinx = xbase+500;
      piny = ybase+300;
    }

  if (pinchar == 'Y')
    {
      pinx = xbase+600;
      piny = ybase+300;
    }

  if (pinchar == 'Z')
    {
      pinx = xbase+550;
      piny = ybase+350;
    }
}

//
//   Connector pin to x,y location  clocstr = Z,loc[1],pin[0]
//       
//
void ccon_to_xy( char *clocstr)
{
  int xbase;
  int ybase;
  int congroup;
  int conbgroup;
  char pinchar;

  xbase = (12 * CHIPHSPACE);
  ybase = 2150;

  congroup = clocstr[1]-'A';   // 0 ..23
  conbgroup = congroup % 3;      // 0..2

  if (conbgroup == 1)
    {
      ybase = ybase + 1250;
    }

  if (conbgroup == 2)
    {
      ybase = ybase + 2500;
    }
  pinchar = locstr[2];
  
  if (pinchar == 'A')
    {
      conpinx = xbase+50;
      conpiny = ybase;
    }
  
  if (pinchar == 'B')
    {
      conpinx = xbase;
      conpiny = ybase+50;
    }

  if (pinchar == 'C')
    {
      conpinx = xbase+50;
      conpiny = ybase+100;
    }

  if (pinchar == 'D')
    {
      conpinx = xbase;
      conpiny = ybase+150;
    }

  if (pinchar == 'E')
    {
      conpinx = xbase+50;
      conpiny = ybase+200;
    }

  if (pinchar == 'F')
    {
      conpinx = xbase;
      conpiny = ybase+250;
    }

  if (pinchar == 'G')
    {
      conpinx = xbase+50;
      conpiny = ybase+300;
    }

  if (pinchar == 'H')
    {
      conpinx = xbase;
      conpiny = ybase+350;
    }
  
  if (pinchar == 'I')
    {
      conpinx = xbase+50;
      conpiny = ybase+400;
    }

  if (pinchar == 'J')
    {
      conpinx = xbase;
      conpiny = ybase+450;
    }

  if (pinchar == 'K')
    {
      conpinx = xbase+50;
      conpiny = ybase+500;
    }

  if (pinchar == 'L')
    {
      conpinx = xbase;
      conpiny = ybase+550;
    }

  if (pinchar == 'M')
    {
      conpinx = xbase+50;
      conpiny = ybase-600;
    }

  if (pinchar == 'N')
    {
      conpinx = xbase;
      conpiny = ybase+650;
    }

  if (pinchar == 'O')
    {
      conpinx = xbase+50;
      conpiny = ybase+700;
    }

  if (pinchar == 'P')
    {
      conpinx = xbase;
      conpiny = ybase+750;
    }

  if (pinchar == 'Q')
    {
      conpinx = xbase+50;
      conpiny = ybase+800;
    }

  if (pinchar == 'R')
    {
      conpinx = xbase;
      conpiny = ybase+850;
    }

  if (pinchar == 'S')
    {
      conpinx = xbase+50;
      conpiny = ybase+900;
    }

  if (pinchar == 'T')
    {
      conpinx = xbase;
      conpiny = ybase+950;
    }

  if (pinchar == 'U')
    {
      conpinx = xbase+50;
      conpiny = ybase+1000;
    }

  if (pinchar == 'V')
    {
      conpinx = xbase;
      conpiny = ybase+1050;
    }

  if (pinchar == 'W')
    {
      conpinx = xbase+50;
      conpiny = ybase+1100;
    }

  if (pinchar == 'X')
    {
      conpinx = xbase;
      conpiny = ybase+1150;
    }

}

//
//  Go thru pin lists and find how big a jumper is needed in terms
//          how many boards needed to be crosses
//
int jumper_needed( int pincount)
{
  int j;
  char jlocstr[10];
  int brd;
  int jneeded;
  int minboard;
  int mxboard;
  int shiftbit;
  int shiftcnt;
  int minbit;
  int maxbit;
  int needbits;

  mxboard = -1;
  minboard = MAXBOARD+1;

  for(j=0; j < pincount; j += 1)
    {
      strncpy(jlocstr,pinlist[j],10);
      if (jlocstr[0] == 'Z')     // connector
        {
	  brd = (jlocstr[1] - 'A')/3;   // brd = 0-7
	}
      else   // chip pin
        {
	  brd = (jlocstr[0] - 'A')/3;
        }
      if (brd > mxboard)
	{
          mxboard = brd;
        }
      if (brd < minboard)
	{
	  minboard = brd;
        }

      minbit = 1 << minboard;
      maxbit = 1 << mxboard;
      shiftbit = minbit;

      needbits = 0;
      shiftcnt = 0;

      while(( shiftbit != maxbit) && (shiftcnt < 8))
        {
          needbits = shiftbit | needbits;
	  shiftbit = shiftbit << 1;
          shiftcnt += 1;
        }
    }

  return(needbits);

}

//
//  Check if a mask specifying jumpers between boards is valid
//

int isvalidmask( int inmask)
{
 int valid;

 valid = FALSE;

   if ((inmask == 3) || (inmask == 6) || (inmask == 12) || (inmask == 24)
    || (inmask == 48) || (inmask == 96) || (inmask == 192))
    {
     valid = TRUE;
   }

   if ((inmask == 7) || (inmask == 14) || (inmask == 28) || (inmask == 56)
    || (inmask == 112) || (inmask == 224) )
    {
     valid = TRUE;
    }

   if ((inmask == 15) || (inmask == 30) || (inmask == 60) || (inmask == 120)
    || (inmask == 240))
    {
     valid = TRUE;
   }

   if ((inmask == 31) || (inmask == 62) || (inmask == 124) || (inmask == 186))

    {
     valid = TRUE;
   }

   if ((inmask == 63) || (inmask == 126) || (inmask == 252) )

    {
     valid = TRUE;
   }

   if ((inmask == 127) || (inmask == 254) || (inmask == 255) )

    {
     valid = TRUE;
   }

 return(valid);
} 
//
// check location to see if already used, calculate index into array
//
int checkloc( char *inlocstr)
{
  
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
      if (isdigit ( inlocstr[1]) && inlocstr[1] != '9' && inlocstr[1] != '0')
	{
         lowval = inlocstr[1]-'0'+24;
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

}   // checkloc

//
//  location string -> index
//
int handleloc( char *inlocstr)
{
  
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
      if (isdigit (( inlocstr[1])) && (inlocstr[1] != '9') && (inlocstr[1] != '0'))
	{
         lowval = inlocstr[1]-'0'+23;
        }
      else
	{
          printf(" loc[1] = %d \n", inlocstr[1]);
          printf("Unexpected character in loc = %s \n",inlocstr);
        }
    }

  locindex = (32 * baseval) + lowval;

  return(locindex);

}   //

//
//  for each group of terms, apply the terms to the package 
//                
void apply_terms_to_pkg(int intermcnt, struct termstuff *term_array,
                          int goodcnt,int thispkgind)
{
  int j;
  char termpinch;
  int termpin;
  int lowerflag;

  if (intermcnt > goodcnt)
    {
      printf("Too many terms in group in line %s \n",linein);
      printf(" terms parsed %d expected = %d \n", intermcnt,goodcnt);
    }

  
  for(j=0; j < intermcnt; j +=1 )
    {
      termpinch = term_array[j].pin;
      lowerflag = FALSE;
      if (islower(termpinch))
        {
          termpinch = toupper(termpinch);
          lowerflag = TRUE;               // invert term_array value
        }

      termpin = pinxlate( termpinch);  // get integer for pin number

      //printf("Termpin = %d from termchar = %c \n",termpin,termpinch);
      if (termpin< MAXPINPERMOD)
	{
          if (strlen(pkgarray[thispkgind].pinterms[termpin])== 0) // nothing there yet
	    {
	      // printf("Setting a termpin %d in %d pkg with %s \n",
	      // termpin, thispkgind,term_array[j].bool);

	   strncpy(pkgarray[thispkgind].pinterms[termpin],term_array[j].bool,10);
            }
          else
	    {
              if (lowerflag)
                {
                  if (isupper( term_array[j].bool[0] ) )  // if upper
		    {
                      for(k=0; k < strlen(term_array[j].bool); k +=1)
			{
                          term_array[j].bool[k] = tolower( term_array[j].bool[k] );
                        }
		    }
                  else
                    {
			
                      for(k=0; k < strlen(term_array[j].bool); k +=1)
			{
                          term_array[j].bool[k] = toupper( term_array[j].bool[k] );
                        }
		    }				   
		}

              if (strncmp(pkgarray[thispkgind].pinterms[termpin],
			  term_array[j].bool,10) == 0 )
                {
                }
              else
                {
                  if (term_array[j].bool[0] == 0 ) // group has too few terms
                    {
		      strncpy(term_array[j].bool,"ZZI",10);  // make forced 
                      strncpy(pkgarray[thispkgind].pinterms[termpin],"ZZI",10);
                    }
                  else
		    {
		      if (pkgarray[thispkgind].chiptype != 'U')
			{
		      printf("Pkg error for pkg %s with term %s overwriting %s for pin %d \n",
                           pkgarray[thispkgind].locstr, 
			     term_array[j].bool,
                             pkgarray[thispkgind].pinterms[termpin],
			     termpin);
			}
		    }
                }
            }

        }

    } 

} //  apply_terms_to_pkg

//
//  Add a load to an bool output
//
void add_new_load(int outarrayind, char *loadstr, int brdnum, char chiptype, int pinnum,char pinchar)
{
  int srcbrd;

  if (outarrayind < MAXBOOL   )
    {
      if (output_array[outarrayind].loadcnt <  MAXLOADS )
	{
          j = output_array[outarrayind].loadcnt;
	  strncpy( output_array[outarrayind].loadlist[j].locstr, loadstr,10);
          output_array[outarrayind].loadlist[j].pinnum = pinnum;
          output_array[outarrayind].loadlist[j].chiptype = chiptype;
          output_array[outarrayind].loadlist[j].brdnum = brdnum;
          output_array[outarrayind].loadlist[j].srcpin = pinchar; 
          cpin_to_xy( loadstr);

          output_array[outarrayind].loadlist[j].pinx = pinx;
          output_array[outarrayind].loadlist[j].piny = piny;

          srcbrd = output_array[outarrayind].srcbrd;

          if (brdnum < output_array[outarrayind].minbrd)
            {
	      output_array[outarrayind].minbrd = brdnum;
            }
          if (brdnum > output_array[outarrayind].maxbrd)
            {
	      output_array[outarrayind].maxbrd = brdnum;
            }

          if (srcbrd < output_array[outarrayind].minbrd)
            {
	      output_array[outarrayind].minbrd = srcbrd;
            }
          if (srcbrd > output_array[outarrayind].maxbrd)
            {
	      output_array[outarrayind].maxbrd = srcbrd;
            }


          output_array[outarrayind].loadcnt+=1;
	}
      else
	{
	  printf("Too many loads at output_array index = %d term = %s\n",
		 outarrayind,output_array[outarrayind].bool);
        }
    }
  else
    {
      printf("Trying to add a load out of range = %d \n", outarrayind);
      printf("Exceeds MAXBOOL parameter \n");
    }

}

//
//  Add a connector load to an bool output
//
void add_new_conload(int outarrayind, char *loadstr, int brdnum, int pinnum)
{
  int srcbrd;

  if (outarrayind < MAXBOOL   )
    {
      if (output_array[outarrayind].loadcnt <  MAXLOADS )
	{
          j = output_array[outarrayind].loadcnt;
	  strncpy( output_array[outarrayind].loadlist[j].locstr, loadstr,10);
          output_array[outarrayind].loadlist[j].pinnum = pinnum;
          output_array[outarrayind].loadlist[j].chiptype = '.';
          output_array[outarrayind].loadlist[j].brdnum = brdnum;
          ccon_to_xy( loadstr);

          output_array[outarrayind].loadlist[j].pinx = pinx;
          output_array[outarrayind].loadlist[j].piny = piny;

          srcbrd = output_array[outarrayind].srcbrd;

          if (brdnum < output_array[outarrayind].minbrd)
            {
	      output_array[outarrayind].minbrd = brdnum;
            }
          if (brdnum > output_array[outarrayind].maxbrd)
            {
	      output_array[outarrayind].maxbrd = brdnum;
            }

          if (srcbrd < output_array[outarrayind].minbrd)
            {
	      output_array[outarrayind].minbrd = srcbrd;
            }
          if (srcbrd > output_array[outarrayind].maxbrd)
            {
	      output_array[outarrayind].maxbrd = srcbrd;
            }

          output_array[outarrayind].loadcnt+=1;
	}
      else
	{
	  printf("Too many loads at output_array index = %d term = %s\n",
		 outarrayind,output_array[outarrayind].bool);
        }
    }
  else
    {
      printf("Trying to add a connector load out of range = %d \n", outarrayind);
      printf("Exceeds MAXBOOL parameter \n");
    }

}

//
// Read all package file info
//
void read_all_pkgs( FILE *pkgfile)
{
  char pkglocs[10];
  char interm[10];
  char inpkg[10];

  char chptype;
  int pkgind;
  char outpin;
  int pinval;

  pkgcnt = 0;

  while (fscanf(pkgfile,"%s %s",inpkg,interm) == 2 )
    {
      // printf(" inpkg = %s interm = %s \n",inpkg,interm);

      pkglocs[0]= inpkg[0];
      pkglocs[1]= inpkg[1];
      pkglocs[2] = 0;

      chptype = inpkg[2];
      outpin = inpkg[3];  
        
      pkgind = handleloc(pkglocs);
 


      if (pkgindex < MAXBOOL)
	{
          strncpy(output_array[outputcnt].bool, interm,10);
          output_array[outputcnt].pkgindex = locindex;
          output_array[outputcnt].chip_type = chptype;
          output_array[outputcnt].loadcnt = 0;
          output_array[outputcnt].srcpin = outpin;
          output_array[outputcnt].isinput = 0;

          output_array[outputcnt].srcbrd = pkgind/96;
         
 
         pinxlate(outpin);
     
          if (chptype == 'S')
	    {
	      output_array[outputcnt].pinnum = pinxlate_s(outpin);
            }
          else
            {
	      output_array[outputcnt].pinnum = pinxlate(outpin);
            }


          if (pkgarray[pkgind].chiptype == '-' ) // package not def yet
	    {
            
             for(i=1;i<17;i+=1)
	      {
	       pkgarray[pkgind].pinterms[i][0] = 0;
	      }
            strncpy(pkgarray[pkgind].locstr,pkglocs,10);
            pkgarray[pkgind].chiptype = chptype;
            pkgcnt += 1;
	    }

	  //  printf("Setting chiptype = %c for %d \n", chptype,pkgind);

          strncpy(output_array[outputcnt].pinstr,inpkg,10);
          strncpy(output_array[outputcnt].loc, pkglocs,10);

          pinval = pinxlate(outpin);

          if (strlen( pkgarray[pkgind].pinterms[pinval])==0)
            {
	      strncpy(pkgarray[pkgind].pinterms[pinval],interm,10);
	    }
          else
	    {
              printf("Duplicate pin in pkg data = %s%c-%s\n",locstr,chptype, interm );
            }

          outputcnt +=1;
        
        }  
 

    }

  fclose(pkgfile);

}

//
// Read all jumper file info
//
void read_all_jmps( FILE *jmpfile)
{
  char pkglocs[10];
  char interm[10];
  char inpkg[10];
  char inmaskstr[10];

  int inmaskval;

  char chptype;
  int pkgind;
  char jmppin;
  int jmpcnt;
  int pinnum;   // O-Z become 1-12

  jmpcnt = 0;

  pkgcnt = 0;

  while (fscanf(jmpfile,"%s %s %s",inpkg,inmaskstr,interm) == 3 )
    {
      pkglocs[0]= inpkg[0];
      pkglocs[1]= inpkg[1];
      pkglocs[2] = 0;

      jmppin = inpkg[3];  
      pinnum = jmppin - 'O' + 1;

      inmaskval = atoi(inmaskstr); // get the integer
        
      if (isvalidmask(inmaskval))
	{
        }
      else
	{
          printf("Bad jumper mask = %s %s %s \n",inpkg,inmaskstr,interm);
          printf("Jumper must connect 2 boards, be continguous < 255 \n");
          exitnow(1);
        }

      pkgind = handleloc(pkglocs);

      if (pkgindex < MAXJMPS)
	{
          if (jmpcnt < MAXRJMPS)
           {
	   
	    jmpdata[jmpcnt].pinchar = jmppin;
            strncpy(jmpdata[jmpcnt].term, interm, 10);
            jmpdata[jmpcnt].pkgind = pkgind;
            jmpdata[jmpcnt].usedmask = inmaskval;
        
           jmpcnt+=1;
           }
          else
           {
            printf("Total jumper count exceed maximum = %d \n", jmpcnt);
            exitnow(1);
           }
        }  
 
    }

  fclose(jmpfile);

}  // read_all_jmps

/* qsort C-string comparison function */ 
int cstring_cmp(const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
	/* strcmp functions works exactly as expected from
	comparison function */ 
} 

//
// sort the list of Input and output terms
//

void sort_io(FILE *outfile)
{
  

  int i;

  for(i=0;i<voutcnt; i +=1)
    {
      outterms_srt[i] = outlistarray[i];   
    }

  for(i=0;i < vincnt; i += 1)
    {
     interms_srt[i] = inlistarray[i];
    }

  
  qsort(outterms_srt, voutcnt,sizeof( char *), cstring_cmp );
  
  qsort(interms_srt,vincnt, sizeof( char *), cstring_cmp );

  fprintf( outfile,"IZZ,\n");
 
  for(i=0;i<vincnt; i += 1)
  {
     fprintf(outfile," %s, \n", interms_srt[i]);
  }

  for(i=0;i<voutcnt-1; i += 1)
    {
      fprintf(outfile," %s, \n", outterms_srt[i]);
    }

  fprintf(outfile,"%s ); \n ",outterms_srt[voutcnt-1]);

  fprintf(outfile,"   \n");

  fprintf(outfile," input IZZ; \n");

  for(i=0;i<vincnt; i += 1)
    {
      fprintf(outfile," input %s; \n", interms_srt[i]);
    }


  for(i=0;i< voutcnt; i += 1)
    {
      fprintf(outfile," output %s; \n", outterms_srt[i]);
    }

  fprintf(outfile,"  \n");

  //for(i=0;i< voutcnt; i += 1)
  // {
  //    fprintf(outfile," reg %s; \n", outterms_srt[i]);
  //  }

  fprintf(outfile,"  \n");


}

//
// Read all connector file info
//
void read_all_cons( FILE *confile)
{
  char pkglocs[10];
  char leftcon[10];
  char rightcon[10];
  char boolterm[10];
  char flipbool[10];
  char linein[100];
  int lpinnum;
  int rpinnum;
  int brdnum;
  int conlocind;
  
  int fnd;
  int cntcnt;

  cntcnt = 0;

  endoffile = FALSE;

  vincnt = 0;
  voutcnt = 0;

  //printf("In read all cons \n");

  getaline(linein,confile); 

  while((endoffile==FALSE) &&(linein[0] != 26))
     {
       //printf("Confile line = %s \n", linein);

       if (linein[0] == 'Z' )
	 {
	   leftcon[0]= linein[0];
           leftcon[1]= linein[1];
           leftcon[2]= linein[2];
           leftcon[3] = 0;
           rightcon[0] = linein[4];
           rightcon[1] = linein[5];
           rightcon[2] = linein[6];
           rightcon[3] = 0;
           boolterm[0] = linein[10];
           boolterm[1] = linein[11];
           boolterm[2] = linein[12];
           boolterm[3] = 0;

           if (boolterm[0] == 'I')   // An input
	     {
               if (vincnt < 1000 )
		 {
		   strncpy(inlistarray[vincnt],boolterm,8);
                   vincnt += 1;
                 }
             }

           if (boolterm[0] == 'O')   // An output
	     {
               if (voutcnt < 1000 )
		 {
		   //printf("For O boolterm = %s \n",boolterm);
		   strncpy(outlistarray[voutcnt],boolterm,8);
                   voutcnt += 1;
                 }
             }

           conlocind = (leftcon[1] - 'A');  // 0-24
           
           if (leftcon[0] != 'Z' )
	     {
	       printf("First term in connector file should begin with Z \n");
               printf("Line = %s \n",linein);
             }
           if (rightcon[0] != 'Z' )
	     {
	       printf("Second term in connector file should begin with Z \n");
               printf("Line = %s \n",linein);
             }
	   if((isalpha(boolterm[0])) && (isalpha(boolterm[1])) &&
	      (isalpha(boolterm[2])))
	     {

             }
	   else
	     {
               if (boolterm[0] != '*')
		 {
	          printf("Bad format for connector file at line = %s \n",linein);
		 }
             }

          
           if (isupper(boolterm[0]))
             {
               flipbool[0] = tolower( boolterm[0]);
               flipbool[1] = tolower( boolterm[1]);
               flipbool[2] = tolower( boolterm[2]);
               flipbool[3] = 0;
	     }
           else
             {
               flipbool[0] = toupper( boolterm[0]);
               flipbool[1] = toupper( boolterm[1]);
               flipbool[2] = toupper( boolterm[2]);
               flipbool[3] = 0;
	     }

           lpinnum = leftcon[2] - 'A' + 1;
           rpinnum = rightcon[2] - 'A' + 1;
           brdnum = (leftcon[1] - 'A') / 3 ;

           if (boolterm[0] != '*')
	     {
             if ((lpinnum < 25)&&(lpinnum > -1))
              {
              strncpy(conlocs[conlocind].boolterms[lpinnum],boolterm,10);
              }
	     if ((rpinnum < 25) && (rpinnum > -1))
             {
              strncpy(conlocs[conlocind].boolterms[rpinnum],flipbool,10);
	     }
             conlocs[conlocind].brdnum =  brdnum;

    
	   // printf("About to update loads \n");


  
           if((boolterm[0] == 'O' )||(boolterm[0] == 'o'))
	     {
              

               fnd = -1;
               fnd = find_in_outputs(boolterm);
               if (fnd != -1)
		 {
                   add_new_conload(fnd,leftcon,brdnum,lpinnum);
		 }
               else
                 {
                   printf("Connector boolean output %s not found \n",boolterm);
		 }
               fnd = -1;
               fnd = find_in_outputs(flipbool);
               if (fnd != -1)
		 {
                   add_new_conload(fnd,rightcon,brdnum,rpinnum);
		 }
	     }


           if((boolterm[0] == 'i' )||(boolterm[0] == 'I'))
	     {
               

               if (outputcnt < (MAXBOOL-1))
            	{
                  strncpy(output_array[outputcnt].bool,boolterm,10);
		  strncpy(output_array[outputcnt].loc,leftcon,10);
                  output_array[outputcnt].pkgindex = -1;
                  output_array[outputcnt].chip_type = '.';
                  output_array[outputcnt].loadcnt = 0;
                  output_array[outputcnt].srcpin = leftcon[2];
                  output_array[outputcnt].srcbrd = brdnum;
                  output_array[outputcnt].isinput = 1;
                  outputcnt += 1;

                  strncpy(output_array[outputcnt].bool,flipbool,10);
                  strncpy(output_array[outputcnt].loc,rightcon,10);
                  output_array[outputcnt].pkgindex = -1;
                  output_array[outputcnt].chip_type = '.';
                  output_array[outputcnt].loadcnt = 0;
                  output_array[outputcnt].srcpin = rightcon[2];
                  output_array[outputcnt].srcbrd = brdnum;
                  output_array[outputcnt].isinput = 1;
          
	         }
	       }
	     }   // not a ***
	 }  // if a Z term

      getaline(linein,confile);
    }


  fclose(confile);

}  // read_all_cons
//
//
//
void getgoodjmps(int needed)
{
  int i;
  int jmpcnt;

  jmpcnt = 0;
 
  for(i=0;i<1200;i+=1)
    {
      jmpgood[i] = 0;
    }

  for (i=0; i < 1200; i+=1)
    {
      if ((jmplocs[i].availmask & needed) == needed)
       {
         jmpgood[i] = 1;
         jmpcnt +=1;
       }
    }

  jmpgoodcnt = jmpcnt;

// printf("%d good jumper \n",jmpcnt);

} // end getgoodjmps

//
//
//
void init_jmplocs()
{
  int i;
  char locstr[20];
  int j;

  for(i=0; i < 96; i += 1)
    {
     for(j=0; j < 12; j += 1)  // for each jumper O..Z
	{
          locstr[0] = (i/32)+ 'A';

          if (( i % 32 ) < 24)
	    { 
              locstr[1] = (i % 32) + 'A';
            }
          else
            {
              locstr[1] = (i % 32) - 24 + '1';
            }
	  locstr[2] = j + 'O';
	  locstr[3] = 0;

	  jmplocs[(i*12)+j].availmask = 255;
          cpin_to_xy( locstr);

          jmplocs[(i*12)+j].xloc = pinx;
          jmplocs[(i*12)+j].yloc = piny;

          strncpy(jmplocs[(i*12)+j].locstr,locstr,7);
	  // printf("Jmploc = %s \n",locstr);
          // printf(" Pinx = %d \n", pinx);
          // printf(" Piny = %d \n", piny);
	}
    }

  // printf("Done with init jmplocs \n");
}

//
//
//
void init_conlocs()
{
  int i;
  char locstr[20];
  int j;

  for(i=0; i < 24; i += 1)
    {
      conlocs[i].locstr[0] = 0;
      conlocs[i].brdnum = i/3;
      conlocs[i].xloc = 0;
      conlocs[i].yloc = 0;
      for(j=0; j < 25; j += 1)
        {
          conlocs[i].boolterms[j][0] = 0;
        }
    }
}

int get_term_list()
{
  int termindex;

  char term[20];
  char tterm[20];
  int endoflist;

  termindex = 0;

  endoflist=FALSE;

  term[0] = 0;
  term[1] = 0;
  term[2] = 0;
  
  while( endoflist == FALSE)
    {
      skipblanks();

      if ( (isalpha(linein[lineindex]))
            || (linein[lineindex] == '*') || (linein[lineindex] == '-'))
        {
	  term[0] = linein[lineindex];
          lineindex+=1;
          term[1] = linein[lineindex];
          lineindex+=1;
          term[2]= linein[lineindex];
          lineindex+=1;
          term[3] = 0;
	  //printf("Got term at 1= %s \n",term);

          if ((term[0]== '*') && (term[1] == '*') && (term[2] == '*'))
            {
	      strncpy(term,"ZZI",6);
            }
	  if ((term[0] ==  '-') && (term[1] == '-') && (term[2]=='-'))
	    {
	      strncpy(term,"ZZO",6);
 	    }
       if ((isalpha(term[0])) && (isalpha(term[1])) && (isalpha(term[2])))
	  {
	   strncpy(term_array[termindex].bool,term,10);
	   // printf("Put away term = %s \n",term);
           termindex += 1;
          }
	else
	 {
	  printf("Badly formed boolean term = %c%c%c in line = %s \n",term[0],
		 term[1],term[2],linein);
          exitnow(0);
         }	

        }

      skipblanks();

      if ((linein[lineindex] == '.' )||(linein[lineindex] == '+') ||
	  (linein[lineindex] == ';'))
	{
          endoflist = TRUE;
        }

    }  // not endoflist

  return(termindex);

}


int get_term_listd()
{
  int termindex;

  char term[20];
  char tterm[20];
  int endoflist;

  termindex = 0;

  endoflist=FALSE;

  term[0] = 0;
  term[1] = 0;
  term[2] = 0;
  
  while( endoflist == FALSE)
    {
      skipblanks();

      if ( (isalpha(linein[lineindex]))
            || (linein[lineindex] == '*') || (linein[lineindex] == '-'))
        {
	  term[0] = linein[lineindex];
          lineindex+=1;
          term[1] = linein[lineindex];
          lineindex+=1;
          term[2]= linein[lineindex];
          lineindex+=1;
          term[3] = 0;
	  // printf("Got term at 1= %s \n",term);

          if ((term[0]== '*') && (term[1] == '*') && (term[2] == '*'))
            {
	      strncpy(term,"ZZI",6);
            }
	  if ((term[0] ==  '-') && (term[1] == '-') && (term[2]=='-'))
	    {
	      strncpy(term,"ZZO",6);
 	    }
          if ((isalpha(term[0])) && (isalpha(term[1])) && (isalpha(term[2])))
	  {
	   strncpy(term_array[termindex].bool,term,10);
	   // printf("Put away term = %s \n",term);
           termindex += 1;
          }
	else
	 {
	  printf("Badly formed boolean term = %d%d%d in line = %s \n",term[0],
		 term[1],term[2],linein);
          exitnow(0);
         }	

        }

      skipblanksandslash();

      if ((linein[lineindex] == '.' )||(linein[lineindex] == '+') ||
	  (linein[lineindex] == ';'))
	{
          endoflist = TRUE;
        }

    }  // not endoflist

  return(termindex);

} // end get_term_listd

int get_term_list_beq()
{
  int termindex;

  char term[20];
  char tterm[20];
  int endoflist;

  termindex = 0;

  endoflist=FALSE;

  term[0] = 0;
  term[1] = 0;
  term[2] = 0;
  
  while( endoflist == FALSE)
    {
      skipblanks();

      if ( (isalpha(linein[lineindex]))
            || (linein[lineindex] == '*') || (linein[lineindex] == '-'))
        {
	  term[0] = linein[lineindex];
          lineindex+=1;
          term[1] = linein[lineindex];
          lineindex+=1;
          term[2]= linein[lineindex];
          lineindex+=1;
          term[3] = 0;
	  // printf("Got term at 1= %s \n",term);

          if ((term[0]== '*') && (term[1] == '*') && (term[2] == '*'))
            {
	      strncpy(term,"ZZI",6);
            }
	  if ((term[0] ==  '-') && (term[1] == '-') && (term[2]=='-'))
	    {
	      strncpy(term,"ZZO",6);
 	    }
          if ((isalpha(term[0])) && (isalpha(term[1])) && (isalpha(term[2])))
	  {
	   strncpy(term_array[termindex].bool,term,10);
	   // printf("Put away term = %s \n",term);
           termindex += 1;
          }
	else
	 {
	  printf("Badly formed boolean term = %d%d%d in line = %s \n",term[0],
		 term[1],term[2],linein);
          exitnow(0);
         }	

        }

      skipblanksandslash();

      if ((linein[lineindex] == '.' )||(linein[lineindex] == '='))
	{
          endoflist = TRUE;
        }

    }  // not endoflist

  return(termindex);

} // end get_term_list_beq


void parse_achip_rhs( char srcpin, int thispkgind)
{
  skippasteq();

  if (srcpin == 'D')
    {
     
      for(j=0; j < 4; j += 1)             // default to forced zero
        {
	  strncpy(term_array[j].bool,"ZZO",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'K';
      term_array[1].pin = 'L';
      term_array[2].pin = 'M';
      term_array[3].pin = 'N';

      apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 4 terms to pkg


      if (linein[lineindex] != '.')
	{
         skippastplus();

         for(j=0; j < 2; j += 1)             // default to forced one
          {
	   strncpy(term_array[j].bool,"ZZI",10);
          }
    

         for(j=0; j < 3; j += 1)             // default to forced one
          {
  	    strncpy(term_array[j].bool,"ZZI",10);
          }

          termcnt = get_term_list();    // into term array.bool

         term_array[0].pin = 'H';
         term_array[1].pin = 'I';
         term_array[2].pin = 'J';
      

        apply_terms_to_pkg(termcnt, term_array,3, thispkgind);   // 3 terms to pkg
 
        if (linein[lineindex] != '.')
    	 {
          skippastplus();

          for(j=0; j < 3; j += 1)             // default to forced one
           {
	    strncpy(term_array[j].bool,"ZZI",10);
           }

          termcnt = get_term_list();    // into term array.bool

          term_array[0].pin = 'A';
          term_array[1].pin = 'B';
          term_array[2].pin = 'C';
      
          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);   // 3 terms to pkg   
	 }

	}

    }
  if (srcpin == 'G')
    {

      for(j=0; j < 4; j += 1)             // default to forced zero
        {
	  strncpy(term_array[j].bool,"ZZO",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'K';
      term_array[1].pin = 'L';
      term_array[2].pin = 'M';
      term_array[3].pin = 'N';

      apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 4 terms to pkg


      if (linein[lineindex] != '.')
	{
         skippastplus();

         for(j=0; j < 3; j += 1)             // default to forced one
          {
  	    strncpy(term_array[j].bool,"ZZI",10);
          }

          termcnt = get_term_list();    // into term array.bool

         term_array[0].pin = 'H';
         term_array[1].pin = 'I';
         term_array[2].pin = 'J';
      

        apply_terms_to_pkg(termcnt, term_array,3, thispkgind);   // 3 terms to pkg
 
        if (linein[lineindex] != '.')
    	 {
          skippastplus();

          for(j=0; j < 3; j += 1)             // default to forced one
           {
	    strncpy(term_array[j].bool,"ZZI",10);
           }

          termcnt = get_term_list();    // into term array.bool

          term_array[0].pin = 'A';
          term_array[1].pin = 'B';
          term_array[2].pin = 'C';
      
          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);   // 3 terms to pkg   
	 }

	}
    }
              
}  // parse_achip

void parse_bchip_rhs( char srcpin, int thispkgind)
{
  skippasteq();
  if ((srcpin == 'I') || (srcpin == 'F') || (srcpin == 'E'))
    {
     
      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'J';
      term_array[1].pin = 'K';

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 4 terms to pkg


      if (linein[lineindex] != '.')
	{
         skippastplus();

         for(j=0; j < 2; j += 1)             // default to forced one
          {
  	    strncpy(term_array[j].bool,"ZZI",10);
          }

         termcnt = get_term_list();    // into term array.bool

         term_array[0].pin = 'L';
         term_array[1].pin = 'M';
          
         apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 3 terms to pkg
         
         if (linein[lineindex] != '.')
      	  {
           skippastplus();

           for(j=0; j < 2; j += 1)             // default to forced one
            {
  	     strncpy(term_array[j].bool,"ZZI",10);
            }
           
           termcnt = get_term_list();    // into term array.bool

           term_array[0].pin = 'N';
           term_array[1].pin = 'A';
      
           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 3 terms to pkg

           if (linein[lineindex] != '.')
      	    {
             skippastplus();

             for(j=0; j < 2; j += 1)             // default to forced one
              {
  	       strncpy(term_array[j].bool,"ZZI",10);
              }

             termcnt = get_term_list();    // into term array.bool

             term_array[0].pin = 'B';
             term_array[1].pin = 'C';
      
             apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 3 terms to pkg
	    } // 4th group
	  }  // 3rd group
	} // 2nd group
    
    } // pin 

              
}  // parse_bchip

void parse_cchip_rhs( char srcpin, int thispkgind)
{
  int groupend;

  skippasteq();
  if (srcpin == 'F')
    {
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 6 ) && (groupend == FALSE))
        {
     
         for(j=0; j < 2; j += 1)             // default to forced one
          {
	   strncpy(term_array[j].bool,"ZZI",10);
           }

          termcnt = get_term_list();    // into term array.bool

	  //   printf("For c chip term_array[0] = %s \n", term_array[0].bool);
	  //  printf("For c chip term_array[1] = %s \n", term_array[1].bool);

         
          if (groupcnt == 0)
            {
             term_array[0].pin = 'D';
             term_array[1].pin = 'E';
            }
     
          if (groupcnt == 1)
            {
             term_array[0].pin = 'H';
             term_array[1].pin = 'I';
            }
     
          if (groupcnt == 2)
            {
             term_array[0].pin = 'J';
             term_array[1].pin = 'K';
            }
     
          if (groupcnt == 3)
            {
             term_array[0].pin = 'L';
             term_array[1].pin = 'M';
            }
     
          if (groupcnt == 4)
            {
             term_array[0].pin = 'N';
             term_array[1].pin = 'A';
            }
     
          if (groupcnt == 5)
            {
             term_array[0].pin = 'B';
             term_array[1].pin = 'C';
            }
     
          if (groupend == FALSE)
	    {
             apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 4 terms to pkg
             groupcnt += 1;
            }


          if (linein[lineindex] == '.')
            {
	      groupend = TRUE;
            }

          if(linein[lineindex] == '+')
            {
	      lineindex += 1;
            }

	}
    }

              
}  // parse_cchip


void parse_dchip_rhs( char srcpin, int thispkgind)
{
  int groupend;

  int thistcount;

  skippasteq();
  if (srcpin == 'F')
    {
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 6 ) && (groupend == FALSE))
        {
     
         for(j=0; j < 6; j += 1)             // default to forced one
          {
	   strncpy(term_array[j].bool,"ZZI",10);
           }

          termcnt = get_term_list();    // into term array.bool

	
          if (linein[lineindex] == '.')
            {
	      groupend = TRUE;
            }

          if(linein[lineindex] == '+')
            {
	      lineindex += 1;
            }

          if (groupcnt == 0)
            {
             term_array[0].pin = 'H';
             term_array[1].pin = 'I';
             term_array[2].pin = 'M';
             term_array[3].pin = 'J';
             term_array[4].pin = 'L';
             term_array[5].pin = 'K';
             thistcount = 6;
            }
     
          if (groupcnt == 1)
            {
             term_array[0].pin = 'N';
             term_array[1].pin = 'M';
             term_array[2].pin = 'J';
             term_array[3].pin = 'L';
             term_array[4].pin = 'K';
             thistcount = 5;
            }
     
          if (groupcnt == 2)
            {
             term_array[0].pin = 'A';
             term_array[1].pin = 'J';
             term_array[2].pin = 'L';
             term_array[3].pin = 'K';
             thistcount = 4;
            }
     
           if (groupcnt == 3)
            {
             term_array[0].pin = 'D';
             term_array[1].pin = 'L';
             term_array[2].pin = 'K';
             thistcount = 3;
            }
     
          if (groupcnt == 4)
            {
             term_array[0].pin = 'E';
             term_array[1].pin = 'K';
             thistcount = 2;
            }

          if (groupcnt == 5)
            {
             term_array[0].pin = 'B';
             term_array[1].pin = 'C';
             thistcount = 2;
            }


          apply_terms_to_pkg(termcnt, term_array,thistcount, thispkgind); 
                                                   //  terms to pkg

          if (groupend == FALSE)
	    {
             groupcnt += 1;
            }
	}

    }
              
}  // parse_dchip


void parse_echip_rhs( char srcpin, int thispkgind)
{
  int groupend;

  skippasteq();
  if (srcpin == 'D')
    {
      groupcnt = 0;
      groupend = FALSE;

    
     
       for(j=0; j < 4; j += 1)             // default to forced one
        {
	   strncpy(term_array[j].bool,"ZZI",10);
        }

        termcnt = get_term_listd();    // into term array.bool, sep by /
       
        term_array[0].pin = 'C';
        term_array[1].pin = 'J';
       
         apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 4 terms to pkg

    }   // if D pin
              

  if (srcpin == 'G')
    {
      groupcnt = 0;
      groupend = FALSE;

         
      for(j=0; j < 4; j += 1)             // default to forced one
        {
	   strncpy(term_array[j].bool,"ZZI",10);
         }

       termcnt = get_term_listd();    // into term array.bool

       
       term_array[0].pin = 'C';
       term_array[1].pin = 'J';
          
       
       apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 4 terms to pkg

    }   // if G pin

  if (srcpin == 'H')
    {
      groupcnt = 0;
      groupend = FALSE;

     
      for(j=0; j < 6; j += 1)             // default to forced one
          {
	   strncpy(term_array[j].bool,"ZZI",10);
           }

       termcnt = get_term_listd();    // into term array.bool
       term_array[0].pin = 'C';
       term_array[1].pin = 'J';
          
       apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 4 terms to pkg

    }   // if H pin
              
  if (srcpin == 'L')
    {
      groupcnt = 0;
      groupend = FALSE;

     
      for(j=0; j < 6; j += 1)             // default to forced one
          {
	   strncpy(term_array[j].bool,"ZZI",10);
           }

      termcnt = get_term_listd();    // into term array.bool

       
      term_array[0].pin = 'C';
      term_array[1].pin = 'J';
          
       
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 4 terms to pkg

    }   // if L pin

  if (srcpin == 'M')
    {
      groupcnt = 0;
      groupend = FALSE;

     
      for(j=0; j < 6; j += 1)             // default to forced one
        {
	   strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_listd();    // into term array.bool

       
      term_array[0].pin = 'C';
      term_array[1].pin = 'J';
          
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 4 terms to pkg

    }   // if M pin

  if (srcpin == 'A')
    {
      groupcnt = 0;
      groupend = FALSE;
     
      for(j=0; j < 4; j += 1)             // default to forced one
          {
	 strncpy(term_array[j].bool,"ZZI",10);
         }

      termcnt = get_term_listd();    // into term array.bool

       
      term_array[0].pin = 'C';
      term_array[1].pin = 'J';
           
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 4 terms to pkg


    }   // if A pin

}  // parse_echip


void parse_fchip_rhs( char srcpin, int thispkgind)
{

  lineindex = 4;   // back up to beginning

  get_term_list_beq();  // get output terms
   
  lineindex = 4;

  skippasteq();

      // match_terms_to_pkg(termcnt,term_array,thispkgind);


   for(j=0; j < 4; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

  termcnt = get_term_list();    // into term array.bool

    
  //printf("In parse_f term_array[0] = %s \n",term_array[0].bool);
  //printf("In parse_f term_array[1] = %s \n",term_array[1].bool);
  // printf("In parse_f term_array[2] = %s \n",term_array[2].bool);

   term_array[0].pin = 'B';
   term_array[1].pin = 'C';
   term_array[2].pin = 'D';


   apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 3 terms to pkg
   skipblanks();

   if (linein[lineindex] != '.')    // enables
      {
        if (linein[lineindex] == ';' )
	  {

	    lineindex +=1;

          for(j=0; j < 2; j += 1)             // default to forced one
           {
	    strncpy(term_array[j].bool,"ZZI",10);
           }

          termcnt = get_term_list();

          term_array[0].pin = 'J';
          term_array[1].pin = 'K';

          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 3 terms to 
	}
      }
              
}  // parse_fchip
 

void parse_gchip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'E')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 3 ) && (groupend == FALSE))
        {
     
        for(j=0; j < 5; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

	//  printf("In parse p , termcnt = %d terms = %s %s \n", termcnt,
	//             term_array[0],
	//         term_array[1]);

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'L';
          term_array[1].pin = 'L';

          apply_terms_to_pkg(1, term_array,1, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'L';
           term_array[1].pin = 'D';
                 
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'C';
           term_array[1].pin = 'B';
           term_array[2].pin = 'A';

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }
 

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
	    if (linein[lineindex] == ';' )  // skip past semi
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if E output pin

  if (srcpin == 'F')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 3 ) && (groupend == FALSE))
        {
     
        for(j=0; j < 5; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

	//  printf("In parse p , termcnt = %d terms = %s %s \n", termcnt,
	//             term_array[0],
	//         term_array[1]);

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'K';
          term_array[1].pin = 'K';

          apply_terms_to_pkg(1, term_array,1, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'K';
           term_array[1].pin = 'D';
      
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'L';
           term_array[1].pin = 'C';
           term_array[2].pin = 'B';
           term_array[3].pin = 'A';

           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
          }
 

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
	    if (linein[lineindex] == ';' )  // skip past semi
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if F output pin
              

  if (srcpin == 'I')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 3 ) && (groupend == FALSE))
        {
     
        for(j=0; j < 5; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

	//  printf("In parse p , termcnt = %d terms = %s %s \n", termcnt,
	//             term_array[0],
	//         term_array[1]);

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'J';
          term_array[1].pin = 'J';

          apply_terms_to_pkg(1, term_array,1, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'J';
           term_array[1].pin = 'D';
      
          
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'K';
           term_array[1].pin = 'L';
           term_array[2].pin = 'C';
           term_array[3].pin = 'B';
           term_array[4].pin = 'A';

           apply_terms_to_pkg(termcnt, term_array,5, thispkgind);  // 2 terms to pkg
          }
 

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
	    if (linein[lineindex] == ';' )  // skip past semi
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if F output pin
              
}  // parse_gchip


void parse_hchip_rhs( char srcpin, int thispkgind)
{
  skippasteq();

  if (srcpin == 'G')
    {
     

      for(j=0; j < 6; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'E';
      term_array[1].pin = 'D';
      term_array[2].pin = 'C';
      term_array[3].pin = 'B';
      term_array[4].pin = 'A';
      term_array[5].pin = 'N';
        
      apply_terms_to_pkg(termcnt, term_array,6, thispkgind);  // 4 terms to pkg
    }

  if (srcpin == 'H')
    {
     

      for(j=0; j < 4; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'J';
      term_array[1].pin = 'K';
      term_array[2].pin = 'L';
      term_array[3].pin = 'M';

      apply_terms_to_pkg(termcnt, term_array,4, thispkgind);   // 3 terms to pkg
    }
              
}  // parse_hchip

void parse_ichip_rhs( char srcpin, int thispkgind)
{
  skippasteq();

  if (srcpin == 'I')
    {
     

      for(j=0; j < 3; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'J';
      term_array[1].pin = 'K';
      term_array[2].pin = 'L';
              
      apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 3 terms to pkg
    }

  if (srcpin == 'E')
    {
     

      for(j=0; j < 3; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'C';
      term_array[1].pin = 'B';
      term_array[2].pin = 'A';
      
      apply_terms_to_pkg(termcnt, term_array,3, thispkgind);   // 3 terms to pkg
    }

  if (srcpin == 'G')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'M';
      term_array[1].pin = 'N';
           

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }
              
}  // parse_ichip

void parse_jchip_rhs( char srcpin, int thispkgind)
{
  skippasteq();

  if (srcpin == 'G')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'E';
      term_array[1].pin = 'D';
                   
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
    }

  if (srcpin == 'H')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'J';
      term_array[1].pin = 'D';
     
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'N')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'L';
      term_array[1].pin = 'K';
           

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'A')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'C';
      term_array[1].pin = 'K';
           

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }
              
}  // parse_jchip

void parse_kchip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'H')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 2 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 3; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'K';
          term_array[1].pin = 'L';
          term_array[2].pin = 'J';
        

          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
         }


        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'M';
           term_array[1].pin = 'N';
           term_array[2].pin = 'C';
      

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }


	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if H output pin

  if (srcpin == 'F')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 2 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 3; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'A';
          term_array[1].pin = 'B';
          term_array[2].pin = 'J';

          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'E';
           term_array[2].pin = 'C';
      

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if F output pin
              
}  // parse_kchip


void parse_lchip_rhs( char srcpin, int thispkgind)
{
  int j;

  skippasteq();

  if (srcpin == 'G')
    {
     
      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'E';
      term_array[1].pin = 'D';
                   
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
    }

  if (srcpin == 'H')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'J';
      term_array[1].pin = 'D';
     
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'N')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'L';
      term_array[1].pin = 'D';
           

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'A')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'C';
      term_array[1].pin = 'D';
           

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }
              
}  // parse_lchip


void parse_mchip_rhs( char srcpin, int thispkgind)
{
  int j;

  skippasteq();

 if (srcpin == 'E')
    {
     
      for(j=0; j < 3; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'M';
      term_array[1].pin = 'N';
      term_array[2].pin = 'A';

      //printf("Term cnt = %d \n", termcnt);

      apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 3 terms to pkg

      skipblanks();

      if (linein[lineindex] != '.')
	{
         skippastplus();

         for(j=0; j < 2; j += 1)             // default to forced one
          {
	   strncpy(term_array[j].bool,"ZZI",10);
          }

         termcnt = get_term_list();    // into term array.bool

        term_array[0].pin = 'J';
        term_array[1].pin = 'L';
     
         apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
         skipblanks();

	 if (linein[lineindex] != '.')
	   {
	    
            skippastplus();

            for(j=0; j < 2; j += 1)             // default to forced one
             {
	      strncpy(term_array[j].bool,"ZZI",10);
             }

             termcnt = get_term_list();    // into term array.bool

            term_array[0].pin = 'B';
            term_array[1].pin = 'C';
           

            apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
	   }
	}
     
    }

  if (srcpin == 'F')
    {
     

      for(j=0; j < 3; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'M';
      term_array[1].pin = 'N';
      term_array[2].pin = 'A';

      apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 3 terms to pkg

      if (linein[lineindex] != '.')
	{
         skippastplus();

         for(j=0; j < 2; j += 1)             // default to forced one
          {
	   strncpy(term_array[j].bool,"ZZI",10);
          }

         termcnt = get_term_list();    // into term array.bool

        term_array[0].pin = 'J';
        term_array[1].pin = 'L';
     
         apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
         skipblanks();

	 if (linein[lineindex] != '.')
	   {

            skippastplus();

            for(j=0; j < 2; j += 1)             // default to forced one
             {
	      strncpy(term_array[j].bool,"ZZI",10);
             }

             termcnt = get_term_list();    // into term array.bool

            term_array[0].pin = 'B';
            term_array[1].pin = 'C';
           

            apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
	   }
	}
     
    }

  if (srcpin == 'I')
    {


      for(j=0; j < 3; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'M';
      term_array[1].pin = 'N';
      term_array[2].pin = 'A';

      apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 3 terms to pkg

      skipblanks();

      if (linein[lineindex] != '.')
	{

         skippastplus();

         for(j=0; j < 2; j += 1)             // default to forced one
          {
	   strncpy(term_array[j].bool,"ZZI",10);
          }

         termcnt = get_term_list();    // into term array.bool

        term_array[0].pin = 'J';
        term_array[1].pin = 'L';
     
         apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
         skipblanks();

	 if (linein[lineindex] != '.')
	   {

            skippastplus();

            for(j=0; j < 2; j += 1)             // default to forced one
             {
	      strncpy(term_array[j].bool,"ZZI",10);
             }

             termcnt = get_term_list();    // into term array.bool

            term_array[0].pin = 'B';
            term_array[1].pin = 'C';
           

            apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
	   }
	}
     
     
    }

              
}  // parse_mchip

void parse_nchip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'I')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 5 ) && (groupend == FALSE))
        {
     
        for(j=0; j < 6; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'G';
          term_array[1].pin = 'F';
          term_array[2].pin = 'E';
          term_array[3].pin = 'A';
          term_array[4].pin = 'J';
         
          apply_terms_to_pkg(termcnt, term_array,5, thispkgind);  // 3 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'E';
           term_array[2].pin = 'A';
           term_array[3].pin = 'J';

           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 3 terms to pkg
          }

         if (groupcnt == 2 )
   	 {
          term_array[0].pin = 'N';
          term_array[1].pin = 'A';
          term_array[2].pin = 'J';
         
          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 3 terms to pkg
         }

        if (groupcnt == 3 )
 	 {
          term_array[0].pin = 'M';
          term_array[1].pin = 'L';
          term_array[2].pin = 'J';
         
          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 3 terms to pkg
         }

        if (groupcnt == 4 )
 	 {
          term_array[0].pin = 'C';
          term_array[1].pin = 'B';
         
         
          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 3 terms to pkg
         }


 	 if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
         else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if I output pin
              
}  // parse_nchip

void parse_ochip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'M')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 2 ) && (groupend == FALSE))
        {
     
        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

     
        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'L';
          term_array[1].pin = 'D';

          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
         }


        if (groupcnt == 1 )
    	 {
         
           term_array[0].pin = 'A';
           term_array[1].pin = 'd';

           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }


         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if M output pin

  if (srcpin == 'I')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 2 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

     
        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'J';
          term_array[1].pin = 'D';

          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
        
           term_array[0].pin = 'G';
           term_array[1].pin = 'd';

           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }


         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if I output pin

  if (srcpin == 'F')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 2 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

     
        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'B';
          term_array[1].pin = 'D';
        
          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {

           term_array[0].pin = 'C';
           term_array[1].pin = 'd';
      
           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }


	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if F output pin
              
}  // parse_ochip

void parse_pchip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'B')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 3 ) && (groupend == FALSE))
        {
     
        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

	//  printf("In parse p , termcnt = %d terms = %s %s \n", termcnt,
	//             term_array[0],
	//         term_array[1]);

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'M';
          term_array[1].pin = 'I';
        

          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
         }


        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'N';
           term_array[1].pin = 'J';
      

           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'A';
           term_array[1].pin = 'L';
      

           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }
 

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if B output pin

  if (srcpin == 'D')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 3 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'F';
          term_array[1].pin = 'I';
        

          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
         }


        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'G';
           term_array[1].pin = 'J';
      

           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }


        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'H';
           term_array[1].pin = 'L';

           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else

          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if D output pin
              
}  // parse_pchip

void parse_qchip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'F')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 4 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 3; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'L';
          term_array[1].pin = 'B';
          term_array[2].pin = 'A';

          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'C';
           term_array[1].pin = 'M';
           term_array[2].pin = 'A';
      
           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'C';
           term_array[1].pin = 'B';
           term_array[2].pin = 'N';
      
           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 3 )
    	 {
           term_array[0].pin = 'L';
           term_array[1].pin = 'M';
           term_array[2].pin = 'N';
      
           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

           if (linein[lineindex] == '.')
	    {
             groupend = TRUE;
            }
          else
           {

            if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
           }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if F output pin

  if (srcpin == 'E')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 4 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'L';
          term_array[1].pin = 'B';
          term_array[2].pin = 'A';
        

          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
         }


        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'C';
           term_array[1].pin = 'M';
           term_array[2].pin = 'A';
      

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }


        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'C';
           term_array[1].pin = 'B';
           term_array[2].pin = 'N';

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 3 )
    	 {
           term_array[0].pin = 'C';
           term_array[1].pin = 'B';
           term_array[2].pin = 'A';

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

           if (linein[lineindex] == '.')
	    {
             groupend = TRUE;
            }
          else
           {

            if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
           }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if E output pin

  if (srcpin == 'I')
    {
     

        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

   
        term_array[0].pin = 'J';
        term_array[1].pin = 'K';

        apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg

    }  // if I output
              
}  // parse_qchip


void parse_rchip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'B')  // just do this for first output
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 3 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 4; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'E';
          term_array[1].pin = 'D';
          term_array[2].pin = 'J';
          term_array[3].pin = 'K';

          apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'I';
           term_array[1].pin = 'H';
           term_array[2].pin = 'G';
           term_array[3].pin = 'F';

           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'C';
           term_array[1].pin = 'L';
      
           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }


	if (linein[lineindex] == '.')
	  {
            if (groupcnt != 2)
	      {
		printf("Missing groups in R chip \n");
                printf("Line with r chip = %s \n",linein);
              }
            groupend = TRUE;

          }
        else
          {
	    if (linein[lineindex] == ';' )  // skip past plus
              {
                lineindex+= 1;
              }
            if (linein[lineindex] == '+' )  // skip past plus
              {
                printf("No plus in R chip \n");
                lineindex+= 1;
              }
          }
       


         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if B output pin
}
//
//
//
void parse_schip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'A')  // just do this for first output
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 3 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 12; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'P';
          
          apply_terms_to_pkg(termcnt, term_array,1, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin  = 'B';
           term_array[1].pin  = 'C';
           term_array[2].pin  = 'D';
           term_array[3].pin  = 'E';
           term_array[4].pin  = 'F';
           term_array[5].pin  = 'G';
           term_array[6].pin  = 'H';
           term_array[7].pin  = 'I';
           term_array[8].pin  = 'J';
           term_array[9].pin  = 'K';
           term_array[10].pin = 'L';
           term_array[11].pin = 'M';

           apply_terms_to_pkg(termcnt, term_array,12, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'O';
           term_array[1].pin = 'N';
      
           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
          }


	if (linein[lineindex] == '.')
	  {
            if (groupcnt != 2)
	      {
		printf("Missing groups in S chip \n");
                printf("Line with S chip = %s \n",linein);
              }
            groupend = TRUE;

          }
        else
          {
	    if (linein[lineindex] == ';' )  // skip past plus
              {
                lineindex+= 1;
              }
            if (linein[lineindex] == '+' )  // skip past plus
              {
                printf("No plus in S chip \n");
                lineindex+= 1;
              }
          }
       

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if A output pin
}

void parse_tchip_rhs( char srcpin, int thispkgind)
{
  int groupend;
  int j;

  skippasteq();

  if (srcpin == 'B')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 4 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 3; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'G';
          term_array[1].pin = 'L';
          term_array[2].pin = 'E';

          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'F';
           term_array[2].pin = 'E';
      
           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'L';
           term_array[2].pin = 'C';
      
           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 3 )
    	 {
           term_array[0].pin = 'G';
           term_array[1].pin = 'F';
           term_array[2].pin = 'C';
      
           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }


           if (linein[lineindex] == '.')
	    {
             groupend = TRUE;
            }
          else
           {

            if (linein[lineindex] == '+' )  // skip past plus
              {
                        lineindex+= 1;
              }
           }
       

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if B output pin

  if (srcpin == 'M')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 4 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'G';
          term_array[1].pin = 'L';
          term_array[2].pin = 'E';
        

          apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
         }


        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'F';
           term_array[2].pin = 'E';
      

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }


        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'L';
           term_array[2].pin = 'C';

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 3 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'L';
           term_array[2].pin = 'E';

           apply_terms_to_pkg(termcnt, term_array,3, thispkgind);  // 2 terms to pkg
          }


           if (linein[lineindex] == '.')
	    {
             groupend = TRUE;
            }
          else
           {

            if (linein[lineindex] == '+' )  // skip past plus
              {
                        lineindex+= 1;
              }
           }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if M output pin


  if (srcpin == 'H')
    {

        for(j=0; j < 1; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        term_array[0].pin = 'J';

        apply_terms_to_pkg(termcnt, term_array,1, thispkgind);  // 1 terms to pkg

    }  // if H output
              
}  // parse_tchip

void parse_uchip_rhs( char srcpin, int thispkgind)
{
  int j;
  int groupend;

  skippasteq();

  if (srcpin == 'F')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 4 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 3; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'A';
          term_array[1].pin = 'c';
          term_array[2].pin = 'b';
          term_array[3].pin = 'E';

          apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'N';
           term_array[1].pin = 'c';
           term_array[2].pin = 'B';
           term_array[3].pin = 'E';
      
           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'K';
           term_array[1].pin = 'C';
           term_array[2].pin = 'b';
           term_array[3].pin = 'E';
      
           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 3 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'C';
           term_array[2].pin = 'B';
           term_array[3].pin = 'E';
      
           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
          }

        skipblanks();

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

   
         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if F output pin

  if (srcpin == 'I')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 4 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 4; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'A';
          term_array[1].pin = 'l';
          term_array[2].pin = 'm';
          term_array[3].pin = 'J';
        

          apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
         }


        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'N';
           term_array[1].pin = 'l';
           term_array[2].pin = 'M';
           term_array[3].pin = 'J';
      
           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
          }


        if (groupcnt == 2 )
    	 {
           term_array[0].pin = 'K';
           term_array[1].pin = 'L';
           term_array[2].pin = 'm';
           term_array[3].pin = 'J';

           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
          }

        if (groupcnt == 3 )
    	 {
           term_array[0].pin = 'D';
           term_array[1].pin = 'L';
           term_array[2].pin = 'M';
           term_array[3].pin = 'J';

           apply_terms_to_pkg(termcnt, term_array,4, thispkgind);  // 2 terms to pkg
          }

        skipblanks();

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

   
         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if I output pin
              
}  // parse_uchip


void parse_vchip_rhs( char srcpin, int thispkgind)
{
  int j;

  skippasteq();

  if (srcpin == 'B')
    {
     

      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'C';
                      
      apply_terms_to_pkg(termcnt, term_array,1, thispkgind);  // 2 terms to pkg
    }

  if (srcpin == 'I')
    {
     

      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'J';
   
     
      apply_terms_to_pkg(termcnt, term_array,1, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'F')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'D';
      term_array[1].pin = 'E';
  
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'M')
    {
     

      for(j=0; j < 2; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'K';
      term_array[1].pin = 'L';
           

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }
              
}  // parse_vchip


void parse_xchip_rhs( char srcpin, int thispkgind)
{
  int groupend;

  skippasteq();

  if (srcpin == 'E')
    {
     
      groupcnt = 0;
      groupend = FALSE;

      while(( groupcnt < 5 ) && (groupend == FALSE))
        {
     

        for(j=0; j < 2; j += 1)             // default to forced one 
         {
	  strncpy(term_array[j].bool,"ZZI",10);
         }

        termcnt = get_term_list();    // into term array.bool

        if (groupcnt == 0 )
	 {
          term_array[0].pin = 'H';
          term_array[1].pin = 'I';
         
          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 3 terms to pkg
         }

        if (groupcnt == 1 )
    	 {
           term_array[0].pin = 'L';
           term_array[1].pin = 'M';
          
           apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 3 terms to pkg
          }

         if (groupcnt == 2 )
   	 {
          term_array[0].pin = 'N';
          term_array[1].pin = 'A';
       
          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 3 terms to pkg
         }

        if (groupcnt == 3 )
 	 {
          term_array[0].pin = 'B';
          term_array[1].pin = 'C';
                 
          apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 3 terms to pkg
         }

        if (groupcnt == 4 )
 	 {
          term_array[0].pin = 'J';
                           
          apply_terms_to_pkg(termcnt, term_array,1, thispkgind);  // 3 terms to pkg
         }

        skipblanks();

	if (linein[lineindex] == '.')
	  {
            groupend = TRUE;
          }
        else
          {
	    if (linein[lineindex] == '+' )  // skip past plus
              {
                lineindex+= 1;
              }
          }

         if (groupend == FALSE)
	  {
           groupcnt += 1;
          }

	} // groups
     
    }  // if I output pin
              
}  // parse_xchip

void parse_ychip_rhs( char srcpin, int thispkgind)
{


  skippasteq();

  if (srcpin == 'E')
    {
       for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'H';
      term_array[1].pin = 'I';
                     
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
    }

  if (srcpin == 'D')
    {
      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'J';
      term_array[1].pin = 'I';
    
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'C')
    {
   
      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'L';
      term_array[1].pin = 'I';
 
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'B')
    {
     

      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'M';           
      term_array[1].pin = 'I';           

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

              
}  // parse_ychip

void parse_zchip_rhs( char srcpin, int thispkgind)
{


  skippasteq();

  if (srcpin == 'G')
    {
     
      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_list();    // into term array.bool

      term_array[0].pin = 'F';
      term_array[1].pin = 'E';
                     
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);  // 2 terms to pkg
    }

  if (srcpin == 'H')
    {
     

      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_listd();    // into term array.bool

      term_array[0].pin = 'J';
      term_array[1].pin = 'I';
     
      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'N')
    {
     

      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_listd();    // into term array.bool

      term_array[0].pin = 'M';
      term_array[1].pin = 'L';

      apply_terms_to_pkg(termcnt, term_array,2, thispkgind);   // 2 terms to pkg
    }

  if (srcpin == 'A')
    {
     

      for(j=0; j < 1; j += 1)             // default to forced one
        {
	  strncpy(term_array[j].bool,"ZZI",10);
        }

      termcnt = get_term_listd();    // into term array.bool

      term_array[0].pin = 'C';           
      term_array[1].pin = 'B';           


      apply_terms_to_pkg(termcnt, term_array,1, thispkgind);   // 2 terms to pkg
    }

              
}  // parse_zchip

  //
  //
  //
 void add_a_load( char *term, char *inlocstr, int brdnum, char chiptype, int pinnum)
 {

    int fnd;
    int termindex;
    char pinchar;

    fnd=-1;

    fnd=find_in_outputs(term);
    
    if (fnd != -1)
      {
        termindex = fnd;
        pinchar = lookuppin(pinnum);

        add_new_load(termindex,inlocstr,brdnum,chiptype,pinnum,pinchar);
      }
    else
      {
        if ((term[0] != 'I') && (term[0] != 'i'))  // not an input term
	  {
           printf("Boolean term not found in output array, term = %s \n", term );
          }
      }

 }   // end add_a_load

void getfirstbool()
{
  lineindex= 0;
  skipdashes();
  lineindex +=1;
  skipblanks();
 
  firstbool[0]= linein[lineindex];
  lineindex += 1;
  firstbool[1] = linein[lineindex];
  lineindex += 1;
  firstbool[2] = linein[lineindex];
  firstbool[3] = 0;
  lineindex += 1;

  // printf("First bool = %s \n",firstbool);

  skipblanks();
}

//
//  Bonehead find of boolean output term in those from pkg file
//
int find_in_outputs(char *inbool)
{
  int outfound;
  int outindex;
  
  outfound = FALSE;

  outindex = 0;
  while((outfound == FALSE) && (outindex < MAXBOOL))
    {

      if (strncmp(output_array[outindex].bool,inbool,10) == 0 )
        {
          outfound = TRUE;
        }
      else
	{
	  outindex += 1;
        }
    }

  if (outfound == FALSE)
    {
      
      if ((inbool[0] != 'i') && (inbool[0] != 'I'))
        {
        printf("Output term not found in package data, term = %s \n", inbool );
        }

       return(-1);

    }
  
  return(outindex);

}   // find_in_outputs
//
//  Output kicad netlist for each board
//
void pkg_outk( int ipkgind, int brdnum, char *inlocstr, char chiptype)
{
  int i;
  int unum;
  
  unum = (ipkgind % 96)+1;
  
  // select_brd_file(brdnum);

  fprintf(outfile," ( /4AC2PCA3 DIP-16ST U%d CDIP ) \n",unum);


  for(i=0;i<MAXPINPERMOD;i+=1)   // markall as outputs
    {
      pkgarray[ipkgind].pinisout[i]=0;
    }

  if (chiptype == 'A')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
    }

  if (chiptype == 'B')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
    }

  if (chiptype == 'C')
    {
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
    }

  if (chiptype == 'D')
    {
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
    }

  if (chiptype == 'E')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
      pkgarray[ipkgind].pinisout[11]= 1;
      pkgarray[ipkgind].pinisout[14]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
      pkgarray[ipkgind].pinisout[1]= 1;
      pkgarray[ipkgind].pinisout[2]= 1;
    }

  if (chiptype == 'F')
    {
      pkgarray[ipkgind].pinisout[4]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
      pkgarray[ipkgind].pinisout[1]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
      pkgarray[ipkgind].pinisout[11]= 1;
      pkgarray[ipkgind].pinisout[26]= 1;
    }

  if (chiptype == 'H')
    {
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
     
    }

  if (chiptype == 'I')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
     
    }

  if (chiptype == 'J')
    {
      pkgarray[ipkgind].pinisout[1]= 1;
      pkgarray[ipkgind].pinisout[2]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
     
    }

  if (chiptype == 'K')
    {
    
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
    }


  if (chiptype == 'L')
    {
      pkgarray[ipkgind].pinisout[1]= 1;
      pkgarray[ipkgind].pinisout[2]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
     
    }

  if (chiptype == 'M')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
         
    }


  if (chiptype == 'N')
    {
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
              
    }

  if (chiptype == 'O')
    {
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
              
    }


  if (chiptype == 'P')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[2]= 1;
      pkgarray[ipkgind].pinisout[3]= 1;
                   
    }

  if (chiptype == 'Q')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
                   
    }

  if (chiptype == 'R')
    {
      pkgarray[ipkgind].pinisout[2]= 1;
      pkgarray[ipkgind].pinisout[3]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
                        
    }


  if (chiptype == 'S')
    {
      pkgarray[ipkgind].pinisout[1]= 1;
                             
    }

  if (chiptype == 'T')
    {
      pkgarray[ipkgind].pinisout[1]= 1;
      pkgarray[ipkgind].pinisout[2]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
                             
    }

  if (chiptype == 'U')
    {
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
                             
    }


  if (chiptype == 'V')
    {
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
      pkgarray[ipkgind].pinisout[1]= 1;
      pkgarray[ipkgind].pinisout[2]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
                             
    }

  if (chiptype == 'W')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[10]= 1;
      pkgarray[ipkgind].pinisout[11]= 1;
      pkgarray[ipkgind].pinisout[14]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[16]= 1;
      pkgarray[ipkgind].pinisout[1]= 1;
      pkgarray[ipkgind].pinisout[2]= 1;
    
    }

  if (chiptype == 'X')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;

    }

  if (chiptype == 'Y')
    {
      pkgarray[ipkgind].pinisout[5]= 1;
      pkgarray[ipkgind].pinisout[6]= 1;
      pkgarray[ipkgind].pinisout[7]= 1;
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[1]= 1;
      pkgarray[ipkgind].pinisout[2]= 1;
      pkgarray[ipkgind].pinisout[3]= 1;
    }

  if (chiptype == 'Z')
    {
      pkgarray[ipkgind].pinisout[8]= 1;
      pkgarray[ipkgind].pinisout[9]= 1;
      pkgarray[ipkgind].pinisout[15]= 1;
      pkgarray[ipkgind].pinisout[1]= 1;
      
    }

 
    for (i = 1; i < 17 ; i ++ )
     {

      if(i == 4)
        {
	  fprintf(outfile,"  ( 4 GND ) \n");
        }
      else
        {
          if (i == 12)
            {
	      fprintf(outfile,"  ( 12 VCC ) \n");
            }
          else
            {
              if (pkgarray[ipkgind].pinterms[i][0] != 0 )
		{
		  if (pkgarray[ipkgind].pinisout[i] == 0 ) // load
		    {
                      add_a_load(pkgarray[ipkgind].pinterms[i], inlocstr, brdnum, chiptype, i);
                    }

	         fprintf(outfile,"  ( %d %s ) \n",i,pkgarray[ipkgind].pinterms[i] );
		}
              else
		{
                fprintf(outfile,"  ( %d ? ) \n", i);
	        }
	    }
         }
 
      }

  fprintf(outfile,")\n"); 
}

//
//  Output kicad netlist for each board
//
void pkg_outk_schip( int ipkgind, int brdnum, char *inlocstr)
{
  int i;
  int unum;
     
  unum = (ipkgind % 96)+1;
  
  // select_brd_file(brdnum);

  fprintf(outfile," ( /4AC2PCA3 DIP-18ST U%d CDIP ) \n",unum);

  for(i=0;i<MAXPINPERMOD;i+=1)   // markall as outputs
    {
      pkgarray[ipkgind].pinisout[i]=0;
    }

  pkgarray[ipkgind].chiptype = 'S';
  pkgarray[ipkgind].pinisout[1] = 1;

  for (i = 1; i < 19 ; i ++ )
    {

      if(i == 4)
        {
	  fprintf(outfile,"  ( 4 GND ) \n");
        }
      else
        {
          if (i == 12)
            {
	      fprintf(outfile,"  ( 12 VCC ) \n");
            }
          else
            {
              if (pkgarray[ipkgind].pinterms[i][0] != 0 )
		{
		  if (pkgarray[ipkgind].pinisout[i] == 0 )
		    {
                                             
                      add_a_load(pkgarray[ipkgind].pinterms[i], inlocstr, brdnum, 'S', i);
                    }

	         fprintf(outfile,"  ( %d %s ) \n",i,pkgarray[ipkgind].pinterms[i] );
		}
              else
		{
                fprintf(outfile,"  ( %d ? ) \n", i);
	        }
	    }
        }
 
    }

  fprintf(outfile,")\n"); 
}


//
//  Output verilog output
//
void pkg_outv( int ipkgind, int brdnum, char *inlocstr)
{
  int i;
  int unum;
  char ctype;
  char pins[20][10];
  int pin_list[30];
  int k;

  for(i=1;i<19;i++)
    {
      strncpy(pins[i],pkgarray[ipkgind].pinterms[i],10);
      
    }

  ctype =pkgarray[ipkgind].chiptype;

  if (ctype == 'A')
    {
      if (pins[5][0] != 0 )  // D
        {
          fprintf(outfilea,"assign %s = ",pins[5]);

	  if ( pins[13][0] != 0 )   // D = KLMN
	    {
	     pin_list[0] = 13;
             pin_list[1] = 14;
             pin_list[2] = 15;
             pin_list[3] = 16;

	     print_veri_pingroupa(4,pin_list,ipkgind);
	    }

          if (pins[9][0] != 0 )          // | HIJ
	    {       
	      fprintf(outfilea," | " ); 

	     pin_list[0] =  9;
             pin_list[1] = 10;
             pin_list[2] = 11;
	 
	     print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          if ( pins[1][0] != 0 )   // ABC
	    {
             fprintf(outfilea," | " );

	     pin_list[0] =  1;
             pin_list[1] =  2;
             pin_list[2] =  3;
         
	    print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          fprintf(outfilea," ; \n");
          fprintf(outfilea,"assign %s = ~%s; //complement \n",flipcase(pins[5]),pins[5]); 
                                                             // E=~D
	}

      if (pins[8][0] != 0 )  // G
        {
          fprintf(outfilea,"assign %s = ",pins[5]);

	  if ( pins[13][0] != 0 )   // G = KLMN
	    {
	     pin_list[0] = 13;
             pin_list[1] = 14;
             pin_list[2] = 15;
             pin_list[3] = 16;

	     print_veri_pingroupa(4,pin_list,ipkgind);
	    }

          if (pins[9][0] != 0 )          // | HIJ
	    {       
	      fprintf(outfilea," | " ); 

	     pin_list[0] =  9;
             pin_list[1] = 10;
             pin_list[2] = 11;
	 
	     print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          if ( pins[1][0] != 0 )   // ABC
	    {
             fprintf(outfilea," | " );

	     pin_list[0] =  1;
             pin_list[1] =  2;
             pin_list[2] =  3;
         
	    print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          fprintf(outfilea," ; \n");
          fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[8]),pins[8]); 
                                                             // f = ~G
	}

    }  // A chip


  if (ctype == 'B')
    {
      
      if (pins[10][0] != 0 )
	{
          fprintf(outfilea,"assign %s = ",pins[10]);
     
          if (pins[11][0] != 0 )  // I=JK+LM+NA+BC
            {
	      pin_list[0] = 11;
              pin_list[1] = 13;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

     
          if (pins[14][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 14;
              pin_list[1] = 15;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }
     
          if (pins[16][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 16;
              pin_list[1] = 1;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

          if (pins[2][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 2;
              pin_list[1] = 3;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

	  fprintf(outfilea," ; \n");
     
      fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[10]),pins[10]);  // h=~I

	}

      if (pins[7][0] != 0 )
	{
          fprintf(outfilea,"assign %s = ",pins[7]);
     
          if (pins[11][0] != 0 )  // F=JK+LM+NA+BC
            {
	      pin_list[0] = 11;
              pin_list[1] = 13;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

     
          if (pins[14][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 14;
              pin_list[1] = 15;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }
     
          if (pins[16][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 16;
              pin_list[1] = 1;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

          if (pins[2][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 2;
              pin_list[1] = 3;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

	  fprintf(outfilea," ; \n");
     
          fprintf(outfilea,"assign %s = ~%s; //complement \n",flipcase(pins[7]),pins[7]);  // E=~F
        }
     
      if (pins[6][0] != 0 )
	{
          fprintf(outfilea,"assign %s = ",pins[6]);
     
          if (pins[11][0] != 0 )  // E=JK+LM+NA+BC
            {
	      pin_list[0] = 11;
              pin_list[1] = 13;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

     
          if (pins[14][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 14;
              pin_list[1] = 15;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }
     
          if (pins[16][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 16;
              pin_list[1] = 1;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

          if (pins[2][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 2;
              pin_list[1] = 3;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

	  fprintf(outfilea," ; \n");
     
          fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[6]),pins[6]);  // E=~F


        }
     
    }

  if (ctype == 'C')
    {
      if (pins[7][0] != 0 )  // F
        {
	  fprintf(outfilea,"assign %s = ",pins[7]);
     
          if (pins[5][0] != 0 )  // F = DE+HI+JK+LM+NA+BC
            {
	      pin_list[0] = 5;
              pin_list[1] = 6;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }
     
          if (pins[9][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 9;
              pin_list[1] = 10;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

          if (pins[11][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 11;
              pin_list[1] = 13;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

          if (pins[14][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 14;
              pin_list[1] = 15;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

          if (pins[16][0] != 0 )
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 16;
              pin_list[1] = 1;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

          if (pins[2][0] != 0 )  //  F = DE+HI+JK+LM+NA+BC
            {
    
              fprintf(outfilea," | ");

	      pin_list[0] = 2;
              pin_list[1] = 3;

 	      print_veri_pingroupa(2,pin_list,ipkgind);
            }

	  fprintf(outfilea," ; \n");

       fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[7]),pins[7]);  // g = ~F

	}
    }

  if (ctype == 'D')
    {
      
      if (pins[7][0] != 0 )  // F Pin
	{
          if (pins[9][0] != 0 )  // H pin
            {
	      fprintf(outfilea,"assign %s = ",pins[7]);

              if (pins[9][0] != 0 )
		{
                 
    	          pin_list[0] = 9;   //H
                  pin_list[1] = 10;  //I
    	          pin_list[2] = 15;  //M
                  pin_list[3] = 11;  //J
                  pin_list[4] = 14;  //L
                  pin_list[5] = 13;  //K

 	          print_veri_pingroupa(6,pin_list,ipkgind);
                 }

              if (pins[16][0] != 0 )
		{
                  fprintf(outfilea," | ");

    	          pin_list[0] = 16;  //N
                  pin_list[1] = 15;  //M
    	          pin_list[2] = 11;  //J
                  pin_list[3] = 14;  //L
                  pin_list[4] = 13;  //K
                
 	          print_veri_pingroupa(5,pin_list,ipkgind);
                 }

              if (pins[1][0] != 0 )  //A
        	 {
                  fprintf(outfilea," | ");

    	          pin_list[0] = 1;   //A
                  pin_list[1] = 11;  //J
    	          pin_list[2] = 14;  //L
                  pin_list[3] = 13;  //K


 	          print_veri_pingroupa(4,pin_list,ipkgind);
                 }
                 
          
              if (pins[5][0] != 0 )  // D
	        {
                  fprintf(outfilea," | ");

    	          pin_list[0] = 5;   // D
                  pin_list[1] = 14;  // L
    	          pin_list[2] = 13;  // K
              
 	          print_veri_pingroupa(3,pin_list,ipkgind);
                 }
               
	      if (pins[6][0] != 0 )  // E
                {
                  fprintf(outfilea," | ");

    	          pin_list[0] = 6;   // E
                  pin_list[1] = 13;  // K
    	                       
 	          print_veri_pingroupa(2,pin_list,ipkgind);
                 }

	      if (pins[2][0] != 0 ) // B
                 {
                  fprintf(outfilea," | ");

    	          pin_list[0] = 2;   //B
                  pin_list[1] = 3;   //C
    	                       
 	          print_veri_pingroupa(2,pin_list,ipkgind);
                 }

	       fprintf(outfilea," ; \n");
	     
            }
	           	      
         // F = NIMJLK + NMJLK + AJLK + DLK + EK +BC


	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[7]),pins[7]);  // g = ~F
        }


    }

  if (ctype == 'E')
    {
      fprintf(outfilea,"assign %s = clkdrv( %s,%s) ; \n",
	      pins[5],pins[3],pins[11]);
           	      
      // D = CJ

      fprintf(outfilea,"assign %s = ~%s; //complement \n",flipcase(pins[5]),pins[5]); 
                        // e = ~D
     
    }

  if (ctype == 'F')
    {
      if ((pins[11][0] != 0 ) && (pins[13][0] != 0 ))
	{
         fprintf(outfilea,"assign %s = ~%s & ~%s & ~%s & %s & %s ; \n",
	      pins[14],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // L = bcdJK   
     
         fprintf(outfilea,"assign %s = ~%s & ~%s &  %s & %s & %s ; \n",
	      pins[15],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // M = bcDJK   
     
         fprintf(outfilea,"assign %s = ~%s &  %s & ~%s & %s & %s ; \n",
	      pins[16],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // N = bCdJK  
     
         fprintf(outfilea,"assign %s = ~%s &  %s &  %s & %s & %s ; \n",
	      pins[1],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // A = bCDJK  
     
         fprintf(outfilea,"assign %s =  %s & ~%s & ~%s & %s & %s ; \n",
	      pins[7],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // F = BcdJK   
     
         fprintf(outfilea,"assign %s =  %s & ~%s &  %s & %s & %s ; \n",
	      pins[8],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // G = BcDJK   
     
         fprintf(outfilea,"assign %s =  %s &  %s & ~%s & %s & %s ; \n",
	      pins[9],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // H = BCdJK  
     
         fprintf(outfilea,"assign %s =  %s &  %s &  %s & %s & %s ; \n",
	      pins[10],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // I = BCDJK  
	}
      else
	{
         fprintf(outfilea,"assign %s = ~%s & ~%s & ~%s  ; \n",
	      pins[14],pins[2],pins[3],pins[5]);
      // L = bcdJK   
     
         fprintf(outfilea,"assign %s = ~%s & ~%s &  %s  ; \n",
	      pins[15],pins[2],pins[3],pins[5]);
      // M = bcDJK   
     
         fprintf(outfilea,"assign %s = ~%s &  %s & ~%s  ; \n",
	      pins[16],pins[2],pins[3],pins[5]);
      // N = bCdJK  
     
         fprintf(outfilea,"assign %s = ~%s &  %s &  %s  ; \n",
	      pins[1],pins[2],pins[3],pins[5]);
      // A = bCDJK  
     
         fprintf(outfilea,"assign %s =  %s & ~%s & ~%s  ; \n",
	      pins[7],pins[2],pins[3],pins[5]);
      // F = BcdJK   
     
         fprintf(outfilea,"assign %s =  %s & ~%s &  %s ; \n",
	      pins[8],pins[2],pins[3],pins[5]);
      // G = BcDJK   
     
         fprintf(outfilea,"assign %s =  %s &  %s & ~%s  ; \n",
	      pins[9],pins[2],pins[3],pins[5]);
      // H = BCdJK  
     
         fprintf(outfilea,"assign %s =  %s &  %s &  %s ; \n",
	      pins[10],pins[2],pins[3],pins[5]);
      // I = BCDJK  
	}


       if (pins[6][0] != 0 )
        {     
	  //fprintf(outfilea,"assign %s =  ~%s |  ~%s ; \n",
	  //   pins[6],pins[11],pins[13]);
	  fprintf(outfilea,"assign %s = ZZI ; \n", pins[6] );

        }

      // E = j + k  
                      
    } // end F



  if (ctype == 'G')  // 
    {

      if (pins[6][0] != 0 )  // E = L & ttt | l & TTT ; where TTT = CBA
	{
          fprintf(outfilea,"assign %s = %s & ~(",pins[6],pins[14]);


           pin_list[0] = 3;  // CBA
           pin_list[1] = 2;
           pin_list[2] = 1;

           print_veri_pingroupa(3,pin_list,ipkgind);

           fprintf(outfilea," ) | %s & (",flipcase(pins[14]));


           pin_list[0] = 3;  // CBA
           pin_list[1] = 2;
           pin_list[2] = 1;

           print_veri_pingroupa(3,pin_list,ipkgind);

           fprintf(outfilea," );\n");
           fprintf(outfilea,"assign %s = ~%s ;\n",flipcase(pins[6]),pins[6]);
	}

      if (pins[7][0] != 0 )  // F = K & ttt | k & TTT ; where TTT = LCBA
	{
          fprintf(outfilea,"assign %s = %s & ~(",pins[7],pins[13]);


	   pin_list[0] = 14;  // LCBA
           pin_list[1] = 3; 
           pin_list[2] = 2;
           pin_list[3] = 1;

           print_veri_pingroupa(4,pin_list,ipkgind);

           fprintf(outfilea," ) | %s & (",flipcase(pins[13]));

           pin_list[0] = 14;  // LCBA
           pin_list[1] = 3;  
           pin_list[2] = 2;
           pin_list[3] = 1;

           print_veri_pingroupa(4,pin_list,ipkgind);

           fprintf(outfilea," );\n");
           fprintf(outfilea,"assign %s = ~%s ;  //complement \n",flipcase(pins[7]),pins[7]);
	}
   

      if (pins[10][0] != 0 )  // I = J & ttt | j & TTT ; where TTT = KLCBA
	{
          fprintf(outfilea,"assign %s = %s & ~(",pins[10],pins[11]);

           pin_list[0] = 13;  // KLCBA
	   pin_list[1] = 14; 
           pin_list[2] = 3; 
           pin_list[3] = 2;
           pin_list[4] = 1;

           print_veri_pingroupa(5,pin_list,ipkgind);

           fprintf(outfilea," ) | %s & (",flipcase(pins[11]));

           pin_list[0] = 13;  // KLCBA
	   pin_list[1] = 14; 
           pin_list[2] = 3; 
           pin_list[3] = 2;
           pin_list[4] = 1;

           print_veri_pingroupa(5,pin_list,ipkgind);

           fprintf(outfilea," );\n");
           fprintf(outfilea,"assign %s = ~%s;  //complement;\n",flipcase(pins[10]),pins[10]);
	}
    }


  if (ctype == 'H')
    {
      if (pins[8][0] != 0 )
        {
	  fprintf(outfilea,"assign %s = ",pins[8]);
         
	  pin_list[0] = 6;   // G = EDCBAN
          pin_list[1] = 5;
          pin_list[2] = 3;
          pin_list[3] = 2;
          pin_list[4] = 1;
          pin_list[5] = 16;
          
 	  print_veri_pingroupa(6,pin_list,ipkgind);
          fprintf(outfilea," ; \n");


	 fprintf(outfilea,"assign %s = ~%s;  //complement  \n",flipcase(pins[8]),pins[8]);
                                     // g = ~F

	}

      if (pins[9][0] != 0 )
        {
	  fprintf(outfilea,"assign %s = ",pins[9]);

         
	  pin_list[0] = 11;   // H=JKLM
          pin_list[1] = 13;
          pin_list[2] = 14;
          pin_list[3] = 15;

          print_veri_pingroupa(4,pin_list,ipkgind);
          fprintf(outfilea," ; \n");
     
         fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[9]),pins[9]);  // i = ~H

        }
    }

  if (ctype == 'I')
    {
      if (pins[10][0] != 0 )
	{
	  fprintf(outfilea,"assign %s = ",pins[10]);
         
	  pin_list[0] = 11;   // I=JKL
          pin_list[1] = 13;
          pin_list[2] = 14;

          print_veri_pingroupa(3,pin_list,ipkgind);
          fprintf(outfilea," ; \n");
     
	 fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[10]),pins[10]);  // h = ~I
        }

      if (pins[6][0] != 0 )
	{
	  fprintf(outfilea,"assign %s = ",pins[6]); 
         
	  pin_list[0] = 3;   // I=JKL
          pin_list[1] = 2;
          pin_list[2] = 1;

          print_veri_pingroupa(3,pin_list,ipkgind);
          fprintf(outfilea," ; \n");

         fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[6]),pins[6]);  // d = ~E
	}
   
      if (pins[8][0] != 0 )
	{
	  fprintf(outfilea,"assign %s = ",pins[8]);

	  pin_list[0] = 15;   // G=MN
          pin_list[1] = 16;

          print_veri_pingroupa(3,pin_list,ipkgind);
          fprintf(outfilea," ; \n");

	 fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[8]),pins[8]);  // f = ~G
        }

    }


  if (ctype == 'J')
    {    
      if (pins[5][0] != 0 )  // D
	{
           // G=DE
        if (pins[8][0] != 0 )
         {
         
          fprintf(outfilea,"assign %s = %s & %s ; \n",  
	      pins[8],pins[5],pins[6] );

         
	  fprintf(outfilea,"assign %s = ~%s ; //complement \n",flipcase(pins[8]),pins[8]);  // f = ~G
          }

         if (pins[9][0] != 0 )  // H=DJ
	  {
          
           fprintf(outfilea,"assign %s = %s & %s ; \n",
	      pins[9],pins[5],pins[11] );

	   fprintf(outfilea,"assign %s = ~%s ;  //complement \n",flipcase(pins[9]),pins[9]);  // i = ~H
     	  }
        }
      else  // No D
	{
           // G=DE
	  if (pins[8][0] != 0 )  // 
         {
         
          fprintf(outfilea,"assign %s = %s; \n",  
	      pins[8],pins[6] );

	  fprintf(outfilea,"assign %s = ~%s; //complement \n",flipcase(pins[8]),pins[8]);  // f = ~G
          }


         if (pins[9][0] != 0 )  // H=DJ
	  {
     
           fprintf(outfilea,"assign %s = %s; \n",
	      pins[9],pins[11] );

	   fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[9]),pins[9]);  // i = ~H
     	  }
        }

      if (pins[13][0] !=0 )
        {
         if (pins[16][0] != 0 )  // N=KL
	  {
        
           fprintf(outfilea,"assign %s = %s & %s ; \n",
	      pins[16],pins[13],pins[14] );

            fprintf(outfilea,"assign %s = ~%s ;  //complement \n",flipcase(pins[16]),pins[16]);  // m = ~N
           }

          if (pins[1][0] != 0 )  // A=KC
	   {
            fprintf(outfilea,"assign %s = %s & %s; \n",
	      pins[1],pins[13],pins[3] );
   
            fprintf(outfilea,"assign %s = ~%s; \n",flipcase(pins[1]),pins[1]);  // b = ~A          
	   }
      	}
      else
       {
         if (pins[16][0] != 0 )  // N=KL
	  {
    
           fprintf(outfilea,"assign %s = %s; \n",
	      pins[16],pins[14] );

            fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[16]),pins[16]);  // m = ~N
           }

          if (pins[1][0] != 0 )  // A=KC
	   {
           
            fprintf(outfilea,"assign %s = %s; \n",
	      pins[1],pins[3] );
   
            fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[1]),pins[1]);  // b = ~A        
	   }
       }
    }


  if (ctype == 'K')
    {
      if (pins[9][0] != 0)
	{
          fprintf(outfilea,"assign %s = ",pins[9]); 
         
          if (pins[13][0] != 0 )
	    {
	     pin_list[0] = 13;   // H=KLJ + MNC
             pin_list[1] = 14;
             pin_list[2] = 11;

             print_veri_pingroupa(3,pin_list,ipkgind);
            }

          if (pins[15][0] != 0 )
	    {
	     fprintf(outfilea," | ");

	     pin_list[0] = 15;   // H=KLJ + MNC
             pin_list[1] = 16;
             pin_list[2] = 3;

             print_veri_pingroupa(3,pin_list,ipkgind);
            }

          fprintf(outfilea," ; \n");
	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[9]),pins[9]);  // i=~H
	}
     
       if (pins[7][0] != 0)
	{
          fprintf(outfilea,"assign %s = ",pins[7]); 
         
          if (pins[1][0] != 0 )
	    {
	     pin_list[0] = 1;   // F=ABJ+DEC
             pin_list[1] = 2;
             pin_list[2] = 11;

             print_veri_pingroupa(3,pin_list,ipkgind);
            }

          if (pins[5][0] != 0 )
	    {
	     fprintf(outfilea," | ");

	     pin_list[0] = 5;   // F = ABJ + DEC
             pin_list[1] = 6;
             pin_list[2] = 3;

             print_veri_pingroupa(3,pin_list,ipkgind);
            }

          fprintf(outfilea," ; \n");
	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[7]),pins[7]);  // i=~H
	}     

    }


  if (ctype == 'L')  // clocked
    {
      if ( pins[5][0] != 0 ) // D
	{
	  if (pins[8][0] != 0 )
	    {
             fprintf(outfileb," %s <= %s & %s ; \n",
	      pins[8],pins[5],pins[6] );

             fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[8]),pins[8]);  // f = ~G

             // G=DE
	    }

          if (pins[9][0] != 0 )
	    {
             fprintf(outfileb," %s <= %s & %s ; \n",
	      pins[9],pins[5],pins[11] );
   
             
             fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[9]),pins[9]); 
                          // i = ~H
             // H=DJ
            }

	  if (pins[16][0] != 0 )
	    {
             fprintf(outfileb," %s <= %s & %s ; \n",
	        pins[16],pins[5],pins[14] );

             fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[16]),pins[16]); 
                     // m = ~N

                     // N=DL
	    }
	  if (pins[1][0] != 0 )
            {

             fprintf(outfileb," %s <= %s & %s ; \n",
	      pins[1],pins[5],pins[3] );

             fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[1]),pins[1]);  

             // b = ~A
   
            // A=DC
            }
	}

      else   // No D
	{
	  if (pins[8][0] != 0 )
	    {
             fprintf(outfileb," %s <= %s ; \n",
		     pins[8],pins[6] );

             fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[8]),pins[8]);  // f = ~G

             // G=DE
	    }

          if (pins[9][0] != 0 )
	    {
             fprintf(outfileb," %s <= %s ; \n",
	      pins[9],pins[11] );
   
             
             fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[9]),pins[9]); 
                          // i = ~H
             // H=DJ
            }

	  if (pins[16][0] != 0 )
	    {
             fprintf(outfileb," %s <= %s ; \n",
	        pins[16],pins[14] );

             fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[16]),pins[16]); 
                     // m = ~N

                     // N=DL
	    }
	  if (pins[1][0] != 0 )
            {

             fprintf(outfileb," %s <= %s ; \n",
	      pins[1],pins[3] );

             fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[1]),pins[1]);  

             // b = ~A
   
            // A=DC
            }
	}

    }


  // M Chip

  if (ctype == 'M')  // clocked
    {
      if (pins[6][0] != 0 )
	{
          
          fprintf(outfileb," %s <= ",pins[6]);
         
	  if (pins[15][0] != 0 ) //M
	    {
              
	      pin_list[0] = 15;   //      // E = MNA+JL+BC
              pin_list[1] = 16;
              pin_list[2] = 1;

              print_veri_pingroupb(3,pin_list,ipkgind);
            }
       
	  if (pins[11][0] != 0 ) //J
	    {
              
              fprintf(outfileb," | ");

	      pin_list[0] = 11;   //      // E = MNA+JL+BC
              pin_list[1] = 14;
          
              print_veri_pingroupb(2,pin_list,ipkgind);
            }
       
          if (pins[2][0] != 0 )  // B
	    {
              fprintf(outfileb," | ");

	      pin_list[0] = 2;   //      // E = MNA+JL+BC
              pin_list[1] = 3;
          
              print_veri_pingroupb(2,pin_list,ipkgind);
            }

	  fprintf(outfileb," ; \n");
                           
                   // E = MNA+JL+BC

         fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[6]),pins[6]);  // d = ~E

	}

      if (pins[7][0] != 0 )
	{
          
          fprintf(outfileb," %s <= ",pins[7]);
         
	  if (pins[15][0] != 0 ) //M
	    {
              
	      pin_list[0] = 15;   //      // F = MNA+JL+BC
              pin_list[1] = 16;
              pin_list[2] = 1;

              print_veri_pingroupb(3,pin_list,ipkgind);
            }
       
	  if (pins[11][0] != 0 ) //J
	    {
              
              fprintf(outfileb," | ");

	      pin_list[0] = 11;   //      // F = MNA+JL+BC
              pin_list[1] = 14;
          
              print_veri_pingroupb(2,pin_list,ipkgind);
            }
       
          if (pins[2][0] != 0 )  // B
	    {
              fprintf(outfileb," | ");

	      pin_list[0] = 2;   //      // F = MNA+JL+BC
              pin_list[1] = 3;
          
              print_veri_pingroupb(2,pin_list,ipkgind);
            }

	  fprintf(outfileb," ; \n");
                           
                   // F = MNA+JL+BC

         fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[7]),pins[7]);  // d = ~E

	}

      if (pins[10][0] != 0 )
	{
          
          fprintf(outfileb," %s <= ",pins[10]);
         
	  if (pins[15][0] != 0 ) //M
	    {
              
	      pin_list[0] = 15;   //      // I = MNA+JL+BC
              pin_list[1] = 16;
              pin_list[2] = 1;

              print_veri_pingroupb(3,pin_list,ipkgind);
            }
       
	  if (pins[11][0] != 0 ) //J
	    {
              
              fprintf(outfileb," | ");

	      pin_list[0] = 11;   //      // I = MNA+JL+BC
              pin_list[1] = 14;
          
              print_veri_pingroupb(2,pin_list,ipkgind);
            }
       
          if (pins[2][0] != 0 )  // B
	    {
              fprintf(outfileb," | ");

	      pin_list[0] = 2;   //      // I = MNA+JL+BC
              pin_list[1] = 3;
          
              print_veri_pingroupb(2,pin_list,ipkgind);
            }

	  fprintf(outfileb," ; \n");
                           
                   // I = MNA+JL+BC

         fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[10]),pins[10]);  // d = ~E

	}
    }  
 // N Chip

  if (ctype == 'N')  // clocked
    {
      if (pins[10][0] != 0 )
        {
          fprintf(outfileb," %s <= ",pins[10]);

	   if (pins[8][0] != 0 )  //I = GFEAJ + DEAJ + NAJ + MLJ + CB;
            {
	      pin_list[0] = 8;   //     FGEAJ
              pin_list[1] = 7;
              pin_list[2] = 6;
              pin_list[3] = 1;
              pin_list[4] = 11;

              print_veri_pingroupb(5,pin_list,ipkgind);
            }

         if (pins[5][0] != 0 )
	    {
              fprintf(outfileb," | ");

	      pin_list[0] = 5;   //     DEAJ
              pin_list[1] = 6;
              pin_list[2] = 1;
              pin_list[3] = 11;

              print_veri_pingroupb(4,pin_list,ipkgind);

            }

          if (pins[16][0] != 0 )
            {
              fprintf(outfileb," | ");

	      pin_list[0] = 16;   //     NAJ
              pin_list[1] = 1;
              pin_list[2] = 11;
              
              print_veri_pingroupb(3,pin_list,ipkgind);

            }

          if (pins[15][0] != 0 )
            {
              fprintf(outfileb," | ");

	      pin_list[0] = 15;   //     MLJ
              pin_list[1] = 14;
              pin_list[2] = 11;
              
              print_veri_pingroupb(3,pin_list,ipkgind);

	    }

	  if (pins[3][0] != 0 )
	    {
              fprintf(outfileb," | ");

	      pin_list[0] = 3;   //    CB
              pin_list[1] = 2;
                         
              print_veri_pingroupb(2,pin_list,ipkgind);

	    }
	   fprintf(outfileb," ; \n");

           // I = GFEAJ + DEAJ + NAJ + MLJ + CB;

	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[10]),pins[10]); 
              // h = ~I
        }

    }

  if (ctype == 'O')   // clocked
    {
      if (pins[15][0] != 0)
	{
	  if (pins[5][0] != 0 )
	    {
             fprintf(outfileb," %s <= %s & %s | ",
	      pins[15],pins[14],pins[5]); // M=LD
              if( pins[1][0] != 0 )
	       {
               fprintf(outfileb," %s & %s ; \n",
		       pins[1],flipcase(pins[5]) ); // Ad
               }
              else
               {
                fprintf(outfileb," ZZO & %s ; \n",
			flipcase(pins[5]) ); // Ad
               }
    	    }
	  else
	    {
	      // fprintf(outfileb," %s <= %s | ", pins[15],pins[14]); // M=LD
              if( pins[1][0] != 0 )
	       {
		 fprintf(outfileb," %s <= %s; \n",pins[15],pins[1]); // Ad
               }
            }

         fprintf(outfilea,"assign %s = ~%s;  //complement \n",
                 flipcase(pins[15]),pins[15]);  // n=~M
        }
 
      if(pins[10][0] != 0 )
	{
	  if (pins[5][0] != 0 )
	   {
            fprintf(outfileb," %s <= %s & %s | ",
	      pins[10],pins[11],pins[5]); // I=JD
            if (pins[8][0] != 0 )
             {
              fprintf(outfileb," %s & %s ; \n",
		      pins[8],flipcase( pins[5]) ); // Gd
	     }
            else
             {
              fprintf(outfileb," ZZO & %s ; \n",
		      flipcase(pins[5]) ); // Gd
             }
	   }
	  else
	   {
	     // fprintf(outfileb," %s <= %s | ",pins[10],pins[11]); // I=JD
            if (pins[8][0] != 0 )
             {
              fprintf(outfileb," %s <= %s ; \n",pins[10],pins[8]); // Gd
	     }
	   }
                
        fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[10]),pins[10]);  // h=~I
        }

      if (pins[7][0] != 0 )
	{
	  if (pins[5][0] != 0)
	    {
             fprintf(outfileb," %s <= %s & %s | ",pins[7],
              pins[2],pins[5]); // F=BD
            if (pins[3][0] != 0 )
             {
              fprintf(outfileb," %s & %s ; \n",
		      pins[3],flipcase(pins[5]) ); // Cd
	      }
            else
             {
              fprintf(outfileb," ZZO & %s ; \n",
		      flipcase( pins[5]) ); // Cd
	     }
	    }
	  else
	    {
	      //fprintf(outfileb," %s <= %s  | ",pins[7],pins[2]); // F=BD
            if (pins[3][0] != 0 )
             {
              fprintf(outfileb," %s <= %s  ; \n",
	        pins[7],pins[3]); // Cd
	      }
	    }

	 fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[7]),pins[7]);  // e=~F
     
	}
    }


  if (ctype == 'P')  // clocked
    {

      if (pins[2][0] != 0 )
	{
          fprintf(outfileb," %s <= ",pins[2]);
          if (pins[15][0] != 0 )
            { 
              pin_list[0] = 15;  // MI
              pin_list[1] = 10;

              print_veri_pingroupb(2,pin_list,ipkgind);

	    }
  
          if (pins[16][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 16;  // NJ
              pin_list[1] = 11;

              print_veri_pingroupb(2,pin_list,ipkgind);
	    }

	  if (pins[1][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 1;  // AL
              pin_list[1] = 14;

              print_veri_pingroupb(2,pin_list,ipkgind);
            }
	  fprintf(outfileb," ; \n");

         fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[2]),pins[2]);  // c = ~B
	}

      if (pins[5][0] != 0 )
        {

          fprintf(outfileb," %s <= ", pins[5]);

          if (pins[7][0] != 0 )
            {
              pin_list[0] = 7;  // FI
              pin_list[1] = 10;

              print_veri_pingroupb(2,pin_list,ipkgind);

	    }
   
          if (pins[8][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 8;  // GJ
              pin_list[1] = 11;

              print_veri_pingroupb(2,pin_list,ipkgind);
	    }
	   

	  if (pins[9][0] != 0 )
            {
              fprintf(outfileb," | ");

              pin_list[0] = 9;  // HL
              pin_list[1] = 14;

              print_veri_pingroupb(2,pin_list,ipkgind);
            }

	  fprintf(outfileb," ; \n");   // D=FI+GJ+HL
     
      fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[5]),pins[5]);  
                                  // e = ~D
        }
     
    }


  if (ctype == 'R')
    {

      if (pins[1][0] != 0 )
	{
	  
 	  fprintf(outfilec,"ram_16x4 rinst_%d({%s,%s,%s,%s},{%s,%s,%s,%s},{%s,%s,%s,%s}, %s, %s, IZZ); \n",
		  rchipcnt,pins[2],pins[1],pins[16],pins[15],
	      pins[6],pins[5],pins[11],pins[13],
              pins[10],pins[9],pins[8],pins[7], pins[3],pins[14]);
          rchipcnt  += 1;
	}

    }

  if (ctype == 'S')
    {

      if (pins[1][0] != 0 )
	{
	  // A = P ; BCDEFGHIJKLM; ON
          // 1 = 18; 2 3 5 6 7 8 9 10 11 13 14 15 ; 17 16 .
          // ram_4096 ( outpin, inpin, addr[0:11], cs, we, clk );
 	  fprintf(outfilec,"ram_4096x1 sinst_%d(%s,%s,{%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s}, %s, %s, IZZ); \n",
		  schipcnt,pins[1],pins[18],pins[2],pins[3],
	          pins[5],pins[6],pins[7],pins[8],pins[9],pins[10],
		  pins[11],pins[13],pins[14],pins[15], pins[17],pins[16]);
              
          schipcnt  += 1;
	}

    }

  if (ctype == 'Q')
    {

      if (pins[7][0] != 0 )
	{
	  fprintf(outfilea,"assign %s = ", pins[7]);

          if (pins[14][0] != 0 )
            {
              pin_list[0] = 14;  // LBA
              pin_list[1] = 2;
              pin_list[2] = 1;

              print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          if (pins[3][0] != 0 )
            {
	      fprintf(outfilea," | ");

              pin_list[0] = 3;  // CMA
              pin_list[1] = 15;
              pin_list[2] = 1;

              print_veri_pingroupa(3,pin_list,ipkgind);
	    } 


          if (pins[3][0] != 0 )
            {
	      fprintf(outfilea," | ");

              pin_list[0] = 3;  // CBN
              pin_list[1] = 2;
              pin_list[2] = 16;

              print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          if (pins[14][0] != 0 )
            {
	      fprintf(outfilea," | ");

              pin_list[0] = 14;  // LMN
              pin_list[1] = 15;
              pin_list[2] = 16;

              print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          fprintf(outfilea," ; \n");
	  fprintf(outfilea,"assign %s = ~%s; //complement \n",flipcase(pins[7]),pins[7]);  // g = ~F
	}


      if (pins[6][0] != 0 )
	{
	  fprintf(outfilea,"assign %s = ", pins[6]);

          if (pins[14][0] != 0 )
            {
              pin_list[0] = 14;  // LBA
              pin_list[1] = 2;
              pin_list[2] = 1;

              print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          if (pins[3][0] != 0 )
            {
	      fprintf(outfilea," | ");

              pin_list[0] = 3;  // CMA
              pin_list[1] = 15;
              pin_list[2] = 1;

              print_veri_pingroupa(3,pin_list,ipkgind);
	    }


          if (pins[3][0] != 0 )
            {
	      fprintf(outfilea," | ");

              pin_list[0] = 3;  // CBN
              pin_list[1] = 2;
              pin_list[2] = 16;

              print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          if (pins[3][0] != 0 )
            {
	      fprintf(outfilea," | ");

              pin_list[0] = 3;  // CBA
              pin_list[1] = 2;
              pin_list[2] = 1;

              print_veri_pingroupa(3,pin_list,ipkgind);
	    }

          fprintf(outfilea," ; \n");
	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[6]),pins[6]);  // f = ~E
	}

      if (pins[10][0] != 0 )  // I
	{
	fprintf(outfilea,"assign %s = %s & %s; \n", pins[10],pins[11],pins[13]);
        fprintf(outfilea,"assign %s = ~%s; \n",flipcase(pins[10]),pins[10]);
       }
    }

  if (ctype == 'T')  //clocked
    {

      if (pins[2][0] != 0 )  // B =
	{
	  fprintf(outfileb," %s <= ",pins[2]);

          if (pins[8][0] != 0 ) 
            {
              pin_list[0] = 8;  // GLE
              pin_list[1] = 14;
              pin_list[2] = 6;

              print_veri_pingroupb(3,pin_list,ipkgind);
	    }

          if (pins[5][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 5;  // DFE
              pin_list[1] = 7;
              pin_list[2] = 6;

              print_veri_pingroupb(3,pin_list,ipkgind);
	    }

          if (pins[5][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 5;  // DLC
              pin_list[1] = 14;
              pin_list[2] = 3;

              print_veri_pingroupb(3,pin_list,ipkgind);
	    }

          if (pins[8][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 8;  // GFC
              pin_list[1] = 7;
              pin_list[2] = 3;

              print_veri_pingroupb(3,pin_list,ipkgind);
	    }

          fprintf(outfileb," ;\n"); // B=GLE+DFE+DLC+GFC
	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[2]),pins[2]);  // a = ~B   
	}

      if (pins[15][0] != 0 )  // M =
	{
	  fprintf(outfileb," %s <= ",pins[15]);

          if (pins[8][0] != 0 ) 
            {
              pin_list[0] = 8;  // GLE
              pin_list[1] = 14;
              pin_list[2] = 6;

              print_veri_pingroupb(3,pin_list,ipkgind);
	    }

          if (pins[5][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 5;  // DFE
              pin_list[1] = 7;
              pin_list[2] = 6;

              print_veri_pingroupb(3,pin_list,ipkgind);
	    }

          if (pins[5][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 5;  // DLC
              pin_list[1] = 14;
              pin_list[2] = 3;

              print_veri_pingroupb(3,pin_list,ipkgind);
	    }

          if (pins[5][0] != 0 )
            {
	      fprintf(outfileb," | ");

              pin_list[0] = 5;  // DLE
              pin_list[1] = 14;
              pin_list[2] = 6;

              print_veri_pingroupb(3,pin_list,ipkgind);
	    }

          fprintf(outfileb," ;\n"); // M=GLE+DFE+DLC+DLE
	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[15]),pins[15]);  // n = ~M
         }

      if (pins[9][0] != 0 )
        {
         fprintf(outfileb," %s <= %s ; \n",pins[9],pins[11]);
         fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[9]),pins[9]);
        }
      // H = J
     
    }

  if (ctype == 'U')
    {

      if (pins[7][0] != 0 )   // F
	{
          if (pins[6][0] != 0 )
	    {
              fprintf(outfilea,"assign %s = %s & ~%s & ~%s & %s | ",
	        pins[7],pins[1],pins[3],pins[2],pins[6]); // F=AcbE
              fprintf(outfilea," %s & ~%s & %s & %s | ",
	        pins[16],pins[3],pins[2],pins[6]); // + NcBE

              fprintf(outfilea," %s & %s & ~%s & %s | ",
	        pins[13],pins[3],pins[2],pins[6]); // KCbE

              fprintf(outfilea," %s & %s & %s & %s; \n",
	        pins[5],pins[3],pins[2],pins[6]); // DCBE
	    }
          else

	    {
              fprintf(outfilea,"assign %s = %s & ~%s & ~%s  | ",
	        pins[7],pins[1],pins[3],pins[2]); // F=Acb
              fprintf(outfilea," %s & ~%s & %s  | ",
	        pins[16],pins[3],pins[2]); // + NcB

              fprintf(outfilea," %s & %s & ~%s  | ",
	        pins[13],pins[3],pins[2]); // KCb

              fprintf(outfilea," %s & %s & %s; \n",
	        pins[5],pins[3],pins[2]); // DCB
	    }

          
      fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[7]),pins[7]);  // g = ~F
	}



      if (pins[10][0] != 0 )  // I
	{
     
	  if (pins[11][0] != 0 )  // J
	    {
              fprintf(outfilea,"assign %s = %s & ~%s & ~%s & %s | ",
	        pins[10],pins[1],pins[14],pins[15],pins[11]); // I=AlmJ
              fprintf(outfilea,"        %s & ~%s & %s & %s | \n",
	        pins[16],pins[14],pins[15],pins[11]); // + NlMJ

              fprintf(outfilea," %s & %s & ~%s & %s | ",
	        pins[13],pins[14],pins[15],pins[11]); // KLmJ

              fprintf(outfilea," %s & %s & %s & %s; \n",
	        pins[5],pins[14],pins[15],pins[11]); // DLMJ
	    }
          else
	    {
              fprintf(outfilea,"assign %s = %s & ~%s & ~%s  | ",
	        pins[10],pins[1],pins[14],pins[15]); // I=Alm
              fprintf(outfilea,"        %s & ~%s & %s  | \n",
	        pins[16],pins[14],pins[15]); // + NlM

              fprintf(outfilea," %s & %s & ~%s  | ",
	        pins[13],pins[14],pins[15]); // KLm

              fprintf(outfilea," %s & %s & %s ; \n",
	        pins[5],pins[14],pins[15]); // DLM
	    }

      fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[10]),pins[10]);  // h = ~I
     
          }

    }

  if (ctype == 'X') // clocked
    {
      
      if (pins[6][0] != 0)  // F
	{
          fprintf(outfileb," %s <= ",pins[6]);

	  if (pins[9][0] != 0 )
	    {
              pin_list[0] = 9;
              pin_list[1] = 10;

              print_veri_pingroupb(2,pin_list,ipkgind);
	    }

	    
          if(pins[14][0] != 0 )
	    {
              fprintf(outfileb," | ");

              pin_list[0] = 14;
              pin_list[1] = 15;

              print_veri_pingroupb(2,pin_list,ipkgind);
            }
          if(pins[16][0] != 0 )
            {
              fprintf(outfileb," | ");

              pin_list[0] = 16;
              pin_list[1] = 1;

              print_veri_pingroupb(2,pin_list,ipkgind);

            }
          if(pins[2][0] != 0 )
            {
              fprintf(outfileb," | ");

              pin_list[0] = 2;
              pin_list[1] = 3;

              print_veri_pingroupb(2,pin_list,ipkgind);

            }
           if(pins[11][0] != 0 ) 
            {
              fprintf(outfileb," | ");

              pin_list[0] = 11;
         
              print_veri_pingroupb(1,pin_list,ipkgind);
            }
           fprintf(outfileb," ; \n");
                
	              // E = HI+LM+NA+BC+J
     
      fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[6]),pins[6]);  // d = ~E
         
        }

      if (pins[7][0] != 0 )
	{

          fprintf(outfileb," %s <= ",pins[7]);

	  if (pins[9][0] != 0 )
	    {
              pin_list[0] = 9;
              pin_list[1] = 10;

              print_veri_pingroupb(2,pin_list,ipkgind);
	    }

	    
          if(pins[14][0] != 0 )
	    {
              fprintf(outfileb," | ");

              pin_list[0] = 14;
              pin_list[1] = 15;

              print_veri_pingroupb(2,pin_list,ipkgind);
            }
          if(pins[16][0] != 0 )
            {
              fprintf(outfileb," | ");

              pin_list[0] = 16;
              pin_list[1] = 1;

              print_veri_pingroupb(2,pin_list,ipkgind);

            }
          if(pins[2][0] != 0 )
            {
              fprintf(outfileb," | ");

              pin_list[0] = 2;
              pin_list[1] = 3;

              print_veri_pingroupb(2,pin_list,ipkgind);

            }
           if(pins[11][0] != 0 ) 
            {
              fprintf(outfileb," | ");

              pin_list[0] = 11;
         
              print_veri_pingroupb(1,pin_list,ipkgind);
            }
           fprintf(outfileb," ; \n");
                
	              // F = HI+LM+NA+BC+J
     
	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[7]),pins[7]);  // g = ~F
	}

    }


  if (ctype == 'Y')  //
    {
      if (pins[6][0] != 0 )
	{
          if (pins[10][0] != 0 )
            {
	  fprintf(outfilea,"assign %s = fromttl(%s,%s) ; \n", // E,g = HI
	      pins[6],pins[9],pins[10] );
            }
          else
	    {
	  fprintf(outfilea,"assign %s = fromttl(%s,ZZO) ; \n", // E,g = HI
	      pins[6],pins[9] );
            }
     
	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[6]),pins[6]);  // g = ~E
	}

      if (pins[5][0] != 0 )
	{

          if (pins[10][0] != 0 )
            {      
	  fprintf(outfilea,"assign %s = fromttl(%s,%s) ; \n",    // D,f = JI
	      pins[5],pins[11],pins[10] );
	    }
          else
	    {
             fprintf(outfilea,"assign %s = fromttl(%s,ZZO) ; \n",    // D,f = JI
	      pins[5],pins[11]);
	    }

          fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[5]),pins[5]);  // f = ~D
   
	}

      if (pins[3][0] != 0 )
	{
      
          if (pins[10][0] != 0 )
            {
	  fprintf(outfilea,"assign %s = fromttl(%s,%s) ; \n",   // C,n=LI
	      pins[3],pins[14],pins[10] );
	    }
          else
            {
	  fprintf(outfilea,"assign %s = fromttl(%s,ZZO) ; \n",   // C,n=LI
	      pins[3],pins[14]);
	    }

	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[3]),pins[3]);  // n = ~C
        }

      if (pins[2][0] != 0 )              
	{
	  if (pins[10][0] !=  0 )
	    {
	   fprintf(outfilea,"assign %s = fromttl(%s,%s) ; \n", // B,a = MI
	      pins[2],pins[15],pins[10] );

            }
          else
	    {
             fprintf(outfilea,"assign %s = fromttl(%s,ZZO) ; \n", // B,a = MI
	        pins[2],pins[15] );
            }

	  fprintf(outfilea,"assign %s = ~%s;  //complement \n",flipcase(pins[2]),pins[2]);  // a = ~B
	}

    }

  if (ctype == 'Z')  // 
    {
      if (pins[8][0] != 0 )
	{
          if ( pins[6][0] != 0 )
	    {
	    fprintf(outfilea,"assign %s = fdiff(%s,%s); \n", // G = F / E
		  pins[8],pins[7],pins[6]);
	    }
          else
	    {
	    fprintf(outfilea,"assign %s = fdiff(%s,%s); \n", // G = F / E
		    pins[8],pins[7],"ZZO");         // forced zero
	    }
 
	}

      if (pins[9][0] != 0 )
	{
         if ( pins[10][0] != 0 )
	    {
	  fprintf(outfilea,"assign %s = fdiff(%s,%s) ; \n",    // H = J/I
	      pins[9],pins[11],pins[10] );
            }
         else
	    {
	  fprintf(outfilea,"assign %s = fdiff(%s,%s) ; \n",    // H = J/I
	      pins[9],pins[11],"ZZO" );
            }

 	}

      if (pins[16][0] != 0 )
	{
           if ( pins[14][0] != 0 )
	    {
	 
	     fprintf(outfilea,"assign %s = fdiff(%s,%s) ; \n",   // N = M/L
	        pins[16],pins[15],pins[14] );
            }
          else
	    {
	 
	     fprintf(outfilea,"assign %s = fdiff(%s,%s) ; \n",   // N = M/L
	        pins[16],pins[15],"ZZO" );
            }

	}

      if (pins[1][0] != 0 )              
	{
          if (pins[2][0] != 0 )
	    {
	     fprintf(outfilea,"assign %s = fdiff(%s,%s) ; \n", // A = C/B
	       pins[1],pins[3],pins[2] );
            }
          else
	    {
	     fprintf(outfilea,"assign %s = fdiff(%s,%s) ; \n", // A = C/B
	       pins[1],pins[3],"ZZO" );
            }


	}

    }



}

//
//  Boolean equation output
//
void pkg_outb( int ipkgind, int brdnum )
{
  int i;
  int unum;
  char ctype;
  char pins[20][10];
  int noplus;

  char tlocstr[20];   // location string
  int firstgrp;

  for(i=1;i<17;i++)
    {
      strncpy(pins[i],pkgarray[ipkgind].pinterms[i],10);
      //  printf("For package = %d Pin = %s i = %d \n",ipkgind,pins[i],i);
    }

  ctype = pkgarray[ipkgind].chiptype;
  strncpy(tlocstr,pkgarray[ipkgind].locstr,10);


  // fprintf(outfile,"ctype = %c ipkgind = %d \n",ctype,ipkgind);

  if (ctype == 'A')
    {
      if (pins[5][0] !=0)
	{
         fprintf(outfile,"%s%cD ",tlocstr,ctype);
         fprintf(outfile,"%s = ",pins[5]);
	 if(pins[13][0] != 0)
	   {
            fprintf(outfile,"%s %s %s %s ",
		pins[13],pins[14],pins[15],pins[16]);             // D=KLMN
           }
         if(pins[9][0] != 0 )
	   {
            fprintf(outfile,"+ %s %s %s ",
	      pins[9],pins[10],pins[11]); // HIJ
	   }
         if(pins[1][0] != 0 )
	   {
           fprintf(outfile,"+ %s %s %s ",
	      pins[1],pins[2],pins[3]); // ABC
           }
         fprintf(outfile," .\n");

         fprintf(outfile,"%s%cE ",tlocstr,ctype);
         fprintf(outfile,"%s = ~%s . \n",flipcase(pins[5]),pins[5]);  // e=~D
	}

      if (pins[8][0] != 0 )
	{
        
         fprintf(outfile,"%s%cG ",tlocstr,ctype);
         fprintf(outfile,"%s = ",pins[8]);
	 if (pins[13][0] != 0 )
           {
	     fprintf(outfile,"%s %s %s %s ",
	      pins[13],pins[14],pins[15],pins[16]); // G=KLMN
           }
         if (pins[9][0] != 9 )
           {
             fprintf(outfile,"+ %s %s %s ",
	      pins[9],pins[10],pins[11]); // HIJ
           }

	 if (pins[2][0] != 0 )
	   {
            fprintf(outfile,"+ %s %s %s ",
	      pins[1],pins[2],pins[3]); // ABC
           }
	 fprintf(outfile," . \n" );

         fprintf(outfile,"%s%cF ",tlocstr,ctype);
	 fprintf(outfile,"%s = ~%s . \n",flipcase(pins[8]),pins[8]);  // f=~G
	}
    }


  if (ctype == 'B')
    {
      if (pins[10][0] != 0 )   // I,h
        {
	  fprintf(outfile,"%s%cI ",tlocstr,ctype);
          fprintf(outfile,"%s = ",pins[10]);

          if (pins[11][0] != 0)
	    {
	      fprintf(outfile,"%s %s ",
		      pins[11],pins[13]);   //jk
            }

          if (pins[14][0] != 0 )
	    {
	      fprintf(outfile," + %s %s ", 
		      pins[14],pins[15]);
	    }

	  if (pins[16][0] != 0 )
            {
              fprintf(outfile," + %s %s ",
		      pins[16],pins[1]);
            }

	  if (pins[2][0]  != 0 )
            {
	      fprintf(outfile," + %s %s ",
                       pins[2],pins[3]); // I=JK+LM+NA+BC
	    }

	      fprintf(outfile," . \n");

         fprintf(outfile,"%s%cH ",tlocstr,ctype);
         fprintf(outfile,"%s = ~%s . \n",flipcase(pins[10]),pins[10]);  // h=~I
	}

      if (pins[7][0] != 0 )   //F,g
        { 
          fprintf(outfile,"%s%cF ",tlocstr,ctype);

          fprintf(outfile,"%s = ",pins[7]);

          if (pins[11][0] != 0)
	    {
	      fprintf(outfile,"%s %s ",
		      pins[11],pins[13]);   //jk
            }

          if (pins[14][0] != 0 )
	    {
	      fprintf(outfile," + %s %s ", 
		      pins[14],pins[15]);
	    }

	  if (pins[16][0] != 0 )
            {
              fprintf(outfile," + %s %s ",
		      pins[16],pins[1]);
            }

	  if (pins[2][0]  != 0 )
            {
	      fprintf(outfile," + %s %s ",
                       pins[2],pins[3]); // F=JK+LM+NA+BC
	    }

	   fprintf(outfile," . \n");

          fprintf(outfile,"%s%cG ",tlocstr,ctype);
          fprintf(outfile,"%s = %s . \n",flipcase(pins[7]),pins[7]);  //g = ~F
        }
     
      if (pins[6][0] != 0 )
        {
          fprintf(outfile,"%s%cE ",tlocstr,ctype);
          fprintf(outfile,"%s = ",pins[6]);

          if (pins[11][0] != 0)
	    {
	      fprintf(outfile,"%s %s ",
		      pins[11],pins[13]);   //jk
            }

          if (pins[14][0] != 0 )
	    {
	      fprintf(outfile," + %s %s ", 
		      pins[14],pins[15]);
	    }

	  if (pins[16][0] != 0 )
            {
              fprintf(outfile," + %s %s ",
		      pins[16],pins[1]);
            }

	  if (pins[2][0]  != 0 )
            {
	      fprintf(outfile," + %s %s ",
                       pins[2],pins[3]); // E=JK+LM+NA+BC
	    }

	   fprintf(outfile," . \n");

       
         fprintf(outfile,"%s%cD ",tlocstr,ctype);
         fprintf(outfile,"%s = ~%s . \n",flipcase(pins[6]),pins[6]);  // d=~E
        }
     

    }

  if (ctype == 'C')
    {
      if (pins[7][0] != 0 )
	{
	 
        fprintf(outfile,"%s%cF ",tlocstr,ctype);
        fprintf(outfile,"%s = ",pins[7]);

        noplus = FALSE;
        if (pins[5][0] != 0 )
          {
	    if (pins[6][0] != 0 )
              {
		fprintf(outfile,"%s %s ",pins[5],pins[6]);
              }
            else
	      {
                fprintf(outfile,"%s ",pins[5]);
	      }
	  }
        else
          {
	    if (pins[6][0] != 0 )
	      {
                fprintf(outfile,"%s ", pins[6]);
              }
            else
              {
                noplus = TRUE;
              }
	  }
        if (noplus == FALSE)
          {
            if (pins[9][0] != 0 )
	      {
	       fprintf(outfile," + ");
	      }
          }

        noplus = FALSE;         
        if (pins[9][0] != 0 )
          {
	    if (pins[10][0] != 0 )
              {
		fprintf(outfile,"%s %s ",pins[9],pins[10]);
              }
            else
	      {
                fprintf(outfile,"%s ",pins[9]);
	      }
	  }
        else
          {
	    if (pins[10][0] != 0 )
	      {
                fprintf(outfile,"%s ", pins[10]);
              }
            else
              {
                noplus = TRUE;
              }
	  }
        if (noplus == FALSE)
          {
            if (pins[11][0] != 0 )
	      {
	       fprintf(outfile," + ");
              }
          }

        noplus = FALSE;         
        if (pins[11][0] != 0 )
          {
	    if (pins[13][0] != 0 )
              {
		fprintf(outfile,"%s %s ",pins[11],pins[13]);
              }
            else
	      {
                fprintf(outfile,"%s ",pins[11]);
	      }
	  }
        else
          {
	    if (pins[13][0] != 0 )
	      {
                fprintf(outfile,"%s ", pins[13]);
              }
            else
              {
                noplus = TRUE;
              }
	  }
        if (noplus == FALSE)
          {
            if (pins[14][0] != 0 )
	      {
	       fprintf(outfile," + ");
              }
          }

        noplus = FALSE;         

        if (pins[14][0] != 0 )
          {
	    if (pins[15][0] != 0 )
              {
		fprintf(outfile,"%s %s ",pins[14],pins[15]);
              }
            else
	      {
                fprintf(outfile,"%s ",pins[14]);
	      }
	  }
        else
          {
	    if (pins[15][0] != 0 )
	      {
                fprintf(outfile,"%s ", pins[15]);
              }
            else
              {
                noplus = TRUE;
              }
	  }
        if (noplus == FALSE)
          {
            if (pins[16][0] != 0 )
              {
	    fprintf(outfile," + ");
	      }
          }

        noplus = FALSE;         

        if (pins[16][0] != 0 )
          {
	    if (pins[1][0] != 0 )
              {
		fprintf(outfile,"%s %s ",pins[16],pins[1]);
              }
            else
	      {
               fprintf(outfile,"%s ",pins[16]);
	      }
	  }
        else
          {
	    if (pins[1][0] != 0 )
	      {
                fprintf(outfile,"%s ", pins[1]);
              }
            else
              {
                noplus = TRUE;
              }
	  }
        if (noplus == FALSE)
          {
            if (pins[2][0] != 0 )
	      {
	       fprintf(outfile," + ");
              }
          }

        noplus = FALSE;         

        if (pins[2][0] != 0 )
          {
	    if (pins[3][0] != 0 )
              {
		fprintf(outfile,"%s %s ",pins[2],pins[3]);
              }
            else
	      {
               fprintf(outfile,"%s ",pins[2]);
	      }
	  }
        else
          {
	    if (pins[3][0] != 0 )
	      {
                fprintf(outfile,"%s ", pins[3]);
              }
            else
              {
                noplus = TRUE;
              }
	  }
      
	 fprintf(outfile,";\n");
      
	         // F = DE+NI+JK+LM+NA+BC

        fprintf(outfile,"%s%cG ",tlocstr,ctype);
        fprintf(outfile,"%s = ~%s . \n",pins[8],pins[7]);  // g = ~F
	}

    }

  if (ctype == 'D') // F = HIMJLK + NMJLK + AJLK + DLK + EK +BC
    {
     
      if (pins[7][0] != 0 )  // F
	{
          firstgrp = TRUE;
          fprintf(outfile,"%s%cF ",tlocstr,ctype);
          fprintf(outfile,"%s = ",pins[7]);
          if (pins[9][0] != 0 )  // H pin
            {
              if (strncmp(pins[9],"ZZO",6) != 0 )
		{
                 fprintf(outfile,"%s %s %s %s %s %s ",
	          pins[9],pins[10],pins[15],pins[11],pins[14],pins[13] );
		 firstgrp = FALSE;
		}
	    }
          if (pins[16] != 0 ) // N pin
            {
	      if (strncmp(pins[16],"ZZO",6) != 0 )
		{
		  if (firstgrp == FALSE)
		    {
                     fprintf(outfile,"+ %s %s %s %s %s ",
	              pins[16],pins[15],pins[11],pins[14],pins[13] );
		    }
                  else
		    {
                     fprintf(outfile,"%s %s %s %s %s ",
	              pins[16],pins[15],pins[11],pins[14],pins[13] );
                     firstgrp = FALSE;
		    }

                }
	    }
	  if (pins[1][0] != 0 )  // A Pin
            {
              if (strncmp(pins[1],"ZZO",6) != 0 )
		{
		  if (firstgrp == FALSE)
		    {
                     fprintf(outfile," + %s %s %s %s ",
	              pins[1],pins[11],pins[14],pins[13] );
                    }
                  else
		    {
                     fprintf(outfile,"%s %s %s %s ",
	              pins[1],pins[11],pins[14],pins[13] );
                     firstgrp = FALSE;
                    }

		}
            }
          if (pins[5][0] != 0 )  // D Pin
	    {
              if (strncmp(pins[5],"ZZO",6) != 0 )
		{
		  if (firstgrp == FALSE)
                    {
                     fprintf(outfile," + %s %s %s ",
	              pins[5],pins[14],pins[13] );
		    }
                  else
		    {
                     fprintf(outfile,"%s %s %s ",
	              pins[5],pins[14],pins[13] );
		     firstgrp = FALSE;
		    }
                    
		}
            }
          if (pins[6][0] != 0 )   // E Pin
            {
              if (strncmp(pins[6],"ZZO",6) != 0 )
		{
                  if (firstgrp == FALSE)
                    {
                     fprintf(outfile,"+ %s %s ",
	              pins[6],pins[13] );
                    }
                  else
                    {
                     fprintf(outfile,"%s %s ",
	              pins[6],pins[13] );
		     firstgrp = FALSE;
                    }

                }
            }

	  if (pins[2][0] != 0 )  // B pin
	    {
	      if (strncmp(pins[2],"ZZO",6) != 0 )
		{
		  if (firstgrp == FALSE)
		    {
                     fprintf(outfile,"+ %s %s ",
	              pins[2],pins[3] );    
                    }
		  else
		    {
                     fprintf(outfile,"%s %s ",
	              pins[2],pins[3] );    
                    }

		}    
	    }

	  fprintf(outfile," . \n");     
          	      
                   // F = HIMJLK + NMJLK + AJLK + DLK + EK +BC
          fprintf(outfile,"%s%cG ",tlocstr,ctype);
          fprintf(outfile,"%s = ~%s . \n",flipcase(pins[7]),pins[7]);  // g = ~F

	}  
     
    }

  if (ctype == 'E')
    {

      fprintf(outfile,"%s%cD ",tlocstr,ctype);
      fprintf(outfile,"%s = %s %s . \n",
	      pins[5],pins[3],pins[11]);
           	      
                      
      // D = CJ

      fprintf(outfile,"%s = %s . \n",pins[5],pins[5]);  // G = D
      fprintf(outfile,"%s = %s .\n",pins[5],pins[5]);  // H = D
      fprintf(outfile,"%s = %s . \n",pins[5],pins[5]);  // L = D
      fprintf(outfile,"%s = %s . \n",pins[5],pins[5]);  // M = D
      fprintf(outfile,"%s = %s .\n",pins[5],pins[5]);  // A = D

      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[5]),pins[5]);  // e = ~D
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[5]),pins[5]);  // f = ~D
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[5]),pins[5]);  // i = ~D
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[5]),pins[5]);  // k = ~D
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[5]),pins[5]);  // n = ~D
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[5]),pins[5]);  // b = ~D

    }


  if (ctype == 'F')
    {  
      fprintf(outfile,"%s%cL ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s ~%s ~%s %s %s . \n",
	      pins[14],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // L = bcdJK   
     
      fprintf(outfile,"%s%cM ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s ~%s  %s %s %s . \n",
	      pins[15],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // M = bcDJK   

      fprintf(outfile,"%s%cN ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s  %s ~%s %s %s . \n",
	      pins[16],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // N = bCdJK  

      fprintf(outfile,"%s%cA ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s  %s  %s %s %s . \n",
	      pins[1],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // A = bCDJK  
     
      fprintf(outfile,"%s%cF ",tlocstr,ctype);
      fprintf(outfile,"%s =  %s ~%s ~%s %s %s . \n",
	      pins[7],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // F = BcdJK   

      fprintf(outfile,"%s%cG ",tlocstr,ctype);
      fprintf(outfile,"%s =  %s ~%s  %s %s %s . \n",
	      pins[8],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // G = BcDJK   

      fprintf(outfile,"%s%cH ",tlocstr,ctype);
      fprintf(outfile,"%s =  %s  %s ~%s %s %s . \n",
	      pins[9],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // H = BCdJK  

      fprintf(outfile,"%s%cI ",tlocstr,ctype);
      fprintf(outfile,"%s =  %s  %s  %s %s %s . \n",
	      pins[10],pins[2],pins[3],pins[5],pins[11],pins[13]);
      // I = BCDJK  

      if (pins[6][0] != 0 )
        {
         fprintf(outfile,"%s%cE ",tlocstr,ctype);
         fprintf(outfile,"%s = ~%s + ~%s . \n",
	      pins[6],pins[11],pins[13]);
	}
    

      // E = j + k  

    }


  if (ctype == 'G')
   {  
      if (pins[6][0] != 0 )  // E = L ttt + l TTT ; C B A.
	{
         fprintf(outfile,"%s%cE ",tlocstr,ctype);
          fprintf(outfile,"%s = %s ttt + %s TTT ; ",
		  pins[6],pins[14],flipcase(pins[14]));
	 if ( pins[3][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[3]);
           }
         if ( pins[2][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[2]);
           }
         if ( pins[1][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[1]);
           }
	 fprintf(outfile," .\n" );

         fprintf(outfile,"%s%cD %s = ~%s .\n",tlocstr,ctype,flipcase(pins[6]),pins[6]);
         }

      if (pins[7][0] != 0 )  // F = K ttt + k TTT ; L C B A.
	{
         fprintf(outfile,"%s%cF ",tlocstr,ctype);
          fprintf(outfile,"%s = %s ttt + %s TTT ; ",
		  pins[7],pins[13],flipcase(pins[13]));

	 if ( pins[14][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[14]);
           }
	 if ( pins[3][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[3]);
           }
         if ( pins[2][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[2]);
           }
         if ( pins[1][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[1]);
           }
	 fprintf(outfile," .\n");

         fprintf(outfile,"%s%cG %s = ~%s .\n",tlocstr,ctype,flipcase(pins[7]),pins[7]);
         }

      if (pins[10][0] != 0 )  // I = J ttt + j TTT ; K L C B A.
	{
         fprintf(outfile,"%s%cI ",tlocstr,ctype);
          fprintf(outfile,"%s = %s ttt + %s TTT ; ",
		  pins[10],pins[11],flipcase(pins[11]));

	 if ( pins[13][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[13]);
           }
	 if ( pins[14][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[14]);
           }
	 if ( pins[3][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[3]);
           }
         if ( pins[2][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[2]);
           }
         if ( pins[1][0] != 0 )
	   {
	     fprintf(outfile,"%s ",pins[1]);
           }
	 fprintf(outfile," .\n");

         fprintf(outfile,"%s%cI %s = ~%s .\n",tlocstr,ctype,flipcase(pins[10]),pins[10]);
         }


    }

  if (ctype == 'H')
    {
      if (pins[8][0] != 0 )  // G = EDCBAN
	{
      fprintf(outfile,"%s%cG ",tlocstr,ctype);
      fprintf(outfile,"%s = %s %s %s %s %s %s . \n",
	      pins[8],pins[6],pins[5],pins[3],pins[2],pins[1],pins[16] );

      fprintf(outfile,"%s%cF ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s; \n",flipcase(pins[8]),pins[8]);  // f = ~G
	}
      
      if (pins[9][0] != 0 )   // H = JKLM
	{
           fprintf(outfile,"%s%cH ",tlocstr,ctype);
           fprintf(outfile,"%s = %s %s %s %s .\n",
	     pins[9],pins[11],pins[13],pins[14],pins[15]);
        
           fprintf(outfile,"%s%cI ",tlocstr,ctype);
	   fprintf(outfile,"%s = ~%s . \n",flipcase(pins[9]),pins[9]);  // i = ~H
	}

    }

  if (ctype == 'I')
    {
      if (pins[10][0] != 0 )    // I=JKL
	{
         fprintf(outfile,"%s%cI ",tlocstr,ctype);
         fprintf(outfile,"%s = %s %s %s . \n",
	      pins[10],pins[11],pins[13],pins[14] );

         fprintf(outfile,"%s%cH ",tlocstr,ctype);
	 fprintf(outfile,"%s = ~%s . \n",flipcase(pins[10]),pins[10]);  // h = ~I

	}

      if (pins[6][0] != 0 )       // E = CBA
	{
        fprintf(outfile,"%s%cE ",tlocstr,ctype);
        fprintf(outfile,"%s = %s %s %s . \n",
	      pins[6],pins[3],pins[2],pins[1]);

        fprintf(outfile,"%s%cD ",tlocstr,ctype);
        fprintf(outfile,"%s = ~%s . \n",flipcase(pins[6]),pins[6]);  // d = ~E
   
        }

      if (pins[8][0] != 0 )  // G = MN
        {
         fprintf(outfile,"%s%cG ",tlocstr,ctype);
         fprintf(outfile,"%s = %s %s . \n",
	      pins[8],pins[15],pins[16]);

         fprintf(outfile,"%s%cF ",tlocstr,ctype);
	 fprintf(outfile,"%s = ~%s . \n",flipcase(pins[8]),pins[8]);  // f = ~G

	}

    }


  if (ctype == 'J')
    {    
      if (pins[5][0] != 0 )
	{
           // G=DE
        if (pins[8][0] != 0 )
         {
          fprintf(outfile,"%s%cG ",tlocstr,ctype);
          fprintf(outfile,"%s = %s %s . \n",  
	      pins[8],pins[5],pins[6] );

          fprintf(outfile,"%s%cF ",tlocstr,ctype);
	  fprintf(outfile,"%s = ~%s . \n",flipcase(pins[8]),pins[8]);  // f = ~G
          }

         if (pins[9][0] != 0 )  // H=DJ
	  {
           fprintf(outfile,"%s%cH ",tlocstr,ctype);
           fprintf(outfile,"%s = %s %s . \n",
	      pins[9],pins[5],pins[11] );

           fprintf(outfile,"%s%cI ",tlocstr,ctype);
	   fprintf(outfile,"%s = ~%s .\n",flipcase(pins[9]),pins[9]);  // i = ~H
     	  }
        }
      else  // No D
	{
           // G=DE
        if (pins[8][0] != 0 )
         {
          fprintf(outfile,"%s%cG ",tlocstr,ctype);
          fprintf(outfile,"%s = %s . \n",  
	      pins[8],pins[6] );

          fprintf(outfile,"%s%cF ",tlocstr,ctype);
	  fprintf(outfile,"%s = ~%s . \n",flipcase(pins[8]),pins[8]);  // f = ~G
          }


         if (pins[9][0] != 0 )  // H=DJ
	  {
           fprintf(outfile,"%s%cH ",tlocstr,ctype);
           fprintf(outfile,"%s = %s . \n",
	      pins[9],pins[11] );

           fprintf(outfile,"%s%cI ",tlocstr,ctype);
	   fprintf(outfile,"%s = ~%s . \n",flipcase(pins[9]),pins[9]);  // i = ~H
     	  }
        }

      if (pins[13][0] !=0 )
        {
         if (pins[16][0] != 0 )  // N=KL
	  {
           fprintf(outfile,"%s%cN ",tlocstr,ctype);
           fprintf(outfile,"%s = %s %s . \n",
	      pins[16],pins[13],pins[14] );

            fprintf(outfile,"%s%cM ",tlocstr,ctype);
            fprintf(outfile,"%s = ~%s . \n",flipcase(pins[16]),pins[16]);  // m = ~N
           }

          if (pins[1][0] != 0 )  // A=KC
	   {
            fprintf(outfile,"%s%cA ",tlocstr,ctype);
            fprintf(outfile,"%s = %s %s . \n",
	      pins[1],pins[13],pins[3] );
   
            fprintf(outfile,"%s%cB ",tlocstr,ctype);
            fprintf(outfile,"%s = ~%s . \n",flipcase(pins[1]),pins[1]);  // b = ~A          
	   }
      	}
      else
       {
         if (pins[16][0] != 0 )  // N=KL
	  {
           fprintf(outfile,"%s%cN ",tlocstr,ctype);
           fprintf(outfile,"%s = %s . \n",
	      pins[16],pins[14] );

            fprintf(outfile,"%s%cM ",tlocstr,ctype);
            fprintf(outfile,"%s = ~%s . \n",flipcase(pins[16]),pins[16]);  // m = ~N
           }

          if (pins[1][0] != 0 )  // A=KC
	   {
            fprintf(outfile,"%s%cA ",tlocstr,ctype);
            fprintf(outfile,"%s = %s . \n",
	      pins[1],pins[3] );
   
            fprintf(outfile,"%s%cB ",tlocstr,ctype);
            fprintf(outfile,"%s = ~%s . \n",flipcase(pins[1]),pins[1]);  // b = ~A        
	   }
       }
    }


  if (ctype == 'K')
    {
      if (pins[9][0] != 0 )
	{
         fprintf(outfile,"%s%cH ",tlocstr,ctype);
         fprintf(outfile,"%s = %s %s %s + ",
	      pins[9],pins[13],pins[14],pins[11]); // H=KLJ
         fprintf(outfile,"%s %s %s . \n",
	      pins[15],pins[16],pins[3]); // MNC

         fprintf(outfile,"%s%cI ",tlocstr,ctype);
	 fprintf(outfile,"%s = ~%s . \n",flipcase(pins[9]),pins[9]); 
                                    // i=~H        
          }

	 if (pins[7][0] != 0 )
	   {
           fprintf(outfile,"%s%cF ",tlocstr,ctype);
           fprintf(outfile,"%s = %s %s %s + ",
	      pins[7],pins[1],pins[2],pins[11]); // ABJ
           fprintf(outfile,"%s %s %s . \n",
	      pins[5],pins[6],pins[3]); // DEC
          
           fprintf(outfile,"%s%cG ",tlocstr,ctype);
           fprintf(outfile,"%s = ~%s . \n",flipcase(pins[7]),pins[7]); 
                                          // g=~F
	}

    }



  if (ctype == 'L')  // clocked
    {
      
      if (pins[5][0] != 0 )
	{
	  if (pins[8][0] != 0 )
	    {
              fprintf(outfile,"%s%cG ",tlocstr,ctype);
	      fprintf(outfile,"%s = %s %s . \n", // G,f = DE
	         pins[8],pins[5],pins[6] );

              fprintf(outfile,"%s%cF ",tlocstr,ctype);
	     fprintf(outfile,"%s = ~%s . \n",flipcase(pins[8]),pins[8]);
                              // f = ~G
	     }

              // G=DE

           if (pins[9][0] != 0 )
	    {
             fprintf(outfile,"%s%cH ",tlocstr,ctype);
	      fprintf(outfile,"%s = %s %s . \n",    // H,i = DJ
	          pins[9],pins[5],pins[11] );

             fprintf(outfile,"%s%cI ",tlocstr,ctype);
              fprintf(outfile,"%s = ~%s . \n",flipcase(pins[9]),pins[9]); 
                          // i = ~H
   
 	      }
   
               // H=DJ

            if (pins[16][0] != 0 )
	     {
              fprintf(outfile,"%s%cN ",tlocstr,ctype);
	      fprintf(outfile,"%s = %s %s . \n",   // N,m = DL
	         pins[16],pins[5],pins[14] );

               fprintf(outfile,"%s%cM ",tlocstr,ctype);
	       fprintf(outfile,"%s = ~%s . \n",flipcase(pins[16]),pins[16]);
                                  // m = ~N
                }

                // N=DL

            if (pins[1][0] != 0 )              
	       {
                fprintf(outfile,"%s%cA ",tlocstr,ctype);
	        fprintf(outfile,"%s = %s %s . \n", // A,b = DC
	          pins[1],pins[5],pins[3] );

                fprintf(outfile,"%s%cB ",tlocstr,ctype);
	        fprintf(outfile,"%s = ~%s . \n",flipcase(pins[1]),pins[1]); 
                            // b = ~A
      	        }
   
               // A=DC
	}
      else
        {
         if (pins[8][0] != 0 )
	    {
              fprintf(outfile,"%s%cG ",tlocstr,ctype);
	      fprintf(outfile,"%s = %s . \n", // G,f = DE
		      pins[8],pins[6] );

              fprintf(outfile,"%s%cF ",tlocstr,ctype);
	     fprintf(outfile,"%s = ~%s .\n",flipcase(pins[8]),pins[8]);
                              // f = ~G
	     }

              // G=DE

           if (pins[9][0] != 0 )
	    {
             fprintf(outfile,"%s%cH ",tlocstr,ctype);
	      fprintf(outfile,"%s = %s . \n",    // H,i = DJ
	          pins[9],pins[11] );

             fprintf(outfile,"%s%cI ",tlocstr,ctype);
              fprintf(outfile,"%s = ~%s . \n",flipcase(pins[9]),pins[9]); 
                          // i = ~H
   
 	      }
   
               // H=DJ

            if (pins[16][0] != 0 )
	     {
              fprintf(outfile,"%s%cN ",tlocstr,ctype);
	      fprintf(outfile,"%s = %s . \n",   // N,m = DL
	         pins[16],pins[14] );

               fprintf(outfile,"%s%cM ",tlocstr,ctype);
	       fprintf(outfile,"%s = ~%s . \n",flipcase(pins[16]),pins[16]);
                                  // m = ~N
                }

                // N=DL

            if (pins[1][0] != 0 )              
	       {
                fprintf(outfile,"%s%cA ",tlocstr,ctype);
	        fprintf(outfile,"%s = %s . \n", // A,b = DC
	          pins[1],pins[3] );

                fprintf(outfile,"%s%cB ",tlocstr,ctype);
	        fprintf(outfile,"%s = ~%s . \n",flipcase(pins[1]),pins[1]); 
                            // b = ~A
      	        }
   
               // A=DC

	}
    }

  if (ctype == 'M')  // clocked
    {

      //printf("Puting out M chip \n");

      if (pins[6][0] != 0 )  // E,d
	{
        fprintf(outfile,"%s%cE ",tlocstr,ctype);
        fprintf(outfile,"%s = ",
		pins[6]);
        if (pins[15][0] != 0 )
          {
           fprintf(outfile," %s %s %s ",pins[15],pins[16],pins[1]);
          }
        if (pins[11][0] != 0 )
	  {
	   fprintf(outfile," + %s %s ",pins[11],pins[14] );
          }
	if (pins[2][0] != 0 )
          {
            fprintf(outfile," + %s %s ", pins[2],pins[3] );
	  }
       
	 fprintf(outfile," . \n");

         fprintf(outfile,"%s%cD ",tlocstr,ctype);
         fprintf(outfile,"%s = ~%s . \n",flipcase(pins[6]),pins[6]);  // d = ~E
                   // E = MNA+JL+BC
	}

      if (pins[7][0] != 0 )   // Pin F,g defined as copy
        {
         fprintf(outfile,"%s%cF ",tlocstr,ctype);
         fprintf(outfile,"%s = ",
		pins[7]);
        if (pins[15][0] != 0 )
          {
           fprintf(outfile," %s %s %s ",pins[15],pins[16],pins[1]);
          }
        if (pins[11][0] != 0 )
	  {
	   fprintf(outfile," + %s %s ",pins[11],pins[14] );
          }
	if (pins[2][0] != 0 )
          {
            fprintf(outfile," + %s %s ", pins[2],pins[3] );
	  }
       
	 fprintf(outfile," . \n");

         fprintf(outfile,"%s%cG ",tlocstr,ctype);
         fprintf(outfile,"%s = ~%s . \n",flipcase(pins[7]),pins[7]);  // g = ~E
        }
       
      if (pins[10][0] != 0 )   // Pin I,h defined as copy
	{
         fprintf(outfile,"%s%cI ",tlocstr,ctype);
         fprintf(outfile,"%s = ", pins[10]);
	
        if (pins[15][0] != 0 )
          {
           fprintf(outfile," %s %s %s ",pins[15],pins[16],pins[1]);
          }
        if (pins[11][0] != 0 )
	  {
	   fprintf(outfile," + %s %s ",pins[11],pins[14] );
          }
	if (pins[2][0] != 0 )
          {
            fprintf(outfile," + %s %s ", pins[2],pins[3] );
	  }
       
	 fprintf(outfile," . \n");

         fprintf(outfile,"%s%cH ",tlocstr,ctype);
	 fprintf(outfile,"%s = ~%s . \n",flipcase(pins[10]),pins[10]);  // h = ~E
	}

    } // M Chip

  if (ctype == 'N')  // clocked
    {
      if (pins[10][0] != 0 ) // I,h = GFEAJ + DEAJ + NAJ + MLJ + CB; 
	{
           fprintf(outfile,"%s%cI ",tlocstr,ctype);
           if (pins[8][0] != 0 )  //G
	     {
              fprintf(outfile,"%s = %s %s %s %s %s ",
	        pins[10],pins[8],pins[7],pins[6],pins[1],pins[11] );
	     }
	   else
	     {
               fprintf(outfile,"%s = ZZO ",pins[10]);
             }

           if (pins[5][0] != 0 ) // DEAJ
             {
               fprintf(outfile,"+ %s %s %s %s ",
	         pins[5],pins[6],pins[1],pins[11] );
             }
           if (pins[16][0] != 0 )  // NAJ
             {
              fprintf(outfile,"+ %s %s %s ",
	         pins[16],pins[1],pins[11] );
             }
           if (pins[15][0] != 0 )  // MLJ
             {
              fprintf(outfile,"+ %s %s %s ",
	           pins[15],pins[14],pins[11] );
	     }
	   if (pins[3][0] != 0 )      // CB
             {
             fprintf(outfile,"+ %s %s ",
    	                 pins[3],pins[2] );
             }
                	      
	   fprintf(outfile," . \n");
                      
          	      
       // I = GFEAJ + DEAJ + NAJ + MLJ + CB;
      fprintf(outfile,"%s%cH ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[10]),pins[10]);  // h = ~I
	}

    }

  if (ctype == 'O')   // clocked
    {
      if (pins[15][0] != 0 )
	{
         fprintf(outfile,"%s%cM ",tlocstr,ctype);
         fprintf(outfile,"%s = %s %s + ",
	      pins[15],pins[14],pins[5]); // M=LD
         if (pins[1][0] != 0 )
           {
            fprintf(outfile,"%s %s . \n",
		 pins[1],flipcase(pins[5]) ); // Ad
           }
         else
           {
            fprintf(outfile,"ZZO %s . \n",
		 flipcase(pins[5]) ); // Ad
           }

         fprintf(outfile,"%s%cN ",tlocstr,ctype);
         fprintf(outfile,"%s = ~%s . \n",flipcase(pins[15]),pins[15]);  // n=~M
        }

      if (pins[10][0] != 0 )
	{
        fprintf(outfile,"%s%cI ",tlocstr,ctype);
        fprintf(outfile,"%s = %s %s + ",
	      pins[10],pins[11],pins[5]); // I=JD
        if (pins[8][0] != 0 )
	  {
           fprintf(outfile,"%s %s . \n",
	   	pins[8],flipcase( pins[5])); // Gd
	  }
        else
	  {
           fprintf(outfile,"ZZO %s . \n",
	   	flipcase( pins[5])); // Gd
	  }

        fprintf(outfile,"%s%cH ",tlocstr,ctype);
        fprintf(outfile,"%s = ~%s . \n",flipcase(pins[10]),pins[10]);  // h=~I
        }

      if (pins[7][0] != 0 )
	{
         fprintf(outfile,"%s%cF ",tlocstr,ctype);
     
         fprintf(outfile,"%s = %s %s + ",
	      pins[7],pins[2],pins[5]); // F=BD
         if (pins[3][0] != 0 )
	   {
           fprintf(outfile,"%s %s . \n",
		pins[3],flipcase (pins[5]) ); // Cd
           }
         else
	   {
           fprintf(outfile,"ZZO %s . \n",
		flipcase (pins[5]) ); // Cd
           }

         fprintf(outfile,"%s%cE ",tlocstr,ctype);
         fprintf(outfile,"%s = ~%s . \n",flipcase(pins[7]),pins[7]);  // e=~F
     
	} 

    }

  if (ctype == 'P')  // clocked
    {
      if (pins[1][0] != 0 )
	{

	  if (pins[2][0] != 0 )
	    {
             fprintf(outfile,"%s%cB ",tlocstr,ctype);
	      fprintf(outfile,"%s = %s %s ", 
			 pins[2],pins[15],pins[10]);
            }

           if (pins[16][0] != 0 )
	       {
                 fprintf(outfile," + %s %s ", 
			 pins[16],pins[11]);
               }

           if (pins[1][0] != 0 )
               {
		 fprintf(outfile," + %s %s ",
                       pins[1],pins[14]);  // B=MI+NJ+AL
	       }

	     fprintf(outfile," . \n");

        fprintf(outfile,"%s%cC ",tlocstr,ctype);
        fprintf(outfile,"%s = ~%s . \n",flipcase(pins[2]),pins[2]);  // c = ~B
        }

      if (pins[5][0] != 0 )
	{

	  if (pins[7][0] != 0 )
	    {
             fprintf(outfile,"%s%cD ",tlocstr,ctype);
	      fprintf(outfile,"%s = %s %s ", 
			 pins[5],pins[7],pins[10]);
            }

           if (pins[8][0] != 0 )
	       {
                 fprintf(outfile," + %s %s ", 
			 pins[8],pins[11]);
               }

           if (pins[9][0] != 0 )
               {
		 fprintf(outfile," + %s %s ",
                       pins[9],pins[14]);  // B=MI+NJ+AL
	       }

	     fprintf(outfile," . \n");

        fprintf(outfile,"%s%cE ",tlocstr,ctype);
        fprintf(outfile,"%s = ~%s . \n",flipcase(pins[5]),pins[5]);  // e = ~D
        }

    }

  if (ctype == 'Q')
    {
      if (pins[7][0] != 0 )
	{
      fprintf(outfile,"%s%cF ",tlocstr,ctype);
      fprintf(outfile,"%s = %s %s %s + %s %s %s + %s %s %s + %s %s %s . \n",
	      pins[7],pins[14],pins[2],pins[1],
	      pins[3],pins[15],pins[1],
              pins[3],pins[2],pins[16],
	      pins[14],pins[15],pins[16]);  // F = LBA+CMA+CBN+LMN

      fprintf(outfile,"%s%cG ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[7]),pins[7]);  // g = ~F

        }

      if (pins[6][0] != 0 )
	{
      fprintf(outfile,"%s%cE ",tlocstr,ctype);
      fprintf(outfile,"%s = %s %s %s + %s %s %s + %s %s %s + %s %s %s . \n",
	      pins[6],pins[14],pins[2],pins[1],
	      pins[3],pins[15],pins[1],
              pins[3],pins[2],pins[16],
	      pins[3],pins[2],pins[1]);  // E = LBA+CMA+CBN+CBA

      fprintf(outfile,"%s%cD ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[6]),pins[6]);  // d = ~E
	}
      
      if (pins[10][0] != 0)
        {
         fprintf(outfile,"%s%cI ",tlocstr,ctype);
         fprintf(outfile,"%s = %s %s . \n",pins[10],pins[11],pins[13]);
      // I = JK

         fprintf(outfile,"%s%cH ",tlocstr,ctype);
	 fprintf(outfile,"%s = ~%s . \n",flipcase(pins[10]),pins[10]);  // h = ~I
        }
     
    }

  if (ctype == 'R')
    {
      if (pins[1][0] != 0 )  // B Pin
	{
	  fprintf(outfile,"%s%cB ",tlocstr,ctype);   // BANM = EDJK; IHGF; CL
      fprintf(outfile,"%s %s %s %s  = %s %s %s %s ; %s %s %s %s ; %s %s \n",
	      pins[2],pins[1],pins[16],pins[15],
	      pins[6],pins[5],pins[11],pins[13],
              pins[10],pins[9],pins[8],pins[7], pins[3],pins[14]);
            
        }

    }

  if (ctype == 'S')
    {
      if (pins[0][0] != 0 )  // A pin
	{

	  fprintf(outfile,"%s%cA ",tlocstr,ctype);   // A = P;BCDFGHIJKLM; ON .
      fprintf(outfile,"%s  = %s ; %s %s %s %s %s %s %s %s %s %s %s %s ; %s %s \n",
	      pins[1],pins[18],pins[2],pins[3],
	      pins[5],pins[6],pins[7],pins[8],
              pins[9],pins[10],pins[11],pins[12],
              pins[13],pins[14],pins[15],pins[17],pins[16]);
        }
     
    }

  if (ctype == 'T')  //clocked
    {
      if (pins[2][0] != 0 )
	{
           fprintf(outfile,"%s%cB ",tlocstr,ctype);
           fprintf(outfile,"%s = %s %s %s + %s %s %s + %s %s %s + %s %s %s . \n",
	      pins[2],pins[8],pins[14],pins[6],
	      pins[5],pins[7],pins[6],
              pins[5],pins[14],pins[3],
	      pins[8],pins[7],pins[3]);  // B=GLE+DFE+DLC+GFC

           fprintf(outfile,"%s%cA ",tlocstr,ctype);
           fprintf(outfile,"%s = ~%s . \n",flipcase(pins[2]),pins[2]);  // a = ~B
        }


      if (pins[15][0] != 0 )
	{
      fprintf(outfile,"%s%cM ",tlocstr,ctype);
      fprintf(outfile,"%s = %s %s %s + %s %s %s + %s %s %s + %s %s %s . \n",
	      pins[15],pins[8],pins[14],pins[6],
	      pins[5],pins[7],pins[6],
              pins[5],pins[14],pins[3],
	      pins[5],pins[14],pins[6]);  // M=GLE+DFE+DLC+DLE

      fprintf(outfile,"%s%cN ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[15]),pins[15]);  // n = ~M       
        }

      if (pins[9][0] != 0 )
        {
      fprintf(outfile,"%s%cH ",tlocstr,ctype);
           // H = J
      fprintf(outfile,"%s = %s . \n",pins[9],pins[11]);  //  H = J

      fprintf(outfile,"%s%cI ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[9]),pins[9]);  //    
        }
     
    }

  if (ctype == 'U')
    {
      if (pins[7][0] != 0 )
	{
      fprintf(outfile,"%s%cF ",tlocstr,ctype);
      fprintf(outfile,"%s = %s ~%s ~%s %s + ",
	      pins[7],pins[1],pins[3],pins[2],pins[6]); // F=AcbE
      fprintf(outfile,"%s ~%s %s %s + ",
	      pins[16],pins[3],pins[2],pins[6]); // + NcBE

      fprintf(outfile,"%s %s ~%s %s + ",
	      pins[13],pins[3],pins[2],pins[6]); // KCbE

      fprintf(outfile,"%s %s %s %s . \n",
	      pins[5],pins[3],pins[2],pins[6]); // DCbE

      fprintf(outfile,"%s%cG ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s .\n",flipcase(pins[7]),pins[7]);  // g = ~F
        }

      if (pins[10][0] != 0 )
	{
      fprintf(outfile,"%s%cI ",tlocstr,ctype);
      fprintf(outfile,"%s = %s ~%s ~%s %s + ",
	      pins[10],pins[1],pins[14],pins[15],pins[11]); // I=AlmJ
      fprintf(outfile,"%s ~%s %s %s + ",
	      pins[16],pins[14],pins[15],pins[11]); // + NlMJ

      fprintf(outfile,"%s %s ~%s %s + ",
	      pins[13],pins[14],pins[15],pins[11]); // KLmJ

      fprintf(outfile,"%s %s %s %s . \n",
	      pins[5],pins[14],pins[15],pins[11]); // DLMJ

       fprintf(outfile,"%s%cH ",tlocstr,ctype);
      fprintf(outfile,"%s = ~%s . \n",flipcase(pins[10]),pins[10]);  // h = ~I
	}     
     

    }

  if (ctype == 'X') // clocked
    {
       
      if (pins[6][0] != 0)
	{
              fprintf(outfile,"%s%cE ",tlocstr,ctype);
              fprintf(outfile,"%s = %s %s ",
		      pins[6],pins[9],pins[10]);

	      if (pins[14][0] != 0 )
		{
                  fprintf(outfile," + %s %s ",
			   pins[14],pins[15]);
                }

	      if (pins[16][0] != 0 )
		{
                  fprintf(outfile," + %s %s ",
			   pins[16],pins[1]);
                }

	      if (pins[2][0] != 0 )
		{
                  fprintf(outfile," + %s %s ",
			   pins[2],pins[3]);
                }

	      if (pins[11][0] != 0 )
		{
                  fprintf(outfile," + %s ",
			   pins[11]);
                }

	      fprintf(outfile," .\n");

	              // E,d = HI+LM+NA+BC+J

         fprintf(outfile,"%s%cD ",tlocstr,ctype);
	 fprintf(outfile,"%s = ~%s . \n",flipcase(pins[6]),pins[6]);  // d = ~E
	}


      if (pins[7][0] != 0)
	{
         fprintf(outfile,"%s%cF ",tlocstr,ctype);
         fprintf(outfile,"%s = %s %s ",
		      pins[7],pins[9],pins[10]);

	      if (pins[14][0] != 0 )
		{
                  fprintf(outfile," + %s %s ",
			   pins[14],pins[15]);
                }

	      if (pins[16][0] != 0 )
		{
                  fprintf(outfile," + %s %s ",
			   pins[16],pins[1]);
                }

	      if (pins[2][0] != 0 )
		{
                  fprintf(outfile," + %s %s ",
			   pins[2],pins[3]);
                }

	      if (pins[11][0] != 0 )
		{
                  fprintf(outfile," + %s ",
			   pins[11]);
                }


	      fprintf(outfile,". \n");


	       // F,g = HI+LM+NA+BC+J
                                 
         fprintf(outfile,"%s%cG ",tlocstr,ctype);
	 fprintf(outfile,"%s = ~%s . \n",flipcase(pins[7]),pins[7]);  // g = ~F

    	}
    }


  if (ctype == 'Y')  //
    {
      if (pins[6][0] != 0 )
	{
          fprintf(outfile,"%s%cE ",tlocstr,ctype);
	  fprintf(outfile,"%s = %s %s . \n", // E,g = HI
	      pins[6],pins[9],pins[10] );

          fprintf(outfile,"%s%cG ",tlocstr,ctype);
	  fprintf(outfile,"%s = ~%s . \n",flipcase(pins[6]),pins[6]);  // g = ~E
	}

      if (pins[5][0] != 0 )
	{
          fprintf(outfile,"%s%cD ",tlocstr,ctype);
	  fprintf(outfile,"%s = %s %s . \n",    // D,f = JI
	      pins[5],pins[11],pins[10] );

          fprintf(outfile,"%s%cF ",tlocstr,ctype);
          fprintf(outfile,"%s = ~%s .\n",flipcase(pins[5]),pins[5]);  // f = ~D
   
	}

      if (pins[3][0] != 0 )
	{
          fprintf(outfile,"%s%cC ",tlocstr,ctype);
	  fprintf(outfile,"%s = %s %s . \n",   // C,n=LI
	      pins[3],pins[14],pins[10] );

           fprintf(outfile,"%s%cN ",tlocstr,ctype);
	  fprintf(outfile,"%s = ~%s . \n",flipcase(pins[3]),pins[3]);  // n = ~C
        }

      if (pins[2][0] != 0 )              
	{
          fprintf(outfile,"%s%cB ",tlocstr,ctype);
	  fprintf(outfile,"%s = %s %s . \n", // B,a = MI
	      pins[2],pins[15],pins[10] );

          fprintf(outfile,"%s%cA ",tlocstr,ctype);
	  fprintf(outfile,"%s = ~%s . \n",flipcase(pins[2]),pins[2]);  // a = ~B
	}

    }

  if (ctype == 'Z')  // 
    {
      if (pins[8][0] != 0 )
	{
          if (pins[6][0] != 0 )
	    {
             fprintf(outfile,"%s%cG ",tlocstr,ctype);
	     fprintf(outfile,"%s = %s/%s . \n", // G = F/E
		  pins[8],pins[7],pins[6]);
            }
          else
	    {
             fprintf(outfile,"%s%cG ",tlocstr,ctype);
	     fprintf(outfile,"%s = %s/%s . \n", // G = F/E
		  pins[8],pins[7],"---");
            }


	}

      if (pins[9][0] != 0 )
	{
          if (pins[10][0] != 0 )
	    {
             fprintf(outfile,"%s%cH ",tlocstr,ctype);
	     fprintf(outfile,"%s = %s/%s . \n",    // H = J/I
	      pins[9],pins[11],pins[10] );
	    }
          else
	    {
             fprintf(outfile,"%s%cH ",tlocstr,ctype);
	     fprintf(outfile,"%s = %s/%s . \n",    // H = J/I
	      pins[9],pins[11],"---");
	    }
 	}

      if (pins[16][0] != 0 )
	{
          if (pins[14][0] != 0 )
	    {
             fprintf(outfile,"%s%cN ",tlocstr,ctype);
	     fprintf(outfile,"%s = %s/%s . \n",   // N = M/L
	      pins[16],pins[15],pins[14] );
            }
          else
	    {
             fprintf(outfile,"%s%cN ",tlocstr,ctype);
	     fprintf(outfile,"%s = %s/%s . \n",   // N = M/L
	      pins[16],pins[15],"---" );
            }

	}

      if (pins[1][0] != 0 )              
	{
          if (pins[2][0] != 0 )
	    {
             fprintf(outfile,"%s%cA ",tlocstr,ctype);
	     fprintf(outfile,"%s = %s/%s . \n", // A = C/B
	        pins[1],pins[3],pins[2] );
            }
          else
            {
             fprintf(outfile,"%s%cA ",tlocstr,ctype);
	     fprintf(outfile,"%s = %s/%s . \n", // A = C/B
	        pins[1],pins[3],"---" );
            }
          
	}

    }

} // end put_outb


//  Put out a netlist for each board
//
             
void pkgs_out_kicad( )
{
  int i;
  int pkgind;
  int brd;

  char locstr[20];

  for (i =0; i < MAXPKGS; i += 1)
    {
      if (pkgarray[i].chiptype != '-' )
	{
          //printf("Chiptype = %c \n",pkgarray[i].chiptype);

          //printf("Loc = %s \n",pkgarray[i].locstr);

          strncpy(locstr,pkgarray[i].locstr,10);

          pkgind = handleloc(locstr);
          brd = (pkgind / 96);

          if (pkgarray[i].chiptype != 'S')
	    {
	      pkg_outk(pkgind, brd, locstr, pkgarray[i].chiptype);
            }
          else
	    {
	      pkg_outk_schip(pkgind, brd, locstr);  // schip has 2 extra pins
            }

        }
    }
}

//
//  Put out a netlist for each board
//
             
void pkgs_out_verilog( )
{
  int i;
  int pkgind;
  int brd;
  char locstr[20];

  for (i =0; i < MAXPKGS; i += 1)
    {
      if (pkgarray[i].chiptype != '-' )
	{
	  // printf("chiptype = %c \n", pkgarray[i].chiptype);

          strncpy(locstr,pkgarray[i].locstr,10);
          pkgind = handleloc(locstr);
          brd = (pkgind / 96);
          pkg_outv(pkgind, brd, locstr);
        }
    }
}

//
//   Keep track of jumper number and corresponding boolean for 
//                    netlist
void add_bool_jumptable(char *boolterm, char *locstr, int brdnum, int jmpnum)
{

  if (brdnum == 1)
    {
      if ( brd1jmpcnt < MAXBRDJMPS)
	{
         strncpy(board1_jmps[brd1jmpcnt].bool, boolterm, 10);
         board1_jmps[brd1jmpcnt].jmpnum = jmpnum;
         brd1jmpcnt +=1;
        }
    }
  if (brdnum == 2)
    {
      if ( brd2jmpcnt < MAXBRDJMPS)
	{
         strncpy(board2_jmps[brd2jmpcnt].bool, boolterm, 10);
         board2_jmps[brd2jmpcnt].jmpnum = jmpnum;
         brd2jmpcnt +=1;
        }
    }
  if (brdnum == 3)
    {
      if ( brd3jmpcnt < MAXBRDJMPS)
	{
        strncpy(board3_jmps[brd3jmpcnt].bool, boolterm, 10);
        board3_jmps[brd1jmpcnt].jmpnum = jmpnum;
        brd3jmpcnt +=1;
	}
    }
  if (brdnum == 4)
    {
       if ( brd4jmpcnt < MAXBRDJMPS)
	{
         strncpy(board4_jmps[brd4jmpcnt].bool, boolterm, 10);
         board4_jmps[brd4jmpcnt].jmpnum = jmpnum;
         brd4jmpcnt +=1;
        }
    }

  if (brdnum == 5)
    {
       if ( brd5jmpcnt < MAXBRDJMPS)
	{
        strncpy(board5_jmps[brd5jmpcnt].bool, boolterm, 10);
        board5_jmps[brd5jmpcnt].jmpnum = jmpnum;
        brd5jmpcnt +=1;
	}
    }
  if (brdnum == 6)
    {
      if ( brd6jmpcnt < MAXBRDJMPS)
	{
        strncpy(board6_jmps[brd6jmpcnt].bool, boolterm, 10);
        board6_jmps[brd6jmpcnt].jmpnum = jmpnum;
        brd6jmpcnt +=1;
        }
    }
  if (brdnum == 7)
    {
      if ( brd7jmpcnt < MAXBRDJMPS)
	{
        strncpy(board7_jmps[brd7jmpcnt].bool, boolterm, 10);
        board7_jmps[brd7jmpcnt].jmpnum = jmpnum;
        brd7jmpcnt +=1;
       }
    }
  if (brdnum == 8)
    {
      if ( brd8jmpcnt < MAXBRDJMPS)
	{
        strncpy(board8_jmps[brd8jmpcnt].bool, boolterm, 10);
        board8_jmps[brd8jmpcnt].jmpnum = jmpnum;
        brd8jmpcnt +=1;
       }
    }

}

//
//
//
void update_jump_needed()
{
  int i;
  int j;
  int upmask;
  int downmask;
  int bitmask;

  for(i=0; i < outputcnt; i += 1)
    {
      if (output_array[i].bool[0] != 0 )
	{
          if ( output_array[i].maxbrd != output_array[i].minbrd)
           {
            bitmask = 1;
            upmask = 0;
         
            for (k=0; k < output_array[i].maxbrd+1; k+=1 ) // count up from bot
	    {
              upmask = bitmask | upmask;
              bitmask =  bitmask << 1;
            }
            bitmask = 256;
            downmask = 0;
         
           for (k=8; k > output_array[i].minbrd-1; k-=1 ) // count down from top
	    {
              downmask = bitmask | downmask;
              bitmask =  bitmask >> 1;
            }
           output_array[i].jmpneeded = downmask & upmask;
	   // if (strncmp(output_array[i].bool,"DAW",10)==0)
	   // {
	   //  printf("DAW needed = %d \n",output_array[i].jmpneeded);
	   // }
	  }
	}
         
    }

}  //

//
//  Put out a netlist for each board
//
             
void pkgs_out_boolean( )
{
  int i;
  int pkgind;
  int brd;
  char *thisloc;

  for (i =0; i < MAXPKGS; i += 1)
    {
      if (pkgarray[i].chiptype != '-' )  // chip is defined
	{
	  //  printf("Boolean output = %s \n",output_array[i].bool);
	  // thisloc = array[i].locstr;
          //pkgind = handleloc(thisloc);
	  // printf("Pkgind = %d \n",output_array[i].pkgindex);
	  // pkgind = output_array[i].pkgindex;

          brd = (i / 96);
          pkg_outb(i, brd);
        }
    }

  fclose(outfile);
}
//
//  Once a good jumper is found an output, 
//
void add_jmp_loads(int boolindex, char *jmplocstr)
{
  int bitmask;
  int m;
  int loadcnt;
  char nextjmp[10];
  char jmplstr[5];

  if (boolindex < MAXBOOL )
    {
      loadcnt = output_array[boolindex].loadcnt;
     bitmask = 128;

     for (m=0; m < 8; m+=1)      // for each possible board
      {
	if ((output_array[boolindex].jmpneeded & bitmask ) > 0 )
	  {
            nextjmp[0] = jmplocstr[0] + ((7-m)*3);   // get correct board field
            nextjmp[1] = jmplocstr[1];
            nextjmp[2] = jmplocstr[2];
            nextjmp[3] = 0;
            jmplstr[0] = nextjmp[0];
            jmplstr[1] = nextjmp[1];
            jmplstr[2] = 0;

	    if (loadcnt < MAXLOADS)
	      {
		//printf("Adding jumper - %s to net %s needed = %d pin = %c\n", jmplocstr,
		//     output_array[boolindex].bool,
		//  output_array[boolindex].jmpneeded,jmplocstr[2]);

		strncpy( output_array[boolindex].loadlist[loadcnt].locstr,jmplstr,5);
             
            
                output_array[boolindex].loadlist[loadcnt].pinnum = 
                                           jmplocstr[2] - 'A'+1;
           
                cpin_to_xy(jmplocstr);
            
	    // printf("Setting pinx = %d piny = %d jmplocstr = %s\n",pinx,piny,jmplocstr);

               output_array[i].loadlist[loadcnt].pinx = pinx;
               output_array[i].loadlist[loadcnt].piny = piny;
               output_array[i].loadlist[loadcnt].brdnum = (nextjmp[0]-'A')/3;
	      }
	    else
	      {
		printf("Max loads exceeded by adding jumps for term = %s  \n",
                         output_array[i].bool);
                printf("Load count = %d \n",loadcnt);
              }
            loadcnt += 1;
          }
        bitmask = bitmask >> 1;
      }
     output_array[boolindex].loadcnt=loadcnt;
    }
  else
    {
      printf("Error: Index to output_array out of range in add_jmp_loads\n");
    }
}

//
//  main program
//

int main (int argc,char *argv[])

 {


  //printf("Init jumpers\n");

   rchipcnt = 0;
   schipcnt = 0;

  init_jmplocs();

  init_conlocs();


  file1 = fopen(argv[1],"r");
  if (file1 == NULL)
   {
    printf("Can't open the input board package file %s\n",argv[1]);
    exitnow(1);
   }

  for(i=0;i < MAXPKGS; i += 1)
    {
      pkgarray[i].locstr[0] = 0;
      pkgarray[i].chiptype = '-';
    }


  for(i=0;i < MAXBOOL; i += 1)
    {
      output_array[i].bool[0] = 0;
      output_array[i].maxbrd = 0;
      output_array[i].minbrd = 9;
    }

  read_all_pkgs(file1);    // read all the pkg info


  printf("Packages read \n");

   file5 = fopen(argv[3],"r");
 
  if (file5 == NULL)
   {
    printf("Can't open the input connector file %s\n",argv[3]);
    exitnow(1);
   }

  // printf("About to read cons file \n");

  read_all_cons(file5);    // read all the connector file

  printf("Connectors read \n");

 
  file2 = fopen(argv[2],"r");

  if (file2 == NULL)
   {
    printf("Can't open the input boolean file %s \n",argv[2]);
    exitnow(1);
   }


  for (i =0; i < 2000; i+= 1)
    {
      locusedarray[i]=0;         // init locused array
    }


  endoffile = FALSE;
  linenum = 1;
  linein[0] = 0;
  while(linein[0] == '*')  /* skip header */
   {
    getaline(linein,file2);
    // printf("Linein = %s \n",linein);
   }

  alltermcnt=0;

  while ((endoffile==FALSE) &&(linein[0] != 26))
  {
     getaline(linein,file2);

     //printf("Got a line = %s \n",linein);

    if (linein[0] == '-')
     {

       //printf("Line in = %s \n",linein);

      if (endoffile == FALSE)
       {
        
        getfirstbool();   // get the first boolean and look it up
                        	// in packages info


        output_index = find_in_outputs(firstbool);  // get pkgindex

        srcpin = output_array[output_index].srcpin;
        pinnum = pinxlate(srcpin);

     
     // printf("In main - sourcepin = %c \n", output_array[output_index].srcpin);

        pkgindex = output_array[output_index].pkgindex;

        chip_type = output_array[output_index].chip_type;

	strncpy(pkgarray[pkgindex].pinterms[pinnum],firstbool,10);

         if (chip_type == 'A')
          {
            parse_achip_rhs(srcpin, pkgindex);
          }

         if (chip_type == 'B')
          {
            parse_bchip_rhs(srcpin, pkgindex);
          }

         if (chip_type == 'C')
          {
            parse_cchip_rhs(srcpin, pkgindex);
          }
         if (chip_type == 'D')
          {
            parse_dchip_rhs(srcpin, pkgindex);
          }

         if (chip_type == 'E')
          {
            parse_echip_rhs(srcpin, pkgindex);
          }

          if (chip_type == 'F')
           {
            parse_fchip_rhs(srcpin, pkgindex);
           }
          if (chip_type == 'G')
           {
	     parse_gchip_rhs(srcpin, pkgindex);
           }

        if (chip_type == 'H')
          {
            parse_hchip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'I')
          {
            parse_ichip_rhs(srcpin, pkgindex);
          }
        if (chip_type == 'J')
          {
            parse_jchip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'K')
          {
            parse_kchip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'L')
          {
            parse_lchip_rhs(srcpin, pkgindex);
          }
        if (chip_type == 'M')
          {
            parse_mchip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'N')
          {
            parse_nchip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'O')
          {
            parse_ochip_rhs(srcpin, pkgindex);

          }


        if (chip_type == 'P')
          {
            parse_pchip_rhs(srcpin, pkgindex);
          }


        if (chip_type == 'Q')
          {
            parse_qchip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'R')
          {
            parse_rchip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'S')
          {
            parse_schip_rhs(srcpin, pkgindex);
          }


        if (chip_type == 'T')
          {
            parse_tchip_rhs(srcpin, pkgindex);
          }


        if (chip_type == 'U')
          {
            parse_uchip_rhs(srcpin, pkgindex);
          }


        if (chip_type == 'V')
          {
            parse_vchip_rhs(srcpin, pkgindex);
          }


        if (chip_type == 'W')  // same as echip
          {
            parse_echip_rhs(srcpin, pkgindex);
          }


        if (chip_type == 'X')
          {
            parse_xchip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'Y')
          {
	    parse_ychip_rhs(srcpin, pkgindex);
          }

        if (chip_type == 'Z')
          {
            parse_zchip_rhs(srcpin, pkgindex);
          }

       }
      }
     }    /* while not end of file */


   fclose(file2);


  outfile = fopen(argv[4],"w");

  if (outfile == NULL)
   {
    printf("Can't open the output boolean file %s \n",argv[4]);
    exitnow(1);
   }
   
  pkgs_out_boolean();


  // fclose(outfile);

  outfile = fopen("netfile","w");

  if (outfile == NULL)
   {
    printf("Can't open the output kicad net file %s \n","netfile");
    exitnow(1);
   }

  
  pkgs_out_kicad();

  //fclose(outfile);

  outfilea = fopen("verifile_nl","w");

  if (outfilea == NULL)
   {
    printf("Can't open the output verilog file %s \n","verifile_nl");
    exitnow(1);
   }

  fprintf(outfilea,"assign ZZI = 1'b1;\n");
  fprintf(outfilea,"assign ZZO = 1'b0;\n");

  outfileb = fopen("verifile_lat","w");

  if (outfileb == NULL)
   {
    printf("Can't open the output verilog file %s \n","verifile_lat");
    exitnow(1);
   }

  fprintf(outfileb,"always@(posedge IZZ )\n");
  fprintf(outfileb,"   begin \n");

  outfilec = fopen("verifile_ram","w");


  if (outfilec == NULL)
   {
    printf("Can't open the output verilog file %s \n","verifile_ram");
    exitnow(1);
   }

  pkgs_out_verilog();

  fprintf(outfileb," end \n");

  fclose(outfileb);
  
  // jumper counts on each board

   brd1jmpcnt =0;
   brd2jmpcnt =0;
   brd3jmpcnt =0;
   brd4jmpcnt =0;
   brd5jmpcnt =0;
   brd6jmpcnt =0;
   brd7jmpcnt =0;
   brd8jmpcnt =0;


  // printf("Print needed jumpers \n");

  update_jump_needed();  // jumpers needed for each output

   
  // calculate optimal jumper placement for each load

  for(i=0; i < outputcnt; i += 1)
    {
      if ((output_array[i].bool[0] != 0 ) && (output_array[i].loadcnt > 0 ))
	{
	  if (output_array[i].minbrd != output_array[i].maxbrd)
	    {
              
            xtotal = 0;
            ytotal = 0;

	    for(j=0; j < output_array[i].loadcnt; j += 1)
	     {

	       //  printf("For pin = %d \n",j);

              strncpy(plocstr,output_array[i].loadlist[j].locstr,10);
             
              tstr[0]= output_array[i].loadlist[j].srcpin;
              tstr[1]= 0;

	      strncat(plocstr,tstr,10);
              cpin_to_xy( plocstr);

	      //printf("For bool = %s pinx,piny = %d %d locstr = %s \n",
	      //  output_array[i].bool, pinx, piny,plocstr);

              xtotal = pinx + xtotal;
              ytotal = piny + ytotal;

             }
       
	   output_array[i].optx = xtotal / j;  // average x,y location
           output_array[i].opty = ytotal / j;

	   //printf("For bool = %s optx,opty = %d %d \n",
	   //	  output_array[i].bool, output_array[i].optx,
	   //	  output_array[i].opty);

          getgoodjmps( output_array[i].jmpneeded);

           bestdist = 10000000;

        
           bestjmp = -1;

           for (k=0; k < 1200; k+=1)
            {
	      if (jmpgood[k] )
               {
                 dist = distance_m( jmplocs[k].xloc, jmplocs[k].yloc, 
			     output_array[i].optx, output_array[i].opty);
                  
                 if (dist < bestdist)
                   {

                     bestjmp = k;
                     bestdist = dist;
                   }
               }
             }

	   if (bestjmp != -1)
	     {
	     strncpy(bjmplocstr, jmplocs[bestjmp].locstr,10);

             add_jmp_loads(i,bjmplocstr); // update loads for boolean

             jmplocs[bestjmp].availmask = jmplocs[bestjmp].availmask &
	       (output_array[i].jmpneeded ^ 0xFF );   // what jumper 
               }
	   
	  }
      }
    }

 
  outfile = fopen("craynetfile","w");

  if (outfile == NULL)
   {
    printf("Can't open the output netlist file %s \n","craynetfile");
    exitnow(1);
   }


  for(i=0; i < outputcnt; i += 1)
    {
      if(output_array[i].bool[0] != 0 )
       {
	 fprintf(outfile,"<<< %s - %d (loc= %s,%c)\n",
		 output_array[i].bool,output_array[i].loadcnt,
                  output_array[i].loc,
                 output_array[i].srcpin);
        for(j=0; j < output_array[i].loadcnt; j += 1)
   	 {
	   fprintf(outfile,"%s,%d\n",output_array[i].loadlist[j].locstr,
		   output_array[i].loadlist[j].pinnum);

         }
       }
    }

  fclose(outfile);

 
  outfile1 = fopen("kicadjmpabc","w");

  if (outfile1 == NULL)
   {
    printf("Can't open the output kicad file %s \n","kicadjmpabc");
    exitnow(1);
   }

  outfile2 = fopen("kicadjmpdef","w");

  if (outfile2 == NULL)
   {
    printf("Can't open the output kicad file %s \n","kicadjmpdef");
    exitnow(1);
   }

  outfile3 = fopen("kicadjmpghi","w");

  if (outfile3 == NULL)
   {
    printf("Can't open the output kicad file %s \n","kicadjmpghi");
    exitnow(1);
   }

  outfile4 = fopen("kicadjmpjkl","w");

  if (outfile4 == NULL)
   {
    printf("Can't open the output kicad file %s \n","kicadjmpjkl");
    exitnow(1);
   }

  outfile5 = fopen("kicadjmpmno","w");

  if (outfile5 == NULL)
   {
    printf("Can't open the output kicad file %s \n","kicadjmpmno");
    exitnow(1);
   }

  outfile6 = fopen("kicadjmppqr","w");

  if (outfile6 == NULL)
   {
    printf("Can't open the output kicad file %s \n","kicadjmppqr");
    exitnow(1);
   }

  outfile7 = fopen("kicadjmpstu","w");

  if (outfile7 == NULL)
   {
    printf("Can't open the output kicad file %s \n","kicadjmpstu");
    exitnow(1);
   }

  outfile8 = fopen("kicadjmpvwx","w");

  if (outfile8 == NULL)
   {
    printf("Can't open the output kicad file %s \n","kicadjmpvwx");
    exitnow(1);
   }


  jumpernum = 0;

  // 
  //  Find all the jumpers
  //            
  //
  for(i=0; i < outputcnt; i += 1)
    {
      if(output_array[i].bool[0] != 0 )
       {
         for(j=0; j < output_array[i].loadcnt; j += 1)
	   {
	    
	       if (output_array[i].loadlist[j].pinnum > 16) // a jumper
	        {
		  if (output_array[i].loadlist[j].locstr[0] != 'Z') 
                                                      // not connector
                    {                
		      // printf("Putting out jumper for %s , x,y = %d %d \n", 
		      //     output_array[i].bool,
		      //     output_array[i].loadlist[j].pinx,
		      //     output_array[i].loadlist[j].piny );
		      add_bool_jumptable(output_array[i].bool, 
			output_array[i].loadlist[j].locstr,
			output_array[i].loadlist[j].brdnum,
                        jumpernum);

		      out_jumper_kicad(output_array[i].loadlist[j].brdnum,
                           output_array[i].loadlist[j].pinx,
			  output_array[i].loadlist[j].piny,jumpernum);
                     jumpernum += 1;
                    }
	       }
	     }
         }
    }
  
  printf("Jumper counts on brd 1 = %d \n",brd1jmpcnt);
  printf("Jumper counts on brd 2 = %d \n",brd2jmpcnt);
  printf("Jumper counts on brd 3 = %d \n",brd3jmpcnt);
  printf("Jumper counts on brd 4 = %d \n",brd4jmpcnt);
  printf("Jumper counts on brd 5 = %d \n",brd5jmpcnt);
  printf("Jumper counts on brd 6 = %d \n",brd6jmpcnt);
  printf("Jumper counts on brd 7 = %d \n",brd7jmpcnt);
  printf("Jumper counts on brd 8 = %d \n",brd8jmpcnt);

  fclose(outfile1);
  fclose(outfile2);
  fclose(outfile3);
  fclose(outfile4);
  fclose(outfile5);
  fclose(outfile6);
  fclose(outfile7);
  fclose(outfile8);

  // printf("About to call out_all_kicadjmps \n");

  out_all_kicadjmps();   // make the 8 netlist files for jumpers
 
  outfile = fopen("connetfile","w");

  if (outfile == NULL)
   {
    printf("Can't open the output kicad connector netfile %s \n","connetfile");
    exitnow(1);
   }

  out_cons_kicad(0,outfile);

  out_cons_kicad(1,outfile);

  out_cons_kicad(2,outfile);

  out_cons_kicad(3,outfile);

  out_cons_kicad(4,outfile);

  out_cons_kicad(5,outfile);

  out_cons_kicad(6,outfile);

  out_cons_kicad(7,outfile);

  fclose(outfile);


  outfile = fopen("veriheader","w");

  if (outfile == NULL)
   {
    printf("Can't open the output verilog header file %s \n","veriheader");
    exitnow(1);
   }

  fprintf(outfile,"module %c%c( ",argv[2][0],argv[2][1]);  


  sort_io(outfile);
  
  //
  // generate negated terms for input terms 
  //

  for(k=0;k < vincnt; k += 1)
    {
      fprintf(outfilea,"assign %s = ~%s; //complement \n",flipcase(interms_srt[k]),
                     interms_srt[k] );
    }

  fclose(outfile);
  fclose(outfilea);
  fclose(outfileb);

 }  // end












