#include "blercounter.h"

BlerCounter::BlerCounter(int32_t window)
    : WindowSize((window <= 0) ? 300 : window)
{
    CRCLike.resize(WindowSize);
}

/**
 * @brief Add crc status in struct
 * @param[in] crc - crc status
 *
 * @return none
 */
void BlerCounter::AddCRC(uint8_t crc)
{
    CRCLike[index] = (crc == 1) ? 1 : 0;
    index          = (index + 1) % WindowSize;

    if (counter < WindowSize)
        counter++;
}

/**
 * @brief Add crc status in struct
 * @param[in] crc - crc status
 *
 * @return none
 */
void BlerCounter::AddCRC(bool crc)
{
    CRCLike[index] = crc ? 1 : 0;
    index          = (index + 1) % WindowSize;

    if (counter < WindowSize)
        counter++;
}

/**
 * @brief Get ratio crc
 * @param none
 *
 * @return Ratio CRC
 */
double BlerCounter::GetRatio() const
{
    if (!counter)
        return 0.0;
    double sum = 0;
    for (uint32_t i = 0; i < counter; i++)
        sum += double(CRCLike[i]);

    return 1.0 - sum / counter;
}

uint32_t BlerCounter::GetCounter() const
{
    return counter;
}

void BlerCounter::Clear()
{
    counter = 0;
    index   = 0;
}
