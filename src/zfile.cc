
/*!
 * \file   zfile.cc
 * \author Adil So
 * \brief  implementation file
 *
 * compressor interface class implementation
 *
 * See licences/zcp.txt for details about copyright and rights to use.
 *
 * Copyright (C) 2011-2016
 *
 * */
#include "zfile.hh"
#include "zfilelz4.hh"
#include "mempool.hh"

/*! 
 * \brief debug flag
 * */
#ifdef __DEBUG__
#  define __ZFILE_READ_DEBUG__     0
#  define __ZFILE_WRITE_DEBUG__    0
#endif


/* ------------------------------------------------------------------------------------------- */
/* --                                    STATIC FUNCTIONS                                   -- */ 
/* ------------------------------------------------------------------------------------------- */
/*!
 * \brief   get file format
 * 
 * \param filename [in] : file name
 *
 * \return file format type (CompressionType_t)
 * */
zFile::CompressionType_t zFile::getFormat(const char * filename)
{
  if (zFilelz4::islz4(filename))
    return e_lz4;
  else
    return e_none;
}

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
zFile * zFile::create(const char * filename, OpenMode_t         mode,
                                             CompressionType_t  type, 
                                             uint16_t           level,
                                             uint32_t           blocsize,
                                             uint16_t           nthread)
{
  zFile * zf;

  // warning 
  __log__(1, ">>> Warning : zFile DEBUG is enabled\n");

  // create the appropriate compressor
  switch(type)
  {
    case e_lz4  : zf = (zFile *) new zFilelz4(filename, mode, blocsize, level, nthread); break;
    case e_none : zf = (zFile *) new zFile   (filename, mode, blocsize, nthread);        break;
    default     : zf = NULL; break;
  }

  return zf;
}

/* ------------------------------------------------------------------------------------------- */
/* --                                      CONSTRUCTORS                                     -- */ 
/* ------------------------------------------------------------------------------------------- */
/*!
 * \brief class constructor
 *
 * \param filename [in] : file name
 * \param mode     [in] : openning mode : read/write
 * \param blocsize [in] : compression bloc size 
 * \param nthread  [in] : number of threads (default : \b __bloc_size__ \b)
 *
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
zFile::zFile (const char * filename, OpenMode_t mode, uint32_t blocsize, uint16_t nthread) :
    _lasterror(e_SuccessError),
    _errormsg(),
    _blocsize(blocsize),
    _listindex()
{
  // initialize memebers
  _file.filename     = filename;
  _file.mode         = mode;
 
  _zip.level         = __compression_level__;
  _zip.noffset       = 0;
  _zip.nzoffset      = 0;
  _zip.curpool       = NULL; 

  _unzip.readpool    = NULL;

  // open file   
  switch(_file.mode)
  {
    case e_ReadMode  : 
    {
      // allocate uncompress memeory pool
      _unzip.readpool = new MemPool(0, _blocsize, MemPool::e_UnzipPool, this, NULL);

      // open file
      _file.stream.open(_file.filename.c_str(), fstream::in  | fstream::binary); 
      break;
    }
    case e_WriteMode : 
    {
      // allocate compression memory pools
      for (int i = 0; i < nthread; i++)
      {
        MemPool * pool = new MemPool(i, _blocsize, MemPool::e_ZipPool, this, NULL);
        if (i == 0)
          _zip.curpool = pool;
        else
          _zip.listfree.push_back(pool);
        _zip.listpool.push_back(pool);
      }
      // open file
      _file.stream.open(_file.filename.c_str(), fstream::out | fstream::binary); 
      break;
    }
    default          : setError(e_OpenError, "Unknown open mode"); break;
  }
  // check for error
  if (_file.stream.fail())
    setError(e_OpenError, "File open error");
  // set postion to beginning
  else
    _file.stream.seekg (0, fstream::beg);
}                      


/*!
 * \brief   class destructor
 * */
