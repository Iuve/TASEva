#include <DeclareHeaders.hh>
#include "pseudoLevelsController.h"
#include "transitionEditor.h"
#include "decayPathEditor.h"
#include "ui_DecayPathEditor.h"
#include "tablecontroller.h"
#include "responsefunction.h"

#include "QDebug"
#include <QDialog>
#include <QInputDialog>



class DialogOptionsWidget;

DecayPathEditor::DecayPathEditor(QWidget *parent) :
    QWidget(parent),
    uiT(new Ui::DecayPathEditor)
{
    uiT->setupUi(this);
    myProject = Project::get();
    g1 = new TransitionEditor();
    DecayPath* decayPath = DecayPath::get();
    pseudoLevelsController_ =  new PseudoLevelsController();

    setComboBoxMethod();

    uiT->tableDaughterLevels->setToolTip("Double click on the row number to get the transition editor open");
    connect(uiT->tableMotherLevels->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(slotEditMTransitions(int)));
    connect(uiT->tableDaughterLevels->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(slotEditDTransitions(int)));
    connect(uiT->tableGrandDaughterLevels->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(slotEditGDTransitions(int)));

//EVa     connect(uiT->tableFeeding, SIGNAL(cellChanged(int,int)),  SLOT(slotUpdateFeedingData(int,int)) );
    connect(uiT->buttonClose, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(uiT->buttonNormBetaFeeding, SIGNAL(clicked(bool)), this, SLOT(slotNormalizeBetaIntensity()));
    connect(uiT->buttonAddLevel, SIGNAL(clicked(bool)), this, SLOT(slotAddLevelEI()));
    connect(uiT->buttonRemoveLevel, SIGNAL(clicked(bool)), this, SLOT(slotRemoveLevel()));
    connect(g1, SIGNAL(signalUpdateTransitionTable(int,int )), this, SLOT(slotEditTransitions(int,int)));
    connect(g1, SIGNAL(signalUpdateLevelTable()),this, SLOT(slotUpdateTablesForward()));
    connect(uiT->buttonAddPseudoLevel, SIGNAL(clicked(bool)), this, SLOT(slotAddPseudoLevel()));
    connect(uiT->comboBoxIntensity, SIGNAL(currentTextChanged(QString)), this, SLOT(slotSetGammaIntensityMethod(QString)));

    connect(g1, SIGNAL(signalTransitionsEdited()), this, SIGNAL(signalDecayPathEdited()));


//Mouse right click actions
    setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* levelAddAction = new QAction("Add level");
    QAction* levelRemoveAction = new QAction("Remove level");
    QAction* normAction = new QAction("Norm. Beta intensity");
    QAction* closeAction = new QAction("Cancel");
    connect(normAction, SIGNAL(triggered()), this, SLOT(slotNormalizeBetaIntensity()));
    connect(levelAddAction, SIGNAL(triggered()), this, SLOT(slotAddLevelEI()));
    connect(levelRemoveAction, SIGNAL(triggered()), this, SLOT(slotRemoveLevel()));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    addAction(normAction);
    addAction(levelAddAction);
    addAction(levelRemoveAction);
    addAction(closeAction);
//Mouse right click acctions END


// table initializaction with default values
    uiT->linePseudoLevEnStep->setText(QString::number(pseudoLevelEnergyStep_));
    uiT->linePseudoLevEnMin->setText(QString::number(pseudoLevelEnergyMin_));
    uiT->linePseudoLevEnMax->setText(QString::number(pseudoLevelEnergyMax_));
    uiT->linePseudoLevTotInten->setText(QString::number(pseudoLevelTotInten_));


    //Initialising Levels tables MOther,Daughter,GrandDaughter
    uiT->tableMotherLevels->setMinimumHeight(150);
    uiT->tableDaughterLevels->setMinimumHeight(150);
    uiT->tableGrandDaughterLevels->setMinimumHeight(150);
// Setting Labels
    //Mother
    uiT->labelMotherIsotope->setText("Isotope : ");
    uiT->labelMotherT12->setText("T1/2 :");
    uiT->labelMotherQvalue->setText("QBeta (keV) : ");
    uiT->labelMotherSpinParity->setText("I : ");
    //Daughter
    uiT->labelDaughterIsotope->setText("Isotope : ");
    uiT->labelDaughterT12->setText("T1/2 :");
    uiT->labelDaughterQvalue->setText("QBeta (keV) : ");
    //GrandDaughter
    uiT->labelGrandDaughterIsotope->setText("Isotope : ");
    uiT->labelGrandDaughteT12->setText("T1/2 :");
    uiT->labelGranDaughterQvalue->setText("QBeta (keV) : ");
}

DecayPathEditor::~DecayPathEditor()
{
    delete pseudoLevelsController_;
    g1->close();
    delete g1;
    delete uiT;
}


void DecayPathEditor::setComboBoxMethod()
{
    vector<string> methodList = pseudoLevelsController_->getIntensityMethodList();
    for (unsigned i=0; i< methodList.size(); i++)
    {
        QString qtext = QString::fromStdString(methodList.at(i));
        uiT->comboBoxIntensity->addItem(qtext);
    }

}

void DecayPathEditor::slotSetGammaIntensityMethod(QString qmethod)
{
    pseudoLevelsController_->setIntensityMethod(qmethod.toStdString());
    cout << "Gamma intensity method set to: " << qmethod.toStdString() << "read: " <<
            pseudoLevelsController_->getIntensityMethod() << endl;
    gammaIntensityMethod_ = qmethod.toStdString();

}


void DecayPathEditor::slotAddPseudoLevel()
{
//    gammaIntensityMethod_ = pseudoLevelsController_->getIntensityMethod();
    cout << "method: " <<  gammaIntensityMethod_ << endl;

    slotUpdatePseudoLevelData();


   if(pseudoLevelEnergyMax_<=pseudoLevelEnergyMin_)
      {
          int r = QMessageBox::warning(this, tr("Error"),
                                       tr("Max energy has to be higher than min energy"),
                                       QMessageBox::Ok);
          if (r == QMessageBox::Ok)
              return;
      }
   if(gammaIntensityMethod_ == "--choose--")
      {
          int r = QMessageBox::warning(this, tr("Error"),
                                       tr("Please first select method for gamma intensity calculation"),
                                       QMessageBox::Ok);
          if (r == QMessageBox::Ok)
              return;
      }
//    pseudoLevelsAdded = true;

//   pseudoLevelsController = new PseudoLevelsController(decay);
   int tabIndex = uiT->tabDecay->currentIndex();
   pseudoLevelsController_->setNuclideIndex(tabIndex);
   pseudoLevelsController_->addPseudoLevels(pseudoLevelEnergyStep_,pseudoLevelEnergyMin_,pseudoLevelEnergyMax_,
                                          pseudoLevelTotInten_, gammaIntensityMethod_);
   emit signalDecayPathEdited();
   emit signalUpdateTables();


}

void DecayPathEditor::slotUpdatePseudoLevelData()
{ //reading values from GUI
    pseudoLevelEnergyStep_ = uiT->linePseudoLevEnStep->text().toDouble();
    pseudoLevelEnergyMin_ = uiT->linePseudoLevEnMin->text().toDouble();
    pseudoLevelEnergyMax_ = uiT->linePseudoLevEnMax->text().toDouble();
    pseudoLevelTotInten_ = uiT->linePseudoLevTotInten->text().toDouble();

    if(uiT->radioStatisticalModelApply->isChecked()){
        ifStatModel_ = true;
    } else {
        ifStatModel_ =false;
    }
    std::cout << pseudoLevelTotInten_ << "  " << ifStatModel_ << std::endl;

}

void DecayPathEditor::slotRemoveLevel()
{
    int tabIndex = uiT->tabDecay->currentIndex();
    DecayPath* decayPath = DecayPath::get();
    QTableWidget *pointerToTable_=0L;
    if(tabIndex ==0){
        pointerToTable_ = uiT->tableMotherLevels;
    } else if(tabIndex == 1){
        pointerToTable_ =uiT->tableDaughterLevels;
    } else if(tabIndex == 2){
        pointerToTable_ = uiT->tableGrandDaughterLevels;
    } else {
        QMessageBox msgBox;
        msgBox.setText("Nuclide not definde");
        msgBox.exec();
    }

    QModelIndexList selection = pointerToTable_->selectionModel()->selectedRows();

    QString qenergy;
    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        qDebug() << index.row();
        QTableWidgetItem* itmE = pointerToTable_->item( index.row(), 0 );
        if (itmE)
        qenergy = itmE->text();

        double levelEnergy = qenergy.toDouble();
        decayPath->GetAllNuclides()->at(tabIndex).RemoveLevel(levelEnergy);

        int currentLevel = g1->getCurrentLevel();
        if(currentLevel > index.row()) g1->setCurrentLevel(currentLevel-1);
        if(currentLevel == index.row()) g1->close();

    }
    emit signalDecayPathEdited();
    emit signalUpdateTables();
}


