#include "DeclareHeaders.hh"
#include "pugixml.hh"
#include "PeriodicTable.hh"

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>

using namespace std;

SaveDecayData::SaveDecayData(string path)
{
    path_ = path + "/";
    //SaveDecayStructure();
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
	DecayPath* decayPath = DecayPath::get();
	std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

    for ( auto it = nuclidesVector->begin(); it != nuclidesVector->end(); ++it )
    {
        int atomicNumber = it->GetAtomicNumber();
        int atomicMass = it->GetAtomicMass();
        double qBeta = it->GetQBeta();
        pugi::xml_document nuclideFile;
        pugi::xml_node nodeNuclide = nuclideFile.append_child("Nuclide");
        nodeNuclide.append_attribute("AtomicNumber") = atomicNumber;
        nodeNuclide.append_attribute("AtomicMass") = atomicMass;
        nodeNuclide.append_attribute("QBeta") = qBeta;
		
        for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
        {
            double levelEnergy = jt->GetLevelEnergy();
            double spin = jt->GetSpin();
            string parity = jt->GetParity();
            double halfLifeTimeInSeconds = jt->GetHalfLifeTime();
            bool isPseudoLevel = jt->isPseudoLevel();

            pugi::xml_node nodeLevel = nodeNuclide.append_child("Level");
            nodeLevel.append_attribute("Energy").set_value(toStringPrecision(levelEnergy,2).c_str());
            nodeLevel.append_attribute("Spin").set_value(toStringPrecision(spin,2).c_str());
            nodeLevel.append_attribute("Parity") = parity.c_str();
            nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,2).c_str());
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
                double finalLevelEnergy = (*kt)->GetFinalLevelEnergy();
                int finalLevelAtomicMass = (*kt)->GetFinalLevelAtomicMass();
                int finalLevelAtomicNumber = (*kt)->GetFinalLevelAtomicNumber();
                bool isAddedTransition = (*kt)->IsAddedTransition();

                pugi::xml_node nodeTransition = nodeLevel.append_child("Transition");
                nodeTransition.append_attribute("Type") = particleType.c_str();
                nodeTransition.append_attribute("TransitionQValue").set_value(toStringPrecision(transitionQValue,2).c_str());
                nodeTransition.append_attribute("Intensity").set_value(toStringPrecision(intensity,6).c_str());
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
                        pugi::xml_node nodeConversion = nodeTransition.child("ElectronConversionCoefficient");
                        nodeConversion.append_attribute("Total").set_value(toStringPrecision(eCC,6).c_str());
                        double* shellECC = (*kt)->GetShellElectronConversionCoefficient();
                        nodeConversion.append_attribute("KC").set_value(toStringPrecision(shellECC[0],6).c_str());
                        nodeConversion.append_attribute("LC").set_value(toStringPrecision(shellECC[1],6).c_str());
                        nodeConversion.append_attribute("MC").set_value(toStringPrecision(shellECC[2],6).c_str());
                        nodeConversion.append_attribute("NC").set_value(toStringPrecision(shellECC[3],6).c_str());

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
                nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,2).c_str());
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
                nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,2).c_str());
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
                        pugi::xml_node nodeConversion = nodeTransition.child("ElectronConversionCoefficient");
                        nodeConversion.append_attribute("Total").set_value(toStringPrecision(eCC,6).c_str());
                        double* shellECC = (*kt)->GetShellElectronConversionCoefficient();
                        nodeConversion.append_attribute("KC").set_value(toStringPrecision(shellECC[0],6).c_str());
                        nodeConversion.append_attribute("LC").set_value(toStringPrecision(shellECC[1],6).c_str());
                        nodeConversion.append_attribute("MC").set_value(toStringPrecision(shellECC[2],6).c_str());
                        nodeConversion.append_attribute("NC").set_value(toStringPrecision(shellECC[3],6).c_str());

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
                nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,2).c_str());
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
                            pugi::xml_node nodeConversion = nodeTransition.child("ElectronConversionCoefficient");
                            nodeConversion.append_attribute("Total").set_value(toStringPrecision(eCC,6).c_str());
                            double* shellECC = (*kt)->GetShellElectronConversionCoefficient();
                            nodeConversion.append_attribute("KC").set_value(toStringPrecision(shellECC[0],6).c_str());
                            nodeConversion.append_attribute("LC").set_value(toStringPrecision(shellECC[1],6).c_str());
                            nodeConversion.append_attribute("MC").set_value(toStringPrecision(shellECC[2],6).c_str());
                            nodeConversion.append_attribute("NC").set_value(toStringPrecision(shellECC[3],6).c_str());

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
                            nodeLevel.append_attribute("HalfLifeTime").set_value(toStringPrecision(halfLifeTimeInSeconds,2).c_str());
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
                                        pugi::xml_node nodeConversion = nodeTransition.child("ElectronConversionCoefficient");
                                        nodeConversion.append_attribute("Total").set_value(toStringPrecision(eCC,6).c_str());
                                        double* shellECC = (*kt)->GetShellElectronConversionCoefficient();
                                        nodeConversion.append_attribute("KC").set_value(toStringPrecision(shellECC[0],6).c_str());
                                        nodeConversion.append_attribute("LC").set_value(toStringPrecision(shellECC[1],6).c_str());
                                        nodeConversion.append_attribute("MC").set_value(toStringPrecision(shellECC[2],6).c_str());
                                        nodeConversion.append_attribute("NC").set_value(toStringPrecision(shellECC[3],6).c_str());

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
