/***   CSIM - Convert a Cray-2 boolean file to C ***/

#include "stdio.h"
#include "ctype.h"

#define MAXC    200             /* Maximum characters in mask */
#define MAXI    200             /* Maximum characters in input line */
#define MAXO   1000             /* Maximum characters in output line */
#define LF     '\012'           /* Line Feed */
#define FF     '\014'           /* Form Feed */
#define NULLCHAR '\000'
#define leftbracket 93
#define rightbracket 91
char vartype[26][26][26];       /* External storage */
char line[MAXI];
char ftn[MAXO];
char token[4];
char mask[MAXC];
char temp[10];
int  varindex[26][26][26];
int  c , cc;
int  n;
int  x , y , z;
int  tokentype;
int  r_add = 1;
int  latch_flag = 1;

void cvtfor();                       /* Function declaration */
void cvtfor_f();
void cvtfor_qt();
void cvtfor_r();
void cvtfor_u();
void gettoken();
void chktoken1();
void chktoken2();
void chktoken3();
void cvttoken();
void invtoken();
void err();

main(argc, argv)
int argc;
char *argv[];

{

  char nextline[MAXI];
  char chiptype;
  char old_module[10];
  char new_module[10];

  FILE *infile , *outfile1 , *outfile2 , *fopen();

  int  eq_sign , period;
  int  tot_latch = 0;
  int  tot_gate = 0;
  int  tot_r = 0;
  int  tot_s = 0;

/****************************************/
/*      Check command line              */
/****************************************/

  if (argc != 8){
    puts("Usage: CSIM p1 p2 p3 p4 p5 p6 p7");
    puts(" p1 = boolean file ");
    puts(" p2 = wire tab file");
    puts(" p3 = output file  ");
    puts(" p4 = module name  ");
    puts(" p5 = input name   ");
    puts(" p6 = compiler     ");
    puts(" p7 = on/off check ");
    exit(1);
  }

/****************************************/
/*******        Part 1            *******/
/****************************************/
/*  - Remove comments                   */
/*  - Find equal sign and period        */
/*  - Combine multiple lines            */
/*  - Remove Form Feeds                 */
/*  - Remove Z-- and z-- terms          */
/*  - Determine variable type           */
/*  - Assign an index to each term      */
/****************************************/

  for ( z = 0; z != 26; z++ ){                  /* Pre-set arrays */
    for ( y = 0; y != 26; y++ ){
      for ( x = 0; x != 26; x++ ){
        vartype[z][y][x] = ' ';
        if ( z == 8 ) vartype[z][y][x] = 'I';
        varindex[z][y][x] = 0;
      }
    }
  }

  z = 8;                                        /* Input terms */
  c = 1;
  for ( y = 0; y != 26; y++ ){
    for ( x = 0; x != 26; x++ ){
      varindex[z][y][x] = c;
      c++;
    }
  }

  infile  = fopen(argv[1] , "r");               /* Check if file exists */
  if (infile == NULL){
    printf("I can't open %s\n", argv[1]);
    exit(1);
  }

  outfile1 = fopen("latch" , "w");              /* Open output files */
  outfile2 = fopen("gate"  , "w");

start:

  while ( ( fgets(line , MAXI , infile ) ) != NULL ){

    if ( line[0] == ' ' ) continue;             /* Comment line */

    if ( line[0] == FF ) {                      /* Remove Form Feed */
      for ( c = 0; line[c] != NULLCHAR; c++ ){
        line[c] = line[c + 1];
      }
    }

    eq_sign = 0;
    period  = 0;
    for ( c = 0; line[c] != NULLCHAR; c++ ){
      if ( line[c] == '=' ){                    /* Find first equal sign */
        if ( eq_sign == 0 ){
          eq_sign = c;
        }
      }
      if ( line[c] == '.' ){                    /* Find first period */
        if ( period == 0 ){
          period = c;
        }
      }
    }

    if ( eq_sign == 0 ) continue;               /* Ignore line */

    for ( c = 0; c != eq_sign; c++ ){           /* Remove Z-- and z-- terms */
      if ( line[c] == ' ' ){
        if ( line[c+1] == 'Z' ) goto start;
        if ( line[c+1] == 'z' ) goto start;
      }
    }

    if ( period  == 0 ){;                       /* Multiple line */

      line[ strlen(line) - 1 ] = ' ';           /* Replace LF with space */

      fgets( nextline , MAXI , infile );        /* Read next line */
      strcat( line , nextline );

      for ( c = eq_sign; line[c] != NULLCHAR; c++ ){
        if ( line[c] == ' ' ){
          while ( line[c+1] == ' ' ){
            for ( cc = c; line[cc] != NULLCHAR; cc++ ){
              line[cc] = line[cc+1];
            }
          }
        }
        if ( line[c] == '.' ){
          if ( period == 0 ){
            period  = c;
          }
        }
      }
    }

    line[ period + 1 ] = LF;                    /* Remove comments */
    line[ period + 2 ] = NULLCHAR;

    chiptype = '?';                             /* Check chip type */

    if ( line[2] == 'L' ) chiptype = 'L';       /* Latches */
    if ( line[2] == 'M' ) chiptype = 'L';
    if ( line[2] == 'N' ) chiptype = 'L';
    if ( line[2] == 'O' ) chiptype = 'L';
    if ( line[2] == 'P' ) chiptype = 'L';
    if ( line[2] == 'T' ) chiptype = 'L';
    if ( line[2] == 'X' ) chiptype = 'L';

    if ( line[2] == 'A' ) chiptype = 'G';       /* Gates */
    if ( line[2] == 'B' ) chiptype = 'G';
    if ( line[2] == 'C' ) chiptype = 'G';
    if ( line[2] == 'D' ) chiptype = 'G';
    if ( line[2] == 'E' ) chiptype = 'G';
    if ( line[2] == 'F' ) chiptype = 'G';
    if ( line[2] == 'H' ) chiptype = 'G';
    if ( line[2] == 'I' ) chiptype = 'G';
    if ( line[2] == 'J' ) chiptype = 'G';
    if ( line[2] == 'K' ) chiptype = 'G';
    if ( line[2] == 'Q' ) chiptype = 'G';
    if ( line[2] == 'R' ) chiptype = 'G';
    if ( line[2] == 'S' ) chiptype = 'G';
    if ( line[2] == 'U' ) chiptype = 'G';
    if ( line[2] == 'W' ) continue;
    if ( line[2] == '+' ) chiptype = 'G';

    if ( chiptype == '?' ){
      puts("Illegal chip type...");
      printf("%s" , line);
      exit(1);
    }

    if ( chiptype == 'L' ) fputs(line , outfile1);
    if ( chiptype == 'G' ) fputs(line , outfile2);

    for ( c = 0; c != eq_sign; c++ ){           /* Assign index */
      if ( line[c] == ' ' ){
        if ( isalpha( line[c+1] ) ){

          z = toupper( line[c+1] ) - 65;
          y = toupper( line[c+2] ) - 65;
          x = toupper( line[c+3] ) - 65;

          if ( varindex[z][y][x] == 0 ){

            if ( chiptype == 'L' ) {              /* Latches */
              vartype[z][y][x] = 'L';
              tot_latch++;
              varindex[z][y][x] = tot_latch;
            }

            if ( chiptype == 'G' ) {              /* Gates */
              vartype[z][y][x] = 'G';
              tot_gate++;
              varindex[z][y][x] = tot_gate;
            }
          }
        }
      }
    }

    if ( line[2] == 'R' ) tot_r++;              /* Type R */
    if ( line[2] == 'S' ) tot_s++;              /* Type S */

  }

  fclose(infile);
  fclose(outfile1);
  fclose(outfile2);

  if ( strcmp ( argv[3] , "none" ) == 0 ) exit(0);

/****************************************/
/*******        Part 2            *******/
/****************************************/
/*  - Read "LATCH"                      */
/*  - Convert all latches to FORTRAN    */
/*  - Write to output file              */
/****************************************/

  outfile1 = fopen(argv[3] , "w");              /* Open output file */

  if ( tot_latch != 0 ){                        /* Any latches ? */

    infile = fopen("latch" , "r");              /* Open input file */

    fprintf(outfile1 , "%s\n"     , "//********************************"     );
    fprintf(outfile1 , "%s%s%s\n" , "//    " , argv[4] , " LATCHES"        );
    fprintf(outfile1 , "%s\n"     , "//******************************"     );
    fprintf(outfile1 , "%s%s%s" , "  void " , argv[4] , "L( "    );
      fprintf(outfile1 , "%s%d%s"   , "int L[" , tot_latch , "],"           );
    fprintf(outfile1 , "%s%d%s"   , "int NL[" , tot_latch , "],"            );
    fprintf(outfile1 , "%s%d%s" , "int G[" , tot_gate  , "],"               );
    fprintf(outfile1 , "%s"     , "int I[625]");
    fprintf(outfile1 ,  "%s\n"     , "   )" );  
    fprintf(outfile1 ,  "%s\n"     , "{" );  
    fprintf(outfile1 ,  "%s \n"    , "double ET,ST,TOTL; ");
    if ( argv[7][0] == 'Y' ){
     fprintf(outfile1 , "%s\n"     , "      int NN,IILZ,TOTON;"         );
    }
    else{
     fprintf(outfile1 , "%s\n"     , "      int NN;"                    );
    }
    
    if ( argv[7][0] == 'Y' ){
     fprintf(outfile1 , "%s\n"     , ",OI[625]"                            );
    }
    else{
     fprintf(outfile1 , "\n" );
    }

    if ( argv[7][0] == 'Y' ){
      fprintf(outfile1 , "%s%s"     , "      COMMON/" , argv[4]            );
      fprintf(outfile1 , "%s\n"     , "X/ON,TOTON"                         );
      fprintf(outfile1 , "%s\n"     , "     IF(ON) { "                  );
      fprintf(outfile1 , "%s\n"     , "       TOTON = TOTON+1"             );
      fprintf(outfile1 , "%s\n"     , "     ELSE"                         );
      fprintf(outfile1 , "%s\n"     , "      for (NN =1;NN<626;NN+=1)"       );
      fprintf(outfile1 , "%s\n"     , "       { "       );
      fprintf(outfile1 , "%s\n"     , "       OI[NN]=OI[NN] ^ I[NN];"       );
      fprintf(outfile1 , "%s\n"     , "       NN = IILZ[625][OI(1),1];"      );
      fprintf(outfile1 , "%s\n"     , "       IF(625 .GT. NN) ON = .t."    );
      fprintf(outfile1 , "%s\n"     , "       } "       );
      fprintf(outfile1 , "%s\n"     , "      ENDIF"                        );
      fprintf(outfile1 , "%s\n"     , "      for ( NN=1; NN < 626; NN ++ 1)" );
      fprintf(outfile1 , "%s\n"     , "       { "       );
      fprintf(outfile1 , "%s\n"     , "        OI[NN]=I[NN];"         );
      fprintf(outfile1 , "%s\n"     , "       } "       );
      fprintf(outfile1 , "%s\n"     , "      IF(ON) { "                  );
    }

    fprintf(outfile1 , "%s\n"     , "           SECOND(ST);"                );

    while ( ( fgets(line , MAXI , infile ) ) != NULL ){
      genmask();
      if ( line[2] == 'L' ) cvtfor();
      if ( line[2] == 'M' ) cvtfor();
      if ( line[2] == 'N' ) cvtfor();
      if ( line[2] == 'O' ) cvtfor();
      if ( line[2] == 'P' ) cvtfor();
      if ( line[2] == 'T' ) cvtfor_qt();
      if ( line[2] == 'X' ) cvtfor();
      fprintf(outfile1 , "%s;\n" , ftn);
    }

    fprintf(outfile1 , "%s\n"     , "           SECOND(ET);"                );
    fprintf(outfile1 , "%s\n"     , "           TOTL=TOTL+(ET-ST);"              );

    latch_flag = 0;

    if ( argv[7][0] == 'Y' ){
      fprintf(outfile1 , "%s\n"     , "      ON = .f."                       );
      // fprintf(outfile1 , "%s%d%s\n"   , "  for(NN=1; NN< ",tot_latch,"; NN+=1)" );
      // fprintf(outfile1 ,  %s\n"     , " {    ");
      fprintf(outfile1 , "%s\n"     , "       L[NN]=L[NN] ^ NL[NN];"  );
      fprintf(outfile1 , "%s%d"     , "      NN = IILZ[" , tot_latch         );
      fprintf(outfile1 , "%s\n"     , ",L(1),1]"                             );
      fprintf(outfile1 , "%s%d"     , "      IF(" , tot_latch                );
      fprintf(outfile1 , "%s\n"     , " >  NN) ON = .t."                   );
    }

    fprintf(outfile1 , "%s"       , "      for ( NN = 1 ; NN < "    );
    fprintf(outfile1 , "%d%s\n"     , tot_latch  ,"; NN+=1) {"                   );
    fprintf(outfile1 , "%s\n"     , "      L[NN]=NL[NN];  }"          );

    if ( argv[7][0] == 'Y' ){
     fprintf(outfile1 , "%s\n"     , "      ENDIF");
    }
    fprintf(outfile1 , "%s\n"     , "      }"   );

    fclose(infile);

  }

/****************************************/
/*******        Part 3            *******/
/****************************************/
/*  - Read "GATE"                       */
/*  - Convert all gates to FORTRAN      */
/*  - Write to output file              */
/****************************************/

  if ( tot_gate != 0 ){                         /* Any gates ? */

    infile = fopen("gate" , "r");               /* Open input file */

    fprintf(outfile1 , "%s\n"     , "//********************************"     );
    fprintf(outfile1 , "%s%s%s\n" , "//*     " , argv[4] , " GATES"          );
    fprintf(outfile1 , "%s\n"     , "//********************************"     );
    fprintf(outfile1 , "%s%s%s" , "  void " , argv[4] , "G( "    );
  
    fprintf(outfile1 , "%s%d%s"   , " int  L[" , tot_latch , "],"       );
    fprintf(outfile1 , "%s%d%s"   , " int NL[" , tot_latch , "],"       );
    fprintf(outfile1 , "%s%d%s" , " int G[" , tot_gate  , "],"         );
    fprintf(outfile1 , "%s" , " int I[625]" );
    fprintf(outfile1 , "%s\n" , " )");
    fprintf(outfile1 , "  { \n");  
    fprintf(outfile1 , "%s\n"     , "     int NN;"                     );
    fprintf(outfile1 , "%s\n"     , "     double ST,ET,TOTL,TOTG,TOTW,TOTM;" );
  
  
    if ( argv[7][0] == 'Y' ){
      fprintf(outfile1 , "%s%s%s\n" , "      COMMON/" , argv[4] , "X/ON"   );
      fprintf(outfile1 , "%s\n"     , "      IF(ON) THEN"                  );
    }

    fprintf(outfile1 , "%s\n"     , "        SECOND(ST);"                );

    while ( ( fgets(line , MAXI , infile ) ) != NULL ){
      genmask();
      if ( line[2] == 'A' ) cvtfor();
      if ( line[2] == 'B' ) cvtfor();
      if ( line[2] == 'C' ) cvtfor();
      if ( line[2] == 'D' ) cvtfor();
      if ( line[2] == 'E' ) cvtfor();
      if ( line[2] == 'F' ) cvtfor_f();
      if ( line[2] == 'H' ) cvtfor();
      if ( line[2] == 'I' ) cvtfor();
      if ( line[2] == 'J' ) cvtfor();
      if ( line[2] == 'K' ) cvtfor();
      if ( line[2] == 'Q' ) cvtfor_qt();
      if ( line[2] == 'R' ) continue;
      if ( line[2] == 'S' ) continue;
      if ( line[2] == 'U' ) cvtfor_u();
      if ( line[2] == '+' ) cvtfor();
      fprintf(outfile1 , "%s;\n" , ftn);
    }

    fprintf(outfile1 , "%s\n"     , "      SECOND(ET);"                );
    fprintf(outfile1 , "%s\n"     , "      TOTG=TOTG+(ET-ST);"              );

    if ( argv[7][0] == 'Y' ){
     fprintf(outfile1 , "%s\n"     , "      ENDIF");
    }

    fprintf(outfile1 , "%s\n"     , "} ");

    fclose(infile);

  }

/****************************************/
/*******        Part 4            *******/
/****************************************/
/*  - Read "GATE"                       */
/*  - Convert type R or S to FORTRAN    */
/*  - Write to output file              */
/****************************************/

  if ( ( tot_r != 0 ) || ( tot_s != 0 ) ){      /* Any memory ? */

    infile = fopen("gate" , "r");               /* Open input file */

    fprintf(outfile1 , "%s\n"     , "********************************"     );
    fprintf(outfile1 , "%s%s%s\n" , "*     " , argv[4] , " MEMORY"         );
    fprintf(outfile1 , "%s\n"     , "********************************"     );
    fprintf(outfile1 , "%s%s%s\n" , "      SUBROUTINE " , argv[4] , "M"    );
    fprintf(outfile1 , "%s\n"     , "      IMPLICIT LOGICAL (A-Z)"         );
    fprintf(outfile1 , "%s\n"     , "      int NN,ADD;"                 );
    fprintf(outfile1 , "%s\n"     , "      REAL ST,ET,TOTL,TOTG,TOTW,TOTM" );
    fprintf(outfile1 , "%s%s"     , "      COMMON/" , argv[4]              );
    fprintf(outfile1 , "%s\n"     , "T/TOTL,TOTG,TOTW,TOTM"                );
    fprintf(outfile1 , "%s%s%s"   , "      COMMON/" , argv[4] , "/"        );
    fprintf(outfile1 , "%s%d%s"   , "L[" , tot_latch , "],"                );
    fprintf(outfile1 , "%s%d%s"   , "NL[" , tot_latch , "],"               );
    fprintf(outfile1 , "%s%d%s\n" , "G(" , tot_gate  , "]"                 );
    if ( argv[7][0] == 'Y' ){
      fprintf(outfile1 , "%s%s%s\n" , "      COMMON/" , argv[4] , "X/ON"   );
    }

    if ( tot_r != 0 ){
      fprintf(outfile1 , "%s"       , "      LOGICAL ");
      fprintf(outfile1 , "%s%d%s"   , "DATA0(" , tot_r * 16 , "),");
      fprintf(outfile1 , "%s%d%s"   , "DATA1(" , tot_r * 16 , "),");
      fprintf(outfile1 , "%s%d%s"   , "DATA2(" , tot_r * 16 , "),");
      fprintf(outfile1 , "%s%d%s\n" , "DATA3(" , tot_r * 16 , ")");
    }

    if ( tot_s != 0 ){
      fprintf(outfile1 , "%s"       , "      LOGICAL ");
      fprintf(outfile1 , "%s%d%s\n" , "DATA0(" , tot_s * 4096 , ")");
    }

    if ( argv[7][0] == 'Y' ){
      fprintf(outfile1 , "%s\n"     , "      IF(ON) { " );
    }

    fprintf(outfile1 , "%s\n"     , "           SECOND(ST)"                );

    while ( ( fgets(line , MAXI , infile ) ) != NULL ){
      if ( ( line[2] != 'R' ) && ( line[2] != 'S' ) ) continue;
      if ( line[2] == 'R' ) cvtfor_r();
      if ( line[2] == 'S' ) continue;
      fprintf(outfile1 , "%s\n" , ftn);
    }

    fprintf(outfile1 , "%s\n"     , "      SECOND(ET);"                );
    fprintf(outfile1 , "%s\n"     , "      TOTM=TOTM+(ET-ST);"              );

    if ( argv[7][0] == 'Y' ){
     fprintf(outfile1 , "%s\n"     , "      }");
    }

    fprintf(outfile1 , "%s\n"     , "      END");

    fclose(infile);

  }

/****************************************/
/*******        Part 5            *******/
/****************************************/
/*    Write display routine to the      */
/*     output file                      */
/****************************************/

  fprintf(outfile1 , "%s\n"     , "********************************");
  fprintf(outfile1 , "%s%s%s\n" , "*     " , argv[4] , " DISPLAY");
  fprintf(outfile1 , "%s\n"     , "********************************");
  fprintf(outfile1 , "%s%s%s\n" , "      SUBROUTINE " , argv[4] , "D");
  fprintf(outfile1 , "%s\n"     , "      IMPLICIT LOGICAL (A-Z)");
  fprintf(outfile1 , "%s\n"     , "      int NN;");
  fprintf(outfile1 , "%s%s%s"   , "      COMMON/" , argv[4] , "/");
  fprintf(outfile1 , "%s%d%s"   , "L[" , tot_latch , "],");
  fprintf(outfile1 , "%s%d%s"   , "NL[" , tot_latch , "],");
  fprintf(outfile1 , "%s%d%s\n" , "G[" , tot_gate  , "]");
  fprintf(outfile1 , "%s%s%s\n" , "      COMMON/" , argv[5] , "I/I(625)");
  fprintf(outfile1 , "%s"       , "      COMMON/NDIS/");
  fprintf(outfile1 , "%s\n"     , "DA(625),DB(625),DC(625),DD(625),DE(625),");
  fprintf(outfile1 , "%s"       , "     &            ");
  fprintf(outfile1 , "%s\n"     , "DF(625),DG(625),DH(625),DI(625),DJ(625),");
  fprintf(outfile1 , "%s"       , "     &            ");
  fprintf(outfile1 , "%s\n"     , "DK(625),DL(625),DM(625),DN(625),DO(625),");
  fprintf(outfile1 , "%s"       , "     &            ");
  fprintf(outfile1 , "%s\n"     , "DP(625),DQ(625),DR(625),DS(625),DT(625),");
  fprintf(outfile1 , "%s"       , "     &            ");
  fprintf(outfile1 , "%s\n"     , "DU(625),DV(625),DW(625),DX(625),DY(625)");

  for ( z = 0; z != 26; z++ ){
    n = 0;
    if ( z == 8 ) continue;
    for ( y = 0; y != 26; y++ ){
      for ( x = 0; x != 26; x++ ){
        n++;
        if ( varindex[z][y][x] != 0 ){
          fprintf(outfile1   , "%s"   , "      "          );
          fprintf(outfile1   , "%c"   , 'D'               );
          fprintf(outfile1   , "%c"   , z + 65            );
          fprintf(outfile1   , "%c"   , '['               );
          if ( n <  10 ){
            fprintf(outfile1 , "%c"   , '0'               );
          }
          if ( n < 100 ){
            fprintf(outfile1 , "%c"   , '0'               );
          }
          fprintf(outfile1   , "%d"   , n                 );
          fprintf(outfile1   , "%s"   , "]="              );
          fprintf(outfile1   , "%c"   , vartype[z][y][x]  );
          fprintf(outfile1   , "%c"   , rightbracket       );
          if ( varindex[z][y][x] <  10 ){
            fprintf(outfile1 , "%c"   , '0'               );
          }
          if ( varindex[z][y][x] < 100 ){
            fprintf(outfile1 , "%c"   , '0'               );
          }
          fprintf(outfile1   , "%d"   , varindex[z][y][x] );
          fprintf(outfile1   , "%c\n" , leftbracket               );
        }
      }
    }
  }

  fprintf(outfile1 , "%s\n"     , "      for (NN=1; NN < 626; NN+=1)" );
  fprintf(outfile1 , "%s\n"     , "         {      "        );
  fprintf(outfile1 , "%s\n"     , "         DI[NN] = I[NN];"        );
  fprintf(outfile1 , "%s\n"     , "         }"         );

/****************************************/
/*******        Part 6            *******/
/****************************************/
/*    Write wire tabs to output file    */
/****************************************/

  if ( strcmp ( argv[2] , "none" ) != 0 ){

    infile  = fopen(argv[2] , "r");             /* Check if file exists */
    if (infile == NULL){
      printf("I can t open %s\n", argv[2]);
      exit(1);
    }

    fprintf(outfile1 , "%s\n"     , "********************************"     );
    fprintf(outfile1 , "%s%s%s\n" , "*     " , argv[4] , " WIRES"          );
    fprintf(outfile1 , "%s\n"     , "********************************"     );
    fprintf(outfile1 , "%s%s%s\n" , "      SUBROUTINE " , argv[4] , "W"    );
    fprintf(outfile1 , "%s\n"     , "      IMPLICIT LOGICAL (A-Z)"         );
    fprintf(outfile1 , "%s\n"     , "      int NN;"                     );
    fprintf(outfile1 , "%s\n"     , "      REAL ST,ET,TOTL,TOTG,TOTW,TOTM" );
    fprintf(outfile1 , "%s%s"     , "      COMMON/" , argv[4]              );
    fprintf(outfile1 , "%s\n"     , "T/TOTL,TOTG,TOTW,TOTM"                );
    fprintf(outfile1 , "%s%s%s"   , "      COMMON/" , argv[4] , "/"        );
    fprintf(outfile1 , "%s%d%s"   , "L[" , tot_latch , "],"                );
    fprintf(outfile1 , "%s%d%s"   , "NL[" , tot_latch , "],"               );
    fprintf(outfile1 , "%s%d%s\n" , "G[" , tot_gate  , "]"                 );
    if ( argv[7][0] == 'Y' ){
      fprintf(outfile1 , "%s%s%s\n" , "      COMMON/" , argv[4] , "X/ON"   );
    }

    old_module[0] = '\0';
    while ( ( fgets(line , MAXI , infile ) ) != NULL ){
      sscanf ( line , "%s" , new_module );
      if ( strcmp ( old_module , new_module ) != 0 ){
        fprintf(outfile1 , "%s"   , "      COMMON/");
        fprintf(outfile1 , "%s"   , new_module     );
        fprintf(outfile1 , "%s"   , "I/"           );
        fprintf(outfile1 , "%s"   , new_module     );
        fprintf(outfile1 , "%s\n" , "I[625]"       );
      }
      strcpy ( old_module , new_module );
    }
    fclose(infile);

    if ( argv[7][0] == 'Y' ){
      fprintf(outfile1 , "%s\n"     , "      IF(ON) {" );
    }

    fprintf(outfile1 , "%s\n"     , "       SECOND(ST);"                );

    infile  = fopen(argv[2] , "r");
    while ( ( fgets(line , MAXI , infile ) ) != NULL ){
      sscanf ( line , "%s" , new_module );
      sprintf(ftn , "%s%s" , "      " , new_module );
      c = 9;

      gettoken();                               /* Ixx */
      if ( tokentype != 1 ) err(1);
      cvttoken();

      gettoken();
      if ( tokentype != 2 ) err(2);
      cvttoken();

      gettoken();                               /* Oxx */
      if ( tokentype != 1 ) err(3);
      cvttoken();
      fprintf(outfile1 , "%s\n" , ftn);
    }
    fclose(infile);

    fprintf(outfile1 , "%s\n"     , "      SECOND(ET);"                );
    fprintf(outfile1 , "%s\n"     , "      TOTW=TOTW+(ET-ST);"              );

    if ( argv[7][0] == 'Y' ){
     fprintf(outfile1 , "%s\n"     , "      ENDIF");
    }

    fprintf(outfile1 , "%s\n"     , "      }");
  }

/****************************************/
/*******        Part 7            *******/
/****************************************/
/*    Write time routine to output      */
/****************************************/

    fprintf(outfile1 , "%s\n"     , "********************************"     );
    fprintf(outfile1 , "%s%s%s\n" , "*     " , argv[4] , " TIME"           );
    fprintf(outfile1 , "%s\n"     , "********************************"     );
    fprintf(outfile1 , "%s%s%s\n" , "      void " , argv[4] , "E"    );
    fprintf(outfile1 , "%s\n"     , "       { "                     );
    fprintf(outfile1 , "%s\n"     , "      LOGICAL ON"                     );
    fprintf(outfile1 , "%s\n"     , "      int TOTON;"                  );
    fprintf(outfile1 , "%s\n"     , "      REAL TOTL,TOTG,TOTW,TOTM"       );
    fprintf(outfile1 , "%s%s"     , "      COMMON/" , argv[4]              );
    fprintf(outfile1 , "%s\n"     , "T/TOTL,TOTG,TOTW,TOTM"                );

    if ( argv[7][0] == 'Y' ){
      if ( tot_r != 0 ){
        fprintf(outfile1 , "%s%s"     , "      COMMON/" , argv[4]          );
        fprintf(outfile1 , "%s\n"     , "X/ON,TOTON"                       );
        fprintf(outfile1 , "%s"       , "      WRITE(50,100)"              );
        fprintf(outfile1 , "%s\n"     , "TOTON,TOTL,TOTG,TOTW,TOTM"        );
        fprintf(outfile1 , "%s"       , "  100 FORMAT(1X,I9,4(F10.3),5X," );
        fprintf(outfile1 , "%s%s\n"   , argv[4] , ")"                     );
      }
      else{
        fprintf(outfile1 , "%s%s"     , "      COMMON/" , argv[4]          );
        fprintf(outfile1 , "%s\n"     , "X/ON,TOTON"                       );
        fprintf(outfile1 , "%s"       , "      WRITE(50,100)"              );
        fprintf(outfile1 , "%s\n"     , "TOTON,TOTL,TOTG,TOTW"             );
        fprintf(outfile1 , "%s"       , "  100 FORMAT(1X,I9,3(F10.3),15X,");
        fprintf(outfile1 , "%s%s\n"   , argv[4] , "]"                     );
      }
    }

    if ( argv[7][0] != 'Y' ){
      if ( tot_r != 0 ){
        fprintf(outfile1 , "%s"       , "      WRITE(50,100)"              );
        fprintf(outfile1 , "%s\n"     , "TOTL,TOTG,TOTW,TOTM"              );
        fprintf(outfile1 , "%s"       , "  100 FORMAT(10X,4(F10.3),5X,"   );
        fprintf(outfile1 , "%s%s\n"   , argv[4] , "]"                     );
      }
      else{
        fprintf(outfile1 , "%s"       , "      WRITE(50,100)"              );
        fprintf(outfile1 , "%s\n"     , "TOTL,TOTG,TOTW"                   );
        fprintf(outfile1 , "%s"       , "  100 FORMAT(10X,3(F10.3),15X,"  );
        fprintf(outfile1 , "%s%s\n"   , argv[4] , "]"                     );
      }
    }

    fprintf(outfile1 , "%s\n"     , "      END");

} /* End of Main */

