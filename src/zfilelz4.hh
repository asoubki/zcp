/*!
 * \file   zfilelz4.hh
 * \author Adil So
 * \brief  Header file
 *
 * lz4 compressor class declaration
 *
 * See licences/zcp.txt for details about copyright and rights to use.
 *
 * Copyright (C) 2011-2016
 *
 * */
#ifndef __ZFILELZ4_HH__
#define __ZFILELZ4_HH__

#include <pthread.h>
#include <map>
#include "zfile.hh"
//#include "mempool.hh"
#include "lz4/xxhash.h"


using namespace std;


/*!
 * \brief lz4 file compressor
 * */
class zFilelz4 : public zFile
{
    friend class zFile;
    /* ------------------------------------------------------------------------- */
    /* --                              CONSTANTS                              -- */
    /* ------------------------------------------------------------------------- */
    /*!< min HC compresseion level */
    static const uint8_t    __min__HClevel__    = 3;
    /*!< checksum seed */
    static const uint32_t   __checksum_seed__   = 0;
   
    /* ------------------------------------------------------------------------- */
    /* --                       TYPEDEFS AND STRUCTURES                       -- */
    /* ------------------------------------------------------------------------- */
    /*!< compress function type def */
    typedef int (*compressFunc_t  )(void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level);
    /*!< uncompress function type def */
    typedef int (*uncompressFunc_t)(void* ctx, const char* src, char* dst, int size, int maxsize);

    /*!
     * \brief type of metadata handled in the lz4 skippable
     * */
    typedef enum 
    {
      e_MetaIndex = 0, /*!< index metadata */
    } MetaData_t;

    /*!
     * \brief Structure of headers and footers of skippable frames
     * */
    typedef struct
    {
      uint32_t magic; /*!< magic number identifying the skippable frame */
      uint32_t size; /*!< size of the skippable frame */          
    } MetaHeader_t;

    /*!
     * \brief Structure of the header of the data stored in the skippable frame 
     * */
    typedef struct
    {
      uint32_t magic;   /*!< magic number of the data (managed by the application) */
      uint32_t version; /*!< version number of the data (managed by the application) */
      uint32_t type;    /*!< type of metadata */
      uint32_t size;    /*!< size of the data */
    } MetaIdentification_t;

    /*!
     * \brief lz4 file header structure 
     * */
    typedef struct 
    {
      uint32_t magic;
      uint8_t  flag;
      uint8_t  blk;
      uint8_t  crc;
      uint8_t  unused;
    } FileHeader_t;  
       
  
    /* ------------------------------------------------------------------------- */
    /* --                            CONSTRUCTORS                             -- */
    /* ------------------------------------------------------------------------- */
  protected : 
    /*!
     * \brief   class constructor
     * 
     * \param filename [in] : file name
     * \param mode     [in] : openning mode : read/write
     * \param blocsize [in] : compression bloc size 
     * \param level    [in] : compression level 
     * \param nthread  [in] : number of threads
     *
     * \return zFile object. null in case of unknown type
     *         use fail method to check the object validity
     *
     * */
    zFilelz4 (const char * filename, OpenMode_t mode, uint32_t blocsize, uint8_t level, uint16_t nthread);

  public :

    /*!
     * \brief   class destructor
     * */
    virtual ~zFilelz4 ();

    /* ------------------------------------------------------------------------- */
    /* --                               METHODS                               -- */
    /* ------------------------------------------------------------------------- */

    /*!
     * \brief   flush data (force physical data write)
     *
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
    virtual void flush();  

    /*!
     * \brief seek position in the uncompressed file. the position provided is 
     *        relative to the uncompressed file
     *
     * \param offset [in] : seek offset (can be negatif) in the uncompressed file
     * \param way    [in] : seek way
     *
     * \return returned a boolean telling if an error accured during the last 
     *         operation 
     */
    //virtual bool seekz(size_t offset, SeekDirection_t way);

