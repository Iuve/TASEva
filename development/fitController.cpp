
#include "fitController.h"
#include "DeclareHeaders.hh"
#include "responsefunction.h"

FitController::FitController()
{

    myProject = Project::get();
    transitionsUsed.clear();
    minEnergy = myProject->getFitEnergyFrom();
    maxEnergy = myProject->getFitEnergyTo();
//    minLevelEn = myProject->getFitLevelsFrom();
//    maxLevelEn = myProject->getFitLevelsTo();
    lambda = myProject->getFitLambda();
    nrOfIterations = myProject->getNoFitIterations();
    histId = std::stoi(myProject->getExpSpecID());
}

std::vector <float> FitController::getBetaIntensities()
{
    return feedings;
}

std::vector<float> FitController::getErrors()
{
    return errors;
}

void FitController::prepareExperiment(Histogram* expHist)
{
    Histogram tempHist = *expHist;
    tempHist.Normalize(1.);
    experiment = tempHist.GetAllData(minEnergy, maxEnergy);
    expNorm = tempHist.GetNrOfCounts(minEnergy,maxEnergy);
    double expSum;
    for (int i = 0; i != experiment.size(); i++)
    {
        expSum += experiment.at(i);
    }
    cout << "number of counts in EXP spec: " << expSum << endl;
    cout << "experiment.size() = " << experiment.size() << endl;
    cout << "Number of counts within fitting limits ("<< minEnergy <<":"<<maxEnergy<<") : " <<expNorm << endl;
}

void FitController::prepareLevelsPart()
{
    cout << "--------------Prepare Transitions to Levels-----------"<< endl;

    DecayPath* decayPath = DecayPath::get();
    ResponseFunction* responseFunction = ResponseFunction::get();
    std::vector<Nuclide>* nuclides = decayPath->GetAllNuclides();
    std::vector<Level>* motherLevels;
    Nuclide* motherNuclide;

    motherNuclide = &(nuclides->at(0));
    motherLevels = motherNuclide->GetNuclideLevels();

    Level* motherLevel = &motherLevels->at(0);
    std::vector<Transition*>* transitions = motherLevel->GetTransitions();  // getting transitions

    cout << "Collecting data from spectrum ID="<<histId<< " for energies between "<<minEnergy<<" and "<<maxEnergy<< endl;

    double simNrOfCounts = 0.;
    for(auto itt = transitions->begin(); itt != transitions->end(); ++itt)
    {
        intensityFitFlags.push_back( (*itt)->GetIntensityFitFlag() );
        transitionsUsed.push_back(*itt);
        Histogram tmpHistogram = *(responseFunction->GetLevelRespFunction( (*itt)->GetPointerToFinalLevel(), histId ));
        //tmpHistogram.Normalize(1.);
        responses.push_back(tmpHistogram.GetAllData(minEnergy, maxEnergy));
        feedings.push_back((*itt)->GetIntensity() );
        cout << "Added to the fitting vector: Energy = " << (*itt)->GetFinalLevelEnergy()
             << ", Intensity = " << (*itt)->GetIntensity()
             << ", Number of counts  = " << tmpHistogram.GetNrOfCounts(minEnergy,maxEnergy)
             << ", FitParam: " << intensityFitFlags.back()
             << ", Size = " << responses.back().size() << endl;

        simNrOfCounts += tmpHistogram.GetNrOfCounts(minEnergy,maxEnergy);
    }
    nrOfLevels = transitionsUsed.size();

//    std::cout << "feedings.size: NO contaminations " << feedings.size() << std::endl;
    std::cout << "Total number of levels (response.size() ): " << responses.size() << std::endl;
    cout << "simNrOfCounts = " << simNrOfCounts << endl;
   cout << "--------------Prepare Level----END-------"<< endl;
}

