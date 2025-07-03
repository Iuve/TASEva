#include "DeclareHeaders.hh"
#include "pugixml.hh"
#include "PeriodicTable.hh"
#include "project.h"
#include "histogram.h"

#include "QtCore"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>

using namespace std;

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

double round_up_to(double value, double precision = 1.0)
{
    return std::round( (value + 0.5*precision) / precision) * precision;
}

double round_to(double value, double precision = 1.0)
{
    return std::round( value / precision) * precision;
}

SaveDecayData::SaveDecayData(string path)
{
    path_ = path + "/";
    //SaveDecayStructure();
 //   GeneralDecayInfo();
}

SaveDecayData::SaveDecayData()
{
 //   GeneralDecayInfo();
}

SaveDecayData::~SaveDecayData()
{

}

string toStringPrecision(double input,int n)
{
    stringstream stream;
    stream << fixed << setprecision(n) << input;
    return stream.str();
}

void SaveDecayData::SaveDecayStructure()
{
    Project* myProject = Project::get();
    bool saveRoundedBetaIntensites = myProject->getSaveRoundedBetaIntensities();
	DecayPath* decayPath = DecayPath::get();
	std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

    for ( auto it = nuclidesVector->begin(); it != nuclidesVector->end(); ++it )
    {
        int atomicNumber = it->GetAtomicNumber();
        int atomicMass = it->GetAtomicMass();
        double qBeta = it->GetQBeta();
        double sn = it->GetSn();
        pugi::xml_document nuclideFile;
        pugi::xml_node nodeNuclide = nuclideFile.append_child("Nuclide");
        nodeNuclide.append_attribute("AtomicNumber") = atomicNumber;
        nodeNuclide.append_attribute("AtomicMass") = atomicMass;
        nodeNuclide.append_attribute("QBeta") = qBeta;
        if(qBeta > 0.)
            nodeNuclide.append_attribute("d_QBeta") = it->GetD_QBeta();
        if(sn > 0.)
            nodeNuclide.append_attribute("Sn") = sn;
		
        for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
        {
            double levelEnergy = jt->GetLevelEnergy();
            double spin = jt->GetSpin();
            string parity = jt->GetParity();
            string spinParity = jt->GetSpinParity();
            double halfLifeTimeInSeconds = jt->GetHalfLifeTime();
            bool isPseudoLevel = jt->isPseudoLevel();

            pugi::xml_node nodeLevel = nodeNuclide.append_child("Level");
            nodeLevel.append_attribute("Energy").set_value(toStringPrecision(levelEnergy,2).c_str());
            nodeLevel.append_attribute("Spin").set_value(toStringPrecision(spin,2).c_str());
            nodeLevel.append_attribute("Parity") = parity.c_str();
            if(!spinParity.empty())
                nodeLevel.append_attribute("SpinParity") = spinParity.c_str();
            nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,9).c_str());
            if(halfLifeTimeInSeconds > 1e-10)
                nodeLevel.append_attribute("d_T12") = jt->GetD_T12();
            nodeLevel.append_attribute("TimeUnit") = "S";
            if( isPseudoLevel )
                nodeLevel.append_attribute("Origin") = "Added";
            else
                nodeLevel.append_attribute("Origin") = "Database";

            for ( auto kt = jt->GetTransitions()->begin(); kt != jt->GetTransitions()->end(); ++kt )
            {
                string particleType =  (*kt)->GetParticleType();
                double transitionQValue =  (*kt)->GetTransitionQValue();
                double intensity = (*kt)->GetIntensity() * 100.;
                double d_intensity = (*kt)->GetD_Intensity() * 100.;
                double finalLevelEnergy = (*kt)->GetFinalLevelEnergy();
                int finalLevelAtomicMass = (*kt)->GetFinalLevelAtomicMass();
                int finalLevelAtomicNumber = (*kt)->GetFinalLevelAtomicNumber();
                bool isAddedTransition = (*kt)->IsAddedTransition();

                pugi::xml_node nodeTransition = nodeLevel.append_child("Transition");
                nodeTransition.append_attribute("Type") = particleType.c_str();
                nodeTransition.append_attribute("TransitionQValue").set_value(toStringPrecision(transitionQValue,2).c_str());
                if(particleType == "B-" || particleType == "B+")
                    if(intensity < 0.001)
                    {
                        nodeTransition.append_attribute("Intensity") = 0;
                    }
                    else
                    {
                        // uncertainty should already be corrected (rounded) for precision
                        if( !saveRoundedBetaIntensites )
                            nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,4).c_str());
                        if(d_intensity > 1e-6) // needed for "official" precision
                        {
                            string intensityString = toStringPrecision(intensity,3);

                            if(intensityString.find('.') == 2) //feeding >= 10
                            {
                                intensity = round_to(intensity, 1);
                                if( saveRoundedBetaIntensites )
                                    nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,0).c_str());
                                nodeTransition.append_attribute("d_Intensity").set_value(toStringPrecision(d_intensity,0).c_str());
                            }
                            else // feeding < 10
                            {
                                if(intensityString[0] == '0')
                                {
                                    if(intensityString[2] == '0' && intensityString[3] == '0')
                                    {
                                        intensity = round_to(intensity, 0.001);
                                        if( saveRoundedBetaIntensites )
                                            nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,3).c_str());
                                        nodeTransition.append_attribute("d_Intensity").set_value(toStringPrecision(d_intensity,3).c_str());
                                    }
                                    else
                                    {
                                        intensity = round_to(intensity, 0.01);
                                        if( saveRoundedBetaIntensites )
                                            nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,2).c_str());
                                        nodeTransition.append_attribute("d_Intensity").set_value(toStringPrecision(d_intensity,2).c_str());
                                    }
                                }
                                else
                                {
                                    intensity = round_to(intensity, 0.1);
                                    if( saveRoundedBetaIntensites )
                                        nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,1).c_str());
                                    nodeTransition.append_attribute("d_Intensity").set_value(toStringPrecision(d_intensity,1).c_str());
                                }
                            }
                        }
                    }
                else
                    nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,4).c_str());
                if( isAddedTransition )
                    nodeTransition.append_attribute("Origin") = "Added";
                else
                    nodeTransition.append_attribute("Origin") = "Database";

                if (particleType == "G")
                {
                    double eCC = (*kt)->GetElectronConversionCoefficient();
                    if (eCC > 0.)
                    {
                        //Check later whether 6 precision is enough
                        pugi::xml_node nodeConversion = nodeTransition.append_child("ElectronConversionCoefficient");
                        nodeConversion.append_attribute("Total").set_value(toStringPrecision(eCC,6).c_str());
                        double* shellECC = (*kt)->GetShellElectronConversionCoefficient();
                        nodeConversion.append_attribute("KC").set_value(toStringPrecision(shellECC[0],6).c_str());
                        nodeConversion.append_attribute("LC").set_value(toStringPrecision(shellECC[1]-shellECC[0],6).c_str());
                        nodeConversion.append_attribute("MC").set_value(toStringPrecision(shellECC[2]-shellECC[1],6).c_str());
                        nodeConversion.append_attribute("NC").set_value(toStringPrecision(shellECC[3]-shellECC[2],6).c_str());

                    }
                }


                pugi::xml_node nodeTargetLevel = nodeTransition.append_child("TargetLevel");
                nodeTargetLevel.append_attribute("Energy").set_value(toStringPrecision(finalLevelEnergy,2).c_str());
                nodeTargetLevel.append_attribute("AtomicNumber") = finalLevelAtomicNumber;
                nodeTargetLevel.append_attribute("AtomicMass") = finalLevelAtomicMass;

            }
        }
        string nuclideOutputFilename = path_ + to_string(atomicMass) + PeriodicTable::GetAtomicNameCap(atomicNumber) + ".xml";
        //fullFileNames_.push_back(nuclideOutputFilename);
        cout << "Saving result of " << nuclideOutputFilename << " : " << nuclideFile.save_file(nuclideOutputFilename.c_str()) << endl;
    }
}

