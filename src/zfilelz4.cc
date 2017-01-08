
/*!
 * \file   zFilelz4.hh
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
#include <string.h>
#include <map>
#include "lz4/lz4.h"
#include "lz4/lz4hc.h"
#include "endian/endian.h"
#include "commun/define.h"
#include "mempool.hh"
#include "zfilelz4.hh"

/* ------------------------------------------------------------------------------------------- */
/* --                                        DEFINES                                        -- */ 
/* ------------------------------------------------------------------------------------------- */

/*!
 * \brief   LZ4 magic number
 * */
#define __LZ4F_MAGICNUMBER__               0x184D2204U

/*!
 * \brief    LZ4 End of stream
 * */ 
#define __LZ4S_EOS__                       0

/*!
 * \brief   LZ4 bloc sizes
 * */
#define __LZ4F_BLOC4_SIZE__                (64 * 1024)
#define __LZ4F_BLOC5_SIZE__                (256 * 1024)
#define __LZ4F_BLOC6_SIZE__                (1024 * 1024)
#define __LZ4F_BLOC7_SIZE__                (4 * 1024 * 1024)
#define __LZ4F_MAXBLOC_SIZE__              __LZ4F_BLOC7_SIZE__

/*!
 * \brief   LZ4 skippable magic numbers
 * */
#define __SKIPPABLE_MAGIC_NUMBER_0__       0x184D2A50      /*!< first available magic number for skippable frames */

/*!
 * \brief   Metadata magic numbers
 * */
#define __METADATA_MAGIC_NUMBER_0__        0xCAFEDECA      

/*! 
 * \brief debug flag
 * */
#ifdef __DEBUG__
#  define __ZFILELZ4_UNZIP_DEBUG__         0
#endif


/* ------------------------------------------------------------------------------------------- */
/* --                                   STATIC FUNCTIONS                                    -- */ 
/* ------------------------------------------------------------------------------------------- */

/*!
 * \brief compress methods 
 * */
static int compress_fast_extState(void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level);
static int compress_fast_continue(void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level);
static int compress_HC_extStateHC(void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level);
static int compress_HC_continue  (void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level);

/*!
 * \brief decompress methods 
 * */
static int decompress_fast         (void* ctx, const char* src, char* dst, int size, int maxsize);
static int decompress_fast_continue(void* ctx, const char* src, char* dst, int size, int maxsize);

/* ------------------------------------------------------------------------------------------- */
/* --                                    STATIC METHODS                                     -- */ 
/* ------------------------------------------------------------------------------------------- */

/*!
  * \brief check if the file is a lz4 file format
  *
  * \param filename [in] : file name
  * 
  * return a boolean 
  * */
bool zFilelz4::islz4 (const char * filename)
{
  zFilelz4 zfile(filename, e_ReadMode, 65536, 1, 1);
  if (zfile.fail())
    return false;
  else
    return true;
}

