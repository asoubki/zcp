/*!
 * \file   zfile.hh
 * \author Adil So
 * \brief  Header file
 * \date   21 December 2016
 *
 * generic file compressor interface class declaration
 *
 * See licences/zcp.txt for details about copyright and rights to use.
 *
 * Copyright (C) 2011-2016
 *
 * */
#ifndef __ZFILE_HH__
#define __ZFILE_HH__

#include <stdint.h>
#include <fstream>
#include <string>
#include <set>
#include <list>

using namespace std;

/*! 
 * \brief debug flag
 * */
#define __DEBUG__
#undef  __DEBUG__ 


/*!< log MACRO */
#ifdef __DEBUG__
#  define __log__(debugflag, ...)   if (debugflag != 0) { fprintf (stdout, __VA_ARGS__); fflush(stdout); }
#else
#  define __log__(debugflag, ...)   
#endif

/*!< predeclaration of the memory pool class */
class MemPool;

/*!
 * \brief compression file generator (abstract class)
 * */
class zFile
{
    friend class MemPool;

    /* ------------------------------------------------------------------------- */
    /* --                              CONSTANTS                              -- */
    /* ------------------------------------------------------------------------- */
  protected : 
    /*!< default number of thread */
    static const uint16_t __threads__              = 16;
    /*!< default bloc size */
    static const uint32_t __bloc_size__            = 65536;
    /*!< default compression level */
    static const uint16_t __compression_level__    = 1;
    
    /* ------------------------------------------------------------------------- */
    /* --                       TYPEDEFS AND STRUCTURES                       -- */
    /* ------------------------------------------------------------------------- */
    /*!
     * \brief Structure of an index entry in the index table 
     * */
    typedef struct _IndexEntry_t
    {
      struct _offset {
        uint32_t n;  /*!< offset of a block in the original stream */
        uint32_t z; /*!< offset of a block in the zipped stream */
        /*!< initilizer */
        _offset (uint32_t offset, uint32_t zoffset) : 
            n(offset),
            z(zoffset)
            { };
      } offset;
      
      struct _size {
        uint32_t n;  /*!< size of a block */
        uint32_t z; /*!< size of a zipped bloc */
        /*!< initilizer */
        _size (uint32_t size, uint32_t zsize) : 
            n(size),
            z(zsize)
            { };
      } size;

      /*!< initilizer */
      _IndexEntry_t (uint32_t offset, uint32_t size, uint32_t zoffset, uint32_t zsize) :
          offset(offset, zoffset),
          size(size, zsize)
          { };

      /*!< comparator */
      struct compare {
        bool operator() (const _IndexEntry_t& p1, const _IndexEntry_t& p2) const
        {
          return p1.offset.n < p2.offset.n;
        }        
      };

    } IndexEntry_t;  



  public :
    /*!
     * \brief  file open modes
     * */
    typedef enum
    {
      e_ReadMode = 0,
      e_WriteMode
    } OpenMode_t;

    /*!
     * \brief  Seek Direction
     * */
    typedef enum
    {
      e_SeekBegin = 0,
      e_SeekCurrent,
      e_SeekEnd,
    } SeekDirection_t;

     /*!
      * \brief types of compressions handled by the library
      * */
     typedef enum
     {
       e_none = 0,
       e_lz4,
       e_lz4hc,      
       e_snappy,      
       e_zlib,      
       e_zstd,
       e_zerr     
     } CompressionType_t;

     /*!
      * \brief types of compressions handled by the library
      * */
     typedef enum
     {
       e_SuccessError = 0,
       e_OpenError,        /*!< invalid openning mode */
       e_ReadError,
       e_WriteError,
       e_compressError,
       e_JobInitError,
       e_JobCreateError,
       e_JobWaitError,
       e_JobUnknownError,
       e_HeaderError,
       e_TailError,
       e_MetaError,
       e_SeekError,
     } zFileError_t;

  protected :
    /* ------------------------------------------------------------------------- */
    /* --                            CONSTRUCTORS                             -- */
    /* ------------------------------------------------------------------------- */
    /*!
     * \brief class constructor
     *
     * \param filename [in] : file name
     * \param mode     [in] : openning mode : read/write
     * \param blocsize [in] : compression bloc size 
     * \param nthread  [in] : number of threads
     *
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
    zFile (const char * filename, OpenMode_t mode, uint32_t blocsize, uint16_t nthread) ;

  public :
    /*!
     * \brief   class destructor
     * */
    virtual ~zFile ();

    /* ------------------------------------------------------------------------- */
    /* --                               METHODS                               -- */
    /* ------------------------------------------------------------------------- */
    /*!
     * \brief   get file format
     * 
     * \param filename [in] : file name
     *
     * \return file format type (CompressionType_t)
     * */
    static CompressionType_t getFormat(const char * filename); 

    /*!
     * \brief   get compression type translation
     * 
     * \param type [in] : compression typê
     *
     * \return compression type const char 
     * */
    static const char * toString(CompressionType_t type); 

    /*!
     * \brief   create the appropriate zFile stream
     * 
     * \param filename [in] : file name
     * \param mode     [in] : openning mode : read/write (default : \b ReadMode \b )
     * \param type     [in] : compression type (default : \b no compression \b)
     * \param level    [in] : compression level (default : \b __compression_level__ \b )
     * \param blocsize [in] : compression chunk size (default : \b __bloc_size__ \b )
     * \param nthread  [in] : number of threads (default : \b __bloc_size__ \b)
     *
     * \note arguments (threads, blocsize, level) are optional (ignored) when mode = read
     *
     * \return zFile object. null in case of unknown type
     *         use fail method to check the object validity
     * */
    static zFile * create(const char * filename, OpenMode_t         mode     = e_ReadMode,
                                                 CompressionType_t  type     = e_none, 
                                                 uint16_t           level    = __compression_level__,
                                                 uint32_t           blocsize = __bloc_size__,
                                                 uint16_t           nthread  = __threads__);                   

