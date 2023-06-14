#ifndef uiDMRScanner_H
#define uiDMRScanner_H

#include <QWidget>
#include <vector>

#include "models/ScannerDMRTableModel.h"
#include <IDMRlib.h>

namespace Ui {
class uiDMRScanner;
}

class uiDMRScanner : public QWidget {
    Q_OBJECT

public:
    explicit uiDMRScanner(QWidget* parent = nullptr);
    ~uiDMRScanner();
    void onStartStopScan();
    bool GetStatus() const;
    void OnNewRadio(const std::vector<dmr_radio_model_t>& radio_list);

signals:
    void UpdateStatusScan(const bool status, const dmr_settings_new_signal& set);

public slots:
    void SwapMode(const int mode);

private:
    bool                  m_status_work { false };
    Ui::uiDMRScanner*     ui;
    ScannerDMRTableModel* m_table_model { nullptr };

    std::vector<dmr_radio_model_t> m_radio_list;
};

#endif // uiDMRScanner_H