void SaveDecayData::SaveSpecifiedDecayStructure(Transition* firstTransition, Transition* secondTransition)
{
    //firstTransition is always Beta and is within first Nuclide (Nuclides vector is sorted)
    //secondTransition is usually Gamma or Neutron and is within second Nuclide (Nuclides vector is sorted)
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

    Nuclide* motherNuclide = &nuclidesVector->at(0);
    Nuclide* daughterNuclide = &nuclidesVector->at(1);

    typedef map < double, pair < Level*, Nuclide* > > MapType;
    MapType additionalLevelsToWrite;

    Level* tempLevel = secondTransition->GetPointerToFinalLevel();
    string secondTransitionType = secondTransition->GetParticleType();
    if( secondTransitionType == "Fake" )
    {
        for ( auto jt = daughterNuclide->GetNuclideLevels()->begin(); jt != daughterNuclide->GetNuclideLevels()->end(); ++jt )
        {
            double tempLevelEnergy = jt->GetLevelEnergy();
            if(tempLevelEnergy == 0.)
            {
                tempLevel = &(*jt);
                break;
            }
        }
    }

    additionalLevelsToWrite.insert (MapType::value_type( tempLevel->GetLevelEnergy(), std::make_pair(tempLevel, tempLevel->GetNuclideAddress()) ));
    int indicator = 0;

    do
    {
        indicator = 0;
        for (auto itm = additionalLevelsToWrite.begin(); itm != additionalLevelsToWrite.end(); ++itm)
        {
            Level* level = (itm->second).first;
            for ( auto itt = level->GetTransitions()->begin(); itt != level->GetTransitions()->end(); ++itt )
            {
                if( (*itt)->GetParticleType() == "Fake")
                    break;

                Level* levelToAdd = (*itt)->GetPointerToFinalLevel();
                double energy = levelToAdd->GetLevelEnergy();
                if( additionalLevelsToWrite.count(energy) == 0 )
                {
                    additionalLevelsToWrite.insert (MapType::value_type( energy, std::make_pair(levelToAdd, levelToAdd->GetNuclideAddress()) ));
                    indicator ++;
                }
            }
        }
    }
    while( indicator > 0 );

    int atomicNumber = motherNuclide->GetAtomicNumber();
    int atomicMass = motherNuclide->GetAtomicMass();
    double qBeta = motherNuclide->GetQBeta();
    pugi::xml_document nuclideFile_1;
    pugi::xml_node nodeNuclide = nuclideFile_1.append_child("Nuclide");
    nodeNuclide.append_attribute("AtomicNumber") = atomicNumber;
    nodeNuclide.append_attribute("AtomicMass") = atomicMass;
    nodeNuclide.append_attribute("QBeta") = qBeta;

    for ( auto jt = motherNuclide->GetNuclideLevels()->begin(); jt != motherNuclide->GetNuclideLevels()->end(); ++jt )
    {
        for ( auto kt = jt->GetTransitions()->begin(); kt != jt->GetTransitions()->end(); ++kt )
        {
            if( *kt == firstTransition)
            {
                double levelEnergy = jt->GetLevelEnergy();
                double spin = jt->GetSpin();
                string parity = jt->GetParity();
                double halfLifeTimeInSeconds = jt->GetHalfLifeTime();

                pugi::xml_node nodeLevel = nodeNuclide.append_child("Level");
                nodeLevel.append_attribute("Energy").set_value(toStringPrecision(levelEnergy,2).c_str());
                nodeLevel.append_attribute("Spin").set_value(toStringPrecision(spin,2).c_str());
                nodeLevel.append_attribute("Parity") = parity.c_str();
                nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,9).c_str());
                nodeLevel.append_attribute("TimeUnit") = "S";

                string particleType =  (*kt)->GetParticleType();
                double transitionQValue =  (*kt)->GetTransitionQValue();
                //double intensity = (*kt)->GetIntensity();
                double finalLevelEnergy = (*kt)->GetFinalLevelEnergy();
                int finalLevelAtomicMass = (*kt)->GetFinalLevelAtomicMass();
                int finalLevelAtomicNumber = (*kt)->GetFinalLevelAtomicNumber();

                pugi::xml_node nodeTransition = nodeLevel.append_child("Transition");
                nodeTransition.append_attribute("Type") = particleType.c_str();
                nodeTransition.append_attribute("TransitionQValue").set_value(toStringPrecision(transitionQValue,2).c_str());
                nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(1,6).c_str());

                pugi::xml_node nodeTargetLevel = nodeTransition.append_child("TargetLevel");
                nodeTargetLevel.append_attribute("Energy").set_value(toStringPrecision(finalLevelEnergy,2).c_str());
                nodeTargetLevel.append_attribute("AtomicNumber") = finalLevelAtomicNumber;
                nodeTargetLevel.append_attribute("AtomicMass") = finalLevelAtomicMass;

                break;
            }
        }
    }
    string nuclideOutputFilename = path_ + "Temp" + to_string(atomicMass) + PeriodicTable::GetAtomicNameCap(atomicNumber) + ".xml";
    //cout << "Saving result of " << nuclideOutputFilename << " : " << nuclideFile_1.save_file(nuclideOutputFilename.c_str()) << endl;
    nuclideFile_1.save_file(nuclideOutputFilename.c_str());
    fullFileNames_.push_back(nuclideOutputFilename);
    shortFileNames_.push_back("Temp" + to_string(atomicMass) + PeriodicTable::GetAtomicNameCap(atomicNumber) + ".xml");

    atomicNumber = daughterNuclide->GetAtomicNumber();
    atomicMass = daughterNuclide->GetAtomicMass();
    qBeta = daughterNuclide->GetQBeta();
    pugi::xml_document nuclideFile_2;
    nodeNuclide = nuclideFile_2.append_child("Nuclide");
    nodeNuclide.append_attribute("AtomicNumber") = atomicNumber;
    nodeNuclide.append_attribute("AtomicMass") = atomicMass;
    nodeNuclide.append_attribute("QBeta") = qBeta;

    for ( auto jt = daughterNuclide->GetNuclideLevels()->begin(); jt != daughterNuclide->GetNuclideLevels()->end(); ++jt )
    {
        for ( auto kt = jt->GetTransitions()->begin(); kt != jt->GetTransitions()->end(); ++kt )
        {
            if( *kt == secondTransition)
            {
                string particleType =  (*kt)->GetParticleType();
                if( particleType == "Fake")
                    break;

                double levelEnergy = jt->GetLevelEnergy();
                double spin = jt->GetSpin();
                string parity = jt->GetParity();
                double halfLifeTimeInSeconds = jt->GetHalfLifeTime();

                pugi::xml_node nodeLevel = nodeNuclide.append_child("Level");
                nodeLevel.append_attribute("Energy").set_value(toStringPrecision(levelEnergy,2).c_str());
                nodeLevel.append_attribute("Spin").set_value(toStringPrecision(spin,2).c_str());
                nodeLevel.append_attribute("Parity") = parity.c_str();
                nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,9).c_str());
                nodeLevel.append_attribute("TimeUnit") = "S";

                double transitionQValue =  (*kt)->GetTransitionQValue();
                //double intensity = (*kt)->GetIntensity();
                double finalLevelEnergy = (*kt)->GetFinalLevelEnergy();
                int finalLevelAtomicMass = (*kt)->GetFinalLevelAtomicMass();
                int finalLevelAtomicNumber = (*kt)->GetFinalLevelAtomicNumber();

                pugi::xml_node nodeTransition = nodeLevel.append_child("Transition");
                nodeTransition.append_attribute("Type") = particleType.c_str();
                nodeTransition.append_attribute("TransitionQValue").set_value(toStringPrecision(transitionQValue,2).c_str());
                nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(1,6).c_str());

                if (particleType == "G")
                {
                    double eCC = (*kt)->GetElectronConversionCoefficient();
                    if (eCC > 0.)
                    {
                        //Check later whether 6 precision is enough
                        pugi::xml_node nodeConversion = nodeTransition.append_child("ElectronConversionCoefficient");
                        nodeConversion.append_attribute("Total").set_value(toStringPrecision(eCC,6).c_str());
                        double* shellECC = (*kt)->GetShellElectronConversionCoefficient();
                        nodeConversion.append_attribute("KC").set_value(toStringPrecision(shellECC[0],6).c_str());
                        nodeConversion.append_attribute("LC").set_value(toStringPrecision(shellECC[1]-shellECC[0],6).c_str());
                        nodeConversion.append_attribute("MC").set_value(toStringPrecision(shellECC[2]-shellECC[1],6).c_str());
                        nodeConversion.append_attribute("NC").set_value(toStringPrecision(shellECC[3]-shellECC[2],6).c_str());

                    }
                }

                pugi::xml_node nodeTargetLevel = nodeTransition.append_child("TargetLevel");
                nodeTargetLevel.append_attribute("Energy").set_value(toStringPrecision(finalLevelEnergy,2).c_str());
                nodeTargetLevel.append_attribute("AtomicNumber") = finalLevelAtomicNumber;
                nodeTargetLevel.append_attribute("AtomicMass") = finalLevelAtomicMass;

                break;
            }
        }
    }

    //vector <Nuclide*> otherNuclidesToWrite;
    vector<double> levelsToClearFromMap;

    for (auto itm = additionalLevelsToWrite.begin(); itm != additionalLevelsToWrite.end(); ++itm)
    {
        Nuclide* nuclideToWrite = (itm->second).second;
        if( nuclideToWrite == daughterNuclide )
        {
            Level* levelToWrite = (itm->second).first;
                double levelEnergy = levelToWrite->GetLevelEnergy();
            levelsToClearFromMap.push_back(levelEnergy);
                double spin = levelToWrite->GetSpin();
                string parity = levelToWrite->GetParity();
                double halfLifeTimeInSeconds = levelToWrite->GetHalfLifeTime();

                pugi::xml_node nodeLevel = nodeNuclide.append_child("Level");
                nodeLevel.append_attribute("Energy").set_value(toStringPrecision(levelEnergy,2).c_str());
                nodeLevel.append_attribute("Spin").set_value(toStringPrecision(spin,2).c_str());
                nodeLevel.append_attribute("Parity") = parity.c_str();
                nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,9).c_str());
                nodeLevel.append_attribute("TimeUnit") = "S";

                for ( auto kt = levelToWrite->GetTransitions()->begin(); kt != levelToWrite->GetTransitions()->end(); ++kt )
                {
                    string particleType =  (*kt)->GetParticleType();
                    double transitionQValue =  (*kt)->GetTransitionQValue();
                    double intensity = (*kt)->GetIntensity();
                    double finalLevelEnergy = (*kt)->GetFinalLevelEnergy();
                    int finalLevelAtomicMass = (*kt)->GetFinalLevelAtomicMass();
                    int finalLevelAtomicNumber = (*kt)->GetFinalLevelAtomicNumber();

                    pugi::xml_node nodeTransition = nodeLevel.append_child("Transition");
                    nodeTransition.append_attribute("Type") = particleType.c_str();
                    nodeTransition.append_attribute("TransitionQValue").set_value(toStringPrecision(transitionQValue,2).c_str());
                    nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,6).c_str());

                    if (particleType == "G")
                    {
                        double eCC = (*kt)->GetElectronConversionCoefficient();
                        if (eCC > 0.)
                        {
                            //Check later whether 6 precision is enough
                            pugi::xml_node nodeConversion = nodeTransition.append_child("ElectronConversionCoefficient");
                            nodeConversion.append_attribute("Total").set_value(toStringPrecision(eCC,6).c_str());
                            double* shellECC = (*kt)->GetShellElectronConversionCoefficient();
                            nodeConversion.append_attribute("KC").set_value(toStringPrecision(shellECC[0],6).c_str());
                            nodeConversion.append_attribute("LC").set_value(toStringPrecision(shellECC[1]-shellECC[0],6).c_str());
                            nodeConversion.append_attribute("MC").set_value(toStringPrecision(shellECC[2]-shellECC[1],6).c_str());
                            nodeConversion.append_attribute("NC").set_value(toStringPrecision(shellECC[3]-shellECC[2],6).c_str());

                        }
                    }

                    pugi::xml_node nodeTargetLevel = nodeTransition.append_child("TargetLevel");
                    nodeTargetLevel.append_attribute("Energy").set_value(toStringPrecision(finalLevelEnergy,2).c_str());
                    nodeTargetLevel.append_attribute("AtomicNumber") = finalLevelAtomicNumber;
                    nodeTargetLevel.append_attribute("AtomicMass") = finalLevelAtomicMass;

                }

        }
    }

    for( auto it = levelsToClearFromMap.begin(); it != levelsToClearFromMap.end(); ++it)
    {
        additionalLevelsToWrite.erase(*it);
    }

    nuclideOutputFilename = path_ + "Temp" + to_string(atomicMass) + PeriodicTable::GetAtomicNameCap(atomicNumber) + ".xml";
    //cout << "Saving result of " << nuclideOutputFilename << " : " << nuclideFile_2.save_file(nuclideOutputFilename.c_str()) << endl;
    nuclideFile_2.save_file(nuclideOutputFilename.c_str());
    fullFileNames_.push_back(nuclideOutputFilename);
    shortFileNames_.push_back("Temp" + to_string(atomicMass) + PeriodicTable::GetAtomicNameCap(atomicNumber) + ".xml");

    if( additionalLevelsToWrite.size() > 0 )
    {
        bool write = false;

        for ( auto it = nuclidesVector->begin(); it != nuclidesVector->end(); ++it )
        {
            for (auto itm = additionalLevelsToWrite.begin(); itm != additionalLevelsToWrite.end(); ++itm)
            {
                Nuclide* nuclideToWrite = (itm->second).second;
                if( nuclideToWrite == &(*it) )
                {
                    write = true;
                    break;
                }
            }

            if( write )
            {
                int atomicNumber = it->GetAtomicNumber();
                int atomicMass = it->GetAtomicMass();
                double qBeta = it->GetQBeta();
                pugi::xml_document nuclideFile;
                pugi::xml_node nodeNuclide = nuclideFile.append_child("Nuclide");
                nodeNuclide.append_attribute("AtomicNumber") = atomicNumber;
                nodeNuclide.append_attribute("AtomicMass") = atomicMass;
                nodeNuclide.append_attribute("QBeta") = qBeta;

                for (auto itm = additionalLevelsToWrite.begin(); itm != additionalLevelsToWrite.end(); ++itm)
                {
                        Level* levelToWrite = (itm->second).first;
                            double levelEnergy = levelToWrite->GetLevelEnergy();
                            double spin = levelToWrite->GetSpin();
                            string parity = levelToWrite->GetParity();
                            double halfLifeTimeInSeconds = levelToWrite->GetHalfLifeTime();

                            pugi::xml_node nodeLevel = nodeNuclide.append_child("Level");
                            nodeLevel.append_attribute("Energy").set_value(toStringPrecision(levelEnergy,2).c_str());
                            nodeLevel.append_attribute("Spin").set_value(toStringPrecision(spin,2).c_str());
                            nodeLevel.append_attribute("Parity") = parity.c_str();
                            nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,9).c_str());
                            nodeLevel.append_attribute("TimeUnit") = "S";

                            for ( auto kt = levelToWrite->GetTransitions()->begin(); kt != levelToWrite->GetTransitions()->end(); ++kt )
                            {
                                string particleType =  (*kt)->GetParticleType();
                                double transitionQValue =  (*kt)->GetTransitionQValue();
                                double intensity = (*kt)->GetIntensity();
                                double finalLevelEnergy = (*kt)->GetFinalLevelEnergy();
                                int finalLevelAtomicMass = (*kt)->GetFinalLevelAtomicMass();
                                int finalLevelAtomicNumber = (*kt)->GetFinalLevelAtomicNumber();

                                pugi::xml_node nodeTransition = nodeLevel.append_child("Transition");
                                nodeTransition.append_attribute("Type") = particleType.c_str();
                                nodeTransition.append_attribute("TransitionQValue").set_value(toStringPrecision(transitionQValue,2).c_str());
                                nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,6).c_str());

                                if (particleType == "G")
                                {
                                    double eCC = (*kt)->GetElectronConversionCoefficient();
                                    if (eCC > 0.)
                                    {
                                        //Check later whether 6 precision is enough
                                        pugi::xml_node nodeConversion = nodeTransition.append_child("ElectronConversionCoefficient");
                                        nodeConversion.append_attribute("Total").set_value(toStringPrecision(eCC,6).c_str());
                                        double* shellECC = (*kt)->GetShellElectronConversionCoefficient();
                                        nodeConversion.append_attribute("KC").set_value(toStringPrecision(shellECC[0],6).c_str());
                                        nodeConversion.append_attribute("LC").set_value(toStringPrecision(shellECC[1]-shellECC[0],6).c_str());
                                        nodeConversion.append_attribute("MC").set_value(toStringPrecision(shellECC[2]-shellECC[1],6).c_str());
                                        nodeConversion.append_attribute("NC").set_value(toStringPrecision(shellECC[3]-shellECC[2],6).c_str());

                                    }
                                }

                                pugi::xml_node nodeTargetLevel = nodeTransition.append_child("TargetLevel");
                                nodeTargetLevel.append_attribute("Energy").set_value(toStringPrecision(finalLevelEnergy,2).c_str());
                                nodeTargetLevel.append_attribute("AtomicNumber") = finalLevelAtomicNumber;
                                nodeTargetLevel.append_attribute("AtomicMass") = finalLevelAtomicMass;

                            }
                }
                nuclideOutputFilename = path_ + "Temp" + to_string(atomicMass) + PeriodicTable::GetAtomicNameCap(atomicNumber) + ".xml";
                //cout << "Saving result of " << nuclideOutputFilename << " : " << nuclideFile.save_file(nuclideOutputFilename.c_str()) << endl;
                nuclideFile.save_file(nuclideOutputFilename.c_str());
                fullFileNames_.push_back(nuclideOutputFilename);
                shortFileNames_.push_back("Temp" + to_string(atomicMass) + PeriodicTable::GetAtomicNameCap(atomicNumber) + ".xml");

                write = false;
            }
        }
    }
}


