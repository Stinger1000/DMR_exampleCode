#ifndef SCANNER_TABLE_MODEL_H
#define SCANNER_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QStringList>

class ScannerDMRTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    ScannerDMRTableModel(const QStringList& model, QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation,
        const QVariant& value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(
        const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void SetRowSelect(const int& row);

    // Add data:
    bool insertRows(
        int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool insertColumns(
        int column, int count, const QModelIndex& parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(
        int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeColumns(
        int column, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    QStringList        header;
    QList<QStringList> model_data;
    int                m_selectedItem { -1 };
};

#endif // SCANNER_TABLE_MODEL_H
