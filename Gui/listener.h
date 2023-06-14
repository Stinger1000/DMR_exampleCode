#ifndef LISTENER_H
#define LISTENER_H

#include <IDMRlib.h>
#include <QObject>

/**
 * @brief The Listener class - a descendant class for communication with the library
 */
class Listener : public QObject, public IDMRlib::IListener {
    Q_OBJECT
public:
    Listener(QObject* parent);
    ~Listener();

    void Test(const std::vector<usb_device_t>& devices);

signals:
    void GetDevices(const std::vector<usb_device_t>& devices);
    void UpdateCount(const trans_count_model_t& model);
    void SpeedAndLostPacket(const std::pair<float, uint64_t> pack);
    void UpdateSpectrum(const std::vector<int32_t>& points);
    void UpdateScanSpectrum(const std::vector<int32_t>& points);
    void UpdateScanSpectrumGains(const std::vector<int32_t>& points);
    void StatusFlash(const int status, const int percent);
    void CounterDump(const uint64_t count);
    void GetStatusBoard(const status_model_t& status);
    void GetDetune(const std::vector<clk_detune_model_t>& detune);
    void GetError(const Error error);
};

#endif // LISTENER_H
