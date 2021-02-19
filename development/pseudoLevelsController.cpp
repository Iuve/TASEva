#include "DeclareHeaders.hh"
#include "pseudoLevelsController.h"
#include "Exceptions.h"
#include "qdebug.h"


#include "QDebug"
#include <QDialog>
#include <QMessageBox>
class DialogOptionsWidget;

PseudoLevelsController::PseudoLevelsController(DecayPath* decay)
{
    std::cout<<"PseudoLevelsController::PseudoLevelsController(Decay* decayVal)"<<std::endl;
    try
    {
        decayPath_= decay;
//TasEva        levelDensity_ = new LevelDensity();
        createIntensityMethodList();
    }
    catch(GenError e)
    {
        throw e;
    }
}


void PseudoLevelsController::createIntensityMethodList()
{
    intensityMethodList_.push_back("--choose--");
    intensityMethodList_.push_back("Equal");
    intensityMethodList_.push_back("AllE1");
    intensityMethodList_.push_back("AllE2");
    intensityMethodList_.push_back("AllM1");
    intensityMethodList_.push_back("AllM2");
}

std::vector<string> PseudoLevelsController::getIntensityMethodList()
{
    return intensityMethodList_;
}


void PseudoLevelsController::addPseudoLevels(double stepEnergy, double minEn, double maxEn, double totInt,  string gammaModel)
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
    } else if (intensityMethod_ == intensityMethodList_.at(5)) //AllM@
    {   addSimplePseudoLevels();
        addRemainingTransition(intensityMethod_);
    } else cout << "method not on the list " << endl;
}


/*Eva
void PseudoLevelsController::applyStatisticalModel()
{
   ifStatisticalModel_ = true;
   std::cout<<"PseudoLevelsController::applyStatisticalModel() "<< deltaE_ << " "
          << minEnergy_<<" "<< maxEnergy_<<" "<< ifStatisticalModel_<<std::endl;
}
*/
void PseudoLevelsController::addSimplePseudoLevels()
{
    std::cout<<"void PseudoLevelsController::addSimplePseudoLevels()"<<endl;
    int nrOfPseudolevels = (int)(maxEnergy_ - minEnergy_)/ deltaE_;
    double singleIntensity = totIntensity_/static_cast<float> (nrOfPseudolevels);

    for(int i =0; i<nrOfPseudolevels; i++)
    {
     double levelEnergy = minEnergy_ + i*deltaE_;
     decayPath_->GetAllNuclides()->at(currentNuclideIndex_).AddLevelEI(levelEnergy,singleIntensity/100.);
    }
}

void PseudoLevelsController::addRemainingTransition(string method)
{
      int nrOfPseudolevels = (int)(maxEnergy_ - minEnergy_)/ deltaE_;
    std::vector<Level> *levels = decayPath_->GetAllNuclides()->at(currentNuclideIndex_).GetNuclideLevels();
    double atomicMass = static_cast<double>(decayPath_->GetAllNuclides()->at(currentNuclideIndex_).GetAtomicMass());
    for(auto it=levels->begin(); it< levels->end(); it++)  //loop over all levels to locate ne pseudolevels
    {
        double energy = it->GetLevelEnergy();
        if(energy < minEnergy_)continue;  //fast out for levels below minEnergy_
        for (int i=0; i<nrOfPseudolevels; i++)  //looking for pseudolevel matching (it) pointer
        {
            double plevelEnergy = minEnergy_ + i*deltaE_;
            if(plevelEnergy == energy )  // found pseudolevel
            {
                it->setAsPseudoLevel();
                for(auto ik=levels->begin(); ik != it; ik++) //scaning for final levels
                {

                    if(ik->isPseudoLevel())continue;  //out for other pseudolevels
                double intensity;
                double transitionEnergy = plevelEnergy - ik->GetLevelEnergy();
                    if(transitionEnergy <= 0)continue;
                if (method == "Equal"){
                    intensity = 1;
                }
                else if(method == "AllE1"){
                    intensity = getE1Intensity(atomicMass, transitionEnergy);
                } else if (method == "AllE2")
                {
                    intensity = getE2Intensity(atomicMass, transitionEnergy);
                } else if (method == "AllM1")
                {
                    intensity = getM1Intensity(atomicMass, transitionEnergy);
                } else if (method == "AllM2")
                {
                    intensity = getM2Intensity(atomicMass, transitionEnergy);
                } else {
                    intensity = 1; }
                if(ik->GetLevelEnergy() == 0)
                {
                    cout << "found level 0 " << ik->GetLevelEnergy() << endl;
                    std::vector<Transition*> *transitions_ = it->GetTransitions();
                    for(auto ikk=transitions_->begin(); ikk<transitions_->end(); ikk++)
                    {
                        double transitionEnergy = (*ikk)->GetTransitionQValue();
                        if(transitionEnergy == energy)(*ikk)->ChangeIntensity(intensity);
                    }

                }else {
                it->AddTransition("G",transitionEnergy, intensity);}
                }
            }
        }
        it->NormalizeTransitionIntensities();
    }
}