zFile::~zFile ()
{
  // close file
  close();

  // free memory pools
  if (_file.mode == e_WriteMode)
  {
    for (list<MemPool*>::iterator it = _zip.listpool.begin(); 
        it != _zip.listpool.end();  it++)
    {
      delete (*it);
    }            
  }
  else
    delete _unzip.readpool;

  // clear index list
  _listindex.clear();

  // close file
  _file.stream.close();
}

/* ------------------------------------------------------------------------------------------- */
/* --                                     PUBLIC METHODS                                    -- */ 
/* ------------------------------------------------------------------------------------------- */

/*!
 * \brief flush last remaining pools before close
 *     
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
void zFile::close()
{
  if (_file.mode == e_WriteMode)
  {
    // write current pool
    if (_zip.curpool->run() == 0)
      nextPool();

    // flush reaming pools 
    while ( _zip.listused.size() != 0 )
      flush();
  }
}


/*!
 * \brief   read data method
 * 
 * \param ptr [in] : input data 
 * \param size [in] : data size to read
 *
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
size_t zFile::read(char * ptr, size_t size)
{
  if (_file.mode == e_WriteMode)
  {
    setError(e_ReadError, "Error reading a file openned in write mode");
    return 0;
  }

  size_t readsize = size;
  char * readptr  = ptr;
  __log__(__ZFILE_READ_DEBUG__, "[READ ] : read order : size = %ld\n", size);

  while (readsize > 0)
  {
    size_t nsize = _unzip.readpool->get(readptr, readsize);
    if (nsize != 0)
    {
      __log__(__ZFILE_READ_DEBUG__, "[READ ] :     > copy data : insize = %ld\n", nsize);
      readptr       += nsize;
      readsize      -= nsize;
    }
    else
    {
      /* read more data : last unzip operation finished with a short input buffer size */
      if ((_unzip.readpool->getReturn() != 2))
      {
        nsize = _unzip.readpool->add (_file.stream, _blocsize);
        __log__(__ZFILE_READ_DEBUG__, "[READ ] :     > read data : insize = %ld (%d)\n", nsize, _blocsize);
        if (_file.stream.fail() && (nsize == 0))
        {
          setError(e_ReadError, "Error reading data");
          break;
        }
      }
      if (_unzip.readpool->run() != 0)
      {
        setError(e_ReadError, "Error running read thread");
        break;        
      }
      if (_unzip.readpool->wait() != 0)
      {
        setError(e_ReadError, "Error joining read thread");
        break;        
      }      
    }
  }

  __log__(__ZFILE_READ_DEBUG__, "[READ ] : end of read operation : size = %ld\n", (size - readsize));
  return (size - readsize);
} 


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
size_t zFile::mwrite(const char * ptr, size_t size)
{
  size_t            treatsize = size;
  const char      * ptreatptr = ptr;
  
  // check open mode
  if (_file.mode != e_WriteMode)
  {
    setError(e_WriteError, "Error writing in a read mode file");
    return 0;
  }
  __log__(__ZFILE_WRITE_DEBUG__, "[WRITE] : write order : size = %ld\n", size);
  
  while (treatsize > 0)
  {
    // copy data
    size_t nsize = _zip.curpool->add(ptreatptr, treatsize);
    if (nsize == 0)
    {
      __log__(__ZFILE_WRITE_DEBUG__, "[WRITE] :     > run pool data : id = %d\n", _zip.curpool->getId());
      // compress data
      uint16_t error = _zip.curpool->run();
      if ( error != 0)
      {
        switch (error) 
        {
          case 1  : setError(e_JobInitError,    "Error initializing write thread"); break;
          case 2  : setError(e_JobCreateError,  "Error running write thread");      break;  
          default : setError(e_JobUnknownError, "Unhandled write thread error");    break;  
        }    
        break;
      }
      nextPool();
    }
    else
    {
      __log__(__ZFILE_WRITE_DEBUG__, "[WRITE] :     > copy data : insize = %ld\n", nsize);
      ptreatptr         += nsize;      
      treatsize         -= nsize;
    }
  }

  return (size - treatsize);
}  