//start nuclide, stop nuclide jako argumenty?
void SaveDecayData::CreateDecayXML(Transition* firstTransition, Transition* secondTransition)
{
    int firstAtomicNumber, firstAtomicMass;
    double firstLevelEnergy, firstTransitionEnergy;
    string firstTransitionType;
    int secondAtomicNumber, secondAtomicMass;
    double secondLevelEnergy, secondTransitionEnergy;
    string secondTransitionType;

    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

    string xmlFileName = path_ + "TempDecay.xml";
    shortFileNames_.push_back("TempDecay.xml");
    fullFileNames_.push_back(xmlFileName);
    ofstream xmlFile(xmlFileName.c_str());
    if (!xmlFile.is_open())
        cout << "Warning message: The file " + (string) xmlFileName + " is not open!" << endl;

    xmlFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    xmlFile << endl;

    for ( auto it = nuclidesVector->begin(); it != nuclidesVector->end(); ++it )
    {
        string nuclideFileName = to_string(it->GetAtomicMass()) + PeriodicTable::GetAtomicNameCap(it->GetAtomicNumber()) + ".xml";
        xmlFile << "<NuclideFile FileName=\"" + nuclideFileName + "\"/>" << endl;

        for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
        {
            for ( auto kt = jt->GetTransitions()->begin(); kt != jt->GetTransitions()->end(); ++kt )
            {
                if( (*kt) == firstTransition )
                {
                    firstAtomicNumber = it->GetAtomicNumber();
                    firstAtomicMass = it->GetAtomicMass();
                    firstLevelEnergy = jt->GetLevelEnergy();
                    firstTransitionEnergy = (*kt)->GetTransitionQValue();
                    firstTransitionType = (*kt)->GetParticleType();
                }
                if( (*kt) == secondTransition )
                {
                    secondAtomicNumber = it->GetAtomicNumber();
                    secondAtomicMass = it->GetAtomicMass();
                    secondLevelEnergy = jt->GetLevelEnergy();
                    secondTransitionEnergy = (*kt)->GetTransitionQValue();
                    secondTransitionType = (*kt)->GetParticleType();
                }
            }
        }
    }

    xmlFile << "<StartLevel AtomicNumber=\"" << firstAtomicNumber << "\" AtomicMass=\"" << firstAtomicMass << "\" Energy=\"0.0\"/>" << endl;
    if( nuclidesVector->size() == 2 )
        xmlFile << "<StopLevel AtomicNumber=\"" << firstAtomicNumber + 1 << "\" AtomicMass=\"" << firstAtomicMass << "\" Energy=\"0.0\"/>" << endl;
    else if( nuclidesVector->size() == 3 )
        xmlFile << "<StopLevel AtomicNumber=\"" << firstAtomicNumber + 1 << "\" AtomicMass=\"" << firstAtomicMass - 1 << "\" Energy=\"0.0\"/>" << endl;
    else
        cout << "Unusual number of nuclides, StopLevel not defined." << endl;
    xmlFile << "<EventLength Value=\"" << decayPath->GetEventDurationInSeconds() << "\" TimeUnit=\"S\"/>" << endl;
    xmlFile << "<CycleLength Value=\"" << decayPath->GetCycleDurationInSeconds() << "\" TimeUnit=\"S\"/>" << endl ;

    xmlFile << "<SpecifyFirstTransition NuclideAtomicNumber=\"" << firstAtomicNumber << "\" NuclideAtomicMass=\"" <<
               firstAtomicMass << "\">" << endl;
    xmlFile << "\t<InitialLevel Energy=\"" << firstLevelEnergy << "\"/>" << endl;
    xmlFile << "\t<SpecifyTransition Type=\"" << firstTransitionType <<
               "\" TransitionQValue=\"" << firstTransitionEnergy << "\"/>" << endl;
    xmlFile << "</SpecifyFirstTransition>" << endl;

    if (secondTransitionEnergy != 0.)
    {
        xmlFile << "<SpecifySecondTransition NuclideAtomicNumber=\"" << secondAtomicNumber << "\" NuclideAtomicMass=\"" <<
                   secondAtomicMass << "\">" << endl;
        xmlFile << "\t<InitialLevel Energy=\"" << secondLevelEnergy << "\"/>" << endl;
        xmlFile << "\t<SpecifyTransition Type=\"" << secondTransitionType <<
                   "\" TransitionQValue=\"" << secondTransitionEnergy << "\"/>" << endl;
        xmlFile << "</SpecifySecondTransition>" << endl;
    }

    xmlFile.close();
    //cout << "File Decay.xml created for transitions " << secondLevelEnergy << " - " << secondTransitionEnergy << "." << endl;
    //cout << "File Name: " << xmlFileName << endl;
}

void SaveDecayData::GeneralDecayInfo()
{
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();
    Nuclide* motherNuclide = &nuclidesVector->at(0);

    averageBetaEnergy_ = 0.;
    d_averageBetaEnergy_ = 0.;
    averageGammaEnergy_ = 0.;
    d_averageGammaEnergy_ = 0.;
    averageNeutronEnergy_ = 0.;
    d_averageNeutronEnergy_ = 0.;
    averageNeutronGammaEnergy_ = 0.;
    neutronPercentage_ = 0.;
    growingIntensity_ = 0.;
    numberOfGammaAddedLevels_ = 0;
    numberOfGammaDatabaseLevels_ = 0;
    numberOfNeutronAddedLevels_ = 0;
    numberOfNeutronDatabaseLevels_ = 0;
    numberOfUniqueAddedGammas_ = 0;
    numberOfUniqueDatabaseGammas_ = 0;
    gammaAverageMultiplicity_ = 0.;
    gammaAverageMultiplicityNotBetaWeighted_ = 0.;

    // Find gamma multiplicy beginning. Neutron and gamma levels have to be separated!
    // It doesn't take IC into account.
    Level* motherLevel = &motherNuclide->GetNuclideLevels()->at(0);
    Level* stopLevel = decayPath->GetPointerToStopLevel();
    for ( auto it = motherLevel->GetTransitions()->begin(); it != motherLevel->GetTransitions()->end(); ++it )
    {
        string particleType =  (*it)->GetParticleType();
        if (particleType != "B-" && particleType != "B+")
        {
            cout << "Something is WRONG with motherLevel and gammaAverageMultiplicity!" << endl;
            continue;
        }
        double betaIntensity = (*it)->GetIntensity();
        Level* daughterLevel = (*it)->GetPointerToFinalLevel();
        if( daughterLevel->GetNeutronLevelStatus() )
        {
            continue;
        }

        // It assumes if it doesn't deexctitate by neutron, it always does by gamma
        double tempMultiplicityValue = CalcGammaMultiplictyFromLevel(daughterLevel, stopLevel);
        gammaAverageMultiplicity_ += betaIntensity * tempMultiplicityValue;
        gammaAverageMultiplicityNotBetaWeighted_ += tempMultiplicityValue;
    }
    // End of gamma multiplicy calculations

    for ( auto lt = motherNuclide->GetNuclideLevels()->begin(); lt != motherNuclide->GetNuclideLevels()->end(); ++lt )
    {
        for ( auto kt = lt->GetTransitions()->begin(); kt != lt->GetTransitions()->end(); ++kt )
        {
            string particleType =  (*kt)->GetParticleType();
            //double transitionQValue =  (*kt)->GetTransitionQValue();
            double intensity = (*kt)->GetIntensity() * 100.;
            double uncertainty = (*kt)->GetD_Intensity() * 100.;
            if(uncertainty < 1e-6)
                uncertainty = 0.;
            double finalLevelEnergy = (*kt)->GetFinalLevelEnergy();
            //int finalLevelAtomicMass = (*kt)->GetFinalLevelAtomicMass();
            //int finalLevelAtomicNumber = (*kt)->GetFinalLevelAtomicNumber();
            //bool isAddedTransition = (*kt)->IsAddedTransition();
            Level* finalLevel = (*kt)->GetPointerToFinalLevel();
            bool isPseudoLevel = finalLevel->isPseudoLevel();
            int numberOfTransitionsFromFinalLevel = finalLevel->GetTransitions()->size();

            if (particleType == "B-" || particleType == "B+")
            {
                (*kt)->CalculateAverageBetaEnergy();
                double averageLvlBetaEnergy = (*kt)->GetAverageBetaEnergy();
                averageBetaEnergy_+= intensity * averageLvlBetaEnergy / 100;
                d_averageBetaEnergy_+= pow( averageLvlBetaEnergy * uncertainty / 100., 2 );

                if(finalLevel->GetNeutronLevelStatus())
                {
                    if(isPseudoLevel)
                        numberOfNeutronAddedLevels_++;
                    else
                        numberOfNeutronDatabaseLevels_++;

                    neutronPercentage_+= intensity;
                    //string neutronsEnergies = " ";
                    int nEnergy = 0;
                    int targetLvlEnergy = 0;
                    for(auto nt = finalLevel->GetTransitions()->begin(); nt != finalLevel->GetTransitions()->end(); ++nt)
                    {
                        //neutronsEnergies += (*nt)->GetParticleType() + to_string_with_precision((*nt)->GetIntensity(),0);
                        //neutronsEnergies += "E" + to_string_with_precision((*nt)->GetTransitionQValue(),0);
                        nEnergy = (*nt)->GetTransitionQValue();
                        targetLvlEnergy = (*nt)->GetFinalLevelEnergy();
                        double nIntensity = (*nt)->GetIntensity();
                        if(targetLvlEnergy > 1.)
                        {
                            averageGammaEnergy_+= targetLvlEnergy * intensity / 100 * nIntensity;
                            // Uncertainty of gamma part in beta-n-gamma transitions should be calculated with
                            // d_beta-n; here it is calculated with d_beta and then multiplied by neutron intensity
                            d_averageGammaEnergy_+= pow( targetLvlEnergy * uncertainty / 100. * nIntensity, 2);
                            averageNeutronGammaEnergy_ += targetLvlEnergy * intensity / 100 * nIntensity;
                        }
                        averageNeutronEnergy_+= nEnergy * intensity / 100 * nIntensity;
                    }

//MK                    outputFile << finalLevelEnergy << " " << intensity << " " << uncertainty << " "
//MK                               << nEnergy << " " << targetLvlEnergy << endl;
                }
                else
                {
                    if(isPseudoLevel)
                        numberOfGammaAddedLevels_++;
                    else
                        numberOfGammaDatabaseLevels_++;

                    for(auto llt = finalLevel->GetTransitions()->begin(); llt != finalLevel->GetTransitions()->end(); ++llt)
                    {
                        if((*llt)->GetParticleType() != "G")
                            continue;
                        bool isAddedTransition = (*llt)->IsAddedTransition();
                        if(isAddedTransition)
                            numberOfUniqueAddedGammas_++;
                        else
                            numberOfUniqueDatabaseGammas_++;
                    }

                    growingIntensity_+= intensity;
                    averageGammaEnergy_+= finalLevelEnergy * intensity / 100;
                    d_averageGammaEnergy_+= pow( finalLevelEnergy * uncertainty / 100, 2);
//MK                    outputFile << finalLevelEnergy << " " << intensity << " "
//MK                               << uncertainty << " " << growingIntensity_<< endl;
                }
            }
        }
    }

    d_averageGammaEnergy_= sqrt(d_averageGammaEnergy_);
    d_averageBetaEnergy_= sqrt(d_averageBetaEnergy_);


}

