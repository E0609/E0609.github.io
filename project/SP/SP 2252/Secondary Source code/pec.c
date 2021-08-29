
/*  
 * crc8.c
 *
 * Computes a 8-bit CRC
 *
 */

#include "Define.h"
//#include "I2C.h"

#define GP  0x107   /* x^8 + x^2 + x + 1 */

static unsigned char crc8_table[256];     /* 8-bit table */
static unsigned char made_table = 0;

void init_crc8 ( )
/*
 * Should be called before any other crc function.
 */
{
  int i, j;
  unsigned char crc;

  if ( ! made_table )
  {
      for ( i = 0; i < 256; i ++ )
      {
          crc = i;
          for ( j = 0; j < 8; j ++ )
          {
              crc = ( crc << 1 ) ^ ( ( crc & 0x80 ) ? GP : 0 );
          }
          crc8_table[i] = crc & 0xFF;
      }
      made_table = 1;
  }
}

static void crc8 ( BYTE *crc, BYTE m )
/*
 * For a byte array whose accumulated crc value is stored in *crc, computes
 * resultant crc obtained by appending m to the byte array
 */
{
  if ( ! made_table )
  {
      init_crc8 ( );
  }

  *crc = crc8_table[( *crc ) ^ m];
}

inline void CalcPEC ( BYTE *crc, BYTE data )
{
  crc8 ( crc, data );
}


