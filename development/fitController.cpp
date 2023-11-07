
#include "fitController.h"
#include "DeclareHeaders.hh"
#include "responsefunction.h"

FitController::FitController()
{

    myProject = Project::get();
    minEnergy = myProject->getFitEnergyFrom();
    maxEnergy = myProject->getFitEnergyTo();
//    minLevelEn = myProject->getFitLevelsFrom();
//    maxLevelEn = myProject->getFitLevelsTo();
    lambda = myProject->getFitLambda();
    nrOfIterations = myProject->getNoFitIterations();
    binning = myProject->getFitBinning();
    expHistId = std::stoi(myProject->getExpSpecID());
}

std::vector <float> FitController::getBetaIntensities()
{
    return feedings;
}

std::vector<float> FitController::getErrors()
{
    return errors;
}

void FitController::applyMaximumLikelyhoodFit (Histogram* expHist)
{
    cout<<"Fit...."<<endl;
    prepareExperiment(expHist);
    std::cout << "1.." << std::endl;
    prepareLevelsPart(expHistId);
    std::cout << "2.." << std::endl;
    prepareContaminationPart( *(myProject->getContaminations()), expHistId );
    std::cout << "2.5." << std::endl;
    BinForFitting();
    std::cout << "3.." << std::endl;
    makeLikelyhoodFit();
    std::cout << "4.." << std::endl;
    findErrors();
    std::cout << "5.." << std::endl;
    notifyDecay();
    std::cout << "6.." << std::endl;
    notifyContaminations();
    std::cout << "7.. end" << std::endl;
}

void FitController::applyBayesianFit (std::vector< std::pair<Histogram*, int> > bayesianHistograms)
{
    cout << "-------------BayesianFit start.-----------------" << endl;
    for(auto ih = bayesianHistograms.begin(); ih != bayesianHistograms.end(); ++ih)
    {
        cout << "------- for histogram: " << (*ih).second << endl;
        prepareExperiment((*ih).first);
        prepareLevelsPart((*ih).second);
        prepareContaminationPart( *(myProject->getContaminations()), (*ih).second );

        BinForFitting();

        bayesianExperiments.push_back(experiment);
        bayesianFeedings.push_back(feedings);
        bayesianResponses.push_back(responses);
        bayesianNormalizationFactors.push_back(normalizationFactor_);
        bayesianExpNorms.push_back(expNorm);
    }

    makeBayesianFit();
    findErrors(); //to be corrected
    notifyDecay();
    //contaminations shouldn't be fitted here
}

void FitController::prepareExperiment(Histogram* expHist)
{
    cout << "------------Preparing EXP histogram. ------------" << endl;
    experiment.clear();
    Histogram tempHist = *expHist;
    tempHist.Normalize(1.);
    experiment = tempHist.GetAllData(minEnergy, maxEnergy);
    expNorm = tempHist.GetNrOfCounts(minEnergy,maxEnergy);
    double expSum = 0.;
    for (int i = 0; i != experiment.size(); i++)
    {
        expSum += experiment.at(i);
    }
    cout << "number of counts in EXP spec: " << expSum << endl;
    cout << "experiment.size() = " << experiment.size() << endl;
    cout << "Number of counts within fitting limits ("<< minEnergy <<":"<<maxEnergy<<") : " <<expNorm << endl;
    cout << "------------Preparing EXP histogram. ----END ----" << endl;


}

