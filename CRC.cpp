#include <iostream>
#include "CRC.h"

using namespace std;
const uint64_t CRC::m_polynomial = 0x42F0E1EBA9EA3693ull;
uint64_t big_endian_table[256]={0};
uint64_t crc = 0x8000000000000000;
CRC::CRC()
{}

CRC::~CRC()
{}

void CRC::create_crc_table()
{
  //uint64_t* crctable = new uint64_t[256];
  /* iterate over all byte values 0 - 255 */
  for (int dividend = 1; dividend < 256; dividend++)
    {
      /* calculate the CRC-64 value for current byte */
      if ((crc & 0x8000000000000000) != 0)
	{
	  crc <<= 1;
	  crc ^= m_polynomial;
	}
      else
	{
	  crc <<= 1;
	}
      for(int j = 0; j < dividend; j++){
	big_endian_table[dividend+j] = crc ^ big_endian_table[j];
      }
    }
  /* store CRC value in lookup table */
  
}
uint64_t CRC::get_crc_code(uint8_t *stream, int length)
{
  uint64_t rem = 0;
  for(int i = 0;i< length;i++){
    rem = (rem <<8)^ big_endian_table[stream[i]^(rem>>(64-8))];
}
  return rem;
}