/*!
 * \brief   flush data (force physical data write)
 * 
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
void zFile::flush()
{
  // get next pool 
  MemPool * pool = _zip.listused.front();
  _zip.listused.pop_front();
  // wait job end
  __log__(__ZFILE_WRITE_DEBUG__, "[FLUSH] : wait pool : id = %d ( listused = %ld )\n", pool->getId(), _zip.listused.size());  
  if (pool->wait() != 0)
  {
    setError(e_JobWaitError, "Error joining write thread");
  }  
  // write pool
  __log__(__ZFILE_WRITE_DEBUG__, "[FLUSH] : flush data %ld\n", pool->getOutsize());  
  _file.stream.write(pool->getOutptr(), pool->getOutsize());
  // update Index table
  _listindex.push_back( IndexEntry_t( _zip.noffset, _zip.nzoffset) );
  _zip.nzoffset  += pool->getOutsize() & 0x7fffffff;
  _zip.noffset   += pool->getInsize();

  // reset pool
  pool->reset();
  // add free pool
  _zip.listfree.push_back(pool);
}     

/*!
 * \brief seek position (through compression)
 *
 * \param offset [in] : seek offset (can be negatif) in the uncompressed file
 * \param way    [in] : seek way
 *
 * \return returned a boolean telling if an error accured during the last 
 *         operation 
 */
bool zFile::seekf(size_t offset, SeekDirection_t way)
{
  if (_file.mode == e_ReadMode)
  {
    switch (way) 
    {
      case e_SeekBegin   : _file.stream.seekg(offset, ios_base::beg); break;
      case e_SeekCurrent : _file.stream.seekg(offset, ios_base::cur); break;
      case e_SeekEnd     : _file.stream.seekg(offset, ios_base::end); break;
    }
    if (_file.stream.fail())
      return false;
    else
      return true;
  }
  else
  {
    setError(e_SeekError, "Seek unhandled for write mode files");
    return false;
  }
}

/*!
 * \brief check end of file 
 *
 * \return returned a boolean telling if an error accured during the last 
 *         operation 
 */
bool zFile::eof() 
{ 
  if (_file.stream.eof())
  {
    if (_unzip.readpool->inEmpty())
      if (_unzip.readpool->outEmpty())
        return true;
      else
        return false;
    else
      return false;
  } 
  return false;
};


/* ------------------------------------------------------------------------------------------- */
/* --                                    PRIVATE METHODS                                    -- */ 
/* ------------------------------------------------------------------------------------------- */

/*! 
 * \brief get next free pool
 *
 * if no memory pool is available, it will flush data and wait for next pool 
 *
 * \return  no returned value. use fail to check for errors
 *          To get error details use getError()
 * */
void zFile::nextPool()
{
  // add current pool to the used pool list
  _zip.listused.push_back(_zip.curpool);

  // flush finished data 
  if (_zip.listfree.size() == 0)
    flush();

  _zip.curpool = _zip.listfree.front();
  _zip.listfree.pop_front(); 
}

/*!
 * \brief   compress function
 * 
 * \param id      [in] : thread id
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
size_t zFile::compress(uint32_t id, void * puser, const char * in, size_t insize, char * out, size_t outsize, uint32_t * preturn)
{
  memcpy(out, in, insize);
  (*preturn) = 0;
  return insize;
}      

/*!
 * \brief   compress function
 * 
 * \param id      [in] : thread id
 * \param puser   [in] : user specific pointer
 * \param in      [in] : input data 
 * \param insize  [in] : input size
 * \param out     [in] : output buffer
 * \param outsize [in] : output buffe size
 * \param psize   [out] : remaining size in the input buffer
 *                            0 -> success
 *                            1 -> short input buffer
 *                            2 -> short output buffer
 *                            3 -> end of input stream
 * \param preturn [out] : unzip operation return code : 
 *
 * \return return number of bytes written
 *         use fail to check for errors
 *         To get error details use getError()
 * */
size_t zFile::uncompress(uint32_t id, void * puser, const char * in, size_t insize, char * out, size_t outsize, uint32_t * preturn, size_t * psize)
{
  memcpy(out, in, insize);
  (*psize)   = 0;
  (*preturn) = 0;
  return insize;
} 

