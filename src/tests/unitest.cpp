#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../zfile.hh"


const char * beginbuffer = "...0.r..w....\n0000e20: 0d02 0200 6007 0707 3291 cf21 0041 0040  ....`...2..!.A.@\n0000e30: e014 bc02 0602 0005 2800 " \
                           "04f4 0000 4b0c  ........(.....K.\n0000e40: 001c 1178 1f45 1a19 9200 9476 0202 0200  ...x.E.....v....\n0000e50: 500a eb9c " \
                           "5d77 6f01 007f 0810 ffa2 050b  P...]wo.........\n0000e60: c40a 01a0 0509 a00a 0112 000a fa0a 0c64  ...............d\n" \
                           "0000e70: 070f fa0a 0001 3600 1136 d405 1444 d405  ......6..6...D..\n0000e80: b000 0024 9849 452d 1992 0058 9a00 7206  " \
                           "...$.IE-...X..r.\n0000e90: 0375 69bf 3e82 cb14 \0";
const char * currentbuffer = "0f02 0023 01f6 0108  .ui.>......#....\n0000ea0: 0200 f247 46e9 1aa2 004c 0330 0303 bf28  ...GF....L.0...(\n0000eb0: 9966 " \
                             "bf28 94aa 3eb8 efef 3eb9 00e4 418d  .f.(..>...>...A.\n0000ec0: abf7 3a6f d22f 3c07 7c26 3a80 fb27 3bc7  ..:o./<.|&:..';.\n" \
                             "0000ed0: 8519 3b1f 41f8 bb67 e492 3a6b cd74 3c07  ..;.A..g..:k.t<.\n0000ee0: d90d 3a7a 9f2f 3bc8 d538 3d8f 4d6c 3dc2  ..:z." \
                             "/;..8=.Ml=.\n0000ef0: f1f5 bd27 24bd 46f0 1aa2 3c14 f104 bc28  ...'$.F...<....(\n0000f00: a9c1 bc44 1a0d bbf7 ad7e bc1d " \
                             "c6de 3a91  ...D.....~....:.\n0000f\0"; 
                          
const char * endbuffer     = "10: 19a8 0cf5 00b8 def4 ac3b 6faf 2cbb c1ac  .........;o.,...\n0000f20: c83b f8f5 201a 8646 f61a a200 3c30 03ba  .;.. ..F..." \
                             ".<0..\n0000f30: 0380 c2ae 271c c2af 0307 b200 40bb 03a0  ....'.......@...\n0000f40: ee08 0004 0200 001c 00b1 b9fc e40c bb5a" \
                             "  ...............Z\n0000f50: 116a 36cc a716 0140 46fc 1aa2 fa0a b303  .j6....@F.......\n0000f60: 033a 9b95 f543 a6fe ba02 " \
                             "bf03 002e 0042  .:...C.........B\n0000f70: 4702 1aa2 320a 0054 0000 1200 0004 0080  G...2..T........\n0000f80: 422e 7559 " \
                             "4707 1aa2 5e04 f015 3030 3fae\0";

/* ------------------------------------------------------------------------ */
/* --                              DEFINES                                  */ 
/* ------------------------------------------------------------------------ */
/*!
 * \brief  Macros to add color to terminal output.
 * */
#define RESET        "\033[0m"
#define BOLD         "\033[1m"
#define UNDERLINE    "\033[4m"
#define RED          "\033[31m"      /* Red */
#define GREEN        "\033[32m"      /* Green */
#define BLUE         "\033[34m"      /* Blue */
#define MAGENTA      "\033[35m"      /* Magenta */
#define WHITE        "\033[37m"      /* White */

/*!
 * \brief code errors
 * */
typedef enum
{
   e_SuccessError = 0, /*!< Success */
   e_UsageError,       /*!< Usage Syntax error */ 
   e_InitError,        /*!< Initialisation error */
   e_ReadError,        /*!< Read error */
   e_SeekError,        /*!< Seek error */
} ErrorCode_t;

/*!
 * \brief check argument validity macro
 * */
