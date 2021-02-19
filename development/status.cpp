#include "DeclareHeaders.hh"
#include "histogram.h"
#include "responsefunction.h"
#include "status.h"
#include "ui_status.h"
#include <QMessageBox>

Status::Status(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Status)
{

/*    DecayPath *decayPath_ = DecayPath::get();
    if(decayPath_==0L)
    {
        QMessageBox msgBox;
        msgBox.setText("First load decay path data");
        msgBox.exec();
       return;
    }
*/    ui->setupUi(this);

    connect(ui->buttonClose, SIGNAL(clicked(bool)), this,SLOT(close()));
//    connect(ui->buttonUpdate, SIGNAL(clicked(bool)), this,SLOT(signalUpdate()));

    // Set the number of columns in the tree
    ui->treeStatus->setColumnCount(2);

    // Set the headers
    ui->treeStatus->setHeaderLabels(QStringList() << "Object" << "Status");
    ui->treeStatus->setColumnWidth(0,250);
    update();
//    ui->treeStatus->expandAll();

}

Status::~Status()
{
    delete ui;
}
void Status::update()
{
    // Add root nodes
    addResponseFunctionRoot();
//    addTreeRoot("Response Function", "Root_first");
}

void Status::addResponseFunctionRoot()
{
    DecayPath* decayPath = DecayPath::get();
    ResponseFunction* responseFunction = ResponseFunction::get();

    // QTreeWidgetItem(QTreeWidget * parent, int type = Type)
    QTreeWidgetItem *respFunItem = new QTreeWidgetItem(ui->treeStatus);
    ui->treeStatus->expandItem(respFunItem);

    // QTreeWidgetItem::setText(int column, const QString & text)
    respFunItem->setText(0, "Response Function");
    respFunItem->setText(1, "Status of all response functions");
//flag 1
    QTreeWidgetItem *responseFuncFlag1 = new QTreeWidgetItem(respFunItem);
     responseFuncFlag1->setText(0, " allFilesReady");
     QString boolText1 = responseFunction->GetAllFilesReadyFlag() ? "true" : "false";
     responseFuncFlag1->setText(1, boolText1);
     if(boolText1=="true")responseFuncFlag1->setBackground(1, QBrush(QColor("green")));
     if(boolText1=="false")responseFuncFlag1->setBackground(1, QBrush(QColor("red")));
//flag 2
     QTreeWidgetItem *responseFuncFlag2 = new QTreeWidgetItem(respFunItem);
      responseFuncFlag2->setText(0, " respFunctionReady");
      QString boolText2 = responseFunction->GetResponseFunctionReady() ? "true" : "false";
      responseFuncFlag2->setText(1, boolText1);
      if(boolText2=="true")responseFuncFlag2->setBackground(1, QBrush(QColor("green")));
      if(boolText2=="false")responseFuncFlag2->setBackground(1, QBrush(QColor("red")));



    std::vector<Nuclide>* nuclides = decayPath->GetAllNuclides();
    Nuclide* motherNuclide;
    std::vector<Level>* motherLevels;

    motherNuclide = &nuclides->at(0);
    motherLevels = motherNuclide->GetNuclideLevels();
    bool motherLevelsAll(true);
    for(unsigned itn = 0;  itn !=motherLevels->size();  ++itn)
    {

    MotherLevelRespContainer* motherLevelRespCont = responseFunction->GetPointerToMotherLevelResp(&motherLevels->at(itn));
      QTreeWidgetItem *motherLevelItem = new QTreeWidgetItem(respFunItem);
      ui->treeStatus->expandItem(motherLevelItem);
      QString qtext1;
      qtext1 = "A= " + QString::number(motherLevelRespCont->AtomicMass)
              + " N= " + QString::number(motherLevelRespCont->AtomicNumber)
              +" E= "+ QString::number(motherLevelRespCont->motherLevelEnergy);
      motherLevelItem->setText(0, qtext1);
//      motherLevelItem->setText(1, " ");
// flags for mother Level
//flag 1
      QTreeWidgetItem *motherLevelFlag1 = new QTreeWidgetItem(motherLevelItem);
       motherLevelFlag1->setText(0, " motherLevelFilesReady");
       QString boolText1 = motherLevelRespCont->motherLevelFilesReady ? "true" : "false";
       motherLevelFlag1->setText(1, boolText1);
       if(boolText1=="true")motherLevelFlag1->setBackground(1, QBrush(QColor("green")));
       if(boolText1=="false")motherLevelFlag1->setBackground(1, QBrush(QColor("red")));
// flag 2
       QTreeWidgetItem *motherLevelFlag2 = new QTreeWidgetItem(motherLevelItem);
        motherLevelFlag2->setText(0, " motherLevelResponseReady");
        QString boolText2 = motherLevelRespCont->motherLevelResponseReady ? "true" : "false";
        motherLevelFlag2->setText(1, boolText2);
        if(boolText2=="true")motherLevelFlag2->setBackground(1, QBrush(QColor("green")));
        if(boolText2=="false")motherLevelFlag2->setBackground(1, QBrush(QColor("red")));
// flag 3
        QTreeWidgetItem *motherLevelFlag3 = new QTreeWidgetItem(motherLevelItem);
         motherLevelFlag3->setText(0, " allDaughterLevelResponsesReady");
         QString boolText3 = motherLevelRespCont->allDaughterLevelResponsesReady ? "true" : "false";
         motherLevelFlag3->setText(1, boolText3);
         if(boolText3=="true")motherLevelFlag3->setBackground(1, QBrush(QColor("green")));
         if(boolText3=="false")motherLevelFlag3->setBackground(1, QBrush(QColor("red")));
 //        flags for levels
         bool levelsAll(true);
         for(auto itl= motherLevelRespCont->LevelResp_.begin(); itl !=motherLevelRespCont->LevelResp_.end(); ++itl)
         {
             QTreeWidgetItem *motherLevelLevel = new QTreeWidgetItem(motherLevelItem);
             ui->treeStatus->expandItem(motherLevelLevel);

              QString qtextl =  "Level @"+QString::number(itl->levelEnergy);
              motherLevelLevel->setText(0, qtextl);

              QTreeWidgetItem *motherLevelLevelFlag1 = new QTreeWidgetItem(motherLevelLevel);
              motherLevelLevelFlag1->setText(0, " levelFilesReady");
             QString boolText1 = itl->levelFilesReady ? "true" : "false";
             motherLevelLevelFlag1->setText(1, boolText1);
             if(boolText1=="true")motherLevelLevelFlag1->setBackground(1, QBrush(QColor("green")));
             if(boolText1=="false")motherLevelLevelFlag1->setBackground(1, QBrush(QColor("red")));
// flag 3
             QTreeWidgetItem *motherLevelLevelFlag3 = new QTreeWidgetItem(motherLevelLevel);
             motherLevelLevelFlag3->setText(0, " LevelResponseReady");
             QString boolText3 = itl->levelResponseReady  ? "true" : "false";
             motherLevelLevelFlag3->setText(1, boolText3);
             if(boolText3=="true")motherLevelLevelFlag3->setBackground(1, QBrush(QColor("green")));
             if(boolText3=="false")motherLevelLevelFlag3->setBackground(1, QBrush(QColor("red")));

             QTreeWidgetItem *motherLevelLevelFlag2 = new QTreeWidgetItem(motherLevelLevel);
             motherLevelLevelFlag2->setText(0, " allTransitionResponsesReady");
            QString boolText2 = itl->allTransitionResponsesReady ? "true" : "false";
            motherLevelLevelFlag2->setText(1, boolText2);
            if(boolText2=="true")motherLevelLevelFlag2->setBackground(1, QBrush(QColor("green")));
            if(boolText2=="false")motherLevelLevelFlag2->setBackground(1, QBrush(QColor("red")));

            bool transitionAll(true);
            for(auto itt=itl->TransitionResp.begin(); itt!=itl->TransitionResp.end(); ++itt)
            {
                QTreeWidgetItem *levelTransition = new QTreeWidgetItem(motherLevelLevel);

                ui->treeStatus->expandItem(levelTransition);
                 QString qtextl =  "Transition @"+QString::number(itt->transitionEnergy);
                 levelTransition->setText(0, qtextl);

                 QTreeWidgetItem *levelTransitionFlag1 = new QTreeWidgetItem(levelTransition);
                 levelTransitionFlag1->setText(0, " xmlFilesCorrect");
                QString boolText1 = itt->xmlFilesCorrect ? "true" : "false";
                levelTransitionFlag1->setText(1, boolText1);
                if(boolText1=="true")levelTransitionFlag1->setBackground(1, QBrush(QColor("green")));
                if(boolText1=="false")levelTransitionFlag1->setBackground(1, QBrush(QColor("red")));

                QTreeWidgetItem *levelTransitionFlag2 = new QTreeWidgetItem(levelTransition);
                levelTransitionFlag2->setText(0, " simulationDone");
               QString boolText2 = itt->simulationDone ? "true" : "false";
               levelTransitionFlag2->setText(1, boolText2);
               if(boolText2=="true")levelTransitionFlag2->setBackground(1, QBrush(QColor("green")));
               if(boolText2=="false")levelTransitionFlag2->setBackground(1, QBrush(QColor("red")));

               QTreeWidgetItem *levelTransitionFlag3 = new QTreeWidgetItem(levelTransition);
               levelTransitionFlag3->setText(0, " sortingDone");
              QString boolText3 = itt->sortingDone ? "true" : "false";
              levelTransitionFlag3->setText(1, boolText3);
              if(boolText3=="true")levelTransitionFlag3->setBackground(1, QBrush(QColor("green")));
              if(boolText3=="false")levelTransitionFlag3->setBackground(1, QBrush(QColor("red")));

              QTreeWidgetItem *levelTransitionFlag4 = new QTreeWidgetItem(levelTransition);
              levelTransitionFlag4->setText(0, " transitionResponseReady");
             QString boolText4 = itt->transitionResponseReady ? "true" : "false";
             levelTransitionFlag4->setText(1, boolText4);
             if(boolText4=="true")levelTransitionFlag4->setBackground(1, QBrush(QColor("green")));
             if(boolText4=="false")levelTransitionFlag4->setBackground(1, QBrush(QColor("red")));
                if(itt->xmlFilesCorrect && itt->simulationDone && itt->sortingDone && itt->transitionResponseReady)
                {
                    levelTransition->setText(1,"All OK");
                    levelTransition->setBackground(1,QBrush(QColor("green")));
                    ui->treeStatus->collapseItem(levelTransition);
                }
             transitionAll = transitionAll && itt->transitionResponseReady;

            }


         if(transitionAll && itl->levelResponseReady)
             {
             motherLevelLevel->setText(1,"All OK");
             motherLevelLevel->setBackground(1,QBrush(QColor("green")));
             ui->treeStatus->collapseItem(motherLevelLevel);
             }
         levelsAll = levelsAll && itl->levelResponseReady;
         }
         if(levelsAll && motherLevelRespCont->motherLevelResponseReady)
         {
          motherLevelItem->setText(1,"All levels OK");
          motherLevelItem->setBackground(1,QBrush(QColor("green")));
          ui->treeStatus->collapseItem(motherLevelItem);

         }
         motherLevelsAll = motherLevelsAll && motherLevelRespCont->motherLevelResponseReady
                                           && motherLevelRespCont->allDaughterLevelResponsesReady;

    }

    if(motherLevelsAll)
    {
        respFunItem->setText(1, "All  OK");
        respFunItem->setBackground(1,QBrush(QColor("green")));
        ui->treeStatus->collapseItem(respFunItem);
    }

//    addTreeChild(treeItem, name + "A", "Child_first");
//    addTreeChild(treeItem, name + "B", "Child_second");
}

void Status::addTreeChild(QTreeWidgetItem *parent,
                  QString name, QString description)
{
    // QTreeWidgetItem(QTreeWidget * parent, int type = Type)
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();

    // QTreeWidgetItem::setText(int column, const QString & text)
    treeItem->setText(0, name);
    treeItem->setText(1, description);

    // QTreeWidgetItem::addChild(QTreeWidgetItem * child)
    parent->addChild(treeItem);
}