/****************************************/
/*******     Function cvtfor      *******/
/****************************************/

void cvtfor()
{
  int  not_flag = 0;

  if ( latch_flag == 1 ){
    sprintf(ftn , "%s" , "      N");
  }
  else{
    sprintf(ftn , "%s" , "      ");
  }

  c = 4;

  gettoken();                                   /* Left of equal sign */
  chktoken1();
  if ( tokentype == 0 ){                        /* False term */
    not_flag = 1;
    invtoken();
  }
  cvttoken();

  gettoken();                                   /* Equal sign */
  chktoken2();
  cvttoken();
  if ( not_flag == 1 ) strcat(ftn , " ~ (\0");

  while ( tokentype != 3 ){                     /* Right of equal sign */
    gettoken();
    chktoken3();
    cvttoken();
  }

  if ( not_flag == 1 ) strcat(ftn , ")\0");

}

/****************************************/
/*******     Function cvtfor_f    *******/
/****************************************/

void cvtfor_f()
{
  sprintf(ftn , "%s" , "      ");

  c = 4;                                        /* L = bcdJK */

  gettoken();                                   /* L */
  if ( tokentype != 1 ) err(4);
  cvttoken();

  while ( tokentype != 2 ) gettoken();          /* = */
  cvttoken();

  gettoken();                                   /* b */
  chktoken1();
  invtoken();
  cvttoken();

  gettoken();                                   /* AND */
  if ( tokentype != 4 ) err(5);
  cvttoken();

  gettoken();                                   /* c */
  chktoken1();
  invtoken();
  cvttoken();

  gettoken();                                   /* AND */
  if ( tokentype != 4 ) err(6);
  cvttoken();

  gettoken();                                   /* d */
  chktoken1();
  invtoken();
  cvttoken();

  gettoken();                                   /* ; */
  if ( tokentype == 9 ){
    tokentype = 4;
    cvttoken();

    gettoken();                                 /* J */
    chktoken1();
    cvttoken();

    gettoken();                                 /* K */
    if ( tokentype == 4){
      cvttoken();

      gettoken();
      chktoken1();
      cvttoken();
    }
  }

  c = 4;                                        /* M = bcDJK */

  strcat(ftn , "\n      \0");

  for ( n = 0; n != 2; n++ ) gettoken();        /* Skip L */

  gettoken();                                   /* M */
  if ( tokentype != 1 ) err(7);
  cvttoken();

  while ( tokentype != 2 ) gettoken();          /* = */
  cvttoken();

  gettoken();                                   /* b */
  invtoken();
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* c */
  invtoken();
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* D */
  cvttoken();

  gettoken();                                   /* ; */
  if ( tokentype == 9 ){
    tokentype = 4;
    cvttoken();

    gettoken();                                 /* J */
    cvttoken();

    gettoken();                                 /* K */
    if ( tokentype == 4){
      cvttoken();

      gettoken();
      cvttoken();
    }
  }

  c = 4;                                        /* N = bCdJK */

  strcat(ftn , "\n      \0");

  for ( n = 0; n != 4; n++ ) gettoken();        /* Skip L,M */

  gettoken();                                   /* N */
  if ( tokentype != 1 ) err(8);
  cvttoken();

  while ( tokentype != 2 ) gettoken();          /* = */
  cvttoken();

  gettoken();                                   /* b */
  invtoken();
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* C */
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* d */
  invtoken();
  cvttoken();

  gettoken();                                   /* ; */
  if ( tokentype == 9 ){
    tokentype = 4;
    cvttoken();

    gettoken();                                 /* J */
    cvttoken();

    gettoken();                                 /* K */
    if ( tokentype == 4){
      cvttoken();

      gettoken();
      cvttoken();
    }
  }

  c = 4;                                        /* A = bCDJK */

  strcat(ftn , "\n      \0");

  for ( n = 0; n != 6; n++ ) gettoken();        /* Skip L,M,N */

  gettoken();                                   /* A */
  if ( tokentype != 1 ) err(9);
  cvttoken();

  while ( tokentype != 2 ) gettoken();          /* = */
  cvttoken();

  gettoken();                                   /* b */
  invtoken();
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* C */
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* D */
  cvttoken();

  gettoken();                                   /* ; */
  if ( tokentype == 9 ){
    tokentype = 4;
    cvttoken();

    gettoken();                                 /* J */
    cvttoken();

    gettoken();                                 /* K */
    if ( tokentype == 4){
      cvttoken();

      gettoken();
      cvttoken();
    }
  }

  c = 4;                                        /* F = BcdJK */

  strcat(ftn , "\n      \0");

  for ( n = 0; n != 8; n++ ) gettoken();        /* Skip L,M,N,A */

  gettoken();                                   /* F */
  if ( tokentype != 1 ) err(10);
  cvttoken();

  while ( tokentype != 2 ) gettoken();          /* = */
  cvttoken();

  gettoken();                                   /* B */
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* c */
  invtoken();
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* d */
  invtoken();
  cvttoken();

  gettoken();                                   /* ; */
  if ( tokentype == 9 ){
    tokentype = 4;
    cvttoken();

    gettoken();                                 /* J */
    cvttoken();

    gettoken();                                 /* K */
    if ( tokentype == 4){
      cvttoken();

      gettoken();
      cvttoken();
    }
  }

  c = 4;                                        /* G = BcDJK */

  strcat(ftn , "\n      \0");

  for ( n = 0; n != 10; n++ ) gettoken();       /* Skip L,M,N,A,F */

  gettoken();                                   /* G */
  if ( tokentype != 1 ) err(11);
  cvttoken();

  while ( tokentype != 2 ) gettoken();          /* = */
  cvttoken();

  gettoken();                                   /* B */
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* c */
  invtoken();
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* D */
  cvttoken();

  gettoken();                                   /* ; */
  if ( tokentype == 9 ){
    tokentype = 4;
    cvttoken();

    gettoken();                                 /* J */
    cvttoken();

    gettoken();                                 /* K */
    if ( tokentype == 4){
      cvttoken();

      gettoken();
      cvttoken();
    }
  }

  c = 4;                                        /* H = BCdJK */

  strcat(ftn , "\n      \0");

  for ( n = 0; n != 12; n++ ) gettoken();       /* Skip L,M,N,A,F,G */

  gettoken();                                   /* H */
  if ( tokentype != 1 ) err(12);
  cvttoken();

  while ( tokentype != 2 ) gettoken();          /* = */
  cvttoken();

  gettoken();                                   /* B */
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* C */
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* d */
  invtoken();
  cvttoken();

  gettoken();                                   /* ; */
  if ( tokentype == 9 ){
    tokentype = 4;
    cvttoken();

    gettoken();                                 /* J */
    cvttoken();

    gettoken();                                 /* K */
    if ( tokentype == 4){
      cvttoken();

      gettoken();
      cvttoken();
    }
  }

  c = 4;                                        /* I = BCDJK */

  strcat(ftn , "\n      \0");

  for ( n = 0; n != 14; n++ ) gettoken();       /* Skip L,M,N,A,F,G,H */

  gettoken();                                   /* I */
  if ( tokentype != 1 ) err(13);
  cvttoken();

  while ( tokentype != 2 ) gettoken();          /* = */
  cvttoken();

  gettoken();                                   /* B */
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* C */
  cvttoken();

  gettoken();                                   /* AND */
  cvttoken();

  gettoken();                                   /* D */
  cvttoken();

  gettoken();                                   /* ; */
  if ( tokentype == 9 ){
    tokentype = 4;
    cvttoken();

    gettoken();                                 /* J */
    cvttoken();

    gettoken();                                 /* K */
    if ( tokentype == 4){
      cvttoken();

      gettoken();
      cvttoken();
    }
  }

  c = 4;                                        /* E = j + k */

  while ( tokentype != 2 ) gettoken();          /* Find = */

  gettoken();                                   /*  b  */
  gettoken();                                   /* AND */
  gettoken();                                   /*  c  */
  gettoken();                                   /* AND */
  gettoken();                                   /*  d  */

  gettoken();                                   /*  ;  */
  if ( tokentype == 3 ) return;

  c = 4;                                        /* E = j + k */

  for ( n = 0; n != 16; n++ ) gettoken();       /* Skip L,M,N,A,F,G,H,I */

  gettoken();
  if ( tokentype == 8 ) return;                 /* *** or E */
  if ( tokentype != 1 ) err(14);
  strcat(ftn , "\n      \0");
  cvttoken();

  gettoken();                                   /* = */
  cvttoken();

  gettoken();                                   /*  b  */
  gettoken();                                   /* AND */
  gettoken();                                   /*  c  */
  gettoken();                                   /* AND */
  gettoken();                                   /*  d  */
  gettoken();                                   /*  ;  */

  gettoken();                                   /*  j  */
  invtoken();
  cvttoken();

  gettoken();
  if ( tokentype == 4 ){
    tokentype = 5;
    cvttoken();

    gettoken();                                 /*  k  */
    gettoken();
    invtoken();
    cvttoken();
  }
}
/****************************************/
/*******     Function cvtfor_qt   *******/
/****************************************/