#define __CHECK_NUMBER_ARGUMENT__()   ({ \
                                         if ((argc-1) < i+1) { \
                                           fprintf(stderr, RED ">>> Usage Error : Missing value for argument '%s'" RESET "\n", argv[i]); \
                                           return e_UsageError; \
                                         } \
                                       })

/*! 
 *
 * \fn      printUsage
 * \brief   Fonction d'affichage de l'usage de l'outil
 *
 * Cette fonction permet l'affichage a l'ecran (stdout) de l'usage de 
 * l'outil
 *
 * \return  Pas de retour
 *
 * ----------------------------------------------------------------------- */
void printUsage ( )
{
  /* -- NAME SECTION -- */
  /* ------------------ */
  printf("\n");
  printf(BOLD "NAME" RESET "\n");
  printf("\tunitest \t zfile lib unitest\n");

  /* -- NAME SECTION -- */
  /* ------------------ */
  printf("\n");
  printf(BOLD "SYNOPSIS" RESET "\n");
  printf("\tzcp \t [" BOLD " options " RESET "] " UNDERLINE "input" RESET "\n");

  /* -- OPTION SECTION -- */
  /* -------------------- */
  printf("\n");
  printf(BOLD "OPTIONS" RESET "\n");
  // -- input file
  printf(BOLD "\t-t, --type " RESET "type\n");        
  printf("\t\t compression type : lz4, lz4hc, zlib, zstd, snappy\n");    
  printf("\n");
  
  printf("\n");
}

/*!
 * \brief   program entry pointer
 *
 * \param  argc [in] : number of arguments
 * \param  argv [in] : argument table
 * */