/* ------------------------------------------------------------------------------------------- */
/* --                                CONSTRUCTORS/DESTRUCTORS                               -- */ 
/* ------------------------------------------------------------------------------------------- */
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
zFilelz4::zFilelz4 (const char * filename, OpenMode_t mode, uint32_t blocsize, uint8_t level, uint16_t nthread) :
    zFile(filename, mode, 
          mode == e_WriteMode ? min(blocsize, (uint32_t)__LZ4F_MAXBLOC_SIZE__) : __LZ4F_MAXBLOC_SIZE__, 
          nthread),
    _blockSizeId(0),
    _xxh32state(NULL)
{
  // warning 
  __log__(1, ">>> Warning : zFilelz4 DEBUG is enabled\n");

  // initalize members
  _file.type            = e_lz4;

  _zip.level            = level;

  _lz4.compressfunc     = NULL;
  _lz4.context          = NULL;
  _lz4.ncontext         = 0;

  _unlz4.uncompressfunc = NULL;
  _unlz4.eos            = false;

  // init xxh32 state 
  _xxh32state = XXH32_createState(); 
  XXH32_reset(_xxh32state, __checksum_seed__);   

  // initialize class members
  // ------------------------
  if (_file.mode == e_WriteMode)
  {
    // init max bloc size
    if      (_blocsize <= __LZ4F_BLOC4_SIZE__)  _blockSizeId = 4; // max 64KB
    else if (_blocsize <= __LZ4F_BLOC5_SIZE__)  _blockSizeId = 5; // max 256KB
    else if (_blocsize <= __LZ4F_BLOC6_SIZE__)  _blockSizeId = 6; // max 1MB
    else                                        _blockSizeId = 7; // max 4MB

    // allocate compression contexts
    _lz4.ncontext = _zip.listpool.size(); 
    _lz4.context  = (void **) malloc (_lz4.ncontext * sizeof (void*));
    int ipool = 0;
    for (list<MemPool*>::iterator it = _zip.listpool.begin(); it != _zip.listpool.end(); it++, ipool++)
    {
      if (_zip.level < __min__HClevel__)
        _lz4.context[ipool] = (LZ4_stream_t*)   LZ4_createStream();
      else
        _lz4.context[ipool] = (LZ4_streamHC_t*) LZ4_createStreamHC();
      (*it)->setUserptr(_lz4.context[ipool]);
    }        

    // select compression function     
    if (_zip.level < __min__HClevel__)
    {
      if (_flag.blockIndependence == 1)
        _lz4.compressfunc     = compress_fast_extState;
      else
        _lz4.compressfunc     = compress_fast_continue;    
    }
    else
    {
      if (_flag.blockIndependence == 1)
        _lz4.compressfunc     = compress_HC_extStateHC;
      else
        _lz4.compressfunc     = compress_HC_continue;
    }
    // write header in case of write mode
    writeHeader();    
  }
  else
  {
    // read lz4 file header
    readHeader();

    // read lz4 file tail
    readTail();

    // select uncompress function
    if (_flag.blockIndependence == 1)
      _unlz4.uncompressfunc = decompress_fast;
    else
      _unlz4.uncompressfunc = decompress_fast_continue;
  }    
}

/*!
 * \brief   class destructor
 *
 * */
zFilelz4::~zFilelz4 ()
{
  // write trailer 
  if (_file.mode == e_WriteMode)
  {
    // flush last pools
    zFile::close();

    // write tail
    writeTail();

    // free lz4 context 
    for (int i = 0; i < _lz4.ncontext; i++)
    {
      if (_zip.level < __min__HClevel__)
        LZ4_freeStream  ( (LZ4_stream_t   *) _lz4.context[i] );
      else
        LZ4_freeStreamHC( (LZ4_streamHC_t *) _lz4.context[i] );
    }  
    free(_lz4.context);
    
  }
  else
  {
    // @todo : check file crc 

  }

  // free xxh32 state
  XXH32_freeState(_xxh32state);  
}

/* ------------------------------------------------------------------------------------------- */
/* --                                    PUBLIC METHODS                                     -- */ 
/* ------------------------------------------------------------------------------------------- */
/*!
 * \brief   flush data (force physical data write)
 * 
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
void zFilelz4::flush()
{
  // get next pool 
  MemPool * pool = _zip.listused.front();

  // compute CRC : -- @todo error management 
  XXH32_update(_xxh32state, pool->getInptr(), pool->getInsize());  

  // wait pool end 
  if (pool->wait() != 0)
  {
    setError(e_JobWaitError, "Error joining write thread");
  }

  // flush pool
  return zFile::flush();
} 

/* ------------------------------------------------------------------------------------------- */
/* --                                   PRIVATE METHODS                                     -- */ 
/* ------------------------------------------------------------------------------------------- */