void DecayPathEditor::slotAddLevelEI()
{
    std::cout << "LevelEditor::slotAddLevel" << std::endl;
    bool ok;
    DecayPath* decayPath = DecayPath::get();
    int tabIndex = uiT->tabDecay->currentIndex();
    double QValue;
    //if(tabIndex >0)QValue = decayPath->GetAllNuclides()->at(tabIndex-1).GetNuclideLevels()->at(0).GetTransitions()->at(0)->GetTransitionQValue();
    if(tabIndex >0) QValue = decayPath->GetAllNuclides()->at(tabIndex-1).GetQBeta();
    if(tabIndex == 0) QValue = 20000; //setting largeenough to have all decaing isomers in mothernuclei

    QString text = QInputDialog::getText(this, tr("New Level"),
                                          tr("Energy(keV : Intensity(%)"), QLineEdit::Normal,
                                          "energy:intensity", &ok);
    QStringList stringList= text.split(":",QString::SkipEmptyParts);

    for (int i = 0; i < stringList.size(); i++) qDebug(stringList.at(i).toUtf8());
    double energy(-1);
    double intensity(101);
    if (ok && stringList.size() == 2 )
     {
         energy = stringList.at(0).toDouble();
         intensity = stringList.at(1).toDouble();
         qDebug() << energy << ": " << intensity << text ;
     } else {
         QMessageBox msgBox;
         msgBox.setText("Data not correct - no changes applied");
         msgBox.exec();
         return;
     }
     if(intensity > 100 || energy <= 0 || energy > QValue){
         QMessageBox msgBox;
         msgBox.setText("Energy or Intensity data not correct - no changes applied");
         msgBox.exec();
         energy = 0;
         intensity = 0;
         return;
     }
    decayPath->GetAllNuclides()->at(tabIndex).AddLevelEI(energy,intensity/100.);

    emit signalDecayPathEdited();
    emit signalUpdateTables();

}