void FitController::prepareContaminationPart()
{
    cout << "--------------Prepare Contaminations-----------"<< endl;

    std::vector<Contamination> contaminations = *(myProject->getContaminations());
    nrOfContaminations = contaminations.size();
    std::cout << "nrOfContaminations: " << nrOfContaminations << endl;
    double contNrOfCounts = 0.;

    double betaFeedingSum = 0.;
    for(int i = 0; i < feedings.size(); ++i)
    {
        betaFeedingSum += feedings.at(i);
    }

    double contFeedingSum = 0.;
    for(int i = 0; i != nrOfContaminations; ++i)
    {
        intensityFitFlags.push_back( contaminations.at(i).GetIntensityFitFlag() );
        contaminations.at(i).hist.Normalize(1.0);
        responses.push_back(contaminations.at(i).hist.GetAllData(minEnergy, maxEnergy));
        feedings.push_back(contaminations.at(i).intensity);
        contFeedingSum += contaminations.at(i).intensity;
        cout << "Added to the fitting vector: Cont nr = " << i
             << ", Intensity = " << feedings.back()
             << ", Number of counts  = " << contaminations.at(i).hist.GetNrOfCounts(minEnergy,maxEnergy)
             << ", FitParam: " << intensityFitFlags.back()
             << ", Size = " << responses.back().size() << endl;
        contNrOfCounts += contaminations.at(i).hist.GetNrOfCounts(minEnergy,maxEnergy);
    }

    cout << "betaFeedingSum = " << betaFeedingSum << endl;
    normalizationFactor_ = 1. - contFeedingSum;
    cout << "contFeedingSum = " << contFeedingSum << endl;
    cout << "normalizationFactor_ = " << normalizationFactor_ << endl;
    int maxIt = feedings.size() - nrOfContaminations;
    for(int i = 0; i < maxIt; ++i)
    {
        feedings.at(i) *= normalizationFactor_ / betaFeedingSum;
    }

    feedingsToFitSum_ = 0.;
    for(int i = 0; i < feedings.size(); i++)
    {
        if( intensityFitFlags.at(i) )
            feedingsToFitSum_ += feedings.at(i);
    }

    cout << "feedingsToFitSum_ = " << feedingsToFitSum_ << endl;
    std::cout << " response.size: with contaminations " << responses.size() << std::endl;
    cout << "contNrOfCounts = " << contNrOfCounts << endl;
    cout << "--------------Prepare Contaminations----END-------"<< endl;
}

