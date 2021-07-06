#include <QtGui>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLineEdit>
#include <QCoreApplication>
#include <QVector>
#include <QMessageBox>
#include <QCheckBox>

#include "addWidget.h"

addWidget::addWidget(QWidget *m_parent, bool value)
: QWidget(m_parent)
{

    this->resize(650, 500);
    QLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignTop);
    verticalLayout->setSpacing(0);
	verticalLayout->setMargin(0); 
	verticalLayout->setContentsMargins( 0, 0, 0, 0 );
    use_75_surface = value;

    
    QVector<QString> vector;
    vector << QString("Tissu Segmentation") << QString("CSF Probability Map") << QString("LH MID Surface") << QString("RH MID Surface")  << QString("Output Directory");

    if(use_75_surface)
    {
        vector.clear();
        vector << QString("Tissu Segmentation") << QString("CSF Probability Map")  << QString("LH MID Surface")  << QString("LH GM Surface") << 
        QString("RH MID Surface")  << QString("RH GM Surface") << QString("Output Directory");
    }


    foreach (const QString inputName, vector) 
    {
    	QWidget *containerWidget = new QWidget();
    	containerWidget->resize(100, 200);
	    QLayout *horizontalLayout = new QHBoxLayout();
	    verticalLayout->addWidget(containerWidget);

	    QPushButton *qpb =  new QPushButton();
	    qpb->setMinimumWidth(100);
        qpb->setMinimumHeight(31);
        qpb->setMaximumHeight(31);
	    qpb->setText(inputName);
	    qpb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
	    if(inputName == QString("Output Directory"))     
        {QObject::connect(qpb,SIGNAL(clicked()),this,SLOT(outputdirectoryQpbTriggered()));} 
		else
        { QObject::connect(qpb,SIGNAL(clicked()),this,SLOT(inputQpbTriggered()));}
	    QLineEdit *lined = new QLineEdit();
	    lined->setMinimumWidth(150);
        lined->setMinimumHeight(31);
        lined->setMaximumHeight(31);
        lined->setObjectName(inputName);
	    lined->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

	    horizontalLayout->addWidget(qpb);
	    horizontalLayout->addWidget(lined);
	    containerWidget->setLayout(horizontalLayout);
    }
  
    QPushButton *apply = new QPushButton( QString("Apply") ,this);
    apply->setGeometry(250, 400, 100, 30);
	apply->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);   
	QObject::connect(apply,SIGNAL(clicked()),this,SLOT(on_apply_clicked()));
	
    this->setLayout(verticalLayout);   
}
void addWidget::inputQpbTriggered()
{
    QObject *sd = QObject::sender();
    QObject *par = sd->parent();
    QLineEdit *le = NULL;
    foreach (QObject *ch, par->children())
    {
        le = qobject_cast<QLineEdit*>(ch);
        if (le)
        {
            break;
        }
    }
    QString path = QFileDialog::getOpenFileName(
    			this,
                "Open File",
                "C://",
                "All files (*.*);; NIfTI File (*.nii *.nii.gz);; NRRD File  (*.nrrd);; Json File (*.json)"
                );

    if (!path.isEmpty())
    {
        le->setText(path);
    }
}
void addWidget::outputdirectoryQpbTriggered()
{
    QObject *sd = QObject::sender();
    QObject *par = sd->parent();
    QLineEdit *le = NULL;
    foreach (QObject *ch, par->children())
    {
        le = qobject_cast<QLineEdit*>(ch);
        if (le)
        {
            break;
        }
    }
    QString path = QFileDialog::getExistingDirectory(
                this,
                "Open Directory",
                "C://",
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                );
    if (!path.isEmpty())
    {
        le->setText(path);        
    }
}
void addWidget::on_apply_clicked()
{   

    QMap<QString, QString> *row = new QMap<QString, QString>;
	QObjectList clist = this->children();
 
    QList<QWidget*> mylineEdits = this->findChildren<QWidget*>();
	QListIterator<QWidget*> it(mylineEdits); 
	QWidget *lineEditField;
	while (it.hasNext()) {
    	lineEditField = it.next(); 
    	if(QLineEdit *lineE = qobject_cast<QLineEdit*>(lineEditField)) 
    	{
    		QString txt = lineE->text();
    		if (txt == QString(""))
    		{ 
    			QMessageBox::warning(
            	this,
            	tr(""),
            	tr("fill in all the fields please.")
        		);
        		return;
    		}
            else
            {
                row->insert( lineE->objectName(), lineE->text());
            }
    	}       
	}
	emit add_to_model(row);
}
