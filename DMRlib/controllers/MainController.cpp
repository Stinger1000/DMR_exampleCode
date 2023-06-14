#include "MainController.h"

MainController::MainController(std::unique_ptr<IDevice> dev, const support_model_t& support_model,
    IDMRlib::IListener* listener, QObject* parent)
    : QObject(parent)
    , proj_timer(this)
    , m_netwrk(std::move(dev))
    , m_status_controller(std::make_unique<StatusController>())
    , m_clock_detune_controller(std::make_unique<ClockDetuneController>())
    , m_spectrum_controller(std::make_unique<SpectrumController>(m_netwrk, this))
    , m_flash_firmware(std::make_unique<FlashFirmwareController>(m_netwrk, this))
    , m_dump(std::make_unique<DumpDebugController>(m_netwrk, this))
    , m_scan_controller(std::make_unique<SpectrumScanController>(m_netwrk, this))
    //, m_workerThread(new QThread())
    , m_listener(listener)
{
    //Registration meta types
    qRegisterMetaType<trans_count_model_t>("trans_count_model_t");
    qRegisterMetaType<DumpSetupModel>("DumpSetupModel");
    qRegisterMetaType<std::vector<int32_t>>("std::vector<int32_t>");
    qRegisterMetaType<std::pair<float, uint64_t>>("std::pair<float, uint64_t>");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<ad_settings_model_t>("ad_settings_model_t");
    qRegisterMetaType<scan_model_t>("scan_model_t");
    qRegisterMetaType<src_model_t>("src_model_t");
    //

    connect(this, &MainController::UpdateCount, this, &MainController::UpdateCount_p_slot, Qt::QueuedConnection);
    connect(this, &MainController::SpeedLostPacket, this, &MainController::SpeedAndLostPacket_p_slot, Qt::QueuedConnection);
    connect(this, &MainController::UpdateSpectrum, this, &MainController::UpdateSpectrum_p_slot, Qt::QueuedConnection);
    connect(this, &MainController::UpdateScanSpectrum, this, &MainController::UpdateScanSpectrum_p_slot, Qt::QueuedConnection);
    connect(this, &MainController::UpdateScanSpectrumGains, this, &MainController::UpdateScanSpectrumGains_p_slot, Qt::QueuedConnection);
    connect(this, &MainController::StatusFlash, this, &MainController::StatusFlash_p_slot, Qt::QueuedConnection);
    connect(this, &MainController::CounterDump, this, &MainController::CounterDump_p_slot, Qt::QueuedConnection);

    connect(m_netwrk.get(), &IDevice::ReceiveData, this, &MainController::MainParser);
    proj_timer.start(1000);

    //connects//
    connect(&proj_timer, &QTimer::timeout, this, &MainController::reqStatus);
    connect(&proj_timer, &QTimer::timeout, this, &MainController::reqPrjStatus);
    connect(&proj_timer, &QTimer::timeout, this, &MainController::reqAdDxco);
    connect(&proj_timer, &QTimer::timeout, this, &MainController::UpdateSpeedAndMiss);

    connect(m_spectrum_controller.get(), &SpectrumController::Spectrum, this, &MainController::UpdateSpectrum);
    connect(m_scan_controller.get(), &SpectrumScanController::SpectrumScan, this, &MainController::UpdateScanSpectrum);
    connect(m_scan_controller.get(), &SpectrumScanController::SpectrumScanGains, this, &MainController::UpdateScanSpectrumGains);

    connect(this, &MainController::EnableCorAndSpec, m_spectrum_controller.get(), &SpectrumController::Enable);

    connect(m_flash_firmware.get(), &FlashFirmwareController::status, this, &MainController::StatusFlash);
    connect(this, &MainController::WriteFlash, m_flash_firmware.get(), &FlashFirmwareController::OnWrite);

    connect(m_dump.get(), &DumpDebugController::counter, this, &MainController::CounterDump);
    connect(this, &MainController::EnableDump, m_dump.get(), &DumpDebugController::OnConfigure);
    connect(m_dump.get(), &DumpDebugController::testCounter, this, &MainController::CounterTest);

    connect(m_flash_firmware.get(), &FlashFirmwareController::SendError, this, &MainController::SendError, Qt::QueuedConnection);
    connect(m_dump.get(), &DumpDebugController::SendError, this, &MainController::SendError, Qt::QueuedConnection);

    connect(this, &MainController::RunScan, m_scan_controller.get(), &SpectrumScanController::requestSpectrumScan);

    SetSupport(support_model.enable, support_model.freq);

    src_model_t src_model { receiver, 0 };
    SetSrc(src_model);
}