/*!
 * \brief   read lz4 file header
 *
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
void zFilelz4::readHeader ()
{
  FileHeader_t buffer = {0};

  _file.stream.read((char*)&buffer, offsetof(FileHeader_t, unused));
  if (_file.stream.fail())
  {
    setError(e_ReadError, "Error reading lz4 file header");
    return;
  }

  // compute header CRC
  uint8_t crc   = (XXH32((const char*)(&buffer.flag), offsetof(FileHeader_t, crc) - offsetof(FileHeader_t, flag), __checksum_seed__) >> 8) & 0xff;  
  // check magic number
  if (le32toh(buffer.magic) != __LZ4F_MAGICNUMBER__)
    setError(e_HeaderError, "Invalid magic number");
  // check flag
  else if (buffer.flag != *(uint8_t*)&_flag)
    setError(e_HeaderError, "Unhandled lz4 file format");
  // check crc
  else if (buffer.crc != crc)
    setError(e_HeaderError, "CRC header error");
  else
  {
    // get bloc size id
    _blockSizeId = ((buffer.blk >> 4) & 7);
  }
}

/*!
 * \brief   read lz4 file tail
 *
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
void zFilelz4::readTail ()
{
  /* ---------------------------------------------- */
  /* --               READ FILE TAIL             -- */
  /* ---------------------------------------------- */
  MetaHeader_t           header;
  MetaIdentification_t * ident; 

  // read tail second header
  _file.stream.seekg(-sizeof(MetaHeader_t), ios_base::end);
  size_t nsize = _file.stream.read((char*)&header, sizeof(MetaHeader_t)).gcount();
  if (_file.stream.fail() || (nsize != sizeof(MetaHeader_t)))
  {
    setError(e_ReadError, "Error reading lz4 tail second header");
    return;
  }
  // check magic number
  header.magic = le32toh(header.magic);
  header.size  = le32toh(header.size);
  if (header.magic == __SKIPPABLE_MAGIC_NUMBER_0__)
  {
    // read Metadata
    char   ptr[header.size];
    _file.stream.seekg(-(size_t)header.size, ios_base::end);
    size_t lastbloc = _file.stream.tellg();
    nsize = _file.stream.read(ptr, header.size).gcount();
    if (_file.stream.fail() || (nsize != header.size))
    {
      setError(e_ReadError, "Error reading lz4 meta data");
      return;
    }
    ident = (MetaIdentification_t *)ptr;
    // check magic number
    ident->magic   = le32toh(ident->magic);
    ident->version = le32toh(ident->version);
    ident->type    = le32toh(ident->type);
    ident->size    = le32toh(ident->size);
    if ( (ident->magic == __METADATA_MAGIC_NUMBER_0__) && (ident->version == 1) && (ident->type == e_MetaIndex) )
    {    
      size_t         indexsize = __sizeof__(IndexEntry_t, offset);
      char         * pptr      = (ptr + sizeof(MetaIdentification_t));
      for (size_t i = 0; i < (ident->size / indexsize) - 1; i++)
      {
        IndexEntry_t * index    = (IndexEntry_t *)(pptr);
        IndexEntry_t * nxtindex = (IndexEntry_t *)(pptr + indexsize);
        _listindex.insert( IndexEntry_t( index->offset.n, nxtindex->offset.n - index->offset.n,
                                         index->offset.z, nxtindex->offset.z - index->offset.z) );
        pptr += indexsize;
      }
      // insert last bloc 
      IndexEntry_t * index    = (IndexEntry_t *)(pptr);
      _listindex.insert( IndexEntry_t( index->offset.n, lastbloc - index->offset.z,
                                       index->offset.z, lastbloc - index->offset.z) );
      

    }
  }  

  // reset file position
  _file.stream.seekp(offsetof(FileHeader_t, unused), ios_base::beg);   
} 