void cvtfor_qt()
{
  if ( strcmp ( mask , "V=Vvv+vVv+vvV+VVV" ) == 0 ){

    if ( latch_flag == 1 ){
      sprintf(ftn , "%s" , "      N");
    }
    else{
      sprintf(ftn , "%s" , "      ");
    }

    c = 4;

    gettoken();                                 /* Left of equal sign */
    cvttoken();

    gettoken();                                 /* Equal sign */
    cvttoken();

    gettoken();                                 /* Right of equal sign */
    cvttoken();

    gettoken();
    tokentype = 6;
    cvttoken();
    tokentype = 4;

    gettoken();
    invtoken();
    cvttoken();

    gettoken();
    tokentype = 6;
    cvttoken();
    tokentype = 4;

    gettoken();
    invtoken();
    cvttoken();

    return;

  }

  if ( strcmp ( mask , "v=Vvv+vVv+vvV+vvv" ) == 0 ){

    if ( latch_flag == 1 ){
      sprintf(ftn , "%s" , "      N");
    }
    else{
      sprintf(ftn , "%s" , "      ");
    }

    c = 4;

    gettoken();                                 /* Left of equal sign */
    invtoken();
    cvttoken();

    gettoken();                                 /* Equal sign */
    cvttoken();

    gettoken();                                 /* A */
    cvttoken();

    gettoken();                                 /* and */
    cvttoken();

    gettoken();                                 /* b */
    invtoken();
    cvttoken();

    gettoken();                                 /* or */
    tokentype = 5;
    cvttoken();

    c = 4;

    gettoken();                                 /* Left of equal sign */

    gettoken();                                 /* Equal sign */

    gettoken();                                 /* A */
    cvttoken();

    gettoken();                                 /* and */
    cvttoken();

    gettoken();                                 /* b */

    gettoken();                                 /* and */

    gettoken();                                 /* c */
    invtoken();
    cvttoken();

    gettoken();                                 /* or */
    tokentype = 5;
    cvttoken();

    c = 4;

    gettoken();                                 /* Left of equal sign */

    gettoken();                                 /* Equal sign */

    gettoken();                                 /* A */

    gettoken();                                 /* and */

    gettoken();                                 /* b */
    invtoken();
    cvttoken();

    gettoken();                                 /* and */
    cvttoken();

    gettoken();                                 /* c */
    invtoken();
    cvttoken();

    return;

  }

  cvtfor();

}
/****************************************/
/*******     Function cvtfor_r    *******/
/****************************************/