MainController::~MainController()
{
    //m_workerThread->quit();
    //m_workerThread->wait();
}

/**
 * @brief Request PROJECT_STATUS command
 * @param nonef
 *
 * @return none
 */
void MainController::reqPrjStatus()
{
    RawPacker wrapper(mtype::COMMON, common::from_pc::GET_PRJ_STATUS);
    m_netwrk->SendData(wrapper.GetPkt());
}

/**
 * @brief Request board status
 * @param none
 *
 * @return none
 */
void MainController::reqStatus()
{
    RawPacker packer(mtype::COMMON, common::from_pc::GET_SYSMON);
    m_netwrk->SendData(packer.GetPkt());
}

/**
 * @brief Request AD DXCO values (clock detunes)
 * @param none
 *
 * @return none
 */
void MainController::reqAdDxco()
{
    RawPacker packer(mtype::ARM, arm::from_pc::GET_DXCO);
    m_netwrk->SendData(packer.GetPkt());
}

/**
 * @brief Update settings (frequencies, gains) of AD transmitter
 * @param[in] settings - contains frequencies and gains
 *
 * @return none
 */
void MainController::UpdateAdFreqConf(const ad_settings_model_t& settings)
{
    if (settings.ad_index > counters.ad_counts) {
        m_listener->GetError(INCORRECT_AD_NUM);
        return;
    }

    RawPacker pkt(mtype::ARM, arm::from_pc::SET_FREQ);
    pkt.push(settings.ad_index);
    pkt.push(settings.tx_enabled);
    pkt.push(static_cast<uint8_t>(settings.rx_gain_mode));

    pkt.push(settings.tx_atten);
    pkt.push(settings.rx_gain_level);

    pkt.push_ule(settings.rx_freq);
    pkt.push_ule(settings.tx_freq);

    m_netwrk->SendData(pkt.GetPkt());
}

/**
 * @brief Get current clock's detune model of data
 * @param none
 *
 * @return (pointer to) the actual clock's detune model of data
 */
const QVector<clk_detune_model_t>* MainController::GetDetuneModel()
{
    auto detune = m_clock_detune_controller->GetDetunes();
    return detune;
}

/**
 * @brief Process response of PRJ_STATUS command
 * @param[in] pkt - object with parsed mtype, command and packet (only payload)
 *
 * @return none
 */
void MainController::processPrjStatus(RawPacker&& pkt)
{
    if (pkt.GetPacketSize() < 16)
        throw std::runtime_error("Wrong PRJ_STATUS packet");

    pkt.skip(4); // version and type of firmware
    counters.ad_counts = pkt.pop_16le();

    emit UpdateCountForStatus(counters);
}

void MainController::OnNewSpectrumSource(int source, int channel)
{
    m_spectrum_controller->OnNewSource(source, channel);
}

/**
 * @brief Process response of ARM commands
 * @param[in] pkt - object with parsed mtype, command and packet (only payload)
 *
 * @return none
 */