void DecayPathEditor::slotNormalizeBetaIntensity()
{
    std:cout << "LevelEditor::slotNormalizeBetaIntensity" << std::endl;

    DecayPath* decayPath= DecayPath::get();
    ResponseFunction* responseFunction = ResponseFunction::get();

    int tabIndex = uiT->tabDecay->currentIndex();
    if(tabIndex == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Can not normalise beta feeding for this nuclei. Check tab index - no changes applied");
        msgBox.exec();
        return;
    }

    std::vector<Nuclide>* nuclides = decayPath->GetAllNuclides();
    std::vector<Level>* motherLevels;
    Nuclide* motherNuclide;

    motherNuclide = &nuclides->at(tabIndex-1);
    motherLevels = motherNuclide->GetNuclideLevels();

    Level* motherLevel = &motherLevels->at(0);

    motherLevel->NormalizeTransitionIntensities();
    responseFunction->UpdateWholeContainerIntensities();
    responseFunction->RefreshFlags();

    emit signalUpdateTables();
//    uiT->tableFeeding->update();
    setTotalIntensityLabel();
}


void DecayPathEditor::setTotalIntensityLabel()
{
    double maxBetaFeeding(0);
    DecayPath* decayPath= DecayPath::get();
    int tabIndex = uiT->tabDecay->currentIndex();
    if(tabIndex == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Can not normalise beta feeding for this nuclei. Check tab index - no changes applied");
        msgBox.exec();
        return;
    }

    std::vector<Nuclide>* nuclides = decayPath->GetAllNuclides();
    std::vector<Level>* motherLevels;
    Nuclide* motherNuclide;

    motherNuclide = &nuclides->at(tabIndex-1);
    motherLevels = motherNuclide->GetNuclideLevels();

    Level* motherLevel = &motherLevels->at(0);

    maxBetaFeeding = motherLevel->GetTotalIntensity();
//            NormalizeTransitionIntensities();

/*    std::vector <Level*> levels = decayPath->GetAllLevels();

    for (unsigned int i = 0; i != levels.size(); ++i)
    {
        maxBetaFeeding += levels.at(i)->GetBetaFeedingFunction();
    }
*/    maxBetaFeeding =maxBetaFeeding*100;
    QString QTotalIntensity_ = QString::number(maxBetaFeeding);
    QString qstr = "Total Beta feeding intensity (%): "+ QTotalIntensity_;
    uiT->labelTotalIntensity->setText(qstr);
}