void cvtfor_r()
{
  int  chip_sel = 0;

  ftn[0] = '\0';

  c = 4;                                        /* Find C.S. term */
  gettoken();
  while ( tokentype != 2 ) gettoken();
  while ( tokentype != 9 ) gettoken();
  gettoken();
  while ( tokentype != 9 ) gettoken();
  gettoken();
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    chip_sel = 1;
    strcat(ftn , "      IF(\0");
    cvttoken();
    strcat(ftn , ")THEN\n\0");
  }

  strcat(ftn , "       ADD=\0");                /* Calculate base address */
  sprintf(temp , "%d" , r_add);
  strcat(ftn , temp);
  strcat(ftn , "\n\0");
  r_add = r_add + 16;

  c = 4;                                        /* Address */
  gettoken();
  while ( tokentype != 2 ) gettoken();
  while ( tokentype != 9 ) gettoken();

  gettoken();                                   /* 2**0 */
  if ( tokentype == 0 ) invtoken();
  if ( tokentype == 1 ){
    strcat(ftn , "       IF(\0");
    cvttoken();
    strcat(ftn , ")ADD=ADD+1\n\0");
  }
  gettoken();
  gettoken();                                   /* 2**1 */
  if ( tokentype == 0 ) invtoken();
  if ( tokentype == 1 ){
    strcat(ftn , "       IF(\0");
    cvttoken();
    strcat(ftn , ")ADD=ADD+2 \n\0");
  }
  gettoken();
  gettoken();                                   /* 2**2 */
  if ( tokentype == 0 ) invtoken();
  if ( tokentype == 1 ){
    strcat(ftn , "       IF(\0");
    cvttoken();
    strcat(ftn , ")ADD=ADD+4  \n\0");
  }
  gettoken();
  gettoken();                                   /* 2**3 */
  if ( tokentype == 0 ) invtoken();
  if ( tokentype == 1 ){
    strcat(ftn , "       IF(\0");
    cvttoken();
    strcat(ftn , ")ADD=ADD+8   \n\0");
  }

  c = 4;                                        /* Find W.E. term */
  gettoken();
  while ( tokentype != 2 ) gettoken();
  while ( tokentype != 9 ) gettoken();
  gettoken();
  while ( tokentype != 9 ) gettoken();
  gettoken();
  gettoken();
  gettoken();
  strcat(ftn , "       IF(\0");
  cvttoken();
  strcat(ftn , ")THEN\n\0");

  c = 4;                                        /* Write data */
  gettoken();
  while ( tokentype != 2 ) gettoken();

  gettoken();                                   /* 2**0 */
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    strcat(ftn , "        DATA0(ADD)=\0");
    cvttoken();
    strcat(ftn , "\n\0");
  }
  gettoken();
  gettoken();                                   /* 2**1 */
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    strcat(ftn , "        DATA1(ADD)=\0");
    cvttoken();
    strcat(ftn , " \n\0");
  }
  gettoken();
  gettoken();                                   /* 2**2 */
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    strcat(ftn , "        DATA2(ADD)=\0");
    cvttoken();
    strcat(ftn , "  \n\0");
  }
  gettoken();
  gettoken();                                   /* 2**3 */
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    strcat(ftn , "        DATA3(ADD)=\0");
    cvttoken();
    strcat(ftn , "   \n\0");
  }

  strcat(ftn , "       ELSE\n\0");              /* ENDIF for W.E. */

  c = 4;                                        /* Read data */

  gettoken();                                   /* 2**0 */
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    strcat(ftn , "        \0");
    cvttoken();
    strcat(ftn , "=DATA0(ADD)\n\0");
  }
  gettoken();
  gettoken();                                   /* 2**1 */
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    strcat(ftn , "        \0");
    cvttoken();
    strcat(ftn , "=DATA1(ADD) \n\0");
  }
  gettoken();
  gettoken();                                   /* 2**2 */
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    strcat(ftn , "        \0");
    cvttoken();
    strcat(ftn , "=DATA2(ADD)  \n\0");
  }
  gettoken();
  gettoken();                                   /* 2**3 */
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ){
    strcat(ftn , "        \0");
    cvttoken();
    strcat(ftn , "=DATA3(ADD)   \n\0");
  }

  strcat(ftn , "       ENDIF\n\0");             /* ENDIF for W.E. */

  if ( chip_sel   == 1 ){                       /* ENDIF for C.S. */
    strcat(ftn , "      ENDIF\0");
  }

}