void MainController::processArm(RawPacker&& pkt) noexcept
{
    try {
        switch (static_cast<arm::from_board>(pkt.GetCommand())) {
        case arm::from_board::GET_DXCO:
            m_clock_detune_controller->ProcessDetunePkt(std::move(pkt));
            break;

        case arm::from_board::GET_SCAN_SPECTRUM:
            m_scan_controller->responceEndScan(std::move(pkt));
            break;

        case arm::from_board::SET_RX_FREQUENCY:
            //if (m_scanner_controller->IsScanning()) {
            //    m_scanner_controller->OnAdFrequencyChanged(std::move(pkt));
            //    break;
            //}
            break;
        case arm::from_board::GET_HOST: {
            //emit HostConfig(m_host->ProcessHostPkt(std::move(pkt)));
            break;
        }
        case arm::from_board::GET_CURRENT_GAIN: {
            //if (m_scanner_controller->IsScanning()) {
            //    m_scanner_controller->OnCurrentGain(std::move(pkt));
            //     break;
            //}
            //if (m_view_status_controller->IsRunning())
            //    m_view_status_controller->OnCurrentGain(std::move(pkt));

            auto gain = pkt.read_32le();

            m_spectrum_controller->OnCurrentGain(gain);
            m_scan_controller->OnCurrentGain(gain);

            break;
        }

        case arm::from_board::SET_FIRMWARE:
            m_flash_firmware->OnFirmware(std::move(pkt));
            break;
        default:
            break;
        }
    } catch (const std::exception& exp) {
        qDebug() << "Catch exception in" << __func__ << ":" << exp.what();
    }
}

/**
 * @brief Get current status model of data
 * @param none
 *
 * @return (pointer to) the actual status model of data
 */
const std::shared_ptr<status_model_t>& MainController::GetStatusModel() const
{
    return m_status_controller->GetStatus();
}

/**
 * @brief Disconnect from board, set them to broadcast destination address
 * @param none
 *
 * @return none
 */
void MainController::OnDisconnectHost()
{
    RawPacker pkt(mtype::ARM, arm::from_pc::DISCONNECT);
    m_netwrk->SendData(pkt.GetPkt());
}

/**
 * @brief Set settings for host
 * @param[in] host_model - model host settings
 *
 * @return none
 */
void MainController::SetHost(const host_model_t& host_model)
{
    RawPacker packet(mtype::ARM, arm::from_pc::SET_HOST);

    for (auto i = 3; i >= 0; i--)
        packet.push(host_model.ip_part.at(i));

    for (auto i = 3; i >= 0; i--)
        packet.push(host_model.ip_pc.at(i));

    packet.push(host_model.port_part);
    packet.push(host_model.port_part >> 8);

    packet.push(host_model.port_pc);
    packet.push(host_model.port_pc >> 8);

    for (auto i = 5; i >= 0; i--)
        packet.push(host_model.mac.at(i));

    for (auto i = 5; i >= 0; i--)
        packet.push(0xFF);

    m_netwrk->SendData(packet.GetPkt());
}

/**
 * @brief On the dump
 * @param[in] model - setting for dump
 *
 * @return none
 */
void MainController::OnDumpOffTimer(const DumpSetupModel& model)
{
    if (model.enable) {
        m_only_dump = true;
    } else {
        m_only_dump = false;
    }
    emit EnableDump(model);
}

void MainController::SetSupport(const bool enable, const uint32_t freq)
{
    RawPacker packet(mtype::ARM, arm::from_pc::SET_SUPPORT);
    packet.push(uint8_t(enable));

    packet.push_ule(freq);

    m_netwrk->SendData(packet.GetPkt());
}

//FOR CONNECT LISTENER
void MainController::UpdateCount_p_slot()
{
    m_listener->UpdateCount(counters);
}

void MainController::SpeedAndLostPacket_p_slot(const std::pair<float, uint64_t> pack)
{
    m_listener->SpeedAndLostPacket(pack);
}

void MainController::UpdateSpectrum_p_slot(const std::vector<int32_t>& points)
{
    m_listener->UpdateSpectrum(points);
}

void MainController::UpdateScanSpectrum_p_slot(const std::vector<int32_t>& points)
{
    m_listener->UpdateScanSpectrum(points);
}

void MainController::UpdateScanSpectrumGains_p_slot(const std::vector<int32_t>& gains)
{
    m_listener->UpdateScanSpectrumGains(gains);
}

void MainController::StatusFlash_p_slot(const int status, const int percent)
{
    m_listener->StatusFlash(status, percent);
}

void MainController::CounterDump_p_slot(const uint64_t count)
{
    m_listener->CounterDump(count);
}

void MainController::GetStatusBoard()
{
    m_listener->GetStatusBoard(*GetStatusModel().get());
}

