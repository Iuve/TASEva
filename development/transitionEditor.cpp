#include "DeclareHeaders.hh"
#include "pseudoLevelsController.h"
#include "transitionEditor.h"
#include "histogram.h"
#include "responsefunction.h"
#include "ui_transitionEditor.h"
#include "project.h"

#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QAction>


TransitionEditor::TransitionEditor(QWidget *parent) :
    QWidget(parent),
    uiL(new Ui::TransitionEditor)
{
    uiL->setupUi(this);
    transitionEditorOpen_ = false;
    pseudoLevelsController_ =  new PseudoLevelsController();

    setComboBoxMethod();

    pointerToTable_= uiL->tableTransition;
//    connect(uiL->tableGamma->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(slotEditGammas(int)));
      connect(uiL->buttonAddTransition, SIGNAL(clicked(bool)), this, SLOT(slotAddTransition()));
      connect(uiL->buttonRemove, SIGNAL(clicked(bool)), this, SLOT(slotRemoveTransition()));
      connect(uiL->buttonNormalise, SIGNAL(clicked(bool)), this, SLOT(slotNormalizeTransitionIntensity()));
      connect(uiL->buttonQuit, SIGNAL(clicked(bool)), this, SLOT(close()));
      connect(uiL->tableTransition, SIGNAL(cellClicked(int,int)), this,SLOT(slotFittingStatusClicked(int,int)));
      connect(uiL->tableTransition, SIGNAL(cellChanged(int,int)),this, SLOT(slotTableChanged(int,int)));
      connect(uiL->comboBoxIntensity, SIGNAL(currentTextChanged(QString)), this, SLOT(slotSetGammaIntensityMethod(QString)));
      connect(uiL->buttonMethod, SIGNAL(clicked(bool)), this, SLOT(slotChangeIntensitiesToMethod()));
      connect(uiL->buttonSaveCustom, SIGNAL(clicked(bool)), this, SLOT(slotSaveCustomIntensities()));
      //Mouse right click actions
          setContextMenuPolicy(Qt::ActionsContextMenu);
          FittingStatusTrueAction_ = new QAction("Set Fitting status TRUE");
          FittingStatusFalseAction_ = new QAction("Set Fitting status FALSE");
          QAction* transitionAddAction = new QAction("Add transition");
          QAction* transitionRemoveAction = new QAction("Remove transition");
          QAction* normAction = new QAction("Norm. transition intensity to 100%");
//          QAction* saveAction = new QAction("Save data");
          QAction* closeAction = new QAction("Close");
          connect(FittingStatusTrueAction_, SIGNAL(triggered()), this, SLOT(slotFittingStatusTrue()));
          connect(FittingStatusFalseAction_, SIGNAL(triggered()), this, SLOT(slotFittingStatusFalse()));
          connect(normAction, SIGNAL(triggered()), this, SLOT(slotNormalizeTransitionIntensity()));
          connect(transitionAddAction, SIGNAL(triggered()), this, SLOT(slotAddTransition()));
          connect(transitionRemoveAction, SIGNAL(triggered()), this, SLOT(slotRemoveTransition()));
          connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
          addAction(FittingStatusTrueAction_);
          addAction(FittingStatusFalseAction_);
          addAction(normAction);
          addAction(transitionAddAction);
          addAction(transitionRemoveAction);
          addAction(closeAction);
      //Mouse right click acctions END


}

TransitionEditor::~TransitionEditor()
{
    setTransitionEditorOpen(false);
    uiL->tableTransition->close();
    delete pseudoLevelsController_;
    delete uiL;
}

void TransitionEditor::setComboBoxMethod()
{
    uiL->comboBoxIntensity->clear();
    pseudoLevelsController_->createGammaIntensityMethodList();
    vector<QString> methodList = pseudoLevelsController_->getGammaIntensityMethodList();
    vector<QString> methodListToolTip = pseudoLevelsController_->getGammaIntensityMethodListToolTip();
    for (unsigned i=0; i< methodList.size(); i++)
    {
  //      QString qtext = QString::fromStdString(methodList.at(i));
        uiL->comboBoxIntensity->addItem(methodList.at(i));
    }

}