void SaveDecayData::SaveGeneralDecayInfo(std::string path)
{
    GeneralDecayInfo();
    double growingIntensity = 0.;

    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();
    Nuclide* motherNuclide = &nuclidesVector->at(0);
    //Nuclide* daughterNuclide = &nuclidesVector->at(1);
    string outputFilename = path + "/GeneralDecayInfo.txt";
    ofstream outputFile(outputFilename.c_str());
    if (!outputFile.is_open())
        cout << "Warning message: The file " + (string) outputFilename + " is not open!" << endl;

    outputFile << "#LevelEnergy | BetaFeeding | Uncertainty | GrowingBetaFeeding(gammas)/Energy(neutrons) | FinalLevel(neutrons) | NeutronIntensity" << endl;

    Level* motherLevel = &motherNuclide->GetNuclideLevels()->at(0);
    Level* stopLevel = decayPath->GetPointerToStopLevel();
    for ( auto it = motherLevel->GetTransitions()->begin(); it != motherLevel->GetTransitions()->end(); ++it )
    {
        string particleType =  (*it)->GetParticleType();
        if (particleType != "B-" && particleType != "B+")
        {
            outputFile << "Something is WRONG with motherLevel and gammaAverageMultiplicity!" << endl;
            continue;
        }

    }
    // End of gamma multiplicy calculations

    for ( auto lt = motherNuclide->GetNuclideLevels()->begin(); lt != motherNuclide->GetNuclideLevels()->end(); ++lt )
    {
        for ( auto kt = lt->GetTransitions()->begin(); kt != lt->GetTransitions()->end(); ++kt )
        {
            string particleType =  (*kt)->GetParticleType();
            //double transitionQValue =  (*kt)->GetTransitionQValue();
            double intensity = (*kt)->GetIntensity() * 100.;
            double uncertainty = (*kt)->GetD_Intensity() * 100.;
            if(uncertainty < 1e-6)
                uncertainty = 0.;
            double finalLevelEnergy = (*kt)->GetFinalLevelEnergy();
            Level* finalLevel = (*kt)->GetPointerToFinalLevel();
//MK            bool isPseudoLevel = finalLevel->isPseudoLevel();
//MK            int numberOfTransitionsFromFinalLevel = finalLevel->GetTransitions()->size();

            if (particleType == "B-" || particleType == "B+")
            {
//MK                (*kt)->CalculateAverageBetaEnergy();

                if(finalLevel->GetNeutronLevelStatus())
                {
//MK                    neutronPercentage += intensity;
                    int nEnergy = 0;
                    int targetLvlEnergy = 0;
                    for(auto nt = finalLevel->GetTransitions()->begin(); nt != finalLevel->GetTransitions()->end(); ++nt)
                    {
                        nEnergy = (*nt)->GetTransitionQValue();
                        targetLvlEnergy = (*nt)->GetFinalLevelEnergy();
                        double nIntensity = (*nt)->GetIntensity();

                        outputFile << finalLevelEnergy << " " << intensity << " " << uncertainty << " "
                                   << nEnergy << " " << targetLvlEnergy << " " << nIntensity << endl;
                    }
                }
                else
                {
                    growingIntensity += intensity;
                    outputFile << finalLevelEnergy << " " << intensity << " "
                               << uncertainty << " " << growingIntensity << endl;
                }
            }
        }
    }


    //averageGammaEnergy /= (100 - neutronPercentage);
    outputFile << "#AverageGammaEnergy = " << averageGammaEnergy_ << " +- " << d_averageGammaEnergy_ << endl;
    outputFile << "#AverageNeutronGammaEnergy = " << averageNeutronGammaEnergy_ << endl;
    outputFile << "#AverageBetaEnergy = " << averageBetaEnergy_ << " +- " << d_averageBetaEnergy_ << endl;
    outputFile << "#averageNeutronEnergy = " << averageNeutronEnergy_ << endl;
    outputFile << "#NeutronPercentage = " << neutronPercentage_<< endl;
    outputFile << "#AverageGammaMultiplicity = " << gammaAverageMultiplicity_ << endl;
    outputFile << "#AverageGammaMultiplicityNotBetaWeighted = " << gammaAverageMultiplicityNotBetaWeighted_ << endl;
    outputFile << "#numberOfGammaAddedLevels + numberOfGammaDatabaseLevels = "
               << numberOfGammaAddedLevels_<< " + " << numberOfGammaDatabaseLevels_<< endl;
    outputFile << "#numberOfNeutronAddedLevels + numberOfNeutronDatabaseLevels = "
               << numberOfNeutronAddedLevels_<< " + " << numberOfNeutronDatabaseLevels_<< endl;
    outputFile << "#numberOfUniqueAddedGammas + numberOfUniqueDatabaseGammas = "
               << numberOfUniqueAddedGammas_ << " + " << numberOfUniqueDatabaseGammas_ << endl;
    outputFile.close();
}

double SaveDecayData::CalcGammaMultiplictyFromLevel(Level* currentLevel, Level* stopLevel, int gammaMultiplicity)
{
    double averageGammaMultiplicityFromThisLvl = 0.;
    for( auto kt = currentLevel->GetTransitions()->begin(); kt != currentLevel->GetTransitions()->end(); ++kt)
    {
        double gammaIntensity = (*kt)->GetIntensity();
        //double gammaEnergy = (*kt)->GetTransitionQValue();
        string particleType =  (*kt)->GetParticleType();
        if( particleType == "Fake")
            continue;
        Level* nextLevel = (*kt)->GetPointerToFinalLevel();
        double nextLevelEnergy = nextLevel->GetLevelEnergy();
        if(nextLevel == stopLevel || nextLevel->GetTransitions()->empty() || nextLevelEnergy < 1)
        {
            averageGammaMultiplicityFromThisLvl += gammaIntensity * gammaMultiplicity;
            continue;
        }
        else
        {
            double intensityMultiplier = CalcGammaMultiplictyFromLevel(nextLevel, stopLevel, gammaMultiplicity + 1);
            averageGammaMultiplicityFromThisLvl += intensityMultiplier * gammaIntensity;
        }
    }
    return averageGammaMultiplicityFromThisLvl;
}

void SaveDecayData::SaveGammaEvolution()
{
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();
    Nuclide* motherNuclide = &nuclidesVector->at(0);
    //Nuclide* daughterNuclide = &nuclidesVector->at(1);

    Project* myProject = Project::get();
    std::pair<int, int> energyGate = myProject->getGammaEvolutionEnergies();
    string tempFilename = "/GammaEvolution_" + to_string(energyGate.first) + "_" +
            to_string(energyGate.second) + ".txt";
    string outputFilename = path_ + tempFilename;
    ofstream outputFile(outputFilename.c_str());
    if (!outputFile.is_open())
        cout << "Warning message: The file " + (string) outputFilename + " is not open!" << endl;

    outputFile << "#GammaIntensityEvolutionToLevelsInRange: " << energyGate.first << " " << energyGate.second << endl;
    outputFile << "#InitialLevelEnergy | TotalGammaIntensityToChoosenLevels | BetaIntensityInitialLevel" << endl;

    for ( auto lt = motherNuclide->GetNuclideLevels()->begin(); lt != motherNuclide->GetNuclideLevels()->end(); ++lt )
    {
        for ( auto kt = lt->GetTransitions()->begin(); kt != lt->GetTransitions()->end(); ++kt )
        {
            double initialLvlEnergy = (*kt)->GetFinalLevelEnergy();
            double betaIntensity = (*kt)->GetIntensity();
            Level* initialLevel = (*kt)->GetPointerToFinalLevel();
            if(initialLevel->GetNeutronLevelStatus())
                continue;
            //double betaIntensitySum = 0.;
            double totalGammaIntensity = 0.;
            for ( auto kt2 = initialLevel->GetTransitions()->begin(); kt2 != initialLevel->GetTransitions()->end(); ++kt2 )
            {
                double finalLevelEn = (*kt2)->GetFinalLevelEnergy();
                if(finalLevelEn >= energyGate.first && finalLevelEn < energyGate.second)
                {
                    totalGammaIntensity += (*kt2)->GetIntensity();
                }
            }
            outputFile << initialLvlEnergy << " " << totalGammaIntensity << " " << betaIntensity << endl;
        }
    }

    outputFile.close();
}