/****************************************/
/*******     Function cvtfor_u    *******/
/****************************************/

void cvtfor_u()
{
  int  not_flag = 0;
  int  enable = 0;

  sprintf(ftn , "%s" , "      ");

  c = 4;                                        /* Find enable term */

  gettoken();
  while ( ! ( ( tokentype == 3 ) || ( tokentype == 9 ) ) ) gettoken();
  if ( tokentype == 3 ) err(15);
  gettoken();
  if ( ( tokentype == 0 ) || ( tokentype == 1 ) ) enable = 1;

  c = 4;

  gettoken();                                   /* Left of equal sign */
  chktoken1();
  if ( tokentype == 0 ){                        /* False term */
    not_flag = 1;
    invtoken();
  }
  cvttoken();

  gettoken();                                   /* Equal sign */
  chktoken2();
  cvttoken();
  if ( not_flag == 1 ) strcat(ftn , " ~(\0");
  if ( enable   == 1 ) strcat(ftn , "(\0");

  while ( tokentype != 3 ){                     /* Right of equal sign */
    gettoken();
    if ( tokentype == 9 ) break;                /* ; */
    chktoken3();
    cvttoken();
  }

  if ( enable   == 1 ){
    strcat(ftn , "]\0");

    tokentype = 4;
    cvttoken();

    gettoken();
    cvttoken();
  }

  if ( not_flag == 1 ) strcat(ftn , "]\0");

}

