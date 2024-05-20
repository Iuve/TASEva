#include <DeclareHeaders.hh>
#include "pseudoLevelsController.h"
#include "transitionEditor.h"
#include "decayPathEditor.h"
#include "ui_DecayPathEditor.h"
#include "tablecontroller.h"
#include "responsefunction.h"
#include "PeriodicTable.hh"

#include "QDebug"
#include <QDialog>
#include <QInputDialog>
#include <QAction>


class DialogOptionsWidget;

DecayPathEditor::DecayPathEditor(QWidget *parent) :
    QWidget(parent),
    uiT(new Ui::DecayPathEditor)
{
    qInfo() << "DecayPathEditor::DecayPathEditor " ;
    uiT->setupUi(this);
    myProject = Project::get();
    g1 = new TransitionEditor();
    DecayPath* decayPath = DecayPath::get();
    pseudoLevelsController_ =  new PseudoLevelsController();

    setComboBoxMethod();
    setComboBoxDeExPath();

    uiT->tableDaughterLevels->setToolTip("Double click on the row number to get the transition editor open");
    connect(uiT->tableMotherLevels->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(slotEditMTransitions(int)));
    connect(uiT->tableDaughterLevels->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(slotEditDTransitions(int)));
    connect(uiT->tableGrandDaughterLevels->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(slotEditGDTransitions(int)));

//EVa     connect(uiT->tableFeeding, SIGNAL(cellChanged(int,int)),  SLOT(slotUpdateFeedingData(int,int)) );
    connect(uiT->tableDaughterLevels, SIGNAL(cellClicked(int,int)), this, SLOT(slotDaughterTableCellClicked(int,int)));
    connect(uiT->tableDaughterLevels, SIGNAL(cellChanged(int,int)), this, SLOT(slotDaughterTableChanged(int,int)));
    connect(uiT->buttonClose, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(uiT->buttonNormBetaFeeding, SIGNAL(clicked(bool)), this, SLOT(slotNormalizeBetaIntensity()));
    connect(uiT->buttonAddLevel, SIGNAL(clicked(bool)), this, SLOT(slotAddLevelEI()));
    connect(uiT->buttonRemoveLevel, SIGNAL(clicked(bool)), this, SLOT(slotRemoveLevel()));
    connect(g1, SIGNAL(signalUpdateTransitionTable(int,int )), this, SLOT(slotEditTransitions(int,int)));
    connect(g1, SIGNAL(signalUpdateLevelTable()),this, SLOT(slotUpdateTablesForward()));
    connect(uiT->buttonAddPseudoLevel, SIGNAL(clicked(bool)), this, SLOT(slotAddPseudoLevel()));
    connect(uiT->comboBoxGammaInten, SIGNAL(currentTextChanged(QString)), this, SLOT(slotSetGammaIntensityMethod(QString)));
    connect(uiT->comboBoxGammaPath, SIGNAL(currentTextChanged(QString)),this, SLOT(slotSetGammaPathMethod(QString)));
    connect(uiT->comboBoxParticleInten, SIGNAL(currentTextChanged(QString)), this, SLOT(slotSetParticleIntensityMethod(QString)));
    connect(uiT->comboBoxParticlePath, SIGNAL(currentTextChanged(QString)),this, SLOT(slotSetParticlePathMethod(QString)));
    connect(uiT->comboBoxDeExcitationPath  , SIGNAL(currentTextChanged(QString)), this, SLOT(slotSetDeExcitationPath(QString)));
    connect(uiT->comboBoxGammaParticleRatio, SIGNAL(currentTextChanged(QString)), this, SLOT(slotSetGammaParticleIntensityRatio(QString)));
    connect(uiT->buttonApplyPathAndInten, SIGNAL(clicked(bool)), this, SLOT(slotApplyPathAndInten(bool)) );
    connect(uiT->buttonMultiplyNeutronIntensity, SIGNAL(clicked(bool)), this, SLOT(slotMultiplyNeutronIntensity()));

    connect(g1, SIGNAL(signalTransitionsEdited()), this, SIGNAL(signalDecayPathEdited()));


//Mouse right click actions
    setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* FittingStatusTrueAction_ = new QAction("Set Fitting status TRUE");
    QAction* FittingStatusFalseAction_ = new QAction("Set Fitting status FALSE");
    QAction* levelAddAction = new QAction("Add level");
    QAction* levelRemoveAction = new QAction("Remove level");
    QAction* normAction = new QAction("Norm. Beta intensity");
    QAction* closeAction = new QAction("Cancel");
    connect(FittingStatusTrueAction_, SIGNAL(triggered()), this, SLOT(slotFittingStatusTrue()));
    connect(FittingStatusFalseAction_, SIGNAL(triggered()), this, SLOT(slotFittingStatusFalse()));
    connect(normAction, SIGNAL(triggered()), this, SLOT(slotNormalizeBetaIntensity()));
    connect(levelAddAction, SIGNAL(triggered()), this, SLOT(slotAddLevelEI()));
    connect(levelRemoveAction, SIGNAL(triggered()), this, SLOT(slotRemoveLevel()));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    addAction(FittingStatusTrueAction_);
    addAction(FittingStatusFalseAction_);
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

    uiT->tableMotherLevels->setMinimumHeight(150);
    uiT->tableDaughterLevels->setMinimumHeight(150);
    uiT->tableGrandDaughterLevels->setMinimumHeight(150);

    uiT->labelIntensityModel->setEnabled(false);
    uiT->labelParticle->setEnabled(false);
    uiT->labelParticle->setText("Particle");
    uiT->comboBoxParticleInten->setEnabled(false);
    uiT->comboBoxParticlePath->setEnabled(false);
    uiT->labelGamma->setEnabled(false);
    uiT->comboBoxGammaInten->setEnabled(false);
    uiT->comboBoxGammaPath->setEnabled(false);
    uiT->labelRatio->setEnabled(false);
    uiT->comboBoxGammaParticleRatio->setEnabled(false);
    uiT->buttonApplyPathAndInten->setEnabled(false);
    uiT->buttonAddPseudoLevel->setEnabled(false);



//    InitLabels();
    boolDaughterTableDataInitialised = true;
}

DecayPathEditor::~DecayPathEditor()
{
    delete pseudoLevelsController_;
    g1->close();
    delete g1;
    delete uiT;
}


void DecayPathEditor::InitLabels()
{
    //Initialising Levels tables MOther,Daughter,GrandDaughter

    DecayPath* decayPath= DecayPath::get();

    for (unsigned int i=1; i<3; i++)
    {
    std::vector<Nuclide>* nuclides_=decayPath->GetAllNuclides();
    std::vector<Level>* levels_ = nuclides_->at(i).GetNuclideLevels();
    int atomicNumber_ = nuclides_->at(i).GetAtomicNumber();
    QString QAtomicNumber = QString::number(atomicNumber_);
    QString QMassNumber = QString::number(nuclides_->at(i).GetAtomicMass());
    QString QElementName = QString::fromStdString(PeriodicTable::GetAtomicName(atomicNumber_));
    QString QQBeta = QString::number(nuclides_->at(i).GetQBeta());
    QString QSn = QString::number(nuclides_->at(i).GetSn());
// Setting Labels
    //Mother
    if( i == 0)
    {
    uiT->labelMotherIsotope->setText("Isotope : " +QElementName+"-"+QMassNumber);
    uiT->labelMotherT12->setText("T1/2 :");
    uiT->labelMotherQvalue->setText("QBeta (keV) = "+QQBeta);
    uiT->labelMotherSpinParity->setText("I : ");
    } else if (i==1)
    {//Daughter
    uiT->labelDaughterIsotope->setText("Isotope : "+QElementName+"-"+QMassNumber);
    uiT->labelDaughterT12->setText("T1/2 :");
    uiT->labelDaughterQvalue->setText("QBeta (keV) = "+QQBeta);
    uiT->labelDaughterSn->setText("Sn (keV) = "+QSn);
    } else if (i==2)
        {//GrandDaughter
    uiT->labelGrandDaughterIsotope->setText("Isotope : ");
    uiT->labelGrandDaughterT12->setText("T1/2 :");
    uiT->labelGrandDaughterQValue->setText("QBeta (keV) : ");
    }
    }
}

void DecayPathEditor::setComboBoxDeExPath()
{
//    QStringList  List = {"--chooose--", "G", "N", "G+N",  "P"};

//    uiT->comboBoxDeExcitationPath->addItems(List);

    vector<QString> mainPathMethodList = pseudoLevelsController_->getMainPathMethodList();
    vector<QString> mainPathMethodListToolTip = pseudoLevelsController_->getMainPathMethodListToolTip();
    for (unsigned i=0; i< mainPathMethodList.size(); i++)
    {
        uiT->comboBoxDeExcitationPath->addItem(mainPathMethodList.at(i));
        uiT->comboBoxDeExcitationPath->setItemData(i, mainPathMethodListToolTip.at(i), Qt::ToolTipRole);
    }

    uiT->comboBoxGammaInten->setEnabled(false);
    uiT->comboBoxGammaPath->setEnabled(false);
    uiT->comboBoxParticleInten->setEnabled(false);
    uiT->comboBoxParticlePath->setEnabled(false);
    uiT->comboBoxGammaParticleRatio->setEnabled(false);
}


void DecayPathEditor::setComboBoxMethod()
{
    // Sets initial values for comboBoxs
    vector<QString> gammaIntenMethodList = pseudoLevelsController_->getGammaIntensityMethodList();
    vector<QString> gammaIntenMethodListToolTip = pseudoLevelsController_->getGammaIntensityMethodListToolTip();
    for (unsigned i=0; i< gammaIntenMethodList.size(); i++)
    {
        uiT->comboBoxGammaInten->addItem(gammaIntenMethodList.at(i));
        uiT->comboBoxGammaInten->setItemData(i, gammaIntenMethodListToolTip.at(i), Qt::ToolTipRole);
    }
    vector<QString> gammaPathMethodList = pseudoLevelsController_->getGammaPathMethodList();
    vector<QString> gammaPathMethodListToolTip = pseudoLevelsController_->getGammaPathMethodListToolTip();
    for (unsigned i=0; i< gammaPathMethodList.size(); i++)
    {
        uiT->comboBoxGammaPath->addItem(gammaPathMethodList.at(i));
        uiT->comboBoxGammaPath->setItemData(i, gammaPathMethodListToolTip.at(i), Qt::ToolTipRole);
    }
    vector<QString> particleIntenMethodList = pseudoLevelsController_->getParticleIntensityMethodList();
    vector<QString> particleIntenMethodListToolTip = pseudoLevelsController_->getParticleIntensityMethodListToolTip();
    for (unsigned i=0; i< particleIntenMethodList.size(); i++)
    {
        uiT->comboBoxParticleInten->addItem(particleIntenMethodList.at(i));
        uiT->comboBoxParticleInten->setItemData(i, particleIntenMethodListToolTip.at(i), Qt::ToolTipRole);
    }
    vector<QString> particlePathMethodList = pseudoLevelsController_->getParticlePathMethodList();
    vector<QString> particlePathMethodListToolTip = pseudoLevelsController_->getParticlePathMethodListToolTip();
    for (unsigned i=0; i< particlePathMethodList.size(); i++)
    {
 //       QString qtext = QString::fromStdString(particlePathMethodList.at(i));
        uiT->comboBoxParticlePath->addItem(particlePathMethodList.at(i));
        uiT->comboBoxParticlePath->setItemData(i, particlePathMethodListToolTip.at(i), Qt::ToolTipRole);
    }


}

void DecayPathEditor::slotSetGammaIntensityMethod(QString qmethod)
{
    pseudoLevelsController_->setGammaIntensityMethod(qmethod.toStdString());
    cout << "Gamma intensity method set to: " << qmethod.toStdString() << "read: " <<
            pseudoLevelsController_->getGammaIntensityMethod() << endl;
//    gammaIntensityMethod_ = qmethod.toStdString();
}

void DecayPathEditor::slotSetGammaPathMethod(QString qmethod)
{
    vector<QString> gammaPathMethodList_ = pseudoLevelsController_->getGammaPathMethodList();
    if (qmethod == gammaPathMethodList_.at(0))
    {
        uiT->comboBoxGammaInten->setEnabled(false);
    } else
    {
        uiT->comboBoxGammaInten->setEnabled(true);
    }
}
void DecayPathEditor::slotSetParticleIntensityMethod(QString qmethod)
{
    pseudoLevelsController_->setParticleIntensityMethod(qmethod.toStdString());
    cout << "Particle intensity method set to: " << qmethod.toStdString() << "read: " <<
            pseudoLevelsController_->getParticleIntensityMethod() << endl;
//    particleIntensityMethod_ = qmethod.toStdString();
}

void DecayPathEditor::slotSetParticlePathMethod(QString qmethod)
{
    vector<QString> particlePathMethodList_ = pseudoLevelsController_->getParticlePathMethodList();
    if (qmethod == particlePathMethodList_.at(0))
    {
        uiT->comboBoxParticleInten->setEnabled(false);
    } else
    {
        uiT->comboBoxParticleInten->setEnabled(true);
    }
}
void DecayPathEditor::slotSetDeExcitationPath(QString particle)
{
    if (particle == "N")
    {
        uiT->labelParticle->setEnabled(true);
        uiT->labelParticle->setText("Neutron");
        uiT->comboBoxParticlePath->setEnabled(true);
        uiT->comboBoxParticlePath->setCurrentIndex(0);
        uiT->comboBoxParticleInten->setEnabled(false);
        uiT->comboBoxParticleInten->setCurrentIndex(0);
        uiT->labelGamma->setEnabled(false);
        uiT->comboBoxGammaInten->setEnabled(false);
        uiT->comboBoxGammaPath->setEnabled(false);
        uiT->labelRatio->setEnabled(false);
        uiT->comboBoxGammaParticleRatio->setEnabled(false);
        uiT->buttonApplyPathAndInten->setEnabled(true);
        uiT->labelIntensityModel->setEnabled(true);

    } else if (particle == "G") {
        uiT->labelParticle->setEnabled(false);
        uiT->labelParticle->setText("Particle");
        uiT->comboBoxParticleInten->setEnabled(false);
        uiT->comboBoxParticlePath->setEnabled(false);
        uiT->labelGamma->setEnabled(true);
        uiT->comboBoxGammaPath->setEnabled(true);
        uiT->comboBoxGammaInten->setEnabled(false);
        uiT->comboBoxGammaPath->setCurrentIndex(0);
        uiT->comboBoxGammaInten->setCurrentIndex(0);
        uiT->labelRatio->setEnabled(false);
        uiT->comboBoxGammaParticleRatio->setEnabled(false);
        uiT->buttonApplyPathAndInten->setEnabled(true);
        uiT->labelIntensityModel->setEnabled(true);


    } else if (particle == "G+N") {
        uiT->labelParticle->setEnabled(true);
        uiT->labelParticle->setText("Neutron");
        uiT->comboBoxParticlePath->setEnabled(true);
        uiT->comboBoxParticleInten->setEnabled(false);
        uiT->comboBoxParticlePath->setCurrentIndex(0);
        uiT->comboBoxParticleInten->setCurrentIndex(0);
        uiT->labelGamma->setEnabled(true);
        uiT->comboBoxGammaPath->setEnabled(true);
        uiT->comboBoxGammaInten->setEnabled(false);
        uiT->comboBoxGammaPath->setCurrentIndex(0);
        uiT->comboBoxGammaInten->setCurrentIndex(0);
        uiT->labelRatio->setEnabled(true);
        uiT->comboBoxGammaParticleRatio->setEnabled(true);
        uiT->buttonApplyPathAndInten->setEnabled(true);
        uiT->labelIntensityModel->setEnabled(true);

    } else if (particle == "P") {
        uiT->comboBoxParticleInten->setEnabled(true);
        uiT->labelParticle->setText("Proton");
        uiT->comboBoxParticlePath->setEnabled(true);
        uiT->comboBoxParticleInten->setEnabled(false);
        uiT->comboBoxParticlePath->setCurrentIndex(0);
        uiT->comboBoxParticleInten->setCurrentIndex(0);
        uiT->labelGamma->setEnabled(false);
        uiT->comboBoxGammaInten->setEnabled(false);
        uiT->comboBoxGammaPath->setEnabled(false);
        uiT->labelRatio->setEnabled(false);
        uiT->comboBoxGammaParticleRatio->setEnabled(false);
        uiT->buttonApplyPathAndInten->setEnabled(false);
        uiT->labelIntensityModel->setEnabled(false);

    }  else {
        uiT->labelParticle->setEnabled(false);
        uiT->labelParticle->setText("Particle");
        uiT->comboBoxParticleInten->setEnabled(false);
        uiT->comboBoxParticlePath->setEnabled(false);
        uiT->comboBoxParticlePath->setCurrentIndex(0);
        uiT->labelGamma->setEnabled(false);
        uiT->comboBoxGammaInten->setEnabled(false);
        uiT->comboBoxGammaPath->setEnabled(false);
        uiT->comboBoxGammaPath->setCurrentIndex(0);
        uiT->labelRatio->setEnabled(false);
        uiT->comboBoxGammaParticleRatio->setEnabled(false);
        uiT->buttonApplyPathAndInten->setEnabled(false);
        uiT->labelIntensityModel->setEnabled(false);

    }

}
void DecayPathEditor::slotSetGammaParticleIntensityRatio(QString)
{

}

void DecayPathEditor::slotApplyPathAndInten(bool flag)
{
    if (!flag)  //reading all the setings
    {
//        QString pathMain_ = uiT->comboBoxDeExcitationPath->currentText();

        pseudoLevelsController_->setPathMainMethod(uiT->comboBoxDeExcitationPath->currentText());
        pseudoLevelsController_->setGammaPathMethod(uiT->comboBoxGammaPath->currentText());
        pseudoLevelsController_->setParticlePathMethod(uiT->comboBoxParticlePath->currentText());
        pseudoLevelsController_->setGammaIntensityMethod(uiT->comboBoxGammaInten->currentText());
        pseudoLevelsController_->setParticleIntensityMethod(uiT->comboBoxParticleInten->currentText());

        pathMain_ = pseudoLevelsController_->getPathMainMethod();
        std::string intenGamma_ = pseudoLevelsController_->getGammaIntensityMethod();
        std::string intenParticle_ = pseudoLevelsController_->getParticleIntensityMethod();
        std::string pathParticle_ = pseudoLevelsController_->getParticlePathMethod();
        std::string pathGamma_ = pseudoLevelsController_->getGammaPathMethod();

        uiT->buttonAddPseudoLevel->setEnabled(true);
// what is it for?  to check that all necessary data are supply by the user i.e no "--choose--"
 /*      if (pathMain_ == "--choose--")
       {

       } else if(pathMain_ == "G")
       {

       } else if(pathMain_ == "N")
        {
           if (pathParticle_ == "GS only")
           {

           } else if (pathParticle_ == "FE only")
           {

           } else if (pathParticle_ == "GS+FE")
           {

           } else if (pathParticle_ == "All allowed")
           {

           }
       } else if(pathMain_ == "G+N")
       {
           if (pathParticle_ == "GS only")
           {

           } else if (pathParticle_ == "FE only")
           {

           } else if (pathParticle_ == "GS+FE")
           {

           } else if (pathParticle_ == "All allowed")
           {

           }
       }
*/
    }
}
void DecayPathEditor::slotAddPseudoLevel()
{
    //gammaIntensityMethod_ = pseudoLevelsController_->getIntensityMethod();
    //cout << "method: " <<  gammaIntensityMethod_ << endl;

    ResponseFunction* responseFunction = ResponseFunction::get();

    slotUpdatePseudoLevelData(); // updates pseudleveldata
    slotApplyPathAndInten(false);   // updates path and intensity

   if(pseudoLevelEnergyMax_<=pseudoLevelEnergyMin_)
      {
          int r = QMessageBox::warning(this, tr("Error"),
                                       tr("Max energy has to be higher than min energy"),
                                       QMessageBox::Ok);
          if (r == QMessageBox::Ok)
              return;
      }
/*   if(gammaIntensityMethod_ == "--choose--"
           || particleIntensityMethod_ == "--choose--")
      {
          int r = QMessageBox::warning(this, tr("Error"),
                                       tr("Please first select method for gamma/particle intensity calculation"),
                                       QMessageBox::Ok);
          if (r == QMessageBox::Ok)
              return;
      }
*/
 //    pseudoLevelsAdded = true;

//   pseudoLevelsController = new PseudoLevelsController(decay);
   int tabIndex = uiT->tabDecay->currentIndex();
   pseudoLevelsController_->setNuclideIndex(tabIndex);
   pseudoLevelsController_->addPseudoLevels(pseudoLevelEnergyStep_,pseudoLevelEnergyMin_,pseudoLevelEnergyMax_,
                                          pseudoLevelTotInten_, pathMain_); //, gammaIntensityMethod_, pseudoLevNeutronE_);
   // to powinnismy zmienic na decay type i intensity method

   emit signalDecayPathEdited();
   responseFunction->UpdateStructure();
   emit signalUpdateTables();


}

void DecayPathEditor::slotUpdatePseudoLevelData()
{ //reading values from GUI
    pseudoLevelEnergyStep_ = uiT->linePseudoLevEnStep->text().toDouble();
    pseudoLevelEnergyMin_ = uiT->linePseudoLevEnMin->text().toDouble();
    pseudoLevelEnergyMax_ = uiT->linePseudoLevEnMax->text().toDouble();
    pseudoLevelTotInten_ = uiT->linePseudoLevTotInten->text().toDouble();
//    pseudoLevNeutronE_ = uiT->linePseudoLevNeutronE->text().toDouble();
    //    pseudoLevSn_ = uiT->linePseudoLevSn->text().toDouble();

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
    ResponseFunction* responseFunction = ResponseFunction::get();
    int tabIndex = uiT->tabDecay->currentIndex();
    double QValue;
    //if(tabIndex >0)QValue = decayPath->GetAllNuclides()->at(tabIndex-1).GetNuclideLevels()->at(0).GetTransitions()->at(0)->GetTransitionQValue();
    if(tabIndex >0) QValue = decayPath->GetAllNuclides()->at(tabIndex-1).GetQBeta();
    if(tabIndex == 0) QValue = 20000; //setting largeenough to have all decaying isomers in mothernuclei

    QString text = QInputDialog::getText(this, tr("New Level"),
                                          tr("Energy(keV) : Feeding Intensity(%)"), QLineEdit::Normal,
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
    decayPath->GetAllNuclides()->at(tabIndex).AddLevelEnergyInten(energy,intensity/100.);

    emit signalDecayPathEdited();
    responseFunction->UpdateStructure();
    emit signalUpdateTables();

}
void DecayPathEditor::slotNormalizeBetaIntensity()
{
    std::cout << "LevelEditor::slotNormalizeBetaIntensity" << std::endl;

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
    //responseFunction->UpdateWholeContainerIntensities();
    responseFunction->UpdateMotherLevelBetaIntensities(motherLevel);
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

        qDebug() << "table index: " << tableIndex ;
        qDebug() << "row INdex: "  << rowIndex ;
    std::vector<Level>* levels_ = nuclides_->at(tableIndex).GetNuclideLevels();
    QString QEnergy_ = QString::number(levels_->at(rowIndex).GetLevelEnergy());
    int atomicNumber_ = nuclides_->at(tableIndex).GetAtomicNumber();
    QString QAtomicNumber = QString::number(atomicNumber_);
    QString QMassNumber = QString::number(nuclides_->at(tableIndex).GetAtomicMass());
    QString QElementName = QString::fromStdString(PeriodicTable::GetAtomicName(atomicNumber_));
    QString qstr = "Transtion Editor for Level: "+ QEnergy_ + " keV in "
            + QElementName + "-" +QMassNumber +
            " (Z= "+  QAtomicNumber + ") nuclide";
    qDebug() << qstr;
    g1->setTransitionLabel(qstr);
    g1->setCurrentLevel(rowIndex);
    g1->setCurrentNuclide(tableIndex);

    std::vector<Transition*>* transitions_ = levels_->at(rowIndex).GetTransitions();

    if(transitions_->size() != 0)
    {
        rowData_.clear();

        qDebug() << "Transitions_ size = "  << transitions_->size() ;
        for(unsigned int i = 0; i != transitions_->size(); ++i)
        {
            QString QTransitionType = QString::fromStdString(transitions_->at(i)->GetParticleType());
            QString QEnergy_ = QString::number(transitions_->at(i)->GetTransitionQValue());
            QString QIntensity_ = QString::number((transitions_->at(i)->GetIntensity() * 100.));            QString QFitFlag_ = transitions_->at(i)->GetIntensityFitFlag() ? "true" : "false";
            QString QFinalLevel_ = QString::number(transitions_->at(i)->GetFinalLevelEnergy());
            qDebug() << QTransitionType <<  QEnergy_ << QIntensity_<< QFitFlag_ << QFinalLevel_ ;
            rowData_.push_back(RowData(QTransitionType, QEnergy_, QIntensity_, QFitFlag_, QFinalLevel_));

        }
    }
    else
    {
        rowData_.clear();
        rowData_.push_back(RowData("n/a", "no transitions", "n/a", "n/a", "n/a"));
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

void DecayPathEditor::setColumnStatus(bool status, int column)
{
//similar to setColumnStatus in ManualFitGraph
    QTableWidget *pointerToTable_ = 0L;
    pointerToTable_ = uiT->tableDaughterLevels;

    QModelIndexList selection = pointerToTable_->selectionModel()->selectedRows();

    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
//        pointerToTable_->setItem(index.row(),column, new QTableWidgetItem(status ? "true": "false"));
        slotStatusClicked(status, index.row(),column);
    }

}
void DecayPathEditor::slotStatusClicked(bool status, int row, int column)
{
     QTableWidget *pointerToTable_ = 0L;
     QTableWidgetItem* tempQTableWidgetItem = new QTableWidgetItem(status ? "true" : "false");
     pointerToTable_ = uiT->tableDaughterLevels;
     QString qstatus = pointerToTable_->item(row,column)->text();
     if((column == 2) && (qstatus != "n/a")){
     pointerToTable_->setItem(row, 2, tempQTableWidgetItem);
     pointerToTable_->show();
    }

    //delete tempQTableWidgetItem;
}
void DecayPathEditor::slotDaughterTableCellClicked(int row, int column)
{
    qDebug() << "DecayPathEditor::slotDaughterTableCellClicked";

    bool status;
    if(column ==0)
    { return;
    } else if(column ==2 ){
        QString qstatus = uiT->tableDaughterLevels->item(row,column)->text();
        if(qstatus == "true") status = false;
        if(qstatus == "false") status = true;
      slotStatusClicked(status, row, column);
    }}

void DecayPathEditor::slotDaughterTableChanged(int row,int column)
{
    qDebug() << "DecayPathEditor::slotDaughterTableChange";
    if(boolDaughterTableDataInitialised){ //wydaje sie jakby bylo ustawione na true ???

        ResponseFunction* responseFunction = ResponseFunction::get();

        DecayPath* decayPath= DecayPath::get();
        std::vector<Nuclide>* nuclides_ = decayPath->GetAllNuclides();
        std::vector<Level>* motherLevels_ = nuclides_->at(0).GetNuclideLevels();
        std::vector<Transition*>* betaTransitions = motherLevels_->at(0).GetTransitions();


        QString qstr = uiT->tableDaughterLevels->item(row,column)->text();
        string str = qstr.toUtf8().constData();
        QString qstr_energy = uiT->tableDaughterLevels->item(row,0)->text();
        double levelEnergy = qstr_energy.toDouble();
        qDebug() << "LevelEnergy= " << levelEnergy << "new value " << qstr;
        int transitionIndex = 0;
        bool transitionExists = "false";
        for(auto i=0; i!=betaTransitions->size(); ++i)
        {
            if(levelEnergy == betaTransitions->at(i)->GetFinalLevelEnergy())
            {
                qDebug() << levelEnergy <<" " << betaTransitions->at(i)->GetFinalLevelEnergy() ;
                transitionIndex = i+1;
                transitionExists = "true";
            }
         }
        if(transitionIndex == 0){return;} else {transitionIndex-- ;}
        qDebug() << "transitionIndex =" << transitionIndex << "Bool TransitionExists = " << transitionExists ;
        if(transitionExists)
        { // ========szukam tutaj========
        if(column==0){ //Energy
            return;
         } else if (column==1){ //BetaFeeding
            double feeding = qstr.toDouble()/100;
            betaTransitions->at(transitionIndex)->ChangeIntensity(feeding);
            responseFunction->ChangeContainerDaughterLevelIntensity( betaTransitions->at(transitionIndex)->GetPointerToFinalLevel(), feeding );
            responseFunction->RefreshFlags();
                    return;


        }  else if(column==2){
            if(qstr=="true")betaTransitions->at(transitionIndex)->SetIntensityFitFlag(true);
            if(qstr=="false")betaTransitions->at(transitionIndex)->SetIntensityFitFlag(false);
            return;
        }  else if(column==3) {
                    return;
        }  else if(column==4) {
                    return;
        }else {

            return;
        }
        }
    }
}

void DecayPathEditor::slotMultiplyNeutronIntensity()
{

    ResponseFunction* responseFunction = ResponseFunction::get();
    DecayPath* decayPath= DecayPath::get();
    std::vector<Nuclide>* nuclides_ = decayPath->GetAllNuclides();
    std::vector<Level>* motherLevels_ = nuclides_->at(0).GetNuclideLevels();
    std::vector<Transition*>* betaTransitions = motherLevels_->at(0).GetTransitions();

    double intensityMultiplier = uiT->lineMultiplyNeutronIntensity->text().toDouble();

    for(auto it = betaTransitions->begin(); it != betaTransitions->end(); ++it)
    {
        Level* tempFinalLevel = (*it)->GetPointerToFinalLevel();
        if(tempFinalLevel->GetNeutronLevelStatus())
        {
            double currentIntensity = (*it)->GetIntensity();
            (*it)->ChangeIntensity(intensityMultiplier * currentIntensity);
        }

    }

        Level* motherLevel = &motherLevels_->at(0);
        motherLevel->NormalizeTransitionIntensities();
        responseFunction->UpdateMotherLevelBetaIntensities(motherLevel);
        responseFunction->RefreshFlags();

        emit signalUpdateTables();
        setTotalIntensityLabel();

}
