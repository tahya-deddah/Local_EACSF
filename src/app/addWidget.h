#ifndef ADDWIDGET_H
#define ADDWIDGET_H

#include <QWidget>
#include <QMap>

class addWidget : public QWidget
{
	Q_OBJECT

public:
    addWidget(QWidget *parent=0, bool value=false);

signals:
   void add_to_model(QMap<QString, QString> *row);
private slots:
   void inputQpbTriggered();
   void outputdirectoryQpbTriggered();
   void on_apply_clicked();
public:
    bool use_75_surface;
};

#endif