#include <iostream>
#include "CRC.h"

using namespace std;
const uint64_t CRC::m_polynomial = 0x42F0E1EBA9EA3693ull;
uint64_t* crctable;
CRC::CRC()
{
  this->create_crc_table();
}

CRC::~CRC()
{}

void CRC::create_crc_table()
{
  crctable = new uint64_t[256];
  /* iterate over all byte values 0 - 255 */
  for (int dividend = 0; dividend < 256; dividend++)
    {
      /* calculate the CRC-64 value for current byte */
      uint64_t crc = (uint64_t)(((uint64_t)dividend)<<56);
      for (uint64_t bit = 0; bit < 8; bit++){
	if ((crc & 0x8000000000000000) != 0)
	  {
	    crc <<= 1;
	    crc ^= m_polynomial;
	    //cout << (int)m_polynomial <<endl;;
	    //cout << crc << endl;;
	  }
	else
	  {
	    crc <<= 1;
	  }
      }
      crctable[dividend] = crc;
    }
  /* store CRC value in lookup table */
  
}
uint64_t CRC::get_crc_code(uint8_t *stream, int length)
{
  uint64_t rem = 0;
  for(int i = 0;i< length;i++){
    rem = (rem <<8)^ crctable[stream[i]^(rem>>(64-8))];
}
  return rem;
}