void FitController::makeFit()
{
    cout << "-------------making fit----------------" << endl;
    int nrOfHistograms = feedings.size();
    int nrOfPoints = experiment.size();
    int nrOfBetaTransitions = nrOfHistograms - nrOfContaminations;
    std::cout << "feedings.size: " << feedings.size() << std::endl;
    std::cout << "experiment.size: " << experiment.size() << std::endl;
    std::cout << "responses.size: " << responses.size() << std::endl;

    normBeforeFit = findNormalisation();
    vector<float> oldFeedings = feedings;
    cout << nrOfHistograms << " " << nrOfPoints << " " << nrOfLevels << endl;
    cout << " nrOfIterations: " << nrOfIterations << endl;
    cout << " lambda: " << lambda << endl;
    string ss;
    for(int it = 0; it !=nrOfIterations; ++it)
    {
        //preparing correct normalization
        double sumForNormalization = 0.;
        for(int i = 0; i < nrOfBetaTransitions; ++i)
        {
            double responseNrOfCounts = 0.;
            for(int j = 0; j < (responses.at(i)).size(); j++)
                responseNrOfCounts += (responses.at(i)).at(j);
            sumForNormalization += responseNrOfCounts * feedings.at(i);
        }

        cout << "sumForNormalization = " << sumForNormalization << endl;
        //normalizationFactor_ * expNorm == a * sumForNormalization;
        double responseMultiplier = normalizationFactor_ * expNorm / sumForNormalization;
        cout << "responseMultiplier = " << responseMultiplier << endl;

      std::cout << "Running fitting iteration # " << it << "\r" << std::flush;
        for(int j = 0; j != nrOfHistograms; j++)
        {
            if( !intensityFitFlags.at(j) )
                continue;

            double sum1 = 0.;
            //cout << "J = " << j << endl;
            //cout << "responses.at(j).size() = " << responses.at(j).size() << endl;
            for(int i = 0; i != nrOfPoints; i++)
            {
                double sum2 = 0;
                for(int k = 0; k !=nrOfHistograms; ++k)
                {
/*
                    if( (responses.at(k)).at(i) > 0 )
                    {
                        cout << "k=" << k << ", i=" << i << ", responses.at(k)).at(i) = " << (responses.at(k)).at(i) << endl;
                    }
*/
                    if( k < nrOfBetaTransitions )
                        sum2 += (responses.at(k)).at(i) * responseMultiplier * oldFeedings.at(k);
                    else
                        sum2 += (responses.at(k)).at(i) * oldFeedings.at(k);
                }

                if(experiment.at(i) > 0)
                {
                    if( j < nrOfBetaTransitions )
                        sum1 += (responses.at(j)).at(i) * responseMultiplier * (experiment.at(i) - sum2)/experiment.at(i);
                    else
                        sum1 += (responses.at(j)).at(i) * (experiment.at(i) - sum2)/experiment.at(i);
                    //cout << " i = " << i << endl;
                    //cout << "experiment.at(i) = " << experiment.at(i) << endl;
                    //cout << "sum2 = " << sum2 << endl;
                    //cout << "(experiment.at(i) - sum2) = " << (experiment.at(i) - sum2) << endl;
                }
                else
                {
 //                   cout << "FitController::makeFit: zero count for " << i << "bin" << endl;
 //                   std::cout << "it: "<< it << " j: " << j << " i: "  <<  i << " sum2: " << sum2 << std::endl;
 //                   std::cin >> ss;
                }
                //cout << "sum2 = " << sum2 << endl;
                //cout << "experiment.at(i) = " << experiment.at(i) << endl;
            }
            //cout << "sum1 = " << sum1 << endl;
            //cout << "exp(2./lambda * sum1) = " << exp(2./lambda * sum1) << endl;
            feedings.at(j) = oldFeedings.at(j) * exp(2./lambda * sum1);
        }

       double feedingsDuringFitSum = 0.;
       for(int i = 0; i < nrOfHistograms; i++)
           if( intensityFitFlags.at(i) )
               feedingsDuringFitSum += feedings.at(i);

       double itShouldEqualOneDuringFitting = 0.;
       for(int i = 0; i < nrOfHistograms; i++)
       {
           if( intensityFitFlags.at(i) )
               feedings.at(i) *= feedingsToFitSum_ / feedingsDuringFitSum;
           itShouldEqualOneDuringFitting += feedings.at(i);
       }
       cout << "feedingsDuringFitSum = " << feedingsDuringFitSum << endl;
       cout << "itShouldEqualOneDuringFitting = " << itShouldEqualOneDuringFitting << endl;

       oldFeedings = feedings;
    }
    std::cout << std::endl;
    normAfterFit =  findNormalisation();

    int allFeedingsSize = feedings.size();
    int betaFeedingsSize = feedings.size() - nrOfContaminations;
    double allConstantFeedingsIntensityBeforeNorm = 0.;
    for(int i = 0; i < allFeedingsSize; i++)
    {
        if( !intensityFitFlags.at(i) )
            allConstantFeedingsIntensityBeforeNorm += feedings.at(i);
    }
    cout << "allConstantFeedingsIntensityBeforeNorm = " << allConstantFeedingsIntensityBeforeNorm << endl;
    double whatShouldBeNonConstantFeedingsIntensity = 1. - allConstantFeedingsIntensityBeforeNorm;
    double itShouldEqualOne = 0.;
    double neededFactor = normAfterFit - allConstantFeedingsIntensityBeforeNorm;

    for(int i = 0; i < allFeedingsSize; i++)
    {
        if( intensityFitFlags.at(i) )
        {
            feedings.at(i) *= whatShouldBeNonConstantFeedingsIntensity / neededFactor;
        }
        itShouldEqualOne += feedings.at(i);
    }
    cout << "whatShouldBeNonConstantFeedingsIntensity = " << whatShouldBeNonConstantFeedingsIntensity << endl;
    cout << "neededFactor = " << neededFactor << endl;
    cout << "itShouldEqualOne = " << itShouldEqualOne << endl;

    double normalizedTotalContaminationsIntensity = 0.;
    for(int i = betaFeedingsSize; i < allFeedingsSize; i++)
    {
        normalizedTotalContaminationsIntensity += feedings.at(i);
    }

    cout << "normalizedTotalContaminationsIntensity = " << normalizedTotalContaminationsIntensity << endl;

    double finalTotalIntensityOfConstantBetaFeedings = 0.;
    double atTheMomentSumOfFittedFeedings = 0.;
    for(int i = 0; i < betaFeedingsSize; i++)
    {
        if( !intensityFitFlags.at(i) )
        {
            feedings.at(i) /= normalizationFactor_;
            finalTotalIntensityOfConstantBetaFeedings += feedings.at(i);
        }
        else
            atTheMomentSumOfFittedFeedings += feedings.at(i);
    }

    cout << "finalTotalIntensityOfConstantBetaFeedings = " << finalTotalIntensityOfConstantBetaFeedings << endl;
    cout << "atTheMomentSumOfFittedFeedings = " << atTheMomentSumOfFittedFeedings << endl;

    double finalTotalIntensityOfFittedParameters = 1. - finalTotalIntensityOfConstantBetaFeedings;
    double checkSum = 0.;
    for(int i = 0; i < betaFeedingsSize; i++)
    {
        if( intensityFitFlags.at(i) )
        {
            feedings.at(i) *= finalTotalIntensityOfFittedParameters / atTheMomentSumOfFittedFeedings;
            checkSum += feedings.at(i);
        }
    }
    cout << "checkSum = " << checkSum << endl;
    cout << "Total normalized beta intensity = " << checkSum+finalTotalIntensityOfConstantBetaFeedings << endl;


    cout << "-------------making fit------END----------" << endl;


}