/*!
 * \brief   write lz4 file header
 *
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
void zFilelz4::writeHeader ()
{ 
  FileHeader_t buffer = {0};

  // encode magic number
  buffer.magic = htole32(__LZ4F_MAGICNUMBER__);
  // encode flag
  buffer.flag  = *(uint8_t*)&_flag;
  // encode content size
  buffer.blk   = ((_blockSizeId & 7) << 4);
  // -- @todo : write content size
  // encode CRC
  buffer.crc   = (XXH32((const char*)(&buffer.flag), offsetof(FileHeader_t, crc) - offsetof(FileHeader_t, flag), __checksum_seed__) >> 8) & 0xff;

  // write header
  _file.stream.write((const char*)(&buffer), offsetof(FileHeader_t, unused));
  if (_file.stream.fail())
    setError(e_WriteError, "Error writing lz4 header");

  // update zoffset
  _zip.nzoffset += offsetof(FileHeader_t, unused);
} 

/*!
 * \brief   write lz4 file tail
 *
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
void zFilelz4::writeTail ()
{
  struct {
    uint32_t end;
    uint32_t crc; 
  } buffer = {0};  

  /* ---------------------------------------------- */
  /* --         WRITE COMPRESSED BLOC END        -- */
  /* ---------------------------------------------- */
  // encode end mark
  buffer.end = htole32(__LZ4S_EOS__);
  // encode crc
  if (_flag.streamChecksum)
  {
    // -- compute CRC and free xxh32 state
    buffer.crc = htole32(XXH32_digest(_xxh32state));
  }
  // write blocs tail
  _file.stream.write((const char*)(&buffer), sizeof(buffer));  
  if (_file.stream.fail())
    setError(e_WriteError, "Error writing lz4 tail");

  /* ---------------------------------------------- */
  /* --              WRITE META-DATA             -- */
  /* ---------------------------------------------- */
  MetaHeader_t         header;
  MetaIdentification_t ident;
  size_t               indexsize = __sizeof__(IndexEntry_t, offset);


  // tail header
  header.magic  = htole32(__SKIPPABLE_MAGIC_NUMBER_0__);
  header.size   = htole32( sizeof(MetaIdentification_t) + _listindex.size() * indexsize + sizeof(MetaHeader_t));
  // meta data identification
  ident.magic   = htole32(__METADATA_MAGIC_NUMBER_0__);
  ident.version = htole32(1);
  ident.type    = htole32(e_MetaIndex);
  ident.size    = htole32(_listindex.size() * indexsize);
  // write tail header
  _file.stream.write((const char *)(&header), sizeof(header));
  if (_file.stream.fail())
    setError(e_WriteError, "Error writing lz4 tail header");
  // write metadata ident
  _file.stream.write((const char *)(&ident), sizeof(ident));
  if (_file.stream.fail())
    setError(e_WriteError, "Error writing lz4 tail metadata identification");
  //write of index
  for (set<IndexEntry_t>::iterator it = _listindex.begin();
       it != _listindex.end(); it++)
  {
    _file.stream.write((const char *)&(*it).offset, indexsize);
  }
  // re-write header : to optimize reverse read
  _file.stream.write((const char *)(&header), sizeof(header));
  if (_file.stream.fail())
    setError(e_WriteError, "Error writing lz4 tail second header");
  
}

/*!
 * \brief   compress function
 * 
 * \param id [in] : id
 * \param puser [in] : user specific pointer
 * \param in [in] : input data 
 * \param insize [in] : input size
 * \param out [in] : output buffer
 * \param outsize [in] : output buffe size
 * \param preturn [out] : compression return : 0 -> unzipped, 0 -> zipped
 *
 * \return return number of bytes written
 *         use fail to check for errors
 *         To get error details use getError()
 * */
size_t zFilelz4::compress(uint32_t id, void * puser, const char * in, size_t insize, char * out, size_t outsize, uint32_t * preturn)
{
  // compress data
  size_t size = _lz4.compressfunc(puser, in, out + sizeof(uint32_t), insize, 
                              outsize - sizeof(uint32_t), _zip.level);
  // -- none comppresible buffer
  if (size <= 0)
  {
    // add size in buffer
    (*(uint32_t *)out) = (htole32(insize) & 0x7fffffff) | (1 << 31);
    // copy input buffer
    memcpy(out + sizeof(uint32_t), in, insize);
    size           = insize + sizeof(uint32_t);
    (*preturn)     = 0;
  }
  // compression succeeded
  else
  {
    // add size in buffer
    (*(uint32_t *)out) = htole32(size);
    size              += sizeof(uint32_t);
    (*preturn)         = 1;
  }
  return size;
}      

/*!
 * \brief   uncompress function
 * 
 * \param id      [in] : id
 * \param puser   [in] : user specific pointer
 * \param in      [in] : input data 
 * \param insize  [in] : input size
 * \param out     [in] : output buffer
 * \param outsize [in] : output buffe size
 * \param psize   [out] : remaining size in the input buffer
 *                            1 -> short input buffer
 *                            2 -> short output buffer
 *                            3 -> end of input stream
 * \param preturn [out] : unzip operation return code : 
 *
 * \return return number of bytes written
 *         use fail to check for errors
 *         To get error details use getError()
 * */