/****************************************/
/*******     Function gettoken    *******/
/****************************************/
/*  type       meaning         example  */
/*  -----   --------------   ---------- */
/*    0     false trigraph       aaa    */
/*    1     true  trigraph       AAA    */
/*    2     equal sign            =     */
/*    3     period                .     */
/*    4     AND operator                */
/*    5     OR  operator          +     */
/*    6     XOR operator          \     */
/*    7     forced "0"           ---    */
/*    8     forced "1"           ***    */
/*    9     semi-colon            ;     */
/****************************************/

void gettoken()
{
  while ( ! ( (line[c] == ' ') && (line[c+1] != ' ') ) ) c++;

  if ( isalpha ( line[c+1] ) ){                 /* And */
    if ( isalpha ( line[c-1] ) ){
      if ( tokentype != 4 ){
        tokentype = 4;
        return;
      }
    }
  }

  if ( isalpha ( line[c+1] ) ){
    if ( ( line[c-1] == '-' ) || ( line[c-1] == '*' ) ){
      if ( tokentype != 4 ){
        tokentype = 4;
        return;
      }
    }
  }

  if ( isalpha ( line[c-1] ) ){
    if ( ( line[c+1] == '-' ) || ( line[c+1] == '*' ) ){
      if ( tokentype != 4 ){
        tokentype = 4;
        return;
      }
    }
  }

  if ( ( line[c-1] == '-' ) || ( line[c-1] == '*' ) ){
    if ( ( line[c+1] == '-' ) || ( line[c+1] == '*' ) ){
      if ( tokentype != 4 ){
        tokentype = 4;
        return;
      }
    }
  }

  if ( isalpha ( line[c+1] ) ){                 /* Trigraph */
    if ( isalpha ( line[c+2] ) ){
      if ( isalpha ( line[c+3] ) ){
        token[0] = line[c+1];
        token[1] = line[c+2];
        token[2] = line[c+3];
        token[3] = NULLCHAR;
        tokentype = 0;
        if ( isupper ( line[c+1] ) ) tokentype = 1;
        c = c + 4;
        return;
      }
    }
  }

  if ( line[c+1] == '=' ){                      /* Equal sign */
    token[0] = '=';
    token[1] = NULLCHAR;
    tokentype = 2;
    c++;
    return;
  }

  if ( line[c+1] == '.' ){                      /* Period */
    token[0] = '.';
    token[1] = NULLCHAR;
    tokentype = 3;
    c++;
    return;
  }

  if ( line[c+1] == '+' ){                      /* Or */
    tokentype = 5;
    c++;
    return;
  }

  if ( line[c+1] == '\\' ){                     /* Exclusive Or */
    token[0] = '\\';
    token[1] = NULLCHAR;
    tokentype = 6;
    c++;
    return;
  }

  if ( line[c+1] == '-' ){                      /* Forced "0" */
    if ( line[c+2] == '-' ){
      if ( line[c+3] == '-' ){
        tokentype = 7;
        c = c + 4;
        return;
      }
    }
  }

  if ( line[c+1] == '*' ){                      /* Forced "1" */
    if ( line[c+2] == '*' ){
      if ( line[c+3] == '*' ){
        tokentype = 8;
        c = c + 4;
        return;
      }
    }
  }

  if ( line[c+1] == ';' ){                      /* Semi-colon */
    tokentype = 9;
    c++;
    return;
  }

  err(16);                                        /* No token found */

}
/****************************************/
/*******     Function chktoken1   *******/
/****************************************/