float FitController::findNormalisation()
{
    float normalization = 0;
    for(int i = 0; i != feedings.size(); ++i)
        normalization += feedings.at(i);

    cout << "FitController::findNormalisation: " << normalization << endl << endl;
    return normalization;
}

void FitController::findErrors()
{
    int nrOfHistograms = feedings.size();
    int nrOfPoints = experiment.size();
//    cout<<"errors calculation"<<endl;
    //errors in %:
    for(int i = 0; i !=nrOfHistograms; ++i )
    {
        if( !intensityFitFlags.at(i) )
        {
            errors.push_back(0.);
            continue;
        }

        double sum = 0;
        for(int k = 0; k != nrOfPoints; ++k)
        {
            if(experiment.at(k)!=0)sum += (responses.at(i)).at(k)* (responses.at(i)).at(k)/experiment.at(k);  //check expNorm added mar 1, 2020
        }
        double error = pow(4./(lambda* lambda) * feedings.at(i)* feedings.at(i) * sum, 0.5);
        errors.push_back(error/feedings.at(i) * 100.);
    }
}

void FitController::notifyDecay()
{
    cout << "-------------Notify Decay ------------" << endl;
    ResponseFunction* responseFunction = ResponseFunction::get();

    int levelIndex = -1;

    for(auto it = transitionsUsed.begin(); it != transitionsUsed.end(); ++it)
    {
        levelIndex ++;
        if( !intensityFitFlags.at(levelIndex) )
            continue;

        //double newIntensity = feedings.at(levelIndex)*normBeforeFit/normAfterFit;
        double newIntensity = feedings.at(levelIndex);
        (*it)->ChangeIntensity(newIntensity);
        responseFunction->ChangeContainerDaughterLevelIntensity( (*it)->GetPointerToFinalLevel(), newIntensity );
    }
    responseFunction->RefreshFlags();
    cout << "-------------Notify Decay -----END-------" << endl;
}

void FitController::notifyContaminations()
{
    std::cout << "---------------notifyContaminations---------------"<< std::endl;
    std::vector<Contamination> contaminations = *(myProject->getContaminations());
    std::cout << " NrOfHistograms: " << feedings.size()
              << " NrOfLevels: " << nrOfLevels
              << " NrOfContaminations: " << nrOfContaminations << std::endl;
    for(int i = nrOfLevels; i != nrOfContaminations+nrOfLevels; ++i)
    {
        if( !intensityFitFlags.at(i) )
            continue;

        std::cout << " Feeding from fit: "<< feedings.at(i)
                  << " Feeding Normalization BEFORE fit: " << normBeforeFit
                 << " Feeding normalization AFTER fit: " << normAfterFit << std::endl;
        int j=i-nrOfLevels;
        //contaminations.at(j).intensity = feedings.at(i)*normBeforeFit/normAfterFit;
        contaminations.at(j).intensity = feedings.at(i);
    }
    myProject->setContaminations(contaminations);

    std::cout << "---------------notifyContaminations----END---------"<< std::endl;
}

void FitController::applyFit (Histogram* expHist)
{
    cout<<"Fit...."<<endl;
    prepareExperiment(expHist);
    std::cout << "1.." << std::endl;
    prepareLevelsPart();
    std::cout << "2.." << std::endl;
    prepareContaminationPart();
    //std::cout << "3.0." << std::endl;
    //normalizeToExperiment();
    std::cout << "3.." << std::endl;
    makeFit();
    std::cout << "4.." << std::endl;
    findErrors();
    std::cout << "5.." << std::endl;
    notifyDecay();
    std::cout << "6.." << std::endl;
    notifyContaminations();
    std::cout << "7.. end" << std::endl;
}