void TransitionEditor::slotSetGammaIntensityMethod(QString qmethod)
{
    pseudoLevelsController_->setGammaIntensityMethod(qmethod.toStdString());
    //gammaIntensityMethod_ = qmethod.toStdString();

}

void TransitionEditor::slotChangeIntensitiesToMethod()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclides_ = decayPath->GetAllNuclides();
    std::vector<Level>* levels_ = nuclides_->at(currentNuclideIndex_).GetNuclideLevels();
    Level* level = (&levels_->at(currentLevelIndex_));

    string method = pseudoLevelsController_->getGammaIntensityMethod();
    pseudoLevelsController_->changeIntensitiesToChoosenMethod(level, method);

    responseFunction->UpdateWholeContainerIntensities();
    responseFunction->RefreshFlags();

   emit signalTransitionsEdited();
   emit signalUpdateTransitionTable(currentNuclideIndex_,  currentLevelIndex_);
}

void TransitionEditor::slotSaveCustomIntensities()
{
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclides_ = decayPath->GetAllNuclides();
    std::vector<Level>* levels_ = nuclides_->at(currentNuclideIndex_).GetNuclideLevels();
    Level* level = (&levels_->at(currentLevelIndex_));
    std::vector<Transition*>* transitionsFromLvl = level->GetTransitions();
    std::vector<double> intensities;

    for(auto itt = transitionsFromLvl->begin(); itt != transitionsFromLvl->end(); itt++)
        intensities.push_back((*itt)->GetIntensity());

    std::cout << "Saving Custom intensities. Intensities size = " << intensities.size() << std::endl;
    Project *myProject = Project::get();
    myProject->setCustomTransitionIntensities(intensities);
}

void TransitionEditor::slotTableChanged(int row, int column)
{
    qDebug() << "TransitionEditor::slotTableChanged row " << row << " column " << column ;
    qDebug() << "OPEN bool: " << transitionEditorOpen_ ;
    if(transitionEditorOpen_)
    {

    ResponseFunction* responseFunction = ResponseFunction::get();
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclides_=decayPath->GetAllNuclides();
    std::vector <Level>* levels_ = nuclides_->at(currentNuclideIndex_).GetNuclideLevels();
    std::vector<Transition*>* transitions_ = levels_->at(currentLevelIndex_).GetTransitions();
   // reading changed cell
    QString itabtext = uiL->tableTransition->item(row,column)->text();
//    string str = itabtext.toUtf8().constData();

    if(column == 0)
    {  // transition type not to be changed
        return;
    } else if (column == 1)
    {   // transition energy not to be changed
        return;
    } else  if (column == 2)
    {  // intensity to be changed
        double newIntensity_ = itabtext.toDouble()/100;
        transitions_->at(row)->ChangeIntensity(newIntensity_);
        if( transitions_->at(row)->GetParticleType() == "B+" || transitions_->at(row)->GetParticleType() == "B-" )
            responseFunction->UpdateWholeContainerIntensities();
        else
            responseFunction->ChangeContainerTransitionIntensity( transitions_->at(row), newIntensity_ );
//        slotNormalizeTransitionIntensity();
    } else if (column == 3)
    {  // free parameter for fit changed by clicking
        return;
    } else if (column == 4)
    { // final level energy not to be changed
        return;
    } else
    {
        std::cout << "Nothing to change row: " << row << " column " << column << std::endl;
        return;
    }

    responseFunction->RefreshFlags();
    emit signalUpdateTransitionTable(currentNuclideIndex_,  currentLevelIndex_);

    }
}

void TransitionEditor::setColumnStatus(bool status, int column)
{

    QTableWidget *pointerToTable_ = 0L;
    pointerToTable_ = uiL->tableTransition;

    QModelIndexList selection = pointerToTable_->selectionModel()->selectedRows();

    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
//        pointerToTable_->setItem(index.row(),column, new QTableWidgetItem(status ? "true": "false"));
        slotFittingStatusClicked(index.row(),column);
    }

}



