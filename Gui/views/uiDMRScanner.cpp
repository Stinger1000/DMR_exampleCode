#include "uiDMRScanner.h"
#include "ui_uiDMRScanner.h"

uiDMRScanner::uiDMRScanner(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::uiDMRScanner)
    , m_table_model(new ScannerDMRTableModel(QStringList({ "Тип сигнала", "Частота", "ID", "CC", "Мощность, dB" }), this))
{
    ui->setupUi(this);

    ui->tbwScanView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbwScanView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tbwScanView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tbwScanView->setModel(m_table_model);

    //signals
    connect(ui->btnStartScan, &QPushButton::clicked, this, &uiDMRScanner::onStartStopScan);
}

uiDMRScanner::~uiDMRScanner()
{
    delete ui;
}

void uiDMRScanner::onStartStopScan()
{
    dmr_settings_new_signal dmr_signal { uint8_t(ui->spbFreq->value()),
        uint8_t(ui->spbLevel->value()),
        uint8_t(ui->spbTime->value()) };

    if (ui->btnStartScan->text() == "Начать сканирование") {
        ui->btnStartScan->setText("Остановить сканирование");
        m_status_work = true;
        emit UpdateStatusScan(true, dmr_signal);

    } else {
        ui->btnStartScan->setText("Начать сканирование");
        m_status_work = false;
        emit UpdateStatusScan(false, dmr_signal);
    }
}

bool uiDMRScanner::GetStatus() const
{
    return m_status_work;
}

/**
 * @brief Displays found dmr radio
 * @param[in] dmr radio - found dmr radio.s
 *
 * @return none
 */
void uiDMRScanner::OnNewRadio(const std::vector<dmr_radio_model_t>& radio_list)
{
    m_radio_list = radio_list;

    if (m_table_model->rowCount())
        m_table_model->removeRows(0, m_table_model->rowCount());

    auto current_row = m_table_model->rowCount();

    m_table_model->insertRows(current_row, static_cast<int>(radio_list.size()));

    for (const auto& i : radio_list) {
        m_table_model->setData(
            m_table_model->index(current_row, 0), i.type_signal == TypeSig::MS ? "MS" : "BS");
        m_table_model->setData(
            m_table_model->index(current_row, 1), i.freq);
        m_table_model->setData(
            m_table_model->index(current_row, 2), QString::fromStdString(i.id));
        m_table_model->setData(
            m_table_model->index(current_row, 3), QString::number(int(i.cc)));
        m_table_model->setData(
            m_table_model->index(current_row, 4), QString::number(i.lvl_db));

        current_row++;
    }
}

void uiDMRScanner::SwapMode(const int mode)
{
    if (mode) {
        ui->grbParamSignal->hide();
        ui->btnStartScan->hide();
    } else {
        ui->grbParamSignal->show();
        ui->btnStartScan->show();
    }
}