void FitController::prepareLevelsPart(int simID)
{
    //In current approach feedings, intensityFitFlags and transitionsUsed are filled
    //with data for every ID, which is not necessary but correct
    feedings.clear();
    responses.clear();
    intensityFitFlags.clear();
    transitionsUsed.clear();

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

    cout << "Collecting data from spectrum ID="<<simID<< " for energies between "<<minEnergy<<" and "<<maxEnergy<< endl;

    double simNrOfCounts = 0.;
    for(auto itt = transitions->begin(); itt != transitions->end(); ++itt)
    {
        intensityFitFlags.push_back( (*itt)->GetIntensityFitFlag() );
        transitionsUsed.push_back(*itt);
        Histogram tmpHistogram = *(responseFunction->GetLevelRespFunction( (*itt)->GetPointerToFinalLevel(), simID ));
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

void FitController::prepareContaminationPart( std::vector< std::pair<int, Contamination> > contaminations, int histID )
{
    cout << "--------------Prepare Contaminations-----------"<< endl;
    if (contaminations.size() == 0)
    {
        normalizationFactor_ = 1.;
        nrOfContaminations = 0;
        cout << "--------------Prepare Contaminations----END-------"<< endl;
        return;
    }

    int nrOfCont = 0;
    double contNrOfCounts = 0.;

    double betaFeedingSum = 0.;
    for(int i = 0; i < feedings.size(); ++i)
    {
        betaFeedingSum += feedings.at(i);
    }

    double contFeedingSum = 0.;
    int normIterator = 0;
    for(unsigned int i = 0; i < contaminations.size(); ++i)
    {
        if(contaminations.at(i).first == histID)
        {
            nrOfCont++;
            intensityFitFlags.push_back( contaminations.at(i).second.GetIntensityFitFlag() );
            contaminations.at(i).second.hist.Normalize(1.0);
            responses.push_back(contaminations.at(i).second.hist.GetAllData(minEnergy, maxEnergy));

            //preparing uniform contamination normalization throughout all bayesian histograms
            if(histID == expHistId)
            {
                bayesianContaminationNormalization.push_back(contaminations.at(i).second.intensity);
            }
            else
            {
                double factor = contaminations.at(i).second.intensity / bayesianContaminationNormalization.at(normIterator);
                contaminations.at(i).second.intensity = bayesianContaminationNormalization.at(normIterator);
                for(int k = 0; k < responses.back().size(); k++)
                    responses.back().at(k) *= factor;
                normIterator++;
            }

            feedings.push_back(contaminations.at(i).second.intensity);
            contFeedingSum += contaminations.at(i).second.intensity;
            cout << "Added to the fitting vector: Cont nr = " << i
                 << ", Intensity = " << feedings.back()
                 << ", Number of counts  = " << contaminations.at(i).second.hist.GetNrOfCounts(minEnergy,maxEnergy)
                 << ", FitParam: " << intensityFitFlags.back()
                 << ", Size = " << responses.back().size() << endl;
            contNrOfCounts += contaminations.at(i).second.hist.GetNrOfCounts(minEnergy,maxEnergy);
        }
    }

    cout << "betaFeedingSum = " << betaFeedingSum << endl;
    normalizationFactor_ = 1. - contFeedingSum;
    cout << "contFeedingSum = " << contFeedingSum << endl;
    cout << "normalizationFactor_ = " << normalizationFactor_ << endl;
    nrOfContaminations = nrOfCont;
    int maxIt = feedings.size() - nrOfContaminations;
    cout << "feeding.size() = " << feedings.size() <<  endl;
    cout << " nrOfContaminations " << nrOfContaminations << endl;
    cout << "maxIt = " << maxIt << endl;
    for(int i = 0; i < maxIt; ++i)
    {
        feedings.at(i) *= normalizationFactor_ / betaFeedingSum;
    }

    //std::cout << "nrOfContaminations: " << nrOfContaminations << endl;

    std::cout << " response.size: with contaminations " << responses.size() << std::endl;
    cout << "contNrOfCounts = " << contNrOfCounts << endl;
    cout << "--------------Prepare Contaminations----END-------"<< endl;
}

void FitController::BinForFitting()
{
    cout << "BinForFitting(), experiment.size() = " << experiment.size() << endl;
    cout << "BinForFitting(), responses.at(10).size() = " << responses.at(10).size() << endl;
    cout << "binning = " << binning << endl;

    int nrOfbinsInGroup = binning - 1;
    std::vector<float> tempExperiment;
    for(unsigned int i = 0; i < experiment.size(); i++)
    {
        if(nrOfbinsInGroup == binning - 1)
        {
            tempExperiment.push_back(experiment.at(i));
            nrOfbinsInGroup = 0;
        }
        else
        {
            tempExperiment.back() += experiment.at(i);
            nrOfbinsInGroup ++;
        }
    }
    experiment = tempExperiment;
 //   cout << "experiment done" << endl;

    std::vector< vector<float> > tempResponses;
    std::vector<float> tempVector;
    for(unsigned int i = 0; i < responses.size(); i++)
    {
        //cout << "i = " << i << endl;
        tempResponses.push_back(tempVector);
        nrOfbinsInGroup = binning - 1;
        for(unsigned int j = 0; j < (responses.at(i)).size(); j++)
        {
            //cout << "j = " << j << endl;
            if(nrOfbinsInGroup == binning - 1)
            {
                //cout << "if" << endl;
                tempResponses.at(i).push_back(responses.at(i).at(j));
                nrOfbinsInGroup = 0;
            }
            else
            {
                //cout << " esle" << endl;
                tempResponses.at(i).back() += responses.at(i).at(j);
                nrOfbinsInGroup ++;
            }
        }
    }
    responses = tempResponses;

    std::cout << "feedings.size: " << feedings.size() << std::endl;
    cout << "BinForFitting(), experiment.size() = " << experiment.size() << endl;
    cout << "BinForFitting(), responses.at(10).size() = " << responses.at(10).size() << endl;
}


void FitController::makeLikelyhoodFit()
{
    cout << "-------------making fit----------------" << endl;
    int nrOfHistograms = feedings.size();
    int nrOfPoints = experiment.size();
    int nrOfBetaTransitions = nrOfHistograms - nrOfContaminations;
    std::cout << "feedings.size: " << feedings.size() << std::endl;
    std::cout << "experiment.size: " << experiment.size() << std::endl;
    std::cout << "responses.size: " << responses.size() << std::endl;

    normBeforeFit = 0.;
    for(int i = 0; i != feedings.size(); ++i)
        normBeforeFit += feedings.at(i);
    cout << "normBeforeFit = " << normBeforeFit << endl;

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

       cout << "feedingsDuringFitSum = " << feedingsDuringFitSum << endl;

       oldFeedings = feedings;
    }
    std::cout << std::endl;
    normAfterFit = 0.;
    for(int i = 0; i != feedings.size(); ++i)
        normAfterFit += feedings.at(i);
    cout << "normAfterFit = " << normAfterFit << endl;

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

void FitController::makeBayesianFit()
{
    cout << "-------------making fit----------------" << endl;

    //2 below lines are potentially not correct
    int nrOfHistograms = bayesianFeedings.at(0).size();
    int nrOfPoints = bayesianExperiments.at(0).size();
    int nrOfBetaTransitions = nrOfHistograms - nrOfContaminations;

    normBeforeFit = 0.;
    for(int i = 0; i != bayesianFeedings.at(0).size(); ++i)
        normBeforeFit += bayesianFeedings.at(0).at(i);
    cout << "normBeforeFit = " << normBeforeFit << endl;

    //vector< vector<float> > oldBayesianFeedings = bayesianFeedings;

    cout << nrOfHistograms << " " << nrOfPoints << " " << nrOfLevels << endl;
    cout << " nrOfIterations: " << nrOfIterations << endl;
    cout << " lambda: " << lambda << endl;

    string ss;
    for(int it = 0; it !=nrOfIterations; ++it)
    {
        std::cout << "Running fitting iteration # " << it << "\r" <<  std::flush;

        vector< pair<double, double> > fraction;
        for(int i = 0; i < nrOfHistograms; i++)
            fraction.emplace_back(0., 0.);
        //cout << "bayesian Experiments.size() " << bayesianExperiments.size() << std::endl;
        for(int id = 0; id < bayesianExperiments.size(); id++)
        {
            //preparing correct normalization
            //cout << "id " << id << std::endl;
            double sumForNormalization = 0.;
            for(int i = 0; i < nrOfBetaTransitions; ++i)
            {
                //cout << "i= " << i << std::endl;
                double responseNrOfCounts = 0.;
                for(int j = 0; j < (bayesianResponses.at(id).at(i)).size(); j++)
                    responseNrOfCounts += (bayesianResponses.at(id).at(i)).at(j);
                sumForNormalization += responseNrOfCounts * bayesianFeedings.at(id).at(i);
            }

            //cout << "sumForNormalization = " << sumForNormalization << endl;
            //normalizationFactor_ * expNorm == a * sumForNormalization;
            double responseMultiplier = bayesianNormalizationFactors.at(id) * bayesianExpNorms.at(id) / sumForNormalization;
            //cout << "responseMultiplier = " << responseMultiplier << endl;

 //           cout << "nrofHisto" << nrOfHistograms << std::endl;
            for(int a = 0; a < nrOfHistograms; a++)
            {
//                cout << "a= " << a << std::endl;
//                cout << "intensitforflag " << intensityFitFlags.size() << std::endl;
                if( !intensityFitFlags.at(a) )
                    continue;
                double sum2 = 0.;
                double sum3 = 0.;
                for(int i = 0; i < nrOfPoints; i++)
                {
                    double sum1 = 0.;
                    for(int b = 0; b < nrOfHistograms; b++)
                    {
//                        if( b < nrOfBetaTransitions )
                        if( (b < nrOfBetaTransitions) && intensityFitFlags.at(b) )
                            sum1 += responseMultiplier * (bayesianResponses.at(id).at(b)).at(i) * bayesianFeedings.at(id).at(b);
                        else
                            sum1 += (bayesianResponses.at(id).at(b)).at(i) * bayesianFeedings.at(id).at(b);
                    }

                    if(sum1 != 0)
                    {
                        if( a < nrOfBetaTransitions )
                            sum2 += responseMultiplier * (bayesianResponses.at(id).at(a)).at(i) * bayesianFeedings.at(id).at(a) * bayesianExperiments.at(id).at(i) / sum1;
                        else
                            sum2 += (bayesianResponses.at(id).at(a)).at(i) * bayesianFeedings.at(id).at(a) * bayesianExperiments.at(id).at(i) / sum1;
                    }
                //cout << "tutut" << std::endl;
                    if( a < nrOfBetaTransitions )
                        sum3 += responseMultiplier * (bayesianResponses.at(id).at(a)).at(i);
                    else
                        sum3 += (bayesianResponses.at(id).at(a)).at(i);
                }
                fraction.at(a).first += sum2;
                fraction.at(a).second += sum3;
            }
        }
        for(int id = 0; id < bayesianExperiments.size(); id++)
            for(int i = 0; i < nrOfHistograms; i++)
            {
                if( !intensityFitFlags.at(i) )
                    continue;

                bayesianFeedings.at(id).at(i) = fraction.at(i).first / fraction.at(i).second;
            }
    }
    std::cout << std::endl;
    feedings = bayesianFeedings.at(0);

    normAfterFit = 0.;
    for(int i = 0; i != feedings.size(); ++i)
        normAfterFit += feedings.at(i);
    cout << "normAfterFit = " << normAfterFit << endl;

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
            feedings.at(i) /= bayesianNormalizationFactors.at(0);
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
        //errors.push_back(error/feedings.at(i) * 100.);
        errors.push_back(error);
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
    std::vector< std::pair<int, Contamination> > contaminations = *(myProject->getContaminations());
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
        contaminations.at(j).second.intensity = feedings.at(i);
    }
    myProject->setContaminations(contaminations);

    std::cout << "---------------notifyContaminations----END---------"<< std::endl;
}

