#include "DeclareHeaders.hh"
#include "twoDimFitController.h"
#include "MyTemplates.h"
#include "responsefunction.h"

#include <QFuture>
#include <QtConcurrentRun>

TwoDimFitController::TwoDimFitController(QWidget *parent)
{
    decayPath = DecayPath::get();
    myProject = Project::get();

    decayPath->FindAndMarkNeutronLevels();
    otherLevelsToFeedingsRatio_ = 0;
    normalizeStartPoint_ = 100;

    //expSpectra2Dbinning_ should be defferent from 1 only when exp 2D spectrum is binned
    expSpectra2Dbinning_ = myProject->getBinning2Dfactor();
    //lines below are needed to fit/work without contributions from other levels
    otherLevelsResponse_ = Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);
    myProject->setGate2DOtherLevelsContribution(otherLevelsResponse_);
    neutronLevelsResponse_ = Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);

}

TwoDimFitController::~TwoDimFitController()
{
}

void TwoDimFitController::setExperimentalHistogram(Histogram* hist)
{
    expGate_ = hist;
    setExperimentalVector();
}

void TwoDimFitController::findCorrespondingLevel()
{
    Level *finalLevel = 0L;
    double tmpFeeding = 0.0;
    double minDeltaEnergy = energy_;

    std::vector<Transition*>* betaTransitions_ = decayPath->GetBetaTransitionsFromFirstNuclide();
    std::vector <bool>  futureResults_;
    for(auto it = betaTransitions_->begin(); it != betaTransitions_->end(); ++it)
    {
        if(minDeltaEnergy >= fabs(energy_ - (*it)->GetFinalLevelEnergy()))
        {
          minDeltaEnergy = fabs(energy_ - (*it)->GetFinalLevelEnergy());
          finalLevel = (*it)->GetPointerToFinalLevel();
          tmpFeeding = (*it)->GetIntensity();
        }
    }

    betaFeedingToLevel_ = tmpFeeding;
    level_ = finalLevel;
    myProject->setCurrent2DFitLevel(level_);
}

void TwoDimFitController::prepareRestLevelsResponseFromOutside()
{
    PrepareNeutronLevelsResponse();
    Histogram otherLevelsResp = prepareRestLevelsResponse();
    myProject->setGate2DOtherLevelsContribution(otherLevelsResp);
    otherLevelsResponse_ = myProject->getGate2DOtherLevelsContribution();
}

void TwoDimFitController::PrepareNeutronLevelsResponse()
{
    //WARNING: what if neutron level is the one fitted? All of neutrons levels would be
    //prepared anyway!!!
    // Function potentially for further development
    Histogram neutronLevelsResponse = *Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);
    std::vector<Transition*>* betaTransitions = decayPath->GetBetaTransitionsFromFirstNuclide();

    for(auto itb = betaTransitions->begin(); itb != betaTransitions->end(); ++itb)
    {
        Level* tmpLevel = (*itb)->GetPointerToFinalLevel();
        std::vector<Transition*>* transitionsFromLevel = tmpLevel->GetTransitions();
        Histogram neutronOneLevelResponse = *Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);

        if( tmpLevel->GetNeutronLevelStatus() )
        {
            for( auto it = transitionsFromLevel->begin(); it != transitionsFromLevel->end(); ++it)
            {
                    Histogram tmpGate = prepareTransitionResponse( (*it), tmpLevel );
                    neutronOneLevelResponse.Add( &tmpGate, (*it)->GetIntensity() );
            }
            neutronLevelsResponse.Add( &neutronOneLevelResponse, (*itb)->GetIntensity() );
        }
    }

    myProject->SetGate2DNeutronLevelsContribution(neutronLevelsResponse);
    neutronLevelsResponse_ = myProject->GetGate2DNeutronLevelsContribution();
}

Histogram TwoDimFitController::prepareRestLevelsResponse()
{
    //we take every level with energy above (energy_ - subtractFromEnergy), without THE level
    //double subtractFromEnergy = 500.;
    std::vector<Transition*>* transitions_ = decayPath->GetBetaTransitionsFromFirstNuclide();

    Histogram xGate = *Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_); // jesli zadziala zrobicporzadnie -AUTOMATYCZNIE

    for(auto it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        Level* tmpLevel = (*it)->GetPointerToFinalLevel();
        if( tmpLevel->GetNeutronLevelStatus() )
            continue;

        if( tmpLevel != level_ )
        {
            std::vector<Transition*>* transitionsFromLevel = tmpLevel->GetTransitions();
            //if( ( tmpLevel->GetLevelEnergy() > energy_ - subtractFromEnergy ) )
            if( ( tmpLevel->GetLevelEnergy() > 10. ) )
            {
                Histogram* levelXGate = Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);
                for(auto itt = transitionsFromLevel->begin(); itt != transitionsFromLevel->end(); ++itt)
                {
                   Histogram tmpGate = prepareTransitionResponse((*itt), tmpLevel);
                   levelXGate->Add(&tmpGate, (*itt)->GetIntensity());
                }
                 xGate.Add(levelXGate, (*it)->GetIntensity() );
            }
        }
    }

    xGate.Add( neutronLevelsResponse_, 1. );
    return xGate;
}

Histogram TwoDimFitController::prepareTransitionResponse(Transition* transition_, Level* tmpLevel_)
{
    cout << histId_ << " " << minEn_ << " " <<maxEn_ << endl;

    double levelEnergy = tmpLevel_->GetLevelEnergy();
    double transitionEnergy = transition_->GetTransitionQValue();
    QString levelDirName = QString("%1").arg(levelEnergy);
    QString transitionDirName = QString("%1").arg(transitionEnergy);
    QString dirName_ = levelDirName + "/" + transitionDirName;
    QDir directory(dirName_);
//    qDebug() << "directory w twoDimFit: " << directory   ;
    if(!directory.exists())
    {
        return Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);
    }
    ResponseFunction* respFunction = ResponseFunction::get();
    Histogram transResp = respFunction->loadXGate(directory, histId_, minEn_, maxEn_);
    // rebin for 2D fitting
    //transResp.Rebin(expSpectra2Dbinning_);

//EVaout?    Histogram* totalResp = new Histogram(levelsResp);
    // we multiply by betafeeding for the sake of correct background estimate
//    totalResp->Add(transResp, betaFeedingToLevel_);
    cout << "transition: " << transition_->GetTransitionQValue() << " has "  << transResp.GetNrOfCounts() << " counts " << endl;
    return transResp;
}

void TwoDimFitController::prepareTransitionResponses()
{
    responses.clear();
    vector<Transition*>* transitions_ = level_->GetTransitions();
    vector<Histogram> tmpGammaRespHist;
    int nr = 0;
    for(auto it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        //futureResults.push_back(QtConcurrent::run(this, &TwoDimFitController::prepareGammaResponse, (*it)));
        Histogram transitionResp = prepareTransitionResponse ( (*it), level_ );
        transitionResp.Scale( betaFeedingToLevel_ );
        tmpGammaRespHist.push_back(transitionResp);
        responses.push_back(transitionResp.GetAllDataD());
    }
    myProject->setTransitionResponseHist(tmpGammaRespHist);
    myProject->setTransitionResponseDouble(responses);
}

void TwoDimFitController::prepareFeedings()
{
    vector<Transition*>* transitions_ = level_->GetTransitions();
    feedings.clear();
    feedingsBeforeFit.clear();
    for(auto it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        feedings.push_back((*it)->GetIntensity());
        feedingsBeforeFit.push_back((*it)->GetIntensity());
        cout << (*it)->GetIntensity() << endl;
    }
}

void TwoDimFitController::calculateSimulatedHistogram()
{//   calculates reconstrusted spectrum: sum of transition response + contribution from other levels
    simGate_ = *Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);
    std::vector <Histogram>* gammaRespHist = myProject->getTransitionResponseHist();
  for(unsigned i = 0; i != gammaRespHist->size(); i++)
  {
      double intensity = feedings.at(i);
      simGate_.Add( &(gammaRespHist->at(i)), intensity );
  }

  calculateRecHistogram();
}

void TwoDimFitController::calculateRecHistogram()
{
      double xMin = normalizeStartPoint_;  // to avoid channel 0
      double xMax = maxEn_;
      recGate_ = *Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);

    double scaleRatio;
    if(otherLevelsToFeedingsRatio_ == 0.)
        otherLevelsToFeedingsRatio_ = otherLevelsResponse_->GetNrOfCounts(xMin,xMax) / ( simGate_.GetNrOfCounts(xMin,xMax) + otherLevelsResponse_->GetNrOfCounts(xMin,xMax) );

    scaleRatio = expGate_->GetNrOfCounts(xMin,xMax) / ( simGate_.GetNrOfCounts(xMin,xMax) + otherLevelsResponse_->GetNrOfCounts(xMin,xMax) );

    myProject->setGateNormFactor(scaleRatio);
    recGate_.Add( &simGate_, scaleRatio );
    recGate_.Add(otherLevelsResponse_, scaleRatio);
}

void TwoDimFitController::calculateDiffHistogram()
{
    diffHist_ = *Histogram::GetEmptyHistogram(0, 100, 100 / expSpectra2Dbinning_);
   double fact;
    fact = 1.0;
    diffHist_.Add( expGate_, fact );
    fact = -1.0;
    diffHist_.Add( &recGate_, fact );   //subtracting histograms

}

void TwoDimFitController::makeXGammaFit()
{
    if(experiment_.size() == 0)
        cout << "no experimental points!" << endl;
    findCorrespondingLevel();
    prepareFeedings();
//EVa    saveResponses();
    makeFit();
    findErrors();
    notifyObservers();
    printResults();

}

void TwoDimFitController::makeFit()
{

    int nrOfHistograms = feedings.size();
    int nrOfPoints = experiment_.size();
    cout << " nrOfHistograms to fit: " << nrOfHistograms << " nrOfPoints in exp.spec: " << nrOfPoints << endl;
    cout << " nrOfIterations: " << nrOfIter_ << endl;
    cout << " lambda: " << lambda_ << endl;
    cout << "responses.size: " << responses.size() << endl;
    cout << "binningFactor_ = " << binningFactor_ << endl;

    vector<double> experimentForFit = experiment_;
    vector<double> otherLevelsContribution = otherLevelsResponse_->GetAllDataD();
    std::vector< vector<double> > responsesForFit = responses;
    responsesForFit.push_back(otherLevelsContribution);

    int minPoint = normalizeStartPoint_;
    int maxPoint = nrOfPoints;
//    for(int j = 0; j != nrOfHistograms + 1; j++)
    for(int j = 0; j != nrOfHistograms; j++)
    {
        if( (responsesForFit.at(j)).size() < maxPoint )
            maxPoint = (responsesForFit.at(j)).size();
    }


    double expNorm = 0;
    for(int i = minPoint; i != maxPoint; i++)
        expNorm += experiment_.at(i);

    //Two ways to go with contribiutions:
    //FIRST: subtraction from experimental data, not fitted; normalization is tricky

    //preparing correct normalization
    double sumForNormalization = 0.;
    for(int i = 0; i < nrOfHistograms; ++i)
    {
        double responseNrOfCounts = 0.;
        for(int j = minPoint; j < maxPoint; j++)
            responseNrOfCounts += (responsesForFit.at(i)).at(j);
        sumForNormalization += responseNrOfCounts * feedings.at(i);
    }
    sumForNormalization += otherLevelsResponse_->GetNrOfCounts(minPoint,maxPoint);

    double responseMultiplier = expNorm / sumForNormalization;
    feedings.push_back( 1. );
    vector<double> oldFeedings = feedings;

    if(binningFactor_ > 1)
        BinForFitting(&experimentForFit, &responsesForFit);

    for(int it = 0; it != nrOfIter_; ++it)
    {
        //std::cout << "Running fitting iteration # " << it << "\r" << std::flush;

        //preparing correct normalization
        sumForNormalization = 0.;
        for(int i = 0; i < nrOfHistograms; ++i)
        {
            double responseNrOfCounts = 0.;
            for(int j = minPoint; j < maxPoint; j++)
                responseNrOfCounts += (responsesForFit.at(i)).at(j);
            sumForNormalization += responseNrOfCounts * feedings.at(i);
        }
        sumForNormalization += otherLevelsResponse_->GetNrOfCounts(minPoint,maxPoint);
        responseMultiplier = expNorm / sumForNormalization;


        double bigCheckSum = 0.;
        for(int j = 0; j != nrOfHistograms + 1; j++)
//        for(int j = 0; j != nrOfHistograms; j++)
        {
            double checkSum = 0.;
            double sum1 = 0.;

            for(int i = minPoint; i < maxPoint; i++)
            {
                double sum2 = 0.;
                for(int k = 0; k !=nrOfHistograms + 1; ++k)
//                for(int k = 0; k !=nrOfHistograms; ++k)
                {
 //std::cout << "Fitting iteration #it " << it << " Hist#j: "<< j << " bin#i: "<<i << " drugi Hist#k "<<k<<"\r" << std::flush;

                    sum2 += (responsesForFit.at(k)).at(i) * responseMultiplier * oldFeedings.at(k);
                }


                if(experimentForFit.at(i) > 0)
                {
                    sum1 += (responsesForFit.at(j)).at(i) * responseMultiplier * (experimentForFit.at(i) - sum2)/experimentForFit.at(i);  
                    checkSum += experimentForFit.at(i) - sum2;
                }
                else
                {
                    checkSum += experimentForFit.at(i) - sum2;
                }
            }
            if(j < nrOfHistograms)
            {
                feedings.at(j) = oldFeedings.at(j) * exp(2./lambda_ * sum1);
            }
            bigCheckSum += checkSum;
        }

        double feedingsDuringFitSum = 0.;
        for(int j = 0; j != nrOfHistograms; j++)
            feedingsDuringFitSum += feedings.at(j);

        for(int j = 0; j != nrOfHistograms; j++)
            feedings.at(j) /= feedingsDuringFitSum;

       oldFeedings = feedings;
    }

    feedings.pop_back();

    double feedingSum = 0.; //without other levels contribution
    for(unsigned i =0; i<feedings.size(); ++i)
    {
        //cout << feedings.at(i) << endl;
        feedingSum += feedings.at(i);
    }

    //normalization
    for(unsigned i =0; i < feedings.size(); ++i)
    {
        double tempFeeding = feedings.at(i) / feedingSum;
        feedings.at(i) = tempFeeding;
        //cout << feedings.at(i) << endl;
    }
}

void TwoDimFitController::BinForFitting(std::vector<double>* experimentForFit, std::vector< vector<double> >* responsesForFit)
{
    int k = 0;
    double binSum = 0.;
    for(int i = 0; i < experimentForFit->size(); i++)
    {
        binSum += experimentForFit->at(i);
        k++;
        if(k == binningFactor_)
        {
            k = 0;
            double averageCount = binSum / binningFactor_;
            for(int j = i; j != i - binningFactor_; j--)
                experimentForFit->at(j) = averageCount;
            binSum = 0.;
        }
    }

    for(int l = 0; l < responsesForFit->size(); l++)
    {
        k = 0;
        binSum = 0.;
        for(int i = 0; i < responsesForFit->at(l).size(); i++)
        {
            binSum += responsesForFit->at(l).at(i);
            k++;
            if(k == binningFactor_)
            {
                k = 0;
                double averageCount = binSum / binningFactor_;
                for(int j = i; j != i - binningFactor_; j--)
                    responsesForFit->at(l).at(j) = averageCount;
                binSum = 0.;
            }
        }
    }
}

void TwoDimFitController::findErrors()
{
    int nrOfHistograms = feedings.size();
    int nrOfPoints = experiment_.size();
    int maxPoint = nrOfPoints;
    for(int j = 0; j != nrOfHistograms; j++)
    {
        if( (responses.at(j)).size() < maxPoint )
            maxPoint = (responses.at(j)).size();
    }
    cout<<"errors calculation"<<endl;
    //errors in %:
    for(int i = 0; i !=nrOfHistograms; ++i )
    {
        double sum = 0;
        for(int k = 0; k != maxPoint; ++k)
            sum += (responses.at(i)).at(k)* (responses.at(i)).at(k)/experiment_.at(k);
        double error = pow(4./(lambda_* lambda_) * feedings.at(i)* feedings.at(i) * sum, 0.5);
        errors.push_back(error/feedings.at(i) * 100.);
    }
}

void TwoDimFitController::notifyObservers()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    vector<Transition*>* transitions_ = level_->GetTransitions();
    int transitionIndex = -1;

    for(auto it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        transitionIndex++;
        double newIntensity = feedings.at(transitionIndex);
        (*it)->ChangeIntensity(newIntensity);
        responseFunction->ChangeContainerTransitionIntensity( *it, newIntensity );
    }
    responseFunction->RefreshFlags();
}

void TwoDimFitController::printResults()
{
    vector<Transition*>* transitions_ = level_->GetTransitions();
    int gammaNr = 0;
    for(auto it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        cout << "Transition " << gammaNr
             << " type: "  << (*it)->GetParticleType()
             << " energy " << (*it)->GetTransitionQValue()
             << " was " << feedingsBeforeFit.at(gammaNr) * 100.
             << " is " << (*it)->GetIntensity() * 100.
             << " +- " << errors.at(gammaNr) << " [%]" << endl;
        gammaNr++;
    }

    /*
    cout << "Other lvls contribution "
         << " was " << feedingsBeforeFit.back() * 100.
         << " is " << feedings.back() * 100.
         << " +- " << errors.back() << " [%]" << endl;
    */

    cout << "Transition fit is done" << endl;
}

void TwoDimFitController::saveResponses()
{
/*    cout <<"TwoDimFitController::saveResponses() - POCZATEK" << endl;
    float energy = level_->GetLevelEnergy();
    string name =num2string(energy) + "_" + num2string(minEn_) + "_" + num2string(maxEn_);
    levelsOutputController *levelsOutController = new levelsOutputController(name, gammaRespHist.size()+1, histId_);
    Histogram* totalLevelResp = level_->GetXGate(histId_, minEn_, maxEn_);

       myProject->getTransitionResponseHist();

    cout <<"TwoDimFitController::saveResponses() - 1" << endl;
    int normalization = 1e8;
    totalLevelResp->Normalize(normalization);
    levelsOutController->saveHistogram(totalLevelResp, histId_);
    delete totalLevelResp;

    vector<Gamma*> gamma = level_->GetGammas();
    cout <<"TwoDimFitController::saveResponses() - 2 - size" << gammaRespHist.size() << endl;
    for (unsigned int i = 0; i != gammaRespHist.size(); ++i)
    {
        gammaRespHist.at(i)->Normalize(normalization*gamma.at(i)->GetTotalIntensity());
        levelsOutController->saveHistogram(gammaRespHist.at(i), i+1);

    }
    cout <<"TwoDimFitController::saveResponses() - 3" << endl;
    cout << "expHist przed wyslaniem" << expGate_->GetNrOfCounts() << endl;
    levelsOutController->saveHistogram(expGate_, gammaRespHist.size()+1);
    delete levelsOutController;
    cout <<"TwoDimFitController::saveResponses() - KONIEC" << endl;
*/}
