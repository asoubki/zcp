#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <string>
//#include <typeinfo>
//#include <cxxabi.h>
#include "zfile.hh"



using namespace std;


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
 * \brief default compression leve
 * */
#define __DEFAULT_COMPRESS_LEVEL__     1

/*!
 * \brief default number of threads
 * */
#define __DEFAULT_THREADS__            8

/*!
 * \brief default bloc size
 * */
#define __DEFAULT_BLOC_SIZE__          64*1024

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
 * \brief code errors
 * */
typedef enum
{
   e_SuccessError = 0, /*!< Success */
   e_UsageError,       /*!< Usage Syntax error */ 
   e_InitError,        /*!< Initialisation error */
   e_ReadError,        /*!< Read error */
   e_WriteError,       /*!< Write error */
} ErrorCode_t;

/*!
 * \brief  get file extention based on the compression type
 *
 * \param type [in] : compression type
 * */
const char * extention (zFile::CompressionType_t type)
{
  switch(type) 
  {
    case zFile::e_lz4     : return ".lz4";
    case zFile::e_lz4hc   : return ".lz4hc";
    case zFile::e_snappy  : return ".snappy";
    case zFile::e_zlib    : return ".gz"; 
    case zFile::e_zstd    : return ".z"; 
    case zFile::e_none    : return ".cpy";
    case zFile::e_zerr    :
    default               : return ".err";    
    
  }
}

/*!
 * \brief  get compression type in str based on the compression type
 *
 * \param type [in] : compression type
 * */