void chktoken1()
{
  if ( tokentype == 0 ) return;
  if ( tokentype == 1 ) return;
  if ( tokentype == 2 ) err(17);
  if ( tokentype == 3 ) err(18);
  if ( tokentype == 4 ) err(19);
  if ( tokentype == 5 ) err(20);
  if ( tokentype == 6 ) err(21);
  if ( tokentype == 7 ) err(22);
  if ( tokentype == 8 ) err(23);
  if ( tokentype == 9 ) err(24);
}

/****************************************/
/*******     Function chktoken2   *******/
/****************************************/

void chktoken2()
{
  if ( tokentype == 0 ) err(25);
  if ( tokentype == 1 ) err(26);
  if ( tokentype == 2 ) return;
  if ( tokentype == 3 ) err(27);
  if ( tokentype == 4 ) err(28);
  if ( tokentype == 5 ) err(29);
  if ( tokentype == 6 ) err(30);
  if ( tokentype == 7 ) err(31);
  if ( tokentype == 8 ) err(32);
  if ( tokentype == 9 ) err(33);
}

/****************************************/
/*******     Function chktoken3   *******/
/****************************************/

void chktoken3()
{
  if ( tokentype == 0 ) return;
  if ( tokentype == 1 ) return;
  if ( tokentype == 2 ) err(34);
  if ( tokentype == 3 ) return;
  if ( tokentype == 4 ) return;
  if ( tokentype == 5 ) return;
  if ( tokentype == 6 ) return;
  if ( tokentype == 7 ) return;
  if ( tokentype == 8 ) return;
  if ( tokentype == 9 ) err(35);
}

