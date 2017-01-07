/*!
 * \file   mempool.hh
 * \author Adil So
 * \brief  Header file
 * \date   21 December 2016
 *
 * memory pool class declaration
 *
 * See licences/zcp.txt for details about copyright and rights to use.
 *
 * Copyright (C) 2011-2016
 *
 * */


#ifndef __MEMPOOL_HH__
#define __MEMPOOL_HH__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "zfile.hh"

using namespace std;

/*! 
 * \brief debug flag
 * */
#ifdef __DEBUG__
#  define __MEMPOOL_ADD_DEBUG__       0
#  define __MEMPOOL_GET_DEBUG__       0
#  define __MEMPOOL_ZIP_DEBUG__       0
#  define __MEMPOOL_UNZIP_DEBUG__     0
#endif


/*!
 * \brief memory pool class
 * */
class MemPool
{
    /* ------------------------------------------------------------------------- */
    /* --                              CONSTANTS                              -- */
    /* ------------------------------------------------------------------------- */    

    /* ------------------------------------------------------------------------- */
    /* --                       TYPEDEFS AND STRUCTURES                       -- */
    /* ------------------------------------------------------------------------- */
  public : 
    /*!
     * \brief run mode
     * */
    typedef enum {
      e_ZipPool,
      e_UnzipPool
    } PoolRunMode_t;

    
    /* ------------------------------------------------------------------------- */
    /* --                            CONSTRUCTORS                             -- */
    /* ------------------------------------------------------------------------- */
  public :
    /*!
     * \brief   class constructor
     *
     * \param id [in] : id passed to the function as argment to the zip or 
     *                  unzip function
     * \param blocsize [in] : bloc size
     * \param mode [in] : run mode : zip or unzip
     * \param zfile [in] : associated zfile object
     * \param puser [in] : user specific pointer passed as argument to the zip or 
     *                     unzip function
     * */
    MemPool (uint32_t id, size_t blocsize, PoolRunMode_t mode, zFile * zfile, void * puser) :
        _poolsize (blocsize)
        {
          // warning 
          __log__(1, ">>> Warning : MemPool DEBUG is enabled\n");
          //
          _in         = (char *) malloc (blocsize);
          _out        = _ptr = (char *) malloc (blocsize + sizeof(uint32_t));
          _insize     = 0;
          _outsize    = 0;
          _zreturn    = 0;
          _puser      = puser;
          _mode       = mode;
          _zfile      = zfile;
          _id         = id;
        };

    /*!
     * \brief   class destructor
     * */
    virtual ~MemPool ()
        {
          free(_in); free(_out);
        };
    /* ------------------------------------------------------------------------- */
    /* --                           GETTERS/SETTERS                           -- */
    /* ------------------------------------------------------------------------- */
    /*!< set user spcific data */
    inline void setUserptr(void * puser) { _puser = puser; };
    /* get in buffer size */
    inline size_t      getInsize()   { return _insize; };
    /* get out buffer size */
    inline size_t      getOutsize()  { return _outsize; };
    /* get in buffer size */
    inline char *      getInptr()    { return _in; };
    /* get out buffer size */
    inline char *      getOutptr()   { return _out; };
    /* get pool id */
    inline uint32_t    getId()       { return _id; };
    /* get zip/unzip return code */
    inline uint32_t    getReturn()   { return _zreturn; };
    /* check if input data pool is empty */
    inline bool        inEmpty()     { return (_insize <= 0); };
    /* check if output data pool is empty */
    inline bool        outEmpty()    { return (_outsize - (_ptr - _out) <= 0); };

    /* ------------------------------------------------------------------------- */
    /* --                               METHODS                               -- */
    /* ------------------------------------------------------------------------- */
    /*!
     * \brief  add data to the input memory pool
     *
     * \param data [in] : data pointer
     * \param size [in] : data size
     *
     * \return number of copied bytes
     * */
     inline size_t add (const char * data, size_t size)
         {
           __log__(__MEMPOOL_ADD_DEBUG__, "    [MEMPOOL][ADD ] : insert %ld : maxsize = %ld, used = %ld\n", size, _poolsize, _insize);
           size_t nsize = min( _poolsize - _insize, size);
           if (nsize == 0)
             return 0;
           else
           {
             memcpy(_in + _insize, data, nsize);
             _insize += nsize;
             return nsize;
           }
         };

