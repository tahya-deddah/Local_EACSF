#ifndef MODEL_H
#define MODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QVariant>

class CsvTableModel : public QAbstractTableModel
{
    Q_OBJECT

public: 
    CsvTableModel( QList<QMap<QString, QString>*> &Data, QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());   
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void SetModelData(const QList<QMap<QString, QString>*> &data);
    QList<QMap<QString, QString>*> GetModelData() const;
    void clear() ;
    void AddRow(QMap<QString, QString> *row);
private:
    QList<QMap<QString, QString> *> rowList;  
};

#endif