void TransitionEditor::slotFittingStatusClicked(int row, int column)
{
    std::cout << "TransitionEditor::slotFittingStatusClicked row " << row << " column " << column << std::endl;
    if(column == 3){
        const QSignalBlocker blocker(uiL->tableTransition);
        DecayPath* decayPath = DecayPath::get();
        std::vector<Nuclide>* nuclides_=decayPath->GetAllNuclides();
        std::vector <Level>* levels_ = nuclides_->at(currentNuclideIndex_).GetNuclideLevels();
        std::vector<Transition*>* transitions_ = levels_->at(currentLevelIndex_).GetTransitions();
    bool fittingStatus_ = transitions_->at(row)->GetIntensityFitFlag();

    fittingStatus_= !fittingStatus_;
    uiL->tableTransition->setItem(row, column, new QTableWidgetItem(fittingStatus_ ? "true" : "false"));
    uiL->tableTransition->show();
// change

    transitions_->at(row)->SetIntensityFitFlag(fittingStatus_);

    emit signalUpdateTransitionTable(currentNuclideIndex_,  currentLevelIndex_);
    }
}

void TransitionEditor::slotAddTransition()
{
    std::cout << "TransitionEditor::slotAddTransition" << std::endl;
    std::cout << "LevelIndex :" << currentLevelIndex_ << std::endl;
    bool ok;
    DecayPath* decayPath= DecayPath::get();

    std::vector<Nuclide>* nuclides_=decayPath->GetAllNuclides();

    std::vector <Level>* levels_ = nuclides_->at(currentNuclideIndex_).GetNuclideLevels();

    QString fullText = QInputDialog::getText(this, tr("New Transition"),
                                          tr("Type (a,b+,b-,g,n) : Energy(keV) : Intensity(%)"), QLineEdit::Normal,
                                          "type1:energy1:intensity1;type2:energy2:intensity2;...", &ok);

    QStringList fullStringList= fullText.split(";",QString::SkipEmptyParts);

    // below loop enables adding multiple transitions at once if info are divided by ';'
    for (int k = 0; k < fullStringList.size(); k++)
    {
        QString text = fullStringList.at(k);
        QStringList stringList= text.split(":",QString::SkipEmptyParts);

        for (int i = 0; i < stringList.size(); i++) qDebug(stringList.at(i).toUtf8());
        double energy(-1);
        double intensity(101);
        std::string transitionType = "";
         if (ok && stringList.size() == 3 )
         {
             transitionType = stringList.at(0).toStdString();
             energy = stringList.at(1).toDouble();
             intensity = stringList.at(2).toDouble();
             std::cout << transitionType <<" : "<< energy << ": " << intensity  << std::endl ;
         } else {
             QMessageBox msgBox;
             msgBox.setText("Data not correct - no changes applied");
             msgBox.exec();
             return;
         }
         if(intensity > 100 || energy < 0){
             QMessageBox msgBox;
             msgBox.setText("Energy or Intensity data not correct - no changes applied");
             msgBox.exec();
             energy = 0;
             intensity = 0;
             return;
         }

         int atomicNumber = nuclides_->at(currentNuclideIndex_).GetAtomicNumber();
         int atomicMass = nuclides_->at(currentNuclideIndex_).GetAtomicMass();
         //double finalLevelEnergy = 0;
         int finalLevelAtomicMass;
         int finalLevelAtomicNumber;
         if(transitionType =="a" || transitionType =="A")
         {
             finalLevelAtomicMass = atomicMass - 4;
             finalLevelAtomicNumber  =  atomicNumber -2 ;
             transitionType = "A";
         } else if(transitionType =="b-" || transitionType =="B-")
         {
             finalLevelAtomicMass = atomicMass ;
             finalLevelAtomicNumber  =  atomicNumber +1 ;
             transitionType = "B-";
         } else if(transitionType =="b+" || transitionType =="B+")
         {
             finalLevelAtomicMass = atomicMass ;
             finalLevelAtomicNumber  =  atomicNumber -1 ;
             transitionType = "B+";
         } else if(transitionType =="n" || transitionType =="N")
         {
             finalLevelAtomicMass = atomicMass -1 ;
             finalLevelAtomicNumber  =  atomicNumber  ;
             transitionType = "N";
         }else if(transitionType =="g" || transitionType =="G")
         {
             finalLevelAtomicMass = atomicMass ;
             finalLevelAtomicNumber  =  atomicNumber  ;
             transitionType = "G";
         } else
         {
             QMessageBox msgBox;
             msgBox.setText("Transition type data not correct - no changes applied");
             msgBox.exec();
             energy = 0;
             intensity = 0;
             transitionType = "";
             return;
         }

         intensity = intensity/100;  //converting to numbers not perCent


    /*
        Level *finalLevel = decay->FindFinalLevel(newGamma, levels.at(currentLevelIndex_));
        if(levels.at(currentLevelIndex_) == finalLevel)
        {
            int r = QMessageBox::warning(this, tr("Error"),
                                         tr("Too small gamma energy, no final level"),
                                         QMessageBox::Ok);
            if (r == QMessageBox::Ok)
                return;
        }
        newGamma->SetFinalLevel(finalLevel);
    */

        levels_->at(currentLevelIndex_).AddTransition(transitionType, energy,intensity);

        emit signalTransitionsEdited();
        emit signalUpdateTransitionTable(currentNuclideIndex_,  currentLevelIndex_);
    }

}