    /*!
     * \brief  add data to the input memory pool from a file stream
     *
     * \param fs   [in] : file stream
     * \param size [in] : data size
     *
     * \return number of copied bytes
     * */
     inline size_t add (fstream& fs, size_t size)
         {
           __log__(__MEMPOOL_ADD_DEBUG__, "    [MEMPOOL][ADD ] : insert from stream %ld : maxsize = %ld, insize = %ld\n", size, _poolsize, _insize);
           size_t nsize = min( _poolsize - _insize, size);
           if (nsize == 0)
             return 0;
           else
           {
             size_t rsize = fs.read(_in + _insize, nsize).gcount();
             _insize += rsize;
             return rsize;
           }
         };         

    /*!
     * \brief  get data ptr from the out pool
     *
     * \param data [in] : data pointer
     * \param size [in] : data size
     *
     * \return number of copied bytes
     * */
     inline size_t get (char * ptr, size_t size)
         {
           __log__(__MEMPOOL_GET_DEBUG__, "    [MEMPOOL][GET ] : get %ld : maxsize = %ld, insize = %ld\n", size, _poolsize, _insize);
           size_t nsize = min( _outsize - (_ptr - _out), size);
           __log__(__MEMPOOL_GET_DEBUG__, "    [MEMPOOL][GET ] :         > size to copy %ld\n", nsize);
           if (nsize <= 0)
             return 0;
           else
           {
             memcpy(ptr, _ptr, nsize);
             _ptr += nsize;
             return nsize;
           }
         };
    /*!
     * \brief  reset memory pool
     * */
     inline void reset ()
         {
           _insize     = 0;
           _outsize    = 0;
           _ptr        = _out;
           _zreturn    = 0;
         };         

    /*!
     * \brief run memory pool job
     *
     * \return error code :
     *             0 = success
     *             1 = init thread attribut error
     *             2 = run thread error
     *             3 = empty pool
     * */
     inline uint16_t run ()
         {
           if (_insize == 0)
             return 3;
           if (pthread_attr_init(&_thattr) != 0)
             return 1;
           if (pthread_create(&_thid, &_thattr, &thread, this) != 0)
             return 2;
           return 0;
         };      

    /*!
     * \brief  wait memory pool job
     *
     * \return error code :
     *             0 = success
     *             1 = join thread error
     * */
     inline uint16_t wait ()
         {
           if (_thid == 0)
             return 0;
           if (pthread_join(_thid, NULL) != 0)
             return 1;
           _thid = 0;
           pthread_attr_destroy(&_thattr);
           return 0;
         }; 

  private :
    /*!
     * \brief  job thread
     *
     * \param pool [in] : Memory pool object
     *
     * \return no return
     * */
     static void * thread (void * data)
         {
           MemPool * pool = static_cast<MemPool*>(data);
           if (pool->_mode == e_ZipPool)           
             pool->_outsize = pool->_zfile->compress(pool->_id, pool->_puser, pool->_in, pool->_insize, pool->_out, pool->_poolsize, &pool->_zreturn);
           else
           {

             size_t remaining;
             __log__(__MEMPOOL_UNZIP_DEBUG__, "    [MEMPOOL][UZIP] : unzip data %d : insize = %ld, outsize = %ld\n", pool->_id, pool->_insize, pool->_outsize);
             pool->_outsize = pool->_zfile->uncompress(pool->_id, pool->_puser, pool->_in, pool->_insize, pool->_out, pool->_poolsize, &pool->_zreturn, &remaining);
             __log__(__MEMPOOL_UNZIP_DEBUG__, "    [MEMPOOL][UZIP] :     > unzipped data %d : outsize = %ld, remain = %ld\n", pool->_id, pool->_outsize, remaining);
             memmove(pool->_in, pool->_in + (pool->_insize - remaining), remaining);
             pool->_insize = remaining;
           }
           pool->_ptr = pool->_out;
           return NULL;
         }; 
         
    
  /* ------------------------------------------------------------------------- */
  /* --                               MEMBERS                               -- */
  /* ------------------------------------------------------------------------- */

    size_t            _poolsize;    /*!< in pool size */
    size_t            _insize;      /*!< current size of the in buffer */
    size_t            _outsize;     /*!< current size of the out buffer */
    char            * _in;          /*!< temporisation buffer */
    char            * _out;         /*!< compression buffer */     
    char            * _ptr;         /*!< current read data of the output pool */     
    uint32_t          _zreturn;    /*!< zip/unzip return code */
    PoolRunMode_t     _mode;        /*!< Pool mode */
    zFile           * _zfile;       /*!< zfile class */
    void            * _puser;       /*!< user specific pointer */
    pthread_t         _thid;        /*!< thread object */
    pthread_attr_t    _thattr;      /*!< thread attribut */
    uint32_t          _id;          /*!< thread number (for debug) */
};

#endif /* __MEMPOOL_HH__ */
