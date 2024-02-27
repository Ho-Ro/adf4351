
#include <cstdint>

class ADF435X {
  public:
    ADF435X( uint32_t ref = 25000000 );
    void setFreq( uint64_t freq_Hz );
    uint32_t getReg( int index );

  private:
    uint16_t INT;
    uint16_t FRAC;
    uint16_t MOD;
    uint8_t DIV;
    uint32_t refIn;
    uint32_t REG[ 6 ];
};