/*Eva

std::vector<SpinAndParity> PseudoLevelsController::findSpinAndParity(Level* parentLevel, double energy)
{
    char parentParity = parentLevel->GetParity();
    int parentSpinNumerator = parentLevel->GetSpinNumerator();
    int parentSpinDenominator = parentLevel->GetSpinDenominator();
    std::vector<SpinAndParity> allSpins;

    //levelDensity
    int atomicMass = decay_->GetAtomicMass();
    int atomicNr = decay_->GetAtomicNumber();


    SpinAndParity spin = SpinAndParity(parentSpinNumerator+ parentSpinDenominator,
                                       parentSpinDenominator, parentParity, 1);
    float intensity = levelDensity_->getDensity (energy, atomicMass, atomicNr, spin);

    allSpins.push_back(SpinAndParity(parentSpinNumerator+ parentSpinDenominator,
                                     parentSpinDenominator, parentParity, intensity));
    if(parentSpinNumerator == 0)
        return allSpins;
    else
    {
       spin = SpinAndParity(parentSpinNumerator, parentSpinDenominator, parentParity, 1);
       intensity = levelDensity_->getDensity (energy, atomicMass, atomicNr, spin);
       allSpins.push_back(SpinAndParity(parentSpinNumerator,
                                        parentSpinDenominator, parentParity, intensity));


        spin = SpinAndParity(parentSpinNumerator- parentSpinDenominator,
                                           parentSpinDenominator, parentParity, 1);
        intensity = levelDensity_->getDensity (energy, atomicMass, atomicNr, spin);
        allSpins.push_back(SpinAndParity(parentSpinNumerator - parentSpinDenominator,
                                         parentSpinDenominator, parentParity, intensity));
        return allSpins;
    }
}

void PseudoLevelsController::setSpinAndParity(Level* parentLevel, Level* pseudoLevel)
{

    char parentParity = parentLevel->GetParity();
    int parentSpinNumerator = parentLevel->GetSpinNumerator();
    int parentSpinDenominator = parentLevel->GetSpinDenominator();

    double randomNumber = (double) rand() /  RAND_MAX ;
    int spinNumerator;
    if(parentSpinNumerator == 0 || randomNumber < 1./3.)
        spinNumerator = parentSpinNumerator + parentSpinDenominator;
    else if(randomNumber >= 1./3. && randomNumber < 2./3)
        spinNumerator = parentSpinNumerator;
    else
        spinNumerator = parentSpinNumerator - parentSpinDenominator;

    pseudoLevel->SetSpinAndParity(parentParity, spinNumerator, parentSpinDenominator);

}

double PseudoLevelsController::findTotalTransitionIntensity
        ( Level* finalLevel, double atomicMass, double energy, std::vector<SpinAndParity> allSpins)
{
    double totalIntensity;
    for(unsigned int i = 0; i != allSpins.size(); ++i)
    {
        totalIntensity += findTransitionIntensity(finalLevel, atomicMass, energy, allSpins.at(i))
                * allSpins.at(i).intensity_;

    }

    return totalIntensity;
}

double PseudoLevelsController::findTransitionIntensity
        (Level* finalLevel, double atomicMass, double energy, SpinAndParity initSpin)
{
    //find group of possible spins, calculate gamma transitions for all of them.
    int initSpinNumerator = initSpin.numerator_;
    int finalSpinDenominator = finalLevel->GetSpinDenominator();
    int finalSpinNumerator = finalLevel->GetSpinNumerator();
    int spinDiff = fabs(initSpinNumerator - finalSpinNumerator)/finalSpinDenominator;
    int spinSumm = fabs(initSpinNumerator + finalSpinNumerator)/finalSpinDenominator;
    char initParity = initSpin.parity_;
    char finalParity = finalLevel->GetParity();
    double transInt = 0;

    if(spinSumm >= 1 && spinDiff <= 1)
    {
        if(initParity == finalParity)
        {
            transInt += getM1Intensity(atomicMass, energy);

        }
        else
        {
            transInt += getE1Intensity(atomicMass, energy);
        }
    }
    if(spinSumm >= 2 && spinDiff <= 2)
    {
        if(initParity == finalParity)
        {
            transInt += getE2Intensity(atomicMass, energy);

        }
        else
        {
            transInt += getM2Intensity(atomicMass, energy);
        }
    }
    return transInt;
}
Eva */

double PseudoLevelsController::getE1Intensity(double atomicMass, double energy)
{
    double intensity = pow(energy/1000., 3.) *pow(10., 7.) * pow(atomicMass, 2./3.);
    //std::cout<<" E1: "<<intensity;
    return  intensity;
}

double PseudoLevelsController::getE2Intensity(double atomicMass, double energy)
{
    double intensity = 7.3 * pow(energy/1000., 5.) * pow(atomicMass, 4./3.);
    //std::cout<<" E2: "<<intensity;
    return  intensity;
}

double PseudoLevelsController::getM1Intensity(double atomicMass, double energy)
{
    double intensity = 5.6 * pow(energy/1000., 3.) *pow(10., 6.);
    //std::cout<<" M1: "<<intensity;
    return  intensity;
}

double PseudoLevelsController::getM2Intensity(double atomicMass, double energy)
{
    double intensity = 3.5 * pow(energy/1000., 5.) * pow(atomicMass, 2./3.);
    //std::cout<<" M2: "<<intensity;
    return  intensity;
}

/*
void PseudoLevelsController::addStatPseudoLevels()
{

}
TAsEva */