void SaveDecayData::SaveENSDecayStructure()
{

    GeneralDecayInfo();

    DecayPath* decayPath = DecayPath::get();
    delayedNeutrons_ = false;
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();
    Nuclide* motherNuclide = &nuclidesVector->at(0);
    Nuclide* daughterNuclide = &nuclidesVector->at(1);
    Nuclide* granddaughterNuclide = &nuclidesVector->at(2);

    int daughterAtomicNumber = daughterNuclide->GetAtomicNumber();
    int daughterAtomicMass = daughterNuclide->GetAtomicMass();
    int motherAtomicNumber = motherNuclide->GetAtomicNumber();
    int motherAtomicMass = motherNuclide->GetAtomicMass();
    int granddaughterAtomicNumber = granddaughterNuclide->GetAtomicNumber();
    int granddaughterAtomicMass = granddaughterNuclide->GetAtomicMass();
//    double qBeta = motherNuclide->GetQBeta();

    string outputFilename = path_ + to_string(motherAtomicMass) +
            PeriodicTable::GetAtomicNameCap(motherAtomicNumber) + "_decay.ens";
    ofstream outputFile(outputFilename.c_str());
    if (!outputFile.is_open())
        cout << "Warning message: The file " + (string) outputFilename + " is not open!" << endl;

    QString test = QString("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
    QString qMass = QString("%1").arg(motherAtomicMass,3,10);
    QString qstr4to5=QString::fromStdString(PeriodicTable::GetAtomicNameCap(motherAtomicNumber));
    QString motherHeader = qMass+qstr4to5;
    QString qMassd = QString("%1").arg(daughterAtomicMass,3,10);
    QString qstr4to5d=QString::fromStdString(PeriodicTable::GetAtomicNameCap(daughterAtomicNumber));
    QString daughterHeader = qMassd+qstr4to5d;
    QString qMassgd = QString("%1").arg(granddaughterAtomicMass,3,10);
    QString qstr4to5gd=QString::fromStdString(PeriodicTable::GetAtomicNameCap(granddaughterAtomicNumber));
    QString granddaughterHeader = qMassgd+qstr4to5gd;

//    outputFile << test.toStdString();

    // write two first lines to ens file
    // first one is identification record
    QString Id = "B-";  // B- only file
    outputFile << setENSIdentificationRec(Id,daughterHeader, motherNuclide).toStdString();
    outputFile << setENSQvalueRec(motherHeader, motherNuclide).toStdString();

    // Normalization record, it is needed apparently
    outputFile << setENSNormRec(daughterHeader).toStdString();
    // Production normalization record, tells how to display intensities
    outputFile << setENSPNormRec(daughterHeader).toStdString();

    QString LComA, RType, Sym, ENSFlag;
    QString GComA;
    ENSFlag=" ";
    LComA = QString("Quasi-level, energy denotes the center of the quasi-level.");
    GComA = QString("Added transition.");
    RType = "L";
    Sym = QString("E");
//    ENSFlag = QString("A");
    outputFile << setENSComRec(daughterHeader, "L", "E", "A", LComA ).toStdString();
    outputFile << setENSComRec(daughterHeader, "G", "E", "A", GComA ).toStdString();

    bool delayedParticle = false;
  // start from mother nuclide in case we will have isomer beta decaying
        for ( auto lt = motherNuclide->GetNuclideLevels()->begin(); lt != motherNuclide->GetNuclideLevels()->end(); ++lt )
    {
        outputFile << setENSParentRec(nuclidesVector,delayedParticle).toStdString();
       // loop over  daughter levels
        for ( auto ll = daughterNuclide->GetNuclideLevels()->begin(); ll != daughterNuclide->GetNuclideLevels()->end(); ++ll )
        {
            Level* currentLevel =  &(*ll);
            if(currentLevel->isPseudoLevel())ENSFlag = QString("A");

            outputFile << setENSLevelRec(daughterHeader, ENSFlag, &(*ll)).toStdString();
            // find beta transition
            Transition* currentBetaTransition;
            for ( auto kt = lt->GetTransitions()->begin(); kt != lt->GetTransitions()->end(); ++kt )
            {
             if((*kt)->GetPointerToFinalLevel() == &(*ll))
             {
                 double totalIntensity = (*kt)->GetIntensity()*100;
                 currentBetaTransition = (*kt);
                 if (totalIntensity > 0.001)
                 {
                     //output beta transition to the level details
                     outputFile << setENSBetaRec(daughterHeader, (*kt)).toStdString();
                 }
             }
            }
            // output transitions from the level
            for ( auto tt = currentLevel->GetTransitions()->rbegin(); tt != currentLevel->GetTransitions()->rend(); ++tt )
            {
                // prepare and write Gamma lines (NOT correct for neutrons and other transitions propably)
                // for now eCC lines are not completly output
                double transitionEnergy = (*tt)->GetTransitionQValue();
                double electConvCoef = 0.; //(*tt)->GetElectronConversionCoefficient();

                string type = (*tt)->GetParticleType();
                if(transitionEnergy != 0.0)
                {
                    double totalIntensity = (*tt)->GetIntensity()*100;
                    if((type == "G") &(totalIntensity > 0.001))
                    outputFile << setENSGammaRec(daughterHeader, (*tt), currentBetaTransition).toStdString();
                if(type == "N") delayedNeutrons_=true;
                // dodace conversje elektronow
                // comment line only if Transition was added during analysis
                }
            }
        }

// to be cut out if the rest works!!!mk - 20240404
  /*      for ( auto kt = lt->GetTransitions()->begin(); kt != lt->GetTransitions()->end(); ++kt )
        {
            string particleType =  (*kt)->GetParticleType();

            if (particleType == "B-" || particleType == "B+")
            {
                // prepare and write Level line
                Level* finalLevel = (*kt)->GetPointerToFinalLevel();

                outputFile << setENSLevelRec(daughterHeader, finalLevel).toStdString();

                // write Beta line
                double totalIntensity = (*kt)->GetIntensity()*100;
                if (totalIntensity > 0.001)
                {
                    outputFile << setENSBetaRec(daughterHeader, (*kt)).toStdString();
                }
                for ( auto tt = finalLevel->GetTransitions()->begin(); tt != finalLevel->GetTransitions()->end(); ++tt )
                {
                    // prepare and write Gamma lines (NOT correct for neutrons and other transitions propably)
                    // for now eCC lines are not completly output
                    double transitionEnergy = (*tt)->GetTransitionQValue();
                    double electConvCoef = 0.; //(*tt)->GetElectronConversionCoefficient();

                    string type = (*tt)->GetParticleType();
                    if(transitionEnergy != 0.0)
                    {
                    //if(type == "N")outputFile << setENSPartRec(granddaughterHeader, (*tt)).toStdString();
                        double totalIntensity = (*tt)->GetIntensity()*100;
                        if((type == "G") &(totalIntensity > 0.001))
                        outputFile << setENSGammaRec(daughterHeader, (*tt), (*kt)).toStdString();
                    if(type == "N") delayedNeutrons_=true;
                    // dodace conversje elektronow
                    // comment line only if Transition was added during analysis
                    }
                }
            }
        } */
    }

    outputFile << "\n";

    // Neutron output
    Transition* betaTransition;
    if(delayedNeutrons_)
    {
        ENSFlag=" ";
        QString Id = "B-n";
     outputFile << setENSIdentificationRec(Id,granddaughterHeader, motherNuclide).toStdString();
     outputFile << setENSParentRec(nuclidesVector,delayedNeutrons_).toStdString();

      // Normalization record, it is needed apparently
     outputFile << setENSNormRec(granddaughterHeader).toStdString();
     // Production normalization record, tells how to display intensities
     outputFile << setENSPNormRec(granddaughterHeader).toStdString();

    for(auto dt=granddaughterNuclide->GetNuclideLevels()->begin(); dt != granddaughterNuclide->GetNuclideLevels()->end(); ++dt)
      {
        if(dt->isPseudoLevel())ENSFlag = QString("A");

        outputFile << setENSLevelRec(granddaughterHeader,ENSFlag, &(*dt)).toStdString();

        vector<Transition*>*  grandTransitions = dt->GetTransitions();
        for (auto ddt = grandTransitions->rbegin(); ddt != grandTransitions->rend(); ++ddt)
        { // correct second pointer it is supose to be for B feeding
            Transition* ptr = NULL;
            outputFile << setENSGammaRec(granddaughterHeader, (*ddt), ptr).toStdString();
        }

     for ( auto lt = daughterNuclide->GetNuclideLevels()->begin(); lt != daughterNuclide->GetNuclideLevels()->end(); ++lt )
         {

 // looking for transition (beta feeding) to the level
         for (auto mlt = motherNuclide->GetNuclideLevels()->begin(); mlt != motherNuclide->GetNuclideLevels()->end(); ++mlt)
         {
             for ( auto mt = mlt->GetTransitions()->begin(); mt != mlt->GetTransitions()->end(); ++mt )
             {
                 if((*mt)->GetPointerToFinalLevel() == &(*lt))
                 {
                 betaTransition = (*mt);}
             }
         }


         for ( auto kt = lt->GetTransitions()->begin(); kt != lt->GetTransitions()->end(); ++kt )
             { // loop over transitions deesciting doubjter level
             string particleType =  (*kt)->GetParticleType();

             if (particleType == "N" )
             {
                 // prepare and write Level line
                 Level* finalLevel = (*kt)->GetPointerToFinalLevel();

               if(finalLevel != &(*dt))continue;

                     double transitionEnergy = (*kt)->GetTransitionQValue();

                     if(transitionEnergy != 0.0)
                     {
                     if(particleType == "N")outputFile << setENSPartRec(granddaughterHeader, (*kt), betaTransition).toStdString();
                     }
              }
              }
           }
    }
    }
    outputFile.close();
}

QString SaveDecayData::setENSParentRec(std::vector<Nuclide>* nuclidesVector, bool delayedParticle)
{
    qDebug()<< delayedParticle;
    Nuclide* parent = &nuclidesVector->at(0);
//    Nuclide* motherNuclide = &nuclidesVector->at(0);
    Nuclide* daughterNuclide = &nuclidesVector->at(1);
    Nuclide* granddaughterNuclide = &nuclidesVector->at(2);
    QChar space = ' ';
    QString qstr;
    QString qstrC;
    QString output;
    double qBeta=0;
    double DqBeta=0;
     for (int i=0; i<80; i++)
     {
         qstr.push_back(space);
         qstrC.push_back(space);
     }
     qstr.push_back("\n");
     qstrC.push_back("\n");
    int atomicNumber = parent->GetAtomicNumber();
    int atomicMass = parent->GetAtomicMass();
    if(!delayedParticle)
    {
    qBeta = parent->GetQBeta();
    DqBeta = parent->GetD_QBeta();
    } else
    {
        qBeta = parent->GetQBeta() - daughterNuclide->GetSn();
    }

    vector<Level>* levels = parent->GetNuclideLevels();
    double energy = levels->at(0).GetLevelEnergy();
    double T12 = levels->at(0).GetHalfLifeTime();
//    double dT12 = levels->at(0).GetD_T12();
    QString unit = "S";
    QString qstr40to49;
    if(T12 > 0.0) //in case T12=0 we write nothing
     {
        // changing units.
        if(T12/60 <= 99 & T12/60 >= 1.0 )
          {  T12=T12/60;
           unit = "M";
          } else if (T12/3600 <= 99 & T12/60 > 99.0)
          { T12= T12/3600;
            unit = "H";
          } else if (T12/86400 <= 99 & T12/3600 > 99.0)
          { T12= T12/86400;
          unit = "D";
          } else if (T12/31556952 <= 99 & T12/86400 > 99.0)
          { T12= T12/31556952;
          unit = "Y";
          }
            qstr40to49 = QString("%1").arg(T12,8,'f',2,space)+QString("%1").arg(unit,2,space);
            if (T12/31556952 > 99.0 )
               { T12= T12/31556952;
               unit = "Y";
               qstr40to49 = QString("%1").arg(T12,7,'E',3,space)+QString("%1").arg(unit,-2,space);
                }
            if(T12 < 1.0)qstr40to49 = QString("%1").arg(T12,7,'E',3,space)+QString("%1").arg(unit,-2,space);

    }
    int spin2 = int( levels->at(0).GetSpin()*2);
    QString parity = QString::fromStdString(levels->at(0).GetParity());
    QString qMass = QString("%1").arg(atomicMass,3,10);
    QString qstr4to5=QString::fromStdString(PeriodicTable::GetAtomicNameCap(atomicNumber));
    QString qstr10to19 = QString("%1").arg(energy,10,'f',2,space);

    QString qstr22to39;
    for (int i=22; i<=39; i++)
    {
        qstr22to39.push_back(space);
    }

    if(spin2 == 0 )
    {
        if (!parity.isEmpty()) {  //spin=0 and known parity
            qstr22to39 = QString("%1").arg(spin2,14,10,space)+QString("%1").arg(parity,1,space)+space;; // spin with just parity
            }
    } else if (spin2 > 0){
        if (spin2 % 2 == 0){  //even spins
            qstr22to39 = QString("%1").arg(spin2/2,14,10,space)+QString("%1").arg(parity,1,space)+space;; // spin with just parity
            } else {  // odd spins
            qstr22to39 = QString("%1/2").arg(spin2,14,10,space)+QString("%1").arg(parity,1,space)+space;
            }
    }

//    QString qstr22to39 = QString("%1").arg(spin,16,'f',1,space)+QString("%1").arg(parity,1,space)+space;
//    QString qstr40to49 = QString("%1").arg(T12,7,'f',2,space)+QString("%1").arg(unit,2,space);
    qstr.replace(0,5,qMass+qstr4to5); // Nuclide Identification
    qstr.replace(5,1,space);
    qstr.replace(6,1,space);
    qstr.replace(7,1,"P");   // P for Parent
    qDebug() << "a " << qstr;
    qstr.replace(8,1,space); //balnk or integer in case of multiple P records in the data set
    qstr.replace(9,10,qstr10to19); // energy
    qDebug() << "b " << qstr;
//    qstr.replace(19,2,qstr20to21); //denergy
    qstr.replace(21,18,qstr22to39); //spin and parity
    qDebug() << "c " << qstr;
    qstr.replace(39,10,qstr40to49); //Half-life with units
    qDebug() << "d " << qstr;
//    qstr.replace(49,6,qstr50to55); // dT1/2
//    qstr.replace(?,?,qstr56to64); // MUST be BLANK
    QString qstr65to74 = QString("%1").arg(qBeta,10,'f',2,space);
    qDebug() << "QBeta " << qstr65to74 ;
    qstr.replace(64,10,qstr65to74);  // qBeta
    qDebug() << "f " << qstr;

//    string tmpstr = toStringPrecision(d_totalIntensity,2);
//    cout << tmpstr.substr( tmpstr.length() - 2 ) << endl;
//    qstr30to31 = QString::fromStdString(tmpstr.substr( tmpstr.length() - 2 ));  // bledy IBeta
    QString qstr75to76;
    if(DqBeta != 0){
        qstr75to76 = QString::number(DqBeta,'g',2);
    }else {
        qstr75to76 = QString("%1").arg(space,2);
    }

//    QString qstr75to76 = QString("%1").arg(DqBeta,2,'f',2,space);
    qDebug() << "DqBETAt " << DqBeta ;
    qstr.replace(74,2,qstr75to76); // dQBeta
//    qstr.replace(76,4,qstr77to80); // IonizationState(for Ionized Atom decay) otherwise blank
    output= qstr;
    if(delayedParticle)
    {
        QString qstr4to5d=QString::fromStdString(PeriodicTable::GetAtomicNameCap(daughterNuclide->GetAtomicNumber()));

        QString qstr10to80 = "  S(n)({+"+ QString("%1").arg(atomicMass,3,10) +"}" + qstr4to5d + ")=" +
                QString("%1").arg(daughterNuclide->GetSn(),8,'f',2,space);
        qstrC.replace(0,5,qMass+qstr4to5); // Nuclide Identification
        qstrC.replace(5,1,space);
        qstrC.replace(6,1,"c");  //'c' for comment
        qstrC.replace(7,1,"P");   // P for Parent
        qstrC.replace(8,1,space); //must be blank
        qstrC.replace(9,71,qstr10to80); //Comment
        output = output+qstrC;
    }
    qDebug() << output;
    return output;
}
QString SaveDecayData::setENSLevelRec(QString header, QString ENSFlag, Level* level)
{
    QChar space = ' ';
    QString qstr;
    QString qstrC;
    QString output;
     for (int i=0; i<80; i++)
     {
         qstr.push_back(space);
         qstrC.push_back(space);
     }
         qstr.push_back("\n");
         qstrC.push_back("\n");

    double energy = level->GetLevelEnergy();
    double T12 = level->GetHalfLifeTime();
    QString unit = "S";
//    double spin = level->GetSpin();
    int spin2  = int( level->GetSpin()*2);
    QString parity = QString::fromStdString(level->GetParity());

 //   QString qMass = QString("%1").arg(atomicMass,3,10);
 //   QString qstr4to5=QString::fromStdString(PeriodicTable::GetAtomicNameCap(atomicNumber));
    QString qstr10to19 = QString("%1").arg(energy,10,'f',2,space);
    QString qstr22to39;
    for (int i=22; i<=39; i++) qstr22to39.push_back(space);

    if(spin2 == 0 )
    {
        if (!parity.isEmpty()) {  //spin=0 and known parity
            qstr22to39 = QString("%1").arg(spin2,14,10,space)+QString("%1").arg(parity,1,space)+space; // spin with just parity
        }
    } else if (spin2 > 0){
        if (spin2 % 2 == 0){  //even spins
            qstr22to39 = QString("%1").arg(spin2/2,14,10,space)+QString("%1").arg(parity,1,space)+space+space+space; // spin with just parity
        } else {  // odd spins
            qstr22to39 = QString("%1/2").arg(spin2,14,10,space)+QString("%1").arg(parity,1,space)+space;
        }
    }

/*    if(!parity.isEmpty() & (spin2 % 2 != 0))
    {
        qstr22to39 = QString("%1/2").arg(spin2,14,10,space)+QString("%1").arg(parity,1,space)+space;
//    QString qstr22to39 = QString("%1").arg(spin,16,'f',1,space)+QString("%1").arg(parity,1,space)+space;
    } else  if (!parity.isEmpty() & (spin2 % 2 == 0)){
        qstr22to39 = QString("%1").arg(spin2,14,10,space)+QString("%1").arg(parity,1,space)+space;; // spin with just parity
    }
 */       QString qstr40to49;
    for (int i=0; i< 10;i++) qstr40to49.push_back(space);

    if(T12 > 0.0) //in case T12=0 we write nothing
     {
        // changing units.
        if(T12/60 <= 99 & T12/60 >= 1.0 )
          {  T12=T12/60;
           unit = "M";
          } else if (T12/3600 <= 99 & T12/3600 >= 1.0)
          { T12= T12/3600;
            unit = "H";
          } else if (T12/86400 <= 99 & T12/86400 >= 1.0)
          { T12= T12/86400;
          unit = "D";
          } else if (T12/31556952 <= 99 & T12/331556952 >= 1.0)
          { T12= T12/31556952;
          unit = "Y";
          } else if(T12 < 1.0 & T12 > 1e-3)
          { T12 = T12*1000;
            unit="MS";
          } else if (T12 < 1e-3 & T12 > 1e-6)
          { T12 = T12*1e6;
            unit="US";
          } else if (T12 < 1e-6 & T12 > 1e-9)
          { T12 = T12*1e9;
            unit="NS";
          } else if (T12 < 1e-9 & T12 > 1e-12)
          { T12 = T12*1e12;
            unit="PS";
          }
            qstr40to49 = QString("%1").arg(T12,8,'f',2,space)+QString("%1").arg(unit,2,space);
            if (T12/31556952 > 99 )
               { T12= T12/31556952;
               unit = "Y";
               qstr40to49 = QString("%1").arg(T12,-9,'E',2,space)+QString("%1").arg(unit,1,space);
                }

            if(T12 < 1.0)qstr40to49 = QString("%1").arg(T12,-7,'E',3,space)+space+QString("%1").arg(unit,2,space);

    }
//    QString qstr65to74 = QString("%1").arg(qBeta,10,'f',2,space);
// output string construction
    qstr.replace(0,5,header); // Nuclide Identification
    qstr.replace(5,1,space);
    qstr.replace(6,1,space);
    qstr.replace(7,1,"L");   // L for Level
    qDebug() << qstr;
    qstr.replace(8,1,space); //balnk or integer in case of multiple P records in the data set
    qstr.replace(9,10,qstr10to19); // energy
//    qstr.replace(19,2,qstr20to21); //denergy
    qDebug() << "a " << qstr;
    qstr.replace(21,18,qstr22to39); //spin and parity
    qDebug() << "b " << qstr;
    qstr.replace(39,10,qstr40to49); //Half-life with units
    qDebug() << "c " << qstr;
//    qstr.replace(49,6,qstr50to55); // dT1/2
//    qstr.replace(?,?,qstr56to64); // L Angular momentum transfer in the reacition determianinf the data set.
//    qstr.replace(64,10,qstr65to74);  // S Spectroscopiv strength for this level as determined from the reaction
//    qstr.replace(74,2,qstr75to76); // dS
     if(level->isPseudoLevel())qstr.replace(76,1,ENSFlag);
     qDebug() << "d " << qstr;
    //      qstr.replace(76,1,"C") ;      // Comment  FLAG used to refer to a particular comment record
//    qstr.replace(77,2,qstr78to79); // Metastabel state is denoted by 'M' or "M1' for the first (lowest) M2, for the second etc.
                                    // For ionized atom decay filed gives the atomic electron shell or subshell in which B- particle is captured.
     if(level->isPseudoLevel()) qstr.replace(79,1,"?"); // The character '?' denotes an uncertain or questionable level.Letter 'S' denotes neutron,proton,alpha,separation
                                 //energy or a leel expected but not observed.

     output = qstr;
 /*   if(level->isPseudoLevel())
    {
        QString qstr10to80 = QString("Added Pseudolevel. Energy denotes center of the pseudolevel") ;
        qstrC.replace(0,5,header); // Nuclide Identification
        qstrC.replace(5,1,space);
        qstrC.replace(6,1,"c");  //'c' for comment
        qstrC.replace(7,1,"L");   // L for level
        qstrC.replace(8,1,space); //must be blank
        qstrC.replace(9,71,qstr10to80); //Comment
        output = output+qstrC;
    }
   */ qDebug() << output;
    return output;
}
QString SaveDecayData::setENSGammaRec(QString header, Transition* transitionFrom, Transition* transitionTo)
{
    QChar space = ' ';
    QString qstr;  // main record
    QString qstr2 ;  // continuation record
    QString qstrC;  // comment record
    QString output;
     for (int i=0; i<80; i++)
     {
         qstr.push_back(space);
         qstr2.push_back(space);
         qstrC.push_back(space);
     }
     qstr.push_back("\n");
     qstr2.push_back("\n");
     qstrC.push_back("\n");

    // GetSn();
    double energy = transitionFrom->GetTransitionQValue();
    double totalIntensity = transitionFrom->GetIntensity()*100;
    QString qstr65to74 = QString("%1").arg(space,10,space);
    if(transitionTo!=NULL){
    double betafeeding =  transitionTo->GetIntensity();
    double normalisedTotalIntensity = totalIntensity*betafeeding; //does not include GAMMAfeeding!!!
      qstr65to74 = QString("%1").arg(normalisedTotalIntensity,10,'f',5,space);
    }
    QString qstr10to19 = QString("%1").arg(energy,10,'f',2,space);
    QString qstr22to29;
    QString qstr30to31;
    if (totalIntensity < 0.001)
    {
    qstr22to29 = QString("%1").arg("0.001",8,space);
    qstr30to31 = QString("%1").arg("LT",2,space);
    } else
    {
    qstr22to29 = QString("%1").arg(totalIntensity,8,'f',2,space);
    qstr30to31 = QString("%1").arg("  ",2,space);
    }
//    QString qstr40to49 = QString("%1").arg(T12,7,'f',2,space)+QString("%1").arg(unit,2,space);

    qstr.replace(0,5,header); // Nuclide Identification
    qstr.replace(5,1,space);
    qstr.replace(6,1,space);
    qstr.replace(7,1,"G");   // G for gamma
    qDebug() << "a " << qstr;
    qstr.replace(8,1,space); //must be blank
    qstr.replace(9,10,qstr10to19); // energy
//    qstr.replace(19,2,qstr20to21); //denergy
    qDebug() << "b " << qstr;
    qstr.replace(21,8,qstr22to29); //RI, relative photon intensity I
    qDebug() << "c " << qstr;
//    qstr.replace(29,2,qstr30to31); //dRI
//    qstr.replace(31,10,qstr32to41); //M - Multipolarity of transition
//    qstr.replace(41,8,qstr42to49); // MR Mixing ratio (sigma) (Sign must be shown explicitly if known.
                                    // If no sign is given it will be assumed to be unknown.)
//    qstr.replace(49,6,qstr50to55); // dMR, Standard uncertainty of Mixing (MR)
//    qstr.replace(55,7,qstr56to62); // CC, Total conversion coeficient
//    qstr.replace(62,2,qstr63to64); // dCC
    qstr.replace(64,10,qstr65to74);  // TI, Relative total transition intensity. (Normalisation given in NORMALISATION record)
    qDebug() << "d " << qstr;
//    qstr.replace(74,2,qstr75to76); // dTI*----
    QString qstr77=" ";
    if(transitionFrom->IsAddedTransition())qstr77="A";

     qstr.replace(76,1,qstr77); // Comment FLAG used to refere to particular comment record.
                              // The symbol '*' denotes multiply placed g-ray.
                              // '&' - a multiplaced transitions with intensity not divided,
                              // '@' - a multiplaced transitions with intensity suitably divided.
                              // '%' - denotes that intensity given as RI is the branching in the SUper DEformed Band.
     qDebug() << "e " << qstr;
    qstr.replace(77,1,' ');  //  Letter 'C' denotes placement confirmed by coincidence.
                              // Symbol '?' denotes questionable coincidence.
    qstr.replace(78,1,space);
    QString qstr80 =" ";
//    if(transitionFrom->IsAddedTransition())qstr80="?";
    qstr.replace(79,1,qstr80); // The Character '?' denotes an uncertain placement of the transition in the lvel scheme.
                               // Letter 'S' denotes an expected, but as yet unobserved, transition.
    qDebug() << "f " << qstr;
    output=qstr;
    double electronConversion = transitionFrom->GetElectronConversionCoefficient();
    double* shellElectronConversion  = transitionFrom->GetShellElectronConversionCoefficient();
    QString qstrCE, qstrKE, qstrLE, qstrME, qstrNE;
    qDebug() << "------------SHellElectronConversion--------------" << shellElectronConversion[2] ;
    if(electronConversion != 0)
    {

        QString qstrCE =QString("CE= %1").arg(electronConversion,-8,'f',6,space);
        QString qstr10to80;
        qDebug() <<qstr10to80;
        qstr10to80.insert(0,qstrCE);
        qDebug() <<qstr10to80;

        if (shellElectronConversion[0] !=0){
            qstrKE = QString(" KC= %1").arg(shellElectronConversion[0], -8,'f',6,space);
            qstr10to80.insert(12,qstrKE);  }

        if (shellElectronConversion[1] !=0){
            qstrLE = QString(" LC= %1").arg(shellElectronConversion[1]-shellElectronConversion[0], -8,'f',6,space);
            qstr10to80.insert(26, qstrLE); }

        if (shellElectronConversion[2] !=0){
            qstrME = QString(" MC= %1").arg(shellElectronConversion[2]-shellElectronConversion[1], -8,'f',6,space);
            qstr10to80.insert(39,qstrME);  }

        if (shellElectronConversion[3] !=0){
            qstrNE = QString(" NC= %1").arg(shellElectronConversion[3]-shellElectronConversion[2], -8,'f',6,space);
            qstr10to80.insert(52,qstrNE); }

        qstr2.replace(0,5,header); // Nuclide Identification
        qstr2.replace(5,1,"2"); // Any alphanumeric character other than 1. Note 'S' is reserved for computer produced records.
        qstr2.replace(6,1,space);  //'c' for comment
        qstr2.replace(7,1,"G");   // G for gamma
        qstr2.replace(8,1,space); //must be blank
        qstr2.replace(9,71,qstr10to80); //Comment
        output = output+qstr2;
    }

/*    if(transitionFrom->IsAddedTransition())
    {
        QString qstr10to80 = QString("Added transition") ;
        qstrC.replace(0,5,header); // Nuclide Identification
        qstrC.replace(5,1,space);
        qstrC.replace(6,1,"c");  //'c' for comment
        qstrC.replace(7,1,"G");   // G for gamma
        qstrC.replace(8,1,space); //must be blank
        qstrC.replace(9,71,qstr10to80); //Comment
        output = output+qstrC;
    }
  */  qDebug() << output;
    return output;
}
QString SaveDecayData::setENSBetaRec(QString header, Transition* transition)
{
    // for B- ONLY for now.20230719
    QChar space = ' ';
    QString qstr;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");

     double totalIntensity = transition->GetIntensity()*100;
     double d_totalIntensity = transition->GetD_Intensity()*100;
     QString qstr22to29;
     QString qstr30to31;
   if (totalIntensity < 0.001)
         {
     qstr22to29 = QString("%1").arg("0.001",8,space);
     qstr30to31 = QString("%1").arg("LT",2,space);
         }

     else
         { //intensity larger than 0.001%

       QStringList qList = getValueAndError(totalIntensity,d_totalIntensity);
       qDebug() << qList;
     //  qstr22to29 = qList.at(0);
     //  qstr30to31 = qList.at(1);
     qstr22to29 = QString("%1").arg(qList.at(0),-8);
     qstr30to31 =QString("%1").arg(qList.at(1),2);
/*     string tmpstr = toStringPrecision(d_totalIntensity,2);
     cout << tmpstr.substr( tmpstr.length() - 2 ) << endl;
     qstr30to31 = QString::fromStdString(tmpstr.substr( tmpstr.length() - 2 ));  // bledy IBeta
//     qstr30to31 = QString::fromStdString(toStringPrecision(d_totalIntensity,2));  // bledy IBeta
*/
     }
    qDebug() << qstr;
//     QString qstr10to19 = ;
    qstr.replace(0,5,header); // Nuclide Identification
    qstr.replace(5,1,space);
    qstr.replace(6,1,space);
    qstr.replace(7,1,"B");   // B for beta
    qstr.replace(8,1,space); // blank
    qDebug() << qstr;
//    qstr.replace(9,10,qstr10to19); // E - End Point energy given only if measured
//    qstr.replace(19,2,qstr10to21); //  DE uncertainty
    qstr.replace(21,7,qstr22to29); //  IB - Intensity of B- branch.
                                   //Intensity units defined by the NORMALIZATION record.
    qstr.replace(29,2,qstr30to31); //  DIB - uncertainty of Beta intensity
    qDebug() << "a " << qstr;
 //   qstr.replace(31,11,space); // blank
    qDebug() << "b " <<qstr;
//    qstr.replace(41,7,qstr42to49); // LOGFT logfT values calculated for uniqueness given in col 78-79
//    qstr.replace(59,6,qstr50to55); //  DFT - standard uncertainty for LOGFT value
 //   qstr.replace(55,21,space); // Must be blank
    qDebug() << "c " << qstr;
//   qstr.replace(76,1,qstr77); //  Comment FLAG : C- denotes coincidence with the following radiation,
                               // A '?' denotes probable coincidence with a following radiation.
//    qstr.replace(77,2,qstr78to79); // Forbiddenness calsification for B- decay e.g. 1U, 2U for first,second uniqueforbiden.
                                   // A balnk filed signifies an allowed transition.
                                   // Nonunique forbiddenness can be indicated in col 78., with col 79 blank.
    qstr.replace(79,1,space); // The character '?' denotes an uncertain or questionable B- decay
                              // Letter 'S' denotes an expected or predicted transition.
     qDebug() << qstr;
    return qstr;
}
QString SaveDecayData::setENSIdentificationRec(QString Id, QString header, Nuclide* parent)
{
    QChar space = ' ';
    QString qstr;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");
     int atomicNumber = parent->GetAtomicNumber();
     int atomicMass = parent->GetAtomicMass();
     QString qMass = QString("%1").arg(atomicMass,3,10);
     QString qstr4to5=QString::fromStdString(PeriodicTable::GetAtomicNameCap(atomicNumber));

//     double qBeta = parent->GetQBeta();

    QString qstr10to39 =  QString("%1%2 %3 DECAY%4").arg(qMass,3,10).arg(qstr4to5,2).arg(Id,3,space).arg(space,16,space) ;
    QDateTime now = QDateTime::currentDateTime();
    const QString timestamp = now.toString(QLatin1String("yyyyMM"));
 //   qDebug() << timestamp;
    QString qstr75to80 = QString("%1").arg(timestamp,6);
//    qDebug() << qstr75to80;
    qstr.replace(0,5,header); // Nuclide Identification
//    qstr.replace(5,4,space); // must be blank
    qstr.replace(9,30,qstr10to39); //DSID - Data Set IDentification
//    qstr.replace(39,26,qstr40to65); //DSREF References to main supporting publications and analyses
//    qstr.replace(65,9,qstr66to74); // PUB publication information
    qstr.replace(74,6,qstr75to80); //DATE The date(year/rmonth) when the data set was placed in ENSDF (for ENSDF entered automaticaly)
    qDebug() << qstr;
    return qstr;
}
QString SaveDecayData::setENSQvalueRec(QString header, Nuclide* nuclide)
{
    QChar space = ' ';
    QString qstr;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");
    int atomicNumber = nuclide->GetAtomicNumber();
    int atomicMass = nuclide->GetAtomicMass();
    double qBeta = nuclide->GetQBeta();
    double qSn = nuclide->GetSn();
    QString qMass = QString("%1").arg(atomicMass,3,10);
    QString qstr4to5=QString::fromStdString(PeriodicTable::GetAtomicNameCap(atomicNumber));
    QString qstr10to19 = QString("%1").arg(qBeta,10,'f',2,space);
    QString qstr22to29 = QString("%1").arg(qSn,10,'f',2,space);

    qstr.replace(0,5,header); // Nuclide Identification
    qstr.replace(5,1,space);
    qstr.replace(6,1,space);
    qstr.replace(7,1,"Q");   // P for Parent
    qstr.replace(8,1,space); //balnk or integer in case of multiple P records in the data set
    qstr.replace(9,10,qstr10to19); // Q-  - Total energy (keV) available for B- decay of the ground sate.
                                    //(Q- > 0 if B- decay energetically possible.
                                    // Q- < 0 represents the Q_EC energy of the Z+1 isobar.
//    qstr.replace(19,2,qstr20to21); //DQ- standard uncertainty of Q-
    qstr.replace(21,8,qstr22to29); // SN - neutron separation energy in keV
//    qstr.replace(29,2,qstr30to31);  //DSN standard uncertainty in SN
//    qstr.replace(31,8,qstr32to39);  // SP - proton separation energy in keV
//    qstr.replace(39,2,qstr40to41); // DSP - standard uncertainty in SP
//    qstr.replace(41,8,qstr42to49); // QA - Total energy (kev) available for apha decay of the ground state
//    qstr.replace(49,6,qstr50to55); // DQA - standard uncertaint in QA
//    qstr.replace(55,25,qstr56to80); // Reference citation(s) for Q-values
    qDebug() << qstr;
    return qstr;
}
QString SaveDecayData::setENSNormRec(QString header)
{
    QChar space = ' ';
    QString qstr;
    double one =1.0;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");
    // GetSn();

     qstr.replace(0,5,header); // Nuclide Identification
     qstr.replace(5,1,space);  // must be blank
     qstr.replace(6,1,space); // must be blank
     qstr.replace(7,1,"N"); // Letter 'N' is required
     qstr.replace(8,1,space); // Blank or an integer in case of multiple P Records in the data set.
                              // It should correspond to the designator on the P record
//    qstr.replace(9,10,qstr10to19); // NR - Multiplier for converting relative photon intensity (RI in GAMMA record) to photons per 100 decays
                                  // of the parent through the decay branch or to photons per 100 neutron captures in an (n,gamma0 reaction.
                                  // Required if the absolute photon intensity can be calculated.
//    qstr.replace(19,2,qstr20to22); // DNR - Standrad uncertainty in NR
//    qsttr.replace(21,8,qstr22to29); // NT - Multiplier for converting relative transition intensity (includeing conversion electrons)
                                    // (TI in the GAMMA record) to transitions per 100 decays of the parent through this decay branch
                                    // or per 100 neutron captures in an (n,gamma) reqaction.
//    qstr.replace(29,2,qstr30to31) ; // DNT - standard uncertainty in NT.
     QString qstr32to39 = QString("%1").arg(one,8,'f',2,space);
     if(delayedNeutrons_)qstr32to39 = QString("%1").arg(neutronPercentage_/100,6,'f',4,space); //Pn here
     qstr.replace(31,8,qstr32to39); // BR - Branching ratio multiplier for converting intensity per 100 decays
                                    // through this decay branch to intensity per 100 decays of the parent nuclide.
                                    // Required if known.
//    qstr.replace(39,2,qstr40to41); // DBR - standard uncertainty in BR
     QString qstr42to49 = QString("%1").arg(one,8,'f',2,space);

     qstr.replace(41,8,qstr42to49); // NB - Multiplier for converting relative B- and EC intensities (IB in the B- recordl IB,IE,TI in the EC record)
                                    // to intensities per 100 decays through this decay branch. Required if known.
//    qstr.replce(49,6,qstr50to55); // DNB - standard uncertainty in NB.
     QString qstr56to62 = QString("%1").arg(neutronPercentage_,6,'f',2,space); //Pn here

      qstr.replace(55,6,qstr56to62); // NP - Multiplier for converting per 100 delayed transition intensities to per 100 decays of precursor.
//    qstr.replace(62,2,qstr63to64); // DNP - standard uncertainty in NP.
//    qstr.replace(64,21,space);     // must be blank
    qDebug() << qstr;
     return qstr;
}
QString SaveDecayData::setENSPNormRec(QString header)
{
    QChar space = ' ';
    double one = 1.0;
    QString qstr;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");
    // GetSn();

     qstr.replace(0,5,header); // Nuclide Identification
     qstr.replace(5,1,space);  // must be blank
     qstr.replace(6,1,"P"); // Letter 'P' (for production) is required
     qstr.replace(7,1,"N"); // Letter 'N' is required
     qstr.replace(8,1,space); // Blank or an integer in case of multiple P Records in the data set.
                              // It should correspond to the designator on the P record
//    qstr.replace(9,10,qstr10to19); // NR x BR - Multiplier for converting relative photon intensity (RI in GAMMA record) to photons per 100 decays
                                  // of the parent. (Normaly NR x BR). If left blank (NR DNR) x (BR DBR) from N record will be used
                                  // for normalisation.
//    qstr.replace(19,2,qstr20to22); // UNC - Standrad uncertainty in NR x BR. If left blank no uncertainty will appear in the publication.
//    qsttr.replace(21,8,qstr22to29); // NT x BR - Multiplier for converting relative transition intensity (includeing conversion electrons)
                                    // (TI in the GAMMA record) to transitions per 100 decays of the parent. (Normaly Nt x BR).
                                    // If left blank (NT DNT) x (BR DBR) from N record will be used for normalisation.
//    qstr.replace(29,2,qstr30to31) ; // UNC - Standrad uncertainty in NT x BR. If left blank no uncertainty will appear in the publication.
//    qstr.replace(31,8,qstr32to41); // Blank
      QString qstr42to49 = QString("%1").arg(one,10,'f',2,space);
      qstr.replace(41,8,qstr42to49); // NB x BR - Multiplier for converting relative B- and EC intensities (IB in the B- recordl IB,IE,TI in the EC record)
                                    // to intensities per 100 decays.
                                       // If left blank (NB DNB) x (BR DBR) from N record will be used for normalisation.
//    qstr.replce(49,6,qstr50to55); // UNC - Standrad uncertainty in NB x BR. If left blank no uncertainty will appear in the publication.
      QString qstr56to62 = QString("%1").arg(neutronPercentage_,10,'f',2,space); //Pn here
      qstr.replace(55,6,qstr56to62); // NP - Multiplier for converting per 100 delayed transition intensities to per 100 decays of precursor.
//    qstr.replace(62,2,qstr63to64); // DNP - standard uncertainty in NP.
//    qstr.replace(64,12,space);     // must be blank
      qstr.replace(76,1,space);  // Blank or 'C' (for comment). If blank, comment associated with the intensity option will appear in the drawing in the Nuclear DAta Sheets.
                                 // If letter 'C' is given, the desired comment to appear in the drawing should be given on the continuation ('nPN') record(s), col 10-80.
      qstr.replace(77,1,"6"); // Intensit option as to what intensity to display in the drawing in the NDS. The available options are:
                                // 1 - TI or RI(1+alpha)  - Relative I(gamma+ce)
                                // 2 - TIxNT or RIxNTx(1+alpha) - I(gamma+ce) per 100 (mode) decays
                                // 3 - TIxNTxBR or
                                //     RIxBRxNRx(1+alpha) -  I(gamma+ce) per 100 parent decays. (default).
                                // 4 - RIxNTxBR  - I(gamma) per 100 parent decays
                                // 5 - RI - Relative I(gamma)
                                // 6 - RI - Relative photon branching from each level
                                // 7 - RI - % photon branching from each level
      qDebug() << qstr;
      return qstr;
}
QString SaveDecayData::setENSPartRec(QString header, Transition* transitionFrom, Transition* transitionTo)
{
    QChar space = ' ';
    QString qstr;
    QString qstrC;
     for (int i=0; i<80; i++)
     {
         qstr.push_back(space);
         qstrC.push_back(space);
     }
     qstr.push_back("\n");
     qstrC.push_back("\n");

     double energy = transitionFrom->GetTransitionQValue();
    double totalIntensity = transitionFrom->GetIntensity();
    double betafeeding =  transitionTo->GetIntensity();
    double normalisedTotalIntensity = totalIntensity*betafeeding/neutronPercentage_*100;


    QString partType = QString::fromStdString(transitionFrom->GetParticleType());
    Level* initLevel = transitionFrom->GetPointerToInitialLevel();
    double energyInitLevel = initLevel->GetLevelEnergy();

    qstr.replace(0,5,header); // Nuclide Identification
    qstr.replace(5,1,space); //blank  any character other than '1' for continuation records
    qstr.replace(6,1,space); // must be blank
    qstr.replace(7,1,"D");   // D - blank for prompt, Letter 'D' for delayed particle emission.
    QString qstr9 = QString("%1").arg(partType,1,space);
    qstr.replace(8,1,qstr9); //Particle - N for neutrons, P for proton, A for Alpha particle.
    QString qstr10to19 = QString("%1").arg(energy,10,'f',2,space);
    qstr.replace(9,10,qstr10to19); // energy
//    qstr.replace(19,2,qstr20to21); //denergy
    QString qstr22to29 = QString("%1").arg(normalisedTotalIntensity,8,'f',6,space);
    qstr.replace(21,18,qstr22to29); //IP -Intensity of (delayed) particle in percent of the total (delayed) particle emissions. I
//    qstr.replace(29,2,qstr30to31); //dIP
    QString qstr32to39 = QString("%1").arg(energyInitLevel,8,'f',2,space);
    qstr.replace(31,8,qstr32to39); // EI - energy of the level in the 'intermediate' mass=A+1 for n and p; A+4 for alpha
    //    QString qstr40to49 = QString("%1").arg(T12,7,'f',2,space)+QString("%1").arg(unit,2,space);
//     qstr.replace(39,10,qstr40to49); // T- Width of the transition in keV
//     qstr.replace(49,6,qstr50to55);  // DT - Uncertainty of T
//     qstr.replace(55,9,qstr56to64); // L - angular momentum transfer of the emitted particle
 //   QString qstr65to74 = QString("%1").arg(totalIntensity,10,'f',2,space);
//     qstr.replace(64,12,space);  //Blank
//       qstr.replace(77,1,space);  // C - comment FLAG used to refer to a particlura comment record
//        qstr.replace(78,1," "); // COIN Letter 'C' denotes placement confirmed by coincidence. Symbol '?' denotes probable coincidence
        qstr.replace(79,1,space); // Blank
//        qstr.replace(80,1," "); // The character '?' denotes an ucertain placement of the transition in the level scheme.
                                // Letter 'S' denotes expected, but as yet unobserved transition

     return qstr;
}
QString SaveDecayData::setENSComRec(QString header, QString RType, QString Sym, QString Flag, QString CText)
{
    QChar space = ' ';
    QString qstr;
    QString qstr10to80;
//    double one =1.0;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");

     qstr.replace(0,5,header); // Nuclide Identification
     qstr.replace(5,1,space);  // must be blank or any alphanueric character other then '1' for continuation records
     qstr.replace(6,1,'C'); // 'C', 'D', 'T' ora 'c', 'd', 't'
     qstr.replace(7,1,RType); // Balnk or RType for record type of records to which the comment pertains
     qstr.replace(8,1,space); // PSYM - Blank or symbol for a (delayed-) particle eg. N,P
//    qstr.replace(9,10,qstr10to19); // NR - Multiplier for converting relative photon intensity (RI in GAMMA record) to photons per 100 decays

     qstr10to80 = QString("%1(%2)$%3").arg(Sym).arg(Flag).arg(CText);
     qstr.replace(9,70,qstr10to80);

     return qstr;
}

QStringList SaveDecayData::getValueAndError(double value, double error)
{
    QStringList outputList;
    QChar space = ' ';
 //   QString valueStr;
    qDebug() << "Input: " << value <<" " << error;
    QString valueStr = QString("%1").arg(value,11,'f',3,space);  //11 arbitray 3 to count the 1/1000a
    QString errorStr = QString("%1").arg(error,11,'f',3,space);
    qDebug() << "valueStr = " << valueStr << "errorStr = " << errorStr;
    errorStr.remove(space);
    valueStr.remove(space);
    qDebug() << "valueStr = " << valueStr << "errorStr = " << errorStr;
    int dotPossitionInValue;
    int dotPossitionInError;
    if (error <= value)
    {
        dotPossitionInValue = valueStr.indexOf(".");
        dotPossitionInError = errorStr.indexOf(".");
        qDebug() <<  " dotPossitionInValue " <<   dotPossitionInValue
                 <<  " dotPossitionInError " <<   dotPossitionInError;
        //find first non-zero character in string errorStr
        signed short dError1=0;
        for(auto i=0; i!=errorStr.length(); i++)
        { //finding possition of first non-zero number in error
            if ((errorStr[i] != "0") && (errorStr[i] != ".")) {
                dError1=i;
                break;}
        }
        signed short dValue1=0;
        if((valueStr.length() - dotPossitionInValue) >= (errorStr.length() - dotPossitionInError))
                dError1++;

/*        if(errorStr[dError1+1] == "0" )
        { //nic nie robimy nie zaaokraglamy bledow

        } else
        {  // strzeba zaokraglic bledy w gore

        }
 */
        int outputValueLength =  dotPossitionInValue + 1 +  (dError1 - dotPossitionInError);
        QString errorStrOut;
        if (dError1 == dotPossitionInError)
        {
            errorStrOut = errorStr.mid(dError1-1,3);
            errorStrOut.remove(".");
            outputValueLength++;
        } else
        {   errorStrOut = errorStr.mid(dError1-1,2);
        }
// cuts out last zeros if they exist in both value and error
        QString tstr1, tstr2;
         tstr1 = valueStr.mid(dValue1,outputValueLength);
         tstr2 = errorStrOut;
         if(tstr1.endsWith('0') && tstr2.endsWith('0'))
         {
             tstr1.chop(1);
             tstr2.chop(1);
         }
         qDebug() << "after choping" << tstr1 << " " << tstr2;
         outputList << tstr1 << tstr2;
//         outputList << valueStr.mid(dValue1,outputValueLength) << errorStrOut ;
               qDebug() << "Output LIST" << outputList;
         return outputList;
    } else
    { // uncertainty larger than value
         outputList <<  QString("%1").arg(value,8,'f',2,space);
         qDebug() << "uncertainty larger than value" << outputList;
         return outputList;
    }
}