/*Eva
void LevelEditor::slotRecalculateDECSpectrum(bool)
{
// DOES nothing
    int histId = std::stoi(myProject->expSpecID());

}

void LevelEditor::slotUpdateFeedingData(int row, int column)
{
//    std::cout << "In slotUpdateFeedingData : " << std::endl;
    Decay* decay= Decay::get();
    std::vector <Level*> levels = decay->GetAllLevels();

    QString qstr = uiT->tableFeeding->item(row,column)->text();
//    qDebug() << "przeczytano: " << qstr << "row: "<<row<<"column: "<<column;
    string str = qstr.toUtf8().constData();
    if(column==0){
//        std::cout << " nie zmieniamy energii poziomu" << std::endl;
        return;
     }   else if(column==1){
         double feeding = qstr.toDouble()/100;
//         qDebug() << "przed ustawieniem: " << feeding <<"bylo " <<levels.at(row)->GetBetaFeedingFunction();
         levels.at(row)->SetBetaFeedingFunction(feeding);
//         std::cout << "ustawiono na : " << levels.at(row)->GetBetaFeedingFunction()<< std::endl;
        }
         else if (column==2){
//        std::cout << "We do not change this parameter" << std::endl;
        return;
    } else if (column==3){
//        std::cout << "We do not change this parameter" << std::endl;
        return;
      } else {
//        std::cout << "inne columny" << std::endl;
        return;
    }
    setTotalIntensityLabel();
}
*/
void DecayPathEditor::slotEditTransitions(int tableIndex, int rowIndex)
{
    g1->setTransitionEditorOpen(false);
    g1->setComboBoxMethod();
    QStringList header;
    std::vector<RowData> rowData_;
    TableController* tableController_ = new TableController();
    DecayPath* decayPath= DecayPath::get();

    std::vector<Nuclide>* nuclides_=decayPath->GetAllNuclides();

        cout << "table index: " << tableIndex << endl;
        cout << "row INdex: "  << rowIndex << endl;
    std::vector<Level>* levels_ = nuclides_->at(tableIndex).GetNuclideLevels();
    QString QEnergy_ = QString::number(levels_->at(rowIndex).GetLevelEnergy());
    QString qstr = "Transtion Editor for Level: "+ QEnergy_ + "keV in Z="
                    + nuclides_->at(tableIndex).GetAtomicNumber()  + " nuclide";
//    g1->setTransitionLabel("TEst");

    g1->setTransitionLabel(qstr);
    g1->setCurrentLevel(rowIndex);
    g1->setCurrentNuclide(tableIndex);

    std::vector<Transition*>* transitions_ = levels_->at(rowIndex).GetTransitions();

    if(transitions_->size() != 0)
    {
        rowData_.clear();

        for(unsigned int i = 0; i != transitions_->size(); ++i)
        {
            QString QTransitionType = QString::fromStdString(transitions_->at(i)->GetParticleType());
            QString QEnergy_ = QString::number(transitions_->at(i)->GetTransitionQValue());
            QString QIntensity_ = QString::number((transitions_->at(i)->GetIntensity() * 100.));
            QString QFitFlag_ = transitions_->at(i)->GetIntensityFitFlag() ? "true" : "false";
            QString QFinalLevel_ = QString::number(transitions_->at(i)->GetFinalLevelEnergy());

            rowData_.push_back(RowData(QTransitionType, QEnergy_, QIntensity_, QFitFlag_, QFinalLevel_));

        }
    }
    else
    {
        rowData_.clear();
        rowData_.push_back(RowData("n/s", "no transitions", "n/a", "n/a", "n/a"));
    }

    tableController_->initializeTable(g1->getUiPointer(), rowData_);
    header << "Type" << "Energy (keV)" << "Intensity[%]" << "FreeParam"<< "Final Level En.";
    tableController_->setHeader(g1->getUiPointer(), header);

    delete tableController_;

    g1->setTransitionEditorOpen(true);
    g1->setTotalIntensityLabel();
    g1->show();
}

void DecayPathEditor::slotUpdateTablesForward()
{
    emit signalUpdateTables();
}


