#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "controllers/ClockDetuneController.h"
#include "controllers/DumpDebugController.h"
#include "controllers/FlashFirmwareController.h"
#include "controllers/SpectrumController.h"
#include "controllers/SpectrumScanController.h"
#include "controllers/StatusController.h"
#include "models/HostModel.h"
#include "utils/RawPacker.h"
#include "utils/USBPacketCollector.h"
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <future>
#include <memory>

class MainController : public QObject {
    Q_OBJECT
public:
    MainController(std::unique_ptr<IDevice> dev, const support_model_t& model,
        IDMRlib::IListener* listener, QObject* parent = nullptr);
    ~MainController();

    void reqStatus();
    void reqPrjStatus();
    void reqAdDxco();

    void processDMR(RawPacker&& pkt) noexcept;
    void processCommon(RawPacker&& pkt) noexcept;
    void processArm(RawPacker&& pkt) noexcept;

    const std::shared_ptr<status_model_t>& GetStatusModel() const;
    const QVector<clk_detune_model_t>*     GetDetuneModel();

    void processPrjStatus(RawPacker&& pkt);
    void UpdateAdFreqConf(const ad_settings_model_t& settings);
    void changeNetConf(const host_model_t& host);
    void MainParser(const QByteArray& data);

public slots:
    void OnNewSpectrumSource(int source, int channel);
    void OnDisconnectHost();
    void SetHost(const host_model_t& host_model);
    void OnDumpOffTimer(const DumpSetupModel& model);
    void SetSupport(const bool enable, const uint32_t freq);

    void GetStatusBoard();
    void GetDetune();
    void SendError(const uint8_t error);
    void SetSrc(const src_model_t& model);

signals:
    void ReadyRead();

    void UpdateCount();
    void UpdateCountForStatus(const trans_count_model_t&);
    void UpdateSpectrum(const std::vector<int32_t>& points);
    void UpdateScanSpectrum(const std::vector<int32_t>& points);
    void UpdateScanSpectrumGains(const std::vector<int32_t>& gains);
    void EnableCorAndSpec(const bool en);
    void LoadConfigAd();
    void EnableDump(const DumpSetupModel& model);
    void CounterDump(const uint64_t count);
    void CounterTest(const bool status);

    void SpeedLostPacket(const std::pair<float, uint64_t> pack);
    void WriteFlash(int region, const std::string& file_name);
    void StatusFlash(int status, int percent);

    void RunScan(const scan_model_t& model);

private slots:
    void UpdateSpeedAndMiss();

    void UpdateCount_p_slot();
    void SpeedAndLostPacket_p_slot(const std::pair<float, uint64_t> pack);
    void UpdateSpectrum_p_slot(const std::vector<int32_t>& points);
    void UpdateScanSpectrum_p_slot(const std::vector<int32_t>& points);
    void UpdateScanSpectrumGains_p_slot(const std::vector<int32_t>& gains);
    void StatusFlash_p_slot(const int status, const int percent);
    void CounterDump_p_slot(const uint64_t count);

private:
    /**
     * @brief Calculate normalized frequency step
     * @param[in] freq - frequency in Hz
     *
     * @return step value (in Hz)
     */
    static inline int32_t calcStep(const int32_t freq)
    {
        static constexpr double sampling_freq = 4915200;    // 1.2288e6 * 4
        static constexpr double bit_depth     = 4294967296; // 2 ^ 32
        static constexpr double dividend      = bit_depth / sampling_freq;

        return static_cast<int32_t>(freq * dividend);
    }

    std::pair<uint64_t, uint64_t> m_speed_and_miss;

    QTimer proj_timer;

    trans_count_model_t counters;

    bool m_only_dump { false };

    std::shared_ptr<IDevice>                 m_netwrk { nullptr };
    std::shared_ptr<StatusController>        m_status_controller { nullptr };
    std::shared_ptr<ClockDetuneController>   m_clock_detune_controller { nullptr };
    std::shared_ptr<SpectrumController>      m_spectrum_controller { nullptr };
    std::shared_ptr<FlashFirmwareController> m_flash_firmware { nullptr };
    std::shared_ptr<DumpDebugController>     m_dump { nullptr };
    std::shared_ptr<SpectrumScanController>  m_scan_controller { nullptr };

    //QThread*            m_workerThread { nullptr };
    USBPacketCollector* m_usb_collector { nullptr };

    IDMRlib::IListener* m_listener { nullptr };
};

#endif // MAINCONTROLLER_H
