#include "DeclareHeaders.hh"
#include "deexcitationModelController.h"
#include "Exceptions.h"
#include <vector>
#include <string>

using namespace std;

DeexcitationModelController::DeexcitationModelController()
{

    try
    {
        createIntensityMethodList();
    }
    catch(GenError e)
    {
        throw e;
    }
}

DeexcitationModelController::~DeexcitationModelController()
{

}

void DeexcitationModelController::createIntensityMethodList()
{
    intensityMethodList_.clear();
    intensityMethodList_.push_back("--choose--");
    intensityMethodList_.push_back("ModelM1");
}
/*
void DeexcitationModelController::applyDeexcitationModel(double stepEnergy, double minEn, double maxEn, double totInt,  string gammaModel)
{

    double QValue = decayPath_->GetAllNuclides()->at(0).GetNuclideLevels()->at(0).GetTransitions()->at(0)->GetTransitionQValue();

    if(QValue <  maxEn )
    {
        QMessageBox msgBox;
        msgBox.setText("Max pseudolevel energy cannt be higher than Q-Beta");
        int r = msgBox.exec();
//       int r = QMessageBox::warning(this, tr("Information"),
//                                     tr("Please first select method for gamma intensity calculation"),
//                                     QMessageBox::Ok);
        if (r == QMessageBox::Ok)
            return;
    }
    //decay_->RemoveAllPseudoLevels();
    deltaE_ = stepEnergy;
    minEnergy_ = minEn;
    maxEnergy_ = maxEn;
    totIntensity_ = totInt;
    if( intensityMethod_ != gammaModel ) cout << " Model Problem " << endl;
//    ifStatisticalModel_ =  ifStatModel;

    std::cout<<"PseudoLevelsController::addPseudoLevels() "<<
            minEnergy_<<" "<< maxEnergy_<<" "<< deltaE_<< " " << totIntensity_ << " " <<ifStatisticalModel_ <<std::endl;
    if(intensityMethod_ == intensityMethodList_.at(0))
    {

        QMessageBox msgBox;
        msgBox.setText("Please first select method for gamma intensity calculation");
        msgBox.exec();
//       int r = QMessageBox::warning(this, tr("Information"),
//                                     tr("Please first select method for gamma intensity calculation"),
//                                     QMessageBox::Ok);
//        if (r == QMessageBox::Ok)
            return;
    } else if (intensityMethod_ == intensityMethodList_.at(1)) //EqualIntensity
    {
        addSimplePseudoLevels();
        addRemainingTransition(intensityMethod_);
    } else if (intensityMethod_ == intensityMethodList_.at(2))  //AllE1
    {   addSimplePseudoLevels();
        addRemainingTransition(intensityMethod_);
    } else if (intensityMethod_ == intensityMethodList_.at(3))  //AllE2
    {   addSimplePseudoLevels();
        addRemainingTransition(intensityMethod_);
    } else if (intensityMethod_ == intensityMethodList_.at(4)) //AllM1
    {   addSimplePseudoLevels();
        addRemainingTransition(intensityMethod_);
    } else if (intensityMethod_ == intensityMethodList_.at(5)) //AllM2
    {   addSimplePseudoLevels();
        addRemainingTransition(intensityMethod_);
    } else if (intensityMethod_ == intensityMethodList_.at(6)) //87Br
    {   addCustomPseudoLevels();
    } else if (intensityMethod_ == intensityMethodList_.at(7)) //ModelM1
    {   applyModelM1();
    } else cout << "method not on the list " << endl;
}
*/
