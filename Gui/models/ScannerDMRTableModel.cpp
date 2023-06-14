#include "ScannerDMRTableModel.h"

ScannerDMRTableModel::ScannerDMRTableModel(const QStringList& model, QObject* parent)
    : QAbstractTableModel(parent)
    , header(model)
{
}

QVariant ScannerDMRTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    switch (orientation) {
    case Qt::Orientation::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            return header.at(section);
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

bool ScannerDMRTableModel::setHeaderData(int /*section*/,
    Qt::Orientation /*orientation*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

int ScannerDMRTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return model_data.size();
}

int ScannerDMRTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return header.size();
}

QVariant ScannerDMRTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return model_data.value(index.row()).value(index.column());
    default:
        return QVariant();
    }
}

bool ScannerDMRTableModel::setData(
    const QModelIndex& index, const QVariant& value, int role)
{
    if (index.row() >= model_data.size())
        return false;
    if (index.column() >= model_data.value(index.row()).size())
        return false;
    if (role != Qt::EditRole)
        return false;
    if (data(index, role) == value)
        return false;

    model_data[index.row()].replace(index.column(), value.toString());

    emit dataChanged(
        this->index(index.row(), 0), this->index(index.row(), header.size())

    );

    return true;
}

Qt::ItemFlags ScannerDMRTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.row() == m_selectedItem) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::ItemIsEnabled;
}

void ScannerDMRTableModel::SetRowSelect(const int& row)
{
    m_selectedItem = row;
}

bool ScannerDMRTableModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (!count)
        return false;

    beginInsertRows(parent, row, row + count - 1);
    for (auto i = row; i < count + row; i++)
        model_data.insert(i, [&]() -> QStringList {
            QStringList result;
            for (auto i = 0; i < header.size(); i++)
                result << "";
            return result;
        }());
    endInsertRows();

    return true;
}

bool ScannerDMRTableModel::insertColumns(
    int /*column*/, int /*count*/, const QModelIndex& /*parent*/)
{
    return false;
}

bool ScannerDMRTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!count)
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (auto i = 0; i < count; i++)
        model_data.removeAt(row);
    endRemoveRows();

    return true;
}

bool ScannerDMRTableModel::removeColumns(
    int /*column*/, int /*count*/, const QModelIndex& /*parent*/)
{
    return false;
}