    /*!
     * \brief seek position in the file compressed file. the position provided is 
     *        relative to the compressed file
     *
     * \param offset [in] : seek offset (can be negatif) in the zipped file
     * \param way    [in] : seek way
     *
     * \return returned a boolean telling if an error accured during the last 
     *         operation 
     */
    //virtual bool seekf(size_t offset, SeekDirection_t way) { return zfile::seek(offset, way); };    

    /*!
     * \brief check end of file 
     *
     * \return returned a boolean telling if an error accured during the last 
     *         operation 
     */
    //virtual inline bool eof() 
    //    { 
    //      if (_file.stream.eof())
    //      {
    //        if (_unzip.readpool->isEmpty())
    //          return true;
    //        else
        //       return false;

        //   } 
        //   return false;
        // };
    

  private :
    /*!
     * \brief   read lz4 file header
     *
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
    void readHeader ();

    /*!
     * \brief   read lz4 file tail
     *
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
    void readTail ();

    /*!
     * \brief   write lz4 file header
     *
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
    void writeHeader ();

    /*!
     * \brief write lz4 file tail
     *
     * \return  no returned value. use fail to check for errors
     *          To get error details use getError()
     * */
    void writeTail ();

    /*!
     * \brief   lz4 compress function
     * 
     * \param id [in] : id
     * \param puser [in] : user specific pointer
     * \param in [in] : input data 
     * \param insize [in] : input size
     * \param out [in] : output buffer
     * \param outsize [in] : output buffe size
     * \param preturn [out] : compression return : 0 -> zipped, 1 -> unzipped
     *
     * \return return number of bytes written
     *         use fail to check for errors
     *         To get error details use getError()
     * */
    virtual size_t compress(uint32_t id, void * puser, const char * in, size_t insize, char * out, size_t outsize, uint32_t * preturn);      

    /*!
     * \brief   lz4 uncompress function
     * 
     * \param id      [in] : id
     * \param puser   [in] : user specific pointer
     * \param in      [in] : input data 
     * \param insize  [in] : input size
     * \param out     [in] : output buffer
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

  protected :
    /*!
     * \brief check if the file is a lz4 file format
     *
     * \param filename [in] : file name
     * 
     * return a boolean 
     * */
    static bool islz4 (const char * filename);
               

    /* ------------------------------------------------------------------------- */
    /* --                               MEMBERS                               -- */
    /* ------------------------------------------------------------------------- */
  private :   
      struct {
        compressFunc_t     compressfunc;   /*!< compress function */
        void            ** context;      /*!< table of contexts */
        uint16_t           ncontext;     /*!< number of contexts */
      } _lz4;
      struct {
        bool               eos;            /*!< end of lz4 stream flag */
        uncompressFunc_t   uncompressfunc; /*!< uncompress function */
      } _unlz4;

      struct __flag { 
        uint8_t	presetDictionary   : 1;	//* bit[0]   : Preset Dictionary
        uint8_t	reserved1          : 1;	//* bit[1]   : Reserved
        uint8_t	streamChecksum     : 1;	//* bit[2]   : chekcsum : 1 = enabled, 0 = disabled
        uint8_t	streamSize         : 1;	//* bit[3]   : Size of uncompressed (original) content ; 0 == unknown
        uint8_t	blockChecksum      : 1;	//* bit[4]   : bloc checksum 
        uint8_t	blockIndependence  : 1;	//* bit[5]   : block link
        uint8_t	versionNumber      : 2;	//* bit[6,7] : LZ4 Version 

        __flag() : 
            presetDictionary(0),  // Not implemented yet
            reserved1(0),
            streamChecksum(1),
            streamSize(0),
            blockChecksum(0),     // not supported for the time being
            blockIndependence(1), // bloc links : 1 = Independent
            versionNumber(1)
            { };
            
      }                             _flag;
      uint8_t	                      _blockSizeId;  /*!< block max size code */
      XXH32_state_t               * _xxh32state;   /*!< XXH32 computation state */

      
 };

 #endif /* __ZFILELZ4_HH__ */


