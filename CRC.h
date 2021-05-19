#include <stdint.h>

class CRC
{
 public:
  CRC();
  ~CRC();
  uint64_t get_crc_code(uint8_t *stream, int length);

 private:
  void create_crc_table();

  uint64_t m_CRC_table[256];
  static const uint64_t m_polynomial;
};
