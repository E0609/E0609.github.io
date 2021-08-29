
#include "Define.h"

//power function for direct format

WORD power ( BYTE x, BYTE n )
{
  if ( n == 0 )
  {
      return 1;
  }

  if ( n > 1 )
  {
      return ( WORD ) x * power ( x, n - 1 );
  }
  //when n = 1
  return x;
}


//Encode as direct format

WORD EncodeAsDirectFmt ( WORD x, WORD m, WORD b, BYTE r )
{
  WORD result;

  if ( r & 0x80 )
  {	//r is negative
      result = ( ( m * x ) + b ) / ( power ( 10, ( 0x100 - r ) ) );
  }
  else
  {	//r is positive
      result = ( ( m * x ) + b )*( power ( 10, r ) );
  }

  return result;
}

DWORD LinearFmt_YtoX ( WORD Y, BYTE Gain )
{
  //Gain is 2's exponent

  DWORD result;
  BYTE N;

  N = ( BYTE ) ( ( Y & 0xF800 ) >> 11 );

  if ( Y & 0x8000 )
  {
      //N < 0
      result = ( ( ( ( ( DWORD ) ( Y & 0x7FF ) ) << Gain ) >> ( 32 - N ) ) );
  }
  else
  {
      //N > 0
      result = ( ( ( ( DWORD ) ( Y & 0x7FF ) ) << N ) << Gain );
  }

  return result;

}

WORD LinearFmt_XtoY ( DWORD X, char N, BYTE Gain )
{

  //Gain is 2's exponent

  WORD result;

  if ( N >= 0 )
  {
      result = ( ( ( X >> N ) >> Gain ) & 0x7FF ) | ( N << 11 );
  }
  else
  {
      result = ( ( ( X << ( - 1 * ( N ) ) ) >> Gain ) & 0x7FF ) | ( 0x8000 | ( ( 16 + ( N ) ) << 11 ) );
  }

  return result;
}

WORD GetAvg ( WORD pBuf[], WORD newData, BYTE* index, BYTE bufSize, DWORD* sum, BYTE exp )
{
  *sum = * sum - pBuf[*index] + newData;

  pBuf[*index] = newData;
  *index = ( *index ) + 1;
  *index = ( ( *index ) >= bufSize ) ? 0 : ( *index );

  return ( WORD ) ( ( *sum ) >> exp );
}


