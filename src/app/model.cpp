#pragma GCC diagnostic ignored "-Wshadow"
#include <QtGui>
#include <QToolTip>

#include "model.h"


CsvTableModel::CsvTableModel(QList<QMap<QString, QString>*> &Data, QObject *parent)
    : QAbstractTableModel(parent)
{  
    rowList = Data;    
}


int CsvTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    if(!rowList.isEmpty()){ return rowList.size();}
    else{return 0;}
}

int CsvTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    if(!rowList.isEmpty()){ return rowList[0]->size();}
    else{return 0;}   
}

QVariant CsvTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::TextAlignmentRole )
            return Qt::AlignRight ;
    if (role == Qt::ToolTipRole )
        return rowList[index.row()]->values().at(index.column()); 
    if (role == Qt::DisplayRole )
       if (!rowList.isEmpty())
            return rowList[index.row()]->values().at(index.column()); 
        else 
            return QVariant();
    else
        return QVariant();
}

QVariant CsvTableModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
  
   if (orientation == Qt::Horizontal){
        QList<QString> headers = rowList[0]->keys();
        return headers[section];
    }
    else
        return QString("%1").arg(section);
     
}
Qt::ItemFlags CsvTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
}

bool CsvTableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    int columns = columnCount();
    beginInsertRows(parent, position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        QMap<QString, QString>* items;
        for (int column = 0; column < columns; ++column)
            items->insert("", "");
        rowList.insert(position, items);
    }

    endInsertRows();
    return true;
}
bool CsvTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(parent, position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
            rowList.removeAt(position);
    } 
    endRemoveRows();
    return true;
}
QList<QMap<QString, QString>*> CsvTableModel::GetModelData() const
{
    return rowList;
}

void CsvTableModel::SetModelData(const  QList<QMap<QString, QString>*> &data)
{
    beginResetModel();
    rowList = data;
    endResetModel();    
}

void CsvTableModel::clear() 
{
    beginResetModel();
    rowList.clear();
    endResetModel();    
}

void CsvTableModel::AddRow(QMap<QString, QString> *row)
{
    beginResetModel();
    rowList << row;
    endResetModel();  
}