const char * tostring (zFile::CompressionType_t type)
{
  switch(type) 
  {
    case zFile::e_lz4     : return "lz4";
    case zFile::e_lz4hc   : return "lz4hc";
    case zFile::e_snappy  : return "snappy";
    case zFile::e_zlib    : return "zlib"; 
    case zFile::e_zstd    : return "zstd"; 
    case zFile::e_none    : return "none";
    case zFile::e_zerr    :
    default               : return "error";    
  }
}



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
  printf("\tzcp \t multi-thread compression program\n");

  /* -- NAME SECTION -- */
  /* ------------------ */
  printf("\n");
  printf(BOLD "SYNOPSIS" RESET "\n");
  printf("\tzcp \t [" BOLD " options " RESET "] " UNDERLINE "input" RESET " " UNDERLINE "output" RESET "\n");

  /* -- OPTION SECTION -- */
  /* -------------------- */
  printf("\n");
  printf(BOLD "OPTIONS" RESET "\n");
  // -- decompress
  printf(BOLD "\t-d, --unzip " RESET "\n");        
  printf("\t\t unzip file\n");    
  printf("\n");
  // -- input file
  printf(BOLD "\t-t, --type " RESET "type\n");        
  printf("\t\t compression type : lz4, lz4hc, zlib, zstd, snappy\n");    
  printf("\n");
  // -- output file
  printf(BOLD "\t-l, --level " RESET "level\n");        
  printf("\t\t compression level : 1 low ... -9 high\n");    
  printf("\n");
  // -- number of thread
  printf(BOLD "\t-p, --threads " RESET "number\n");        
  printf("\t\t number of threads : default 8\n");    
  printf("\n");
  // -- number of thread
  printf(BOLD "\t-b, --bloc-size " RESET "size\n");        
  printf("\t\t block size in bytes, KB (xxxK) or MB (xxxM) : default 64KB\n");    
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
  string                   outputfile("");
  zFile::CompressionType_t compresstype  = zFile::e_none;
  int16_t                  i16level      = __DEFAULT_COMPRESS_LEVEL__;
  int16_t                  i16thread     = __DEFAULT_THREADS__;
  int32_t                  blocksize     = __DEFAULT_BLOC_SIZE__;
  bool                     unzip         = false;


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
        if ( (strcmp(argv[i],"-d") == 0) || (strcmp(argv[i],"--unzip") == 0) ) {
          unzip = true;
        }
        else if ( (strcmp(argv[i],"-l") == 0) || (strcmp(argv[i],"--level") == 0) ) {
          __CHECK_NUMBER_ARGUMENT__();
          i16level = atoi(argv[i+1]);
          i++;
        }
        else if ( (strcmp(argv[i],"-p") == 0) || (strcmp(argv[i],"--threads") == 0) ) {
          __CHECK_NUMBER_ARGUMENT__();
          i16thread = atoi(argv[i+1]);
          i++;
        }
        else if ( (strcmp(argv[i],"-t") == 0) || (strcmp(argv[i],"--type") == 0) ) {
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
        else if ( (strcmp(argv[i],"-b") == 0) || (strcmp(argv[i],"--bloc-size") == 0) ) {
          char unit;
          char eol;
          __CHECK_NUMBER_ARGUMENT__();
          /* the block size can be xK or xM */
          if (sscanf(argv[i+1], "%d%[kKmM]%c", &blocksize, &unit, &eol) == 2)
          {
            // convert blocksize in bytes
            switch (unit)
            {
              case 'k' :
              case 'K' : blocksize *= 1024; break;
              case 'm' :
              case 'M' : blocksize *= 1024 * 1024; break;
              default  : 
              {
                fprintf(stderr, RED ">>> Usage Error : Unknow bloc unit '%s'" RESET "\n", argv[i+1]);
                return e_UsageError;                
              }
            }
          }
          /* the block size can be x (bytes) */
          else if (sscanf(argv[i+1], "%d%c", &blocksize, &eol) == 1)
          {
            // blocksize already in bytes
          }
          else
          {
            fprintf(stderr, RED ">>> Usage Error : Unknow bloc size '%s'" RESET "\n", argv[i+1]);
            return e_UsageError;                
          }
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
        else if (outputfile.length() == 0)
          outputfile = argv[i];
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
  if (!unzip)
  {
    if (compresstype == zFile::e_zerr)
    {
      fprintf(stderr, RED ">>> Syntax Error : invalid compression type" RESET "\n");
      return e_UsageError;    
    }
    else
    {
      switch(compresstype)
      {
        case zFile::e_snappy :
        case zFile::e_zlib   :
        case zFile::e_zstd   :
        case zFile::e_lz4hc  :
        {
          fprintf(stderr, RED ">>> Implement Error : format not implemented yet" RESET "\n");
          return e_UsageError;            
        }
        default : break;
      }
    }
  }
  else
  {
    // argument non initialized
    if ( compresstype == zFile::e_none )
      compresstype = zFile::getFormat(inputfile.c_str());
  }
  if (outputfile.length() == 0)
  {
    if (!unzip)
      outputfile = inputfile + extention(compresstype);
    else
    {
      size_t lastindex = inputfile.find_last_of("."); 
      if ( lastindex == string::npos)
      {
        fprintf(stderr, RED ">>> Syntax Error : output file must be set (no extension detected)" RESET "\n");
        return e_UsageError;            
      }
      else
        outputfile      = inputfile.substr(0, lastindex); 
    }
    
  }

  /* print arguments */
  printf(BOLD UNDERLINE "ARGUMENTS [%s]:" RESET "\n\n", unzip ? "UNZIP" : "ZIP");
  printf("  > " UNDERLINE "input file" RESET "  : %s\n", inputfile.c_str());
  printf("  > " UNDERLINE "output file" RESET " : %s\n", outputfile.c_str());
  printf("  > " UNDERLINE "type" RESET "        : %s\n", tostring(compresstype));
  if (!unzip)
  {
    printf("  > " UNDERLINE "level" RESET "       : %d\n", i16level);
    printf("  > " UNDERLINE "threads" RESET "     : %d\n", i16thread);
    printf("  > " UNDERLINE "bloc size" RESET "   : %d bytes\n", blocksize);
  }
  printf("\n");
  printf("\n");
  fflush(stdout);

  /* ------------------------------------------------------------------------------------ */
  /* --                                   INITIALIZATION                               -- */
  /* ------------------------------------------------------------------------------------ */
  /* openning files */
  zFile * zfin = NULL;
  if (unzip)
    zfin  = zFile::create(inputfile.c_str(), zFile::e_ReadMode, compresstype);
  else
    zfin  = zFile::create(inputfile.c_str(), zFile::e_ReadMode);
  if (zfin->fail())
  {
    fprintf(stderr, RED ">>> I/O Error : Error openning input file '%s' : %s" RESET "\n", inputfile.c_str(), zfin->strError());
    delete zfin;
    return e_InitError;        
  }
  zFile * zfout = NULL;
  if (unzip)
    zfout = zFile::create(outputfile.c_str(), zFile::e_WriteMode);
  else
    zfout = zFile::create(outputfile.c_str(), zFile::e_WriteMode, compresstype, i16level, blocksize, i16thread);  
  if (zfout->fail())
  {
    fprintf(stderr, RED ">>> I/O Error : Error openning output file '%s' : %s" RESET "\n", outputfile.c_str(), zfout->strError());
    delete zfin;
    delete zfout;
    return e_InitError;        
  }

  /* ------------------------------------------------------------------------------------ */
  /* --                                 PROCESSING DATA                                -- */
  /* ------------------------------------------------------------------------------------ */
  ErrorCode_t     error = e_SuccessError;
  char          * inbuffer = (char *) malloc (blocksize);
  struct stat     st;  
  stat(inputfile.c_str(), &st);
  size_t          filesize  = st.st_size;
  size_t          ngread    = 0;
  int             lastratio = 0;

  //int aux;
  //printf(">>> in  Class Name = %s\n", abi::__cxa_demangle(typeid(zfin).name(),  0, 0, &aux));
  //printf(">>> out Class Name = %s\n", abi::__cxa_demangle(typeid(zfout).name(), 0, 0, &aux));

  while (!zfin->eof())
  {
    // read
    size_t nread = zfin->read(inbuffer, blocksize);
    if (zfin->fail())
    {
      if (!zfin->eof())
      {
        fprintf(stderr, RED ">>> I/O Error : Error reading input file : %s" RESET "\n", zfin->strError());
        error = e_ReadError;        
        break;
      }
    }
    ngread += nread;
    // write
    zfout->mwrite(inbuffer, nread);
    if (zfout->fail())
    {
      fprintf(stderr, RED ">>> I/O Error : Error writing into output file : %s" RESET "\n", zfout->strError());
      error = e_WriteError;        
      break;
    }

    // Progress
    int ratio = int(100. * float(ngread)/float(filesize));
    if (ratio != lastratio)
    {
      printf("   > progress: %ld MB / %ld MB (%2d %%)\r", ngread/1024/1024, filesize/1024/1024, ratio);
      fflush(stdout); 
      lastratio = ratio;
    }
  }
  printf("\n");
  printf("\n");
  printf("\n");
  

  /* ------------------------------------------------------------------------------------ */
  /* --                                      CLOSE                                    -- */
  /* ------------------------------------------------------------------------------------ */
  /* closing input and output files */
  zfin->close();
  zfout->close();
  printf("  > " UNDERLINE "Compress Ratio" RESET "  : %2.2f %%\n", zfout->getRatio());
  printf("\n");
  printf("\n");
  delete zfin;
  delete zfout;
  free(inbuffer);


  return error;


}