/****************************************/
/*******     Function cvttoken    *******/
/****************************************/

void cvttoken()
{
  int debug;

  debug = 0;

  if ( (tokentype == 0) || (tokentype == 1) ){

    if ( tokentype == 0 ){                      /* False term */
      strcat(ftn , "~ \0");
    }

    z = toupper( token[0] ) - 65;
    y = toupper( token[1] ) - 65;
    x = toupper( token[2] ) - 65;
  
    if (debug)
      {
	printf("In cvttoken - token in = %c%c%c \n",token[0],token[1],
	       token[2]);
        printf("x = %d y = %d z = %d \n",x,y,z);
        printf("vartype z,y,x = %c \n",vartype[z][y][x]);
      }
    if ( vartype[z][y][x] == 'G' ) strcat(ftn , "G[\0");
    if ( vartype[z][y][x] == 'I' ) strcat(ftn , "I[\0");
    if ( vartype[z][y][x] == 'L' ) strcat(ftn , "L[\0");

    if ( varindex[z][y][x] < 10  ) strcat(ftn , "0\0");
    if ( varindex[z][y][x] < 100 ) strcat(ftn , "0\0");

    sprintf(temp , "%d" , varindex[z][y][x] );
    strcat(ftn , temp);

    strcat(ftn , "]\0");
  }

  if ( tokentype == 2 ) strcat(ftn , token);    /* Equal sign */

  if ( tokentype == 3 ) return;                 /* Period */

  if ( tokentype == 4 ) strcat(ftn , " & \0");  /* AND operator */

  if ( tokentype == 5 ) strcat(ftn , " | \0");  /* OR  operator */

  if ( tokentype == 6 ) strcat(ftn , " ^ \0");  /* XOR operator */

  if ( tokentype == 7 ) strcat(ftn , " FORCE0\0");  /* Forced "0" */

  if ( tokentype == 8 ) strcat(ftn , "FORCE1\0");  /* Forced "1" */

  if ( tokentype == 9 ) return;                 /* Semi-colon */

}
/****************************************/
/*******     Function invtoken    *******/
/****************************************/

void invtoken()
{
  int debug;
  debug = 0;
  if ( debug) { printf("In invtoken - token = %c %c %c \n",token[0],
		       token[1], token[2]); }
   
  if ( tokentype == 1 ){                        /* True --> False */
    token[0] = tolower ( token[0] );
    token[1] = tolower ( token[1] );
    token[2] = tolower ( token[2] );
    tokentype = 0;
    if ( debug) { printf("In invtoken- tf- token out  = %c%c%c \n",token[0],
		      token[1],token[2]); }
    return;
  }

  if ( tokentype == 0 ){                        /* False --> True */
    token[0] = toupper ( token[0] );
    token[1] = toupper ( token[1] );
    token[2] = toupper ( token[2] );
    tokentype = 1;

    if ( debug) { printf("In invtoken- ft- token out  = %c%c%c \n",token[0],
		      token[1],token[2]); }
    return;
  }

}

/****************************************/
/*******     Function err         *******/
/****************************************/

void err(int errno)
{
  printf("%s\n" , "Format error" );
  printf("%s" , line );
  printf("errono = %d " ,errno);
  printf("%*c" , c + 1 , '\030' );
  exit(1);
}

/****************************************/
/*******     Function genmask     *******/
/****************************************/

int genmask()
{
  n = 0;

  for ( c = 0; line[c] != '.'; c++ ){

    if ( line[c] == ' ' ){

      if ( line[c + 1] == '+' ){
        mask[n] = '+';
        n++;
      }

      if ( line[c + 1] == '=' ){
        mask[n] = '=';
        n++;
      }

      if ( line[c + 1] == '/' ){
        mask[n] = '/';
        n++;
      }

      if ( line[c + 1] == '-' ){
        mask[n] = '-';
        n++;
      }

      if ( line[c + 1] == '*' ){
        mask[n] = '*';
        n++;
      }

      if ( line[c + 1] == ';' ){
        mask[n] = ';';
        n++;
      }

      if ( isalpha( line[c+1] ) ){

        if ( isupper( line[c+1] ) ){
          mask[n] = 'V';
          n++;
        }

        if ( islower( line[c+1] ) ){
          mask[n] = 'v';
          n++;
        }
      }
    }
  }
  mask[n] = '\0';
}
