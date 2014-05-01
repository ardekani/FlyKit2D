/*!
\file   fkTypes.h
\brief  Data type definitions used through out FlyKit Code
\author Reza Ardekani
\date   09/12/2013
*/
#ifndef _FKTYPES_H_
#define _FKTYPES_H_

typedef unsigned char           FkInt8U;   /**< typedef for unsigned char. */
typedef signed char             FkInt8S;   /**< typedef for signed char. */
typedef unsigned short          FkInt16U;  /**< typedef for unsigned short. */
typedef signed short            FkInt16S;  /**< typedef for signed short. */
typedef unsigned long           FkInt32U;  /**< typedef for unsigned long. */
typedef signed long             FkInt32S;  /**< typedef for signed long. */
typedef signed long long        FkInt64S;  /**< typedef for signed 64-bit int -- OK for Win32 and Linux32. */
typedef unsigned long long      FkInt64U;  /**< typedef for unsigned 64-bit int -- OK for Win32 and Linux32. */
typedef float                   FkReal32F; /**< typedef for single precision floating point. */
typedef double                  FkReal64F; /**< typedef for double precision floating point. */
typedef void                    FkVoid;    /**< typedef for void. */
typedef FkInt64S                FkTime64S; /**< signed time */
typedef FkInt64U                FkTime64U; /**< unsigned time */

#define FK_MAX(a,b) ((a>b)? a : b)
#define FK_MIN(a,b) ((a>b)? b : a)

#endif  //#ifndef _VRTYPES_H_

