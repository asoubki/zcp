/*!
 * \file   define.hh
 * \author Adil So
 * \brief  zcp defines include files
 * \date   12 Janvier 2017
 *
 * general defines/macros used in thr projects 
 *
 * See licences/zcp.txt for details about copyright and rights to use.
 *
 * Copyright (C) 2011-2016
 *
 * */
#ifndef __DEFINE_H__
#define __DEFINE_H__

/*!
 * \brief environment variable/define
 * */
#if _WIN32 || _WIN64
   #if _WIN64
     #define __ENV_64BIT__
  #else
    #define __ENV_32BIT__
  #endif
#elif __GNUC__
  #if __x86_64__ || __ppc64__
    #define __ENV_64BIT__
  #else
    #define __ENV_32BIT__
  #endif
#endif

/*!
 * \brief structure member size
 * */
#define __sizeof__(type, member)       sizeof(((type *)0)->member)


#endif