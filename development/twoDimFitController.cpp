#include "DeclareHeaders.hh"
#include "twoDimFitController.h"
#include "MyTemplates.h"
#include "responsefunction.h"

#include <QFuture>
#include <QtConcurrentRun>

TwoDimFitController::TwoDimFitController(QWidget *parent)
{
    decayPath= DecayPath::get();
    myProject = Project::get();

    //histogramVisibilityFactor_ = 1;
    decayPath->FindAndMarkNeutronLevels();
    otherLevelsToFeedingsRatio_ = 0;
}

TwoDimFitController::~TwoDimFitController()
{
    //if(levelsResp != 0L)
    //    delete levelsResp;
    //gammaRespHist->clear();
}

void TwoDimFitController::setExperimentalHistogram(Histogram* hist)
{
    expGate_ = hist;
    setExperimentalVector();
}

void TwoDimFitController::findCorrespondingLevel()
{
    //cout << "TwoDimFitController::findCorrespondingLevel() BEGIN" << endl;
    Level *finalLevel = 0L;
    double tmpFeeding = 0.0;
    double minDeltaEnergy = energy_;
    //cout << "energy_: " << energy_ << endl;
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

    //cout << "TwoDimFitController::findCorrespondingLevel() END" << endl;
}

void TwoDimFitController::prepareRestLevelsResponseFromOutside()
{
    cout << "==>TwoDimFitController::prepareRestLevelsResponseFromOutside() POCZATEK " << endl;
    PrepareNeutronLevelsResponse();
    Histogram otherLevelsResp = prepareRestLevelsResponse();
    myProject->setGate2DOtherLevelsContribution(otherLevelsResp);
    otherLevelsResponse_ = myProject->getGate2DOtherLevelsContribution();
    cout << "==>TwoDimFitController::prepareRestLevelsResponseFromOutside() KONIEC " << endl;
}

void TwoDimFitController::PrepareNeutronLevelsResponse()
{
    //WARNING: what if neutron level is the one fitted? All of neutrons levels would be
    //prepared anyway!!!
    // Function potentially for further development
    Histogram neutronLevelsResponse = *Histogram::GetEmptyHistogram();
    std::vector<Transition*>* betaTransitions = decayPath->GetBetaTransitionsFromFirstNuclide();

    for(auto itb = betaTransitions->begin(); itb != betaTransitions->end(); ++itb)
    {
        Level* tmpLevel = (*itb)->GetPointerToFinalLevel();
        std::vector<Transition*>* transitionsFromLevel = tmpLevel->GetTransitions();
        Histogram neutronOneLevelResponse = *Histogram::GetEmptyHistogram();

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
    //neutronResponseReady_ = true;
}

Histogram TwoDimFitController::prepareRestLevelsResponse()
{
    //Xgate!
    cout << "===>TwoDimFitController::prepareRestLevelsResponse() POCZATEK" << endl;

    //we take every level with energy above (energy_ - subtractFromEnergy), without THE level
    double subtractFromEnergy = 500.;
    std::vector<Transition*>* transitions_ = decayPath->GetBetaTransitionsFromFirstNuclide();

    Histogram xGate = *Histogram::GetEmptyHistogram(); // jesli zadziala zrobicporzadnie -AUTOMATYCZNIE

    for(auto it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        Level* tmpLevel = (*it)->GetPointerToFinalLevel();
        if( tmpLevel->GetNeutronLevelStatus() )
            continue;

        if( tmpLevel != level_ )
        {
            std::vector<Transition*>* transitionsFromLevel = tmpLevel->GetTransitions();
            if( ( tmpLevel->GetLevelEnergy() > energy_ - subtractFromEnergy ) )
            {
                Histogram* levelXGate = Histogram::GetEmptyHistogram();
                cout << "liczba przejsc w poziomie " << transitionsFromLevel->size() << endl;

                for(auto itt = transitionsFromLevel->begin(); itt != transitionsFromLevel->end(); ++itt)
                {
                   cout << "szukamy dla przejscia: " << (*itt)->GetTransitionQValue()
                         << " z poziomu " << tmpLevel->GetLevelEnergy()
                          << " i wolamay preapreTransitinoREsponse: " << endl;
                   Histogram tmpGate = prepareTransitionResponse((*itt), tmpLevel);
                   levelXGate->Add(&tmpGate, (*itt)->GetIntensity());
                   cout << "ADDING ONE TRANSITION COUNTS: " << tmpGate.GetNrOfCounts()
                         << " multiplied by (transition intensity):" <<   (*itt)->GetIntensity() << endl;
                }
                  cout << " So we have in levelXGate spectrum: " << levelXGate->GetNrOfCounts() << " counts."
                       << " to mulitiplied by (beta intensity) " << (*it)->GetIntensity() << endl;
                 xGate.Add(levelXGate, (*it)->GetIntensity() );
            }
        }
        cout << "TwoDimFitController::prepareRestLevelsResponse() FINISHED LEVEL: " << tmpLevel->GetLevelEnergy() << endl;
    }
    cout << "XGATE nr of Counts without neutrons contribution: " << xGate.GetNrOfCounts() << endl ;
    xGate.Add( neutronLevelsResponse_, 1. );
    cout << "FINAL XGATE nr of CouNTS: " << xGate.GetNrOfCounts() << endl ;
    cout << "===>TwoDimFitController::prepareRestLevelsResponse() KONIEC" << endl;

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
        return Histogram::GetEmptyHistogram();
    }
    ResponseFunction* respFunction = ResponseFunction::get();
    Histogram transResp = respFunction->loadXGate(directory, histId_, minEn_, maxEn_);
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
        cout << "gamma " << (*it)->GetTransitionQValue() << " has Nr " << nr++ << endl;

//        responses.push_back(gammaResp->GetAllData(minEn_, maxEn_));
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
    simGate_ = *Histogram::GetEmptyHistogram(); // jesli zadziala zrobicporzadnie -AUTOMATYCZNIE
    cout << "TwoDimFitController::calculateSimulatedHistogram - POCZATEK" << endl;

    //gammaRespHist->clear();
    std::vector <Histogram>* gammaRespHist = myProject->getTransitionResponseHist();
//    cout << "gammaRespHist.size = " << gammaRespHist.size() << " feedings.size() = " << feedings.size() << endl;
  for(unsigned i = 0; i != gammaRespHist->size(); i++)
  {
      double intensity = feedings.at(i);
      cout << "counts in gamma resp function: " << gammaRespHist->at(i).GetNrOfCounts() << endl;
      cout << "intensity: " << intensity << endl;
      simGate_.Add( &(gammaRespHist->at(i)), intensity );
  }
  cout << "Total number of Counts in SIM spectrum: " << simGate_.GetNrOfCounts() << endl;
  cout << "counts in levels resp function: " << otherLevelsResponse_->GetNrOfCounts() << endl;
  cout << "TwoDimFitController::calculateSimulatedHistogram - KONIEC" << endl;
  calculateRecHistogram();
}

void TwoDimFitController::calculateRecHistogram()
{
      cout << "TwoDimFitController::calculateRecHistogram - START" << endl;
      cout << "Total number of Counts in SIM spectrum: " << simGate_.GetNrOfCounts() << endl;
      cout << "counts in otherLevelsResponse_ function: " << otherLevelsResponse_->GetNrOfCounts() << endl;
      double xMin = 100;  // to avoid channel 0
      //double xMax = expGate_->GetXMax();
      double xMax = maxEn_;
      //float xMax = ui2D->lineDisplayXmax->text().toDouble() ;  //expHist->GetNrOfBins();
      recGate_ = *Histogram::GetEmptyHistogram();

      cout << "BEFORE CALCULATIONS OF SIMULATED AND RECONSTRUCTED SPECTRA:" << endl;
      cout << "xMin: " << xMin << endl;
      cout << "xMax: " << xMax << endl;
      cout << "simGate_.GetNrOfCounts(xMin,xMax): " << simGate_.GetNrOfCounts(xMin,xMax) << endl;
      cout << "otherLevelsResponse_->GetNrOfCounts(xMin,xMax): " << otherLevelsResponse_->GetNrOfCounts(xMin,xMax) << endl;
      cout << "expGate_->GetNrOfCounts(xMin,xMax): " << expGate_->GetNrOfCounts(xMin,xMax) << endl;

    double scaleRatio;
    if(otherLevelsToFeedingsRatio_ == 0.)
        otherLevelsToFeedingsRatio_ = otherLevelsResponse_->GetNrOfCounts(xMin,xMax) / ( simGate_.GetNrOfCounts(xMin,xMax) + otherLevelsResponse_->GetNrOfCounts(xMin,xMax) );

    scaleRatio = expGate_->GetNrOfCounts(xMin,xMax) / ( simGate_.GetNrOfCounts(xMin,xMax) + otherLevelsResponse_->GetNrOfCounts(xMin,xMax) );
    cout << "otherLevelsToFeedingsRatio_: " << otherLevelsToFeedingsRatio_ << endl;

    cout << "scaleRatio = " << scaleRatio << endl;
    myProject->setGateNormFactor(scaleRatio);
    //myProject->setGateOtherLevelsNormFactor(scaleRatio);
    recGate_.Add( &simGate_, scaleRatio );
    recGate_.Add(otherLevelsResponse_, scaleRatio);
    cout << "AFTER CALCULATIONS OF SIMULATED AND RECONSTRUCTED SPECTRA:" << endl;
    cout << "simGate_.GetNrOfCounts(xMin,xMax): " << simGate_.GetNrOfCounts(xMin,xMax) << endl;
    cout << "otherLevelsResponse_->GetNrOfCounts(xMin,xMax): " << otherLevelsResponse_->GetNrOfCounts(xMin,xMax) << endl;
    cout << "recGate_.GetNrOfCounts(xMin,xMax): " << recGate_.GetNrOfCounts(xMin,xMax) << endl;
    cout << "expGate_->GetNrOfCounts(xMin,xMax): " << expGate_->GetNrOfCounts(xMin,xMax) << endl;

    cout << "TwoDimFitController::calculateRecedHistogram - STOP" << endl;
}

void TwoDimFitController::calculateDiffHistogram()
{
    cout << "TwoDimFitController::calculatedDiffHistogram() - POCZATEK" << endl;
    //cout << "EXP-wlasciwosci" << expGate_->GetXMin()<<" " << expGate_->GetXMax() <<" " << expGate_->GetNrOfBins() << endl;
//    diffHist_->Histogram::GetEmptyHistogram(expHist->GetXMin(), expHist->GetXMax(), expHist->GetNrOfBins());
    diffHist_ = *Histogram::GetEmptyHistogram(); // jesli zadziala zrobicporzadnie -AUTOMATYCZNIE
    //double xMin = 10.0;
    //double xMax = 4000;
    //double expNorm_ = expGate_->GetNrOfCounts(xMin,xMax);
    //cout << "DiffHisto: Norm exp spec 0-4000: " << expNorm_ << endl;
   double fact;
    fact = 1.0;
    diffHist_.Add( expGate_, fact );
    fact = -1.0;
    diffHist_.Add( &recGate_, fact );   //subtracting histograms
    cout << " liczba zliczen w diff: " << diffHist_.GetNrOfCounts() << endl;
    cout << "TwoDimFitController::calculatedDiffHistogram() - KONIEC" << endl;

}

void TwoDimFitController::makeXGammaFit()
{
    if(experiment_.size() == 0)
        cout << "no experimental points!" << endl;
    findCorrespondingLevel();
    cout << "1->" ;
    prepareFeedings();
    cout << "----------2--------->" ;
    //prepareTransitionResponses(); /* petla po gammach (bez branchingu),
//    ale poszczegolne funkcje odpowiedzi mnozone przez feeding beta dopoziou */
//    cout << "----------3-------->" << endl;
//EVa    saveResponses();
    cout << "--------------4---------->" << endl;
    makeFit();
    cout << "-----------5---------->" ;
    findErrors();
    cout << "6->" ;
    notifyObservers();

    cout << "7->" ;
    printResults();
    cout << "8->END" << endl;

}

void TwoDimFitController::makeFit()
{
    cout << " ===> TwoDimFitController::makeFit() POCZATEK" << endl;

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

    int minPoint = 100;
    int maxPoint = maxEn_;
/*    int maxPoint = nrOfPoints;
    for(int j = 0; j != nrOfHistograms + 1; j++)
    {
        cout << "responsesForFit.at(j).size, j =  " << j << ": " << (responsesForFit.at(j)).size() << endl;
        if( (responsesForFit.at(j)).size() < maxPoint )
            maxPoint = (responsesForFit.at(j)).size();
    }
*/
    cout << "maxPoint: " << maxPoint << endl;

    double expNorm = 0;
    for(int i = minPoint; i != maxPoint; i++)
        expNorm += experiment_.at(i);
    cout << "expNorm = " << expNorm << endl;

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
    cout << "sumForNormalization 1 = " << sumForNormalization << endl;
    sumForNormalization += otherLevelsResponse_->GetNrOfCounts(minPoint,maxPoint);

    cout << "sumForNormalization 2 = " << sumForNormalization << endl;
    double responseMultiplier = expNorm / sumForNormalization;
    cout << "responseMultiplier (including otherLevelsContribution) = " << responseMultiplier << endl;
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
            cout << "i=" << i <<", responseNrOfCounts=" << responseNrOfCounts <<
                    ", feedings.at(i)=" << feedings.at(i) << endl;
        }
        sumForNormalization += otherLevelsResponse_->GetNrOfCounts(minPoint,maxPoint);
        cout << "sumForNormalization = " << sumForNormalization << endl;
        responseMultiplier = expNorm / sumForNormalization;
        cout << "responseMultiplier = " << responseMultiplier << endl;


        double bigCheckSum = 0.;
        for(int j = 0; j != nrOfHistograms + 1; j++)
        {
            double checkSum = 0.;
            double sum1 = 0.;

            for(int i = minPoint; i < maxPoint; i++)
            {
                double sum2 = 0.;
                for(int k = 0; k !=nrOfHistograms + 1; ++k)
                {
 //std::cout << "Fitting iteration #it " << it << " Hist#j: "<< j << " bin#i: "<<i << " drugi Hist#k "<<k<<"\r" << std::flush;

                    sum2 += (responsesForFit.at(k)).at(i) * responseMultiplier * oldFeedings.at(k);
                }


                if(experimentForFit.at(i) > 0)
                {
                    sum1 += (responsesForFit.at(j)).at(i) * responseMultiplier * (experimentForFit.at(i) - sum2)/experimentForFit.at(i);  
                    checkSum += experimentForFit.at(i) - sum2;
/*
                        cout << "i=" << i << ", sum1 = " << sum1 << endl;
                        cout << "(responsesForFit.at(j)).at(i) = " << (responsesForFit.at(j)).at(i) <<
                                ", responseMultiplier = " << responseMultiplier << endl;
                        cout << "experimentForFit.at(i) = " << experimentForFit.at(i) <<
                                ", sum2 = " << sum2 << endl;
                        cout << "whole expr = " << (responsesForFit.at(j)).at(i) * responseMultiplier * (experimentForFit.at(i) - sum2)/experimentForFit.at(i) << endl;
*/
                }
                else
                {
                    checkSum += experimentForFit.at(i) - sum2;
                    cout << "FitController::makeFit: zero count for " << i << "bin" << "\r" << endl; //<< flush;
                    cout << "sum2 = " << sum2 << ", experiment = " << experimentForFit.at(i) << endl;
                }
            }
            cout << "At the end of histogram " << j << ": sum1 = " << sum1 << endl;
            if(j < nrOfHistograms)
            {
                feedings.at(j) = oldFeedings.at(j) * exp(2./lambda_ * sum1);
                cout << "j = " << j <<", zmieniam feeding." << endl;
            }
            cout << "At the end of histogram " << j << ": checkSum = " << checkSum << endl;
            bigCheckSum += checkSum;
        }
        cout << "At the end of iteration " << it << ": bigCheckSum = " << bigCheckSum << endl;

        double feedingsDuringFitSum = 0.;
        for(int j = 0; j != nrOfHistograms; j++)
            feedingsDuringFitSum += feedings.at(j);

        for(int j = 0; j != nrOfHistograms; j++)
            feedings.at(j) /= feedingsDuringFitSum;

       oldFeedings = feedings;
    }

    feedings.pop_back();

    //double feedingSumWithOtherLevelsContr = 0.;
    double feedingSum = 0.; //without other levels contribution
    for(unsigned i =0; i<feedings.size(); ++i)
    {
        cout << feedings.at(i) << endl;
        feedingSum += feedings.at(i);
    }
    cout << "feedingSum = " << feedingSum << endl;

    //normalization
    for(unsigned i =0; i < feedings.size(); ++i)
    {
        double tempFeeding = feedings.at(i) / feedingSum;
        feedings.at(i) = tempFeeding;
        cout << feedings.at(i) << endl;
    }

    cout << " ===> TwoDimFitController::makeFit() KONIEC" << endl;

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
        //cout << "responses.at(j).size, j =  " << j << ": " << (responses.at(j)).size() << endl;
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
    cout <<" notify level " << endl;
    vector<Transition*>* transitions_ = level_->GetTransitions();
    int transitionIndex = -1;

    cout << "set new intensities " << endl;
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
