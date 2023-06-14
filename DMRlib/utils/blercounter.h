#ifndef BLERCOUNTER_H
#define BLERCOUNTER_H

#include <QVector>
#include <cstdint>

class BlerCounter {
public:
    explicit BlerCounter(int32_t window = 300);

    void AddCRC(uint8_t crc);
    void AddCRC(bool crc);

    double GetRatio() const;
    uint32_t GetCounter() const;
    void Clear();

private:
    uint32_t WindowSize { 0 };
    uint32_t index { 0 };
    uint32_t counter { 0 };
    QVector<uint8_t> CRCLike;
};

#endif // BLERCOUNTER_H