int main(int argc, char** argv) 
{
  string                   inputfile("");
  zFile::CompressionType_t compresstype  = zFile::e_none;


  /* ------------------------------------------------------------------------------------ */
  /* --                               ARGUMENTS MANAGEMENT                             -- */
  /* ------------------------------------------------------------------------------------ */
  /* Print Usage */
  if (argc <= 1)
  {
    printUsage();
    return e_SuccessError;
  }

  /* Parse Arguments */
  for (int i = 1; i < argc; i++) 
  {
    switch(argv[i][0])
    {
      case '-' :
      {
        if ( (strcmp(argv[i],"-t") == 0) || (strcmp(argv[i],"--type") == 0) ) {
          __CHECK_NUMBER_ARGUMENT__();
          if (strcmp(argv[i+1], "lz4") == 0)
            compresstype = zFile::e_lz4;
          else if (strcmp(argv[i+1], "lz4hc") == 0)
            compresstype = zFile::e_lz4hc;
          else if (strcmp(argv[i+1], "snappy") == 0)
            compresstype = zFile::e_snappy;
          else if (strcmp(argv[i+1], "zlib") == 0)
            compresstype = zFile::e_zlib;
          else if (strcmp(argv[i+1], "zstd") == 0)
            compresstype = zFile::e_zstd;
          else if (strcmp(argv[i+1], "none") == 0)
            compresstype = zFile::e_none;
          else
            compresstype = zFile::e_zerr;          
          i++;
        }
        else
        {
          fprintf(stderr, RED ">>> Usage Error : Unknown argument option '%s'" RESET "\n", argv[i]);
          return e_UsageError;
        }
        break;
      }
      default :
      {
        if (inputfile.length() == 0)
          inputfile = argv[i];
        else
        {
          fprintf(stderr, RED ">>> Usage Error : Unknown option '%s'" RESET "\n", argv[i]);
          return e_UsageError;
        }
      }
    }
  }


  /* check arguments */
  if (inputfile.length() == 0)
  {
    fprintf(stderr, RED ">>> Syntax Error : no input file provided" RESET "\n");
    return e_UsageError;
  }
  // argument non initialized
  if ( compresstype == zFile::e_none )
    compresstype = zFile::getFormat(inputfile.c_str());

  /* print arguments */
  printf(BOLD UNDERLINE "ARGUMENTS :" RESET "\n\n");
  printf("  > " UNDERLINE "input file" RESET "  : %s\n", inputfile.c_str());
  printf("  > " UNDERLINE "type" RESET "        : %s\n", zFile::toString(compresstype));
  printf("\n");
  fflush(stdout);

  /* ------------------------------------------------------------------------------------ */
  /* --                                   INITIALIZATION                               -- */
  /* ------------------------------------------------------------------------------------ */
  /* openning files */
  zFile * zfin = zFile::create(inputfile.c_str(), zFile::e_ReadMode, compresstype);
  if (zfin->fail())
  {
    fprintf(stderr, RED ">>> I/O Error : Error openning input file '%s' : %s" RESET "\n", inputfile.c_str(), zfin->strError());
    delete zfin;
    return e_InitError;        
  }

  /* ------------------------------------------------------------------------------------ */
  /* --                                 PROCESSING DATA                                -- */
  /* ------------------------------------------------------------------------------------ */
  ErrorCode_t     error   = e_SuccessError;
  uint32_t        size    = 512;
  char            inbuffer[size + 1];
  size_t          seekpos = 15128;
  inbuffer[size] = '\0';
  // Check seek from beginning
  zfin->seekz(seekpos, zFile::e_SeekBegin);
  if (zfin->fail())
  {
    fprintf(stderr, RED ">>> Seek Error : %s" RESET "\n", zfin->strError());
    error = e_SeekError;
  }
  else
  {
    zfin->read(inbuffer, size);
    if (zfin->fail())
    {
      fprintf(stderr, RED ">>> I/O Error : %s" RESET "\n", zfin->strError());
      error = e_ReadError;
    }
    else if (memcmp(inbuffer, beginbuffer, size) != 0)
    {
      fprintf(stderr, RED ">>> Diff Error : read bloc is not correct" RESET "\n");
      error = e_ReadError;      
    }
    else
    {
      // Check seek from current
      zfin->seekz(seekpos, zFile::e_SeekCurrent);
      if (zfin->fail())
      {
        fprintf(stderr, RED ">>> Seek Error : %s" RESET "\n", zfin->strError());
        error = e_SeekError;
      }
      else
      {
        zfin->read(inbuffer, 512);
        if (zfin->fail())
        {
          fprintf(stderr, RED ">>> I/O Error : %s" RESET "\n", zfin->strError());
          error = e_ReadError;
        }
        else if (memcmp(inbuffer, currentbuffer, size) != 0)
        {
          fprintf(stderr, RED ">>> Diff Error : read bloc is not correct" RESET "\n");
          error = e_ReadError;      
        }
        else
        {
          // Check seek from end
          zfin->seekz(seekpos, zFile::e_SeekEnd);
          if (zfin->fail())
          {
            fprintf(stderr, RED ">>> Seek Error : %s" RESET "\n", zfin->strError());
            error = e_SeekError;
          }
          else
          {
            zfin->read(inbuffer, 512);
            if (zfin->fail())
            {
              fprintf(stderr, RED ">>> I/O Error : %s" RESET "\n", zfin->strError());
              error = e_ReadError;
            }
            else if (memcmp(inbuffer, endbuffer, size) != 0)
            {
              //printf("##@##%s##@##\n", inbuffer);
              //printf("##@##%s##@##\n", endbuffer);
              //for (int i =0; i < 512; i++)
              //  if (inbuffer[i] != currentbuffer[i])
              //    printf("%d -> %x != %x\n", i, currentbuffer[i], inbuffer[i]);
              fprintf(stderr, RED ">>> Diff Error : read bloc is not correct" RESET "\n");
              error = e_ReadError;      
            }
            else
            {
              printf(GREEN "  > Seek test succeeded\n" RESET);
            }
          }
        }
      }
    }
  }

  /* ------------------------------------------------------------------------------------ */
  /* --                                      CLOSE                                    -- */
  /* ------------------------------------------------------------------------------------ */
  /* closing input and output files */
  zfin->close();
  printf("\n");
  printf("\n");
  delete zfin;

  return error;
}