size_t zFilelz4::uncompress(uint32_t id, void * puser, const char * in, size_t insize, char * out, size_t outsize, uint32_t * preturn, size_t * psize)
{
  int32_t      zblocsize, blocsize;
  bool         uncompressed;
  const char * inptr     = in;
  char       * outptr    = out;
  size_t       inptrsz   = 0;
  size_t       outptrsz  = 0;

  // init 
  (*preturn) = 0;
  __log__(__ZFILELZ4_UNZIP_DEBUG__, "    [LZ4][UNZIP] : uncompress data : id = %d; insize %ld, outsize = %ld\n", id, insize, outsize);

  while ((inptrsz + sizeof(uint32_t)) < insize) 
  {
    /* read bloc size */
    memcpy(&zblocsize, inptr, sizeof(uint32_t));
    zblocsize     = le32toh(zblocsize);
    uncompressed  = zblocsize >> 31;
    zblocsize     = zblocsize & 0x7fffffff;  
    if ( inptrsz + zblocsize + sizeof(uint32_t) > insize )
    {
      (*preturn) = 1;
      break;    
    }
    __log__(__ZFILELZ4_UNZIP_DEBUG__, "    [LZ4][UNZIP] :     unzip bloc : zblocsize = %d (%s), remain out = %ld\n", 
                                      zblocsize, (uncompressed) ? "unzipped" : "zipped", outsize - outptrsz);

    /* end of stream */
    if (zblocsize == __LZ4S_EOS__)
    {
      // // read stream crc
      // uint32_t crc; 
      // nsize =_file.stream.read((char *)&crc, sizeof(crc)).gcount();
      // if (_file.stream.fail()  || (nsize != sizeof(crc)))
      // {
      //   setError(e_ReadError, "Error reading file chekcsum");
      //   break;
      // }
      // if (XXH32_digest(_xxh32state) != le32toh(crc))
      // {
      //   setError(e_TailError, "Error in file chekcsum");
      //   break;
      // }
      // _unlz4.eos = true;
      // break;
      (*preturn) = 3;
      inptrsz    = insize;
      break;    
    }
    // uncompress data
    if (uncompressed)
    {
      if (  outptrsz + zblocsize > outsize )
      {
        (*preturn) = 2;
        break;    
      }
      memcpy(outptr, inptr + sizeof(uint32_t), zblocsize);
      blocsize  = zblocsize;
      outptr   += zblocsize;
      outptrsz += zblocsize;
    }
    else
    {
      blocsize = _unlz4.uncompressfunc(NULL, inptr + sizeof(uint32_t), outptr, zblocsize, outsize - outptrsz) ;
      if (blocsize <= 0)
      {
        (*preturn) = 2;
        break;    
      }
      outptr   += blocsize;
      outptrsz += blocsize;
    }  
    inptr     += zblocsize + sizeof(uint32_t) /* zblocsize */;   
    inptrsz   += zblocsize + sizeof(uint32_t);
    __log__(__ZFILELZ4_UNZIP_DEBUG__, "    [LZ4][UNZIP] :         > blocsize = %d\n", blocsize);
  }

  // remaining data
  (*psize) = insize - inptrsz;

  __log__(__ZFILELZ4_UNZIP_DEBUG__, "    [LZ4][UNZIP] : end of unzip operation : size %ld/%ld, remaining %ld, return = %d\n", 
                                    outptrsz, outsize, (*psize), (*preturn));
  
  return outptrsz;    
  //XXH32_update(_xxh32state, _unlz4.unzdata, _unlz4.unzblocsize);  
}









/* ------------------------------------------------------------------------------------------- */
/* --                                      LZ4 SPECIFIC                                     -- */ 
/* ------------------------------------------------------------------------------------------- */

/*!
 * \brief compress methods 
 * */
static int compress_fast_extState(void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level)
{
  return LZ4_compress_fast_extState (ctx, src, dst, srcSize, dstSize, level);
}
static int compress_fast_continue(void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level)
{
  return LZ4_compress_fast_continue ( (LZ4_stream_t*) ctx, src, dst, srcSize, dstSize, level);
}
static int compress_HC_extStateHC(void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level)
{
  return LZ4_compress_HC_extStateHC (ctx, src, dst, srcSize, dstSize, level);
}
static int compress_HC_continue  (void* ctx, const char* src, char* dst, int srcSize, int dstSize, int level)
{
  return LZ4_compress_HC_continue   ( (LZ4_streamHC_t*) ctx, src, dst, srcSize, dstSize);  
}
    
/*!
 * \brief decompress methods 
 * */
static int decompress_fast(void* ctx, const char* src, char* dst, int size, int maxsize)
{
  return LZ4_decompress_safe (src, dst, size, maxsize);
}
static int decompress_fast_continue(void* ctx, const char* src, char* dst, int size, int maxsize)
{
  return LZ4_decompress_fast_continue ( (LZ4_streamDecode_t*) ctx, src, dst, size);
}