void TransitionEditor::slotNormalizeTransitionIntensity()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclides_=decayPath->GetAllNuclides();
    std::vector <Level>* levels_ = nuclides_->at(currentNuclideIndex_).GetNuclideLevels();
    levels_->at(currentLevelIndex_).NormalizeTransitionIntensities();

    responseFunction->UpdateWholeContainerIntensities();
    if( &nuclides_->at(currentNuclideIndex_) != &nuclides_->at(0) )
        emit signalTransitionsEdited();
    emit signalUpdateTransitionTable(currentNuclideIndex_,  currentLevelIndex_);

}


void TransitionEditor::slotRemoveTransition()
{
    DecayPath* decayPath= DecayPath::get();
    QTableWidget *pointerToTable_ =0L;
    pointerToTable_ = uiL->tableTransition;

    QModelIndexList selection = pointerToTable_->selectionModel()->selectedRows();

    QString qenergy;
    QString qtransitionType;
    int numberOfTransitions = 0;
    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        qDebug() << "index.row()= : " << index.row();
        QTableWidgetItem* itmE = pointerToTable_->item( index.row(), 1 );
        QTableWidgetItem* itmT = pointerToTable_->item( index.row(), 0 );
        if (itmE)  qenergy = itmE->text();
        if (itmT) qtransitionType = itmT->text();
        qDebug() << "Energy , Type" << qenergy <<" , " << qtransitionType;

        qDebug() << " NuclideIndex and LevelIndex" << currentNuclideIndex_ << " : " << currentLevelIndex_;
        std::string type = qtransitionType.toStdString();
        double energy = qenergy.toDouble();
        std::vector<Level>* levels_ = decayPath->GetAllNuclides()->at(currentNuclideIndex_).GetNuclideLevels();
        numberOfTransitions = levels_->at(currentLevelIndex_).GetTransitions()->size();
        levels_->at(currentLevelIndex_).RemoveTransition(type,energy);
        std::cout << "Transition removed (type,energy) " << "(" << type <<"." << energy <<")" << endl;
    }
    if(numberOfTransitions > 1)
    {
    emit signalUpdateTransitionTable(currentNuclideIndex_, currentLevelIndex_);
    } else
    {
     uiL->buttonQuit->click();
    }
// emit signal to update leveltable
    emit signalTransitionsEdited();
    emit signalUpdateLevelTable();

}



void TransitionEditor::setTotalIntensityLabel()
{
   if(transitionEditorOpen_)
   {
    double totalTransitionIntensity_(0);
    unsigned int nrOfRow = uiL->tableTransition->rowCount();
    for(unsigned int i=0; i != nrOfRow; ++i)
       {
      totalTransitionIntensity_ = totalTransitionIntensity_ + uiL->tableTransition->item(i,2)->text().toDouble();
       }
    QString QTotalIntensity_ = QString::number(totalTransitionIntensity_);
    QString qstr = "Summed transition intensity (%): "+ QTotalIntensity_;
    uiL->labelTransitionIntensity->setText(qstr);
   }
}
/*Eva
void GammaEditor::slotEditGammas(int rowIndex)
{
    std::cout <<"row Number Clicked " << rowIndex <<std::endl;
}
*/

void TransitionEditor::setTransitionLabel(QString Label)
{
    uiL->labelTransition->setText(Label);
}