    /*!
     * \brief   read data method
     * 
     * \param ptr [in] : input data 
     * \param size [in] : data size to read
     *
     * \return return number of bytes read
     *         use fail to check for errors
     *         To get error details use getError()
     * */
    virtual size_t read(char * ptr, size_t size);  

    /*!
     * \brief   write data method in multithread mode
     * 
     * \param ptr [in] : input data 
     * \param size [in] : data size
     *
     * \return return number of bytes written
     *         use fail to check for errors
     *         To get error details use getError()
     * */
    virtual size_t mwrite(const char * ptr, size_t size);  

    /*!
     * \brief   flush data (force physical data write)
     * 
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
    virtual void flush();    

    /*!
     * \brief flush last remaining pools before close stream
     *     
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
     void close();      

    /*!
     * \brief seek position in the file. the position provided is 
     *        relative to the file
     *
     * This function do not take into account the file type. it seek the requested 
     * position in the openned file
     *
     * \param offset [in] : seek offset (can be negatif) in the file
     * \param way    [in] : seek way
     *
     * \return returned a boolean telling if an error accured during the last 
     *         operation 
     */
    virtual bool seekf(size_t offset, SeekDirection_t way);

    /*!
     * \brief seek position in the uncompressed file. the position provided is 
     *        relative to the uncompressed file
     *
     * \param offset [in] : seek offset (can be negatif) in the unzipped file
     * \param way    [in] : seek way
     *
     * \return returned a boolean telling if an error accured during the last 
     *         operation 
     */
    virtual bool seekz(size_t offset, SeekDirection_t way);

    /*!
     * \brief check for errors
     *
     * \return returned a boolean telling if an error accured during the last 
     *         operation 
     */
    virtual inline bool fail() { return _lasterror != e_SuccessError; };
    
    /*!
     * \brief check end of file 
     *
     * \return returned a boolean telling if an error accured during the last 
     *         operation 
     */
    virtual bool eof(); 

  protected :
    /*! 
     * \brief get next free pool
     *
     * if no memory pool is available, it will flush data and wait for next pool 
     *
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
    void nextPool();

    /*!
     * \brief   compress function
     * 
     * \param id      [in] : id
     * \param puser   [in] : user specific pointer
     * \param in      [in] : input data 
     * \param insize  [in] : input size
     * \param out     [in] : output buffer
     * \param outsize [in] : output buffe size
     * \param preturn [out] : compression return : 0 -> zipped, 1 -> unzipped
     *
     * \return return number of bytes written
     *         use fail to check for errors
     *         To get error details use getError()
     * */
    virtual size_t compress(uint32_t id, void * puser, const char * in, size_t insize, char * out, size_t outsize, uint32_t * preturn);      

    /*!
     * \brief   uncompress function
     * 
     * \param id [in] : id
     * \param puser [in] : user specific pointer
     * \param in [in] : input data 
     * \param insize [in] : input size
     * \param out [in] : output buffer
     * \param outsize [in] : output buffe size
     * \param preturn [out] : unzip operation return code
     *                            1 -> short input buffer
     *                            2 -> short output buffer
     *                            3 -> end of input stream
     * \param psize   [out] : remaining size in the input buffer
     *
     * \return return number of bytes written
     *         use fail to check for errors
     *         To get error details use getError()
     * */
    virtual size_t uncompress(uint32_t id, void * puser, const char * in, size_t insize, char * out, size_t outsize, uint32_t * preturn, size_t * psize);      

  public :
    /* ------------------------------------------------------------------------- */
    /* --                           GETTERS/SETTERS                           -- */
    /* ------------------------------------------------------------------------- */
    /* ------------------------------------------------------------------------- */
    /* --                           GETTERS/SETTERS                           -- */
    /* ------------------------------------------------------------------------- */
    ///*!< get compressor type */
    //inline CompressionType_t  getType()    { return _zip.type; };
    /*!< get last error */
    inline zFileError_t       getError()   { return _lasterror; };
    /*!< get compression ratio */
    inline double             getRatio()   { return ((double)_zip.noffset * 100.) / ((double)_zip.nzoffset); };
    /*!< get error translation */
    inline const char *       strError()   { return _errormsg.c_str(); };


  protected :
    /*!< set error : @todo : add customize error message */
    inline void                setError(zFileError_t error, const char * message) { _lasterror = error; _errormsg = message; };

  /* ------------------------------------------------------------------------- */
  /* --                               MEMBERS                               -- */
  /* ------------------------------------------------------------------------- */
  protected :
    zFileError_t                              _lasterror; /*!< last error */
    string                                    _errormsg;  /*!< error message */
    uint32_t                                  _blocsize;  /*!< compression blocsize */
    set<IndexEntry_t, IndexEntry_t::compare>  _listindex; /* index list */
    struct {
         uint16_t          level; /*!< compression level */
         size_t            noffset; /*< total input size */
         size_t            nzoffset; /*< total output size */
         MemPool         * curpool; /*!< Current zip memory Pool */
         list<MemPool *>   listfree; /*<! list of free pools */
         list<MemPool *>   listused; /*<! list of used pools */
         list<MemPool *>   listpool; /*<! list of pools */
    } _zip;
    struct {
         MemPool         * readpool; /*!< Current read memory Pool */
    } _unzip;
    struct {
         fstream           stream; /*!< file descriptor */
         string            filename; /*!< file name */
         OpenMode_t        mode; /*!< open mode */
         CompressionType_t type;
    } _file;
  };


 #endif /* __ZFILE_HH__ */