void MainController::GetDetune()
{
    m_listener->GetDetune(std::vector<clk_detune_model_t>(GetDetuneModel()->begin(), GetDetuneModel()->end()));
}

void MainController::SendError(const uint8_t error)
{
    m_listener->GetError(static_cast<Error>(error));
}

void MainController::SetSrc(const src_model_t& model)
{
    RawPacker pkt(mtype::DMR, dmr::from_pc::SET_SRC);

    pkt.push(model.type_src);
    pkt.push(model.num_chan);

    for (auto i = 0; i < 6; i++)
        pkt.push(0x00);

    m_netwrk->SendData(pkt.GetPkt());
}

void MainController::UpdateSpeedAndMiss()
{
    static std::pair<float, uint64_t> model;
    model.first  = double(m_speed_and_miss.first) / 1024;
    model.second = m_speed_and_miss.second;
    emit SpeedLostPacket(model);
    m_speed_and_miss.first  = 0;
    m_speed_and_miss.second = 0;
}

void MainController::MainParser(const QByteArray& data)
{
    if (data.size() < 16)
        return;

    int        size_command = uint16_t(((data.at(8) << 8) & 0xFF00) | (data.at(9) & 0xFF));
    QByteArray data_resize(data.data(), size_command + 8);

    RawPacker parser(data_resize);

    //calc speed and miss
    static uint16_t last_pnum = 0;
    uint8_t         num_pack  = static_cast<uint8_t>(data_resize.at(13));
    m_speed_and_miss.first += data_resize.size();
    m_speed_and_miss.second += num_pack - ((last_pnum + 1) % 256);
    last_pnum = num_pack;

    switch (parser.GetMType()) {
    case mtype::ARM:
        if (m_only_dump)
            return;
        processArm(std::move(parser));
        break;
    case mtype::COMMON:
        if (m_only_dump)
            return;
        processCommon(std::move(parser));
        break;
    case mtype::DMR:
        processDMR(std::move(parser));
        break;
    default:
        qFatal("<MainController> Get unknown command");
    }

    //emit ReadyRead();
    emit m_netwrk.get()->RecDataStatus();
}

/**
 * @brief Process response of Common commands
 * @param[in] pkt - object with parsed mtype, command and packet (only payload)
 *
 * @return none
 */
void MainController::processCommon(RawPacker&& pkt) noexcept
{
    try {
        switch (static_cast<common::from_board>(pkt.GetCommand())) {
        case common::from_board::GET_SYSMON:
            m_status_controller->ProcessStatusPkt(std::move(pkt));
            break;
        case common::from_board::GET_CLK_TACTS:
            // XXX: may be need calculate in future
            break;
        case common::from_board::GET_PRJ_STATUS:
            processPrjStatus(std::move(pkt));
            break;
        default:
            break;
        }
    } catch (const std::exception& exp) {
        qDebug() << "Catch exception in" << __func__ << ":" << exp.what();
    }
}

/**
 * @brief Process response of CDMA commands
 * @param[in] pkt - object with parsed mtype, command and packet (only payload)
 *
 * @return none
 */
void MainController::processDMR(RawPacker&& pkt) noexcept
{
    try {
        dmr::from_board command = static_cast<dmr::from_board>(pkt.GetCommand());
        switch (static_cast<dmr::from_board>(command)) {

        case dmr::from_board::RX_SAMPLES:
            m_dump->OnResponse(std::move(pkt));
            break;

        case dmr::from_board::SPECTRUM:
            m_spectrum_controller->OnSpectrum(std::move(pkt));
            break;

        case dmr::from_board::GET_TX_SAMPLES:
            //for data
            break;

        case dmr::from_board::SCAN_FFT:
            m_scan_controller->OnSpectrumScan(std::move(pkt));
            break;

        default:
            qDebug() << "Catch unknown command:" << pkt.GetCommand();
            //skip not use com
            break;
        }
    } catch (const std::exception& exp) {
        // XXX: emit signal with error msg to GUI thread
        qDebug() << "Catch exception in " << __func__ << ": " << exp.what();
    }
}
