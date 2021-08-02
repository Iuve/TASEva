
#include "DeclareHeaders.hh"
#include "QDebug"
#include "pugixml.hh"

#include <iostream>
#include <sstream>
#include <vector>

std::string g_xmlInputFileName;

void SetXmlInputFileName(std::string xmlFileName) {g_xmlInputFileName = xmlFileName;}

const double energyLevelUncertainty(0.2); // used in FindPointerToLevel

double CalculateHalfLifeTimeInSeconds(double time, std::string unit)
{
    if (time == 0.0)
		return time;
    else
    {
			if(unit == "F")
				return time * pow(10., -9.);
			else if(unit == "MS")
				return time * 0.001;
			else if(unit == "US")
				return time * pow(10., -6.);
			else if(unit == "S")
			    return time;
			else if(unit == "M")
				return time * 60.;
			else if(unit == "H")
				return time * 3600.;
			else if(unit == "Y")
				return time * 31556926.;
			else
				return 0.;
    }
}

double stringToDouble (std::string strValue)
{
    QString qstr = QString::fromStdString(strValue);
    return qstr.toDouble();
}

using namespace std;

LoadDecayData::LoadDecayData()
{
	
	LoadDataFromXml();
//	cout << "SUCCESSFUL DATA LOADING" << endl;

}

LoadDecayData::LoadDecayData(string xmlFileName)
{
	g_xmlInputFileName = xmlFileName;
	LoadDataFromXml();
}

LoadDecayData::~LoadDecayData()
{

}

void LoadDecayData::LoadDataFromXml()
{
	pugi::xml_document dir;
    if (!dir.load_file(g_xmlInputFileName.c_str())) cout << "Error connected to " << g_xmlInputFileName << " file." << endl; // Exception

    for (pugi::xml_node nuclideFile = dir.child("NuclideFile"); nuclideFile; nuclideFile = nuclideFile.next_sibling("NuclideFile"))
    {
		allNuclides_.push_back(LoadNuclideData(nuclideFile.attribute("FileName").value()));
	}
	
	SetPointersToTransitions();
	
	FindPointersToFinalLevels();
	
	int tempAtomicNumber = dir.child("StartLevel").attribute("AtomicNumber").as_int();
	int tempAtomicMass = dir.child("StartLevel").attribute("AtomicMass").as_int();
    //double tempEnergy = dir.child("StartLevel").attribute("Energy").as_double();
    string tempEnergyStr = dir.child("StartLevel").attribute("Energy").value();
    double tempEnergy = stringToDouble(tempEnergyStr);
    startLevel_ = FindPointerToLevel(tempAtomicNumber, tempAtomicMass, tempEnergy, energyLevelUncertainty);

    SortNuclidesVector();
    startLevel_ = FindPointerToLevel(tempAtomicNumber, tempAtomicMass, tempEnergy, energyLevelUncertainty);
	
	tempAtomicNumber = dir.child("StopLevel").attribute("AtomicNumber").as_int();
	tempAtomicMass = dir.child("StopLevel").attribute("AtomicMass").as_int();
    //tempEnergy = dir.child("StopLevel").attribute("Energy").as_double();
    tempEnergyStr = dir.child("StopLevel").attribute("Energy").value();
    tempEnergy = stringToDouble(tempEnergyStr);

    stopLevel_ = FindPointerToLevel(tempAtomicNumber, tempAtomicMass, tempEnergy, energyLevelUncertainty);
	
    //double tempTime = dir.child("EventLength").attribute("Value").as_double();
    string tempTimeStr = dir.child("EventLength").attribute("Value").value();
    double tempTime = stringToDouble(tempTimeStr);
	string tempTimeUnit = dir.child("EventLength").attribute("TimeUnit").value();
	double tempTimeInSeconds = CalculateHalfLifeTimeInSeconds(tempTime, tempTimeUnit);
	eventDurationInSeconds_ = tempTimeInSeconds;
	
    //tempTime = dir.child("CycleLength").attribute("Value").as_double();
    tempTimeStr = dir.child("CycleLength").attribute("Value").value();
    tempTime = stringToDouble(tempTimeStr);
	tempTimeUnit = dir.child("CycleLength").attribute("TimeUnit").value();
	tempTimeInSeconds = CalculateHalfLifeTimeInSeconds(tempTime, tempTimeUnit);
	cycleDurationInSeconds_ = tempTimeInSeconds;

    //qDebug() << dir.child("EventLength").attribute("Value").as_double();
	
	cout << "Event length set to " << eventDurationInSeconds_ << " seconds." << endl;
	cout << "Cycle length set to " << cycleDurationInSeconds_ << " seconds." << endl;
	
	if( !dir.child("SpecifyFirstTransition").empty() )
	{
		int nuclideAtomicNumber = dir.child("SpecifyFirstTransition").attribute("NuclideAtomicNumber").as_int();
		int nuclideAtomicMass = dir.child("SpecifyFirstTransition").attribute("NuclideAtomicMass").as_int();
		pugi::xml_node initialLevel = dir.child("SpecifyFirstTransition").child("InitialLevel");
        //double initialLevelEnergy = initialLevel.attribute("Energy").as_double();
        string initialLevelEnergyStr = initialLevel.attribute("Energy").value();
        double initialLevelEnergy = stringToDouble(initialLevelEnergyStr);
		pugi::xml_node specifyTransition = dir.child("SpecifyFirstTransition").child("SpecifyTransition");
		string specifiedType = specifyTransition.attribute("Type").value();
        //double specifiedEnergy = specifyTransition.attribute("TransitionQValue").as_double();
        string specifiedEnergyStr = specifyTransition.attribute("TransitionQValue").value();
        double specifiedEnergy = stringToDouble(specifiedEnergyStr);
		
		RecalculateIntensities(nuclideAtomicNumber, nuclideAtomicMass, initialLevelEnergy,
		specifiedType, specifiedEnergy);
		
        if( !dir.child("SpecifySecondTransition").empty() )
        {
            nuclideAtomicNumber = dir.child("SpecifySecondTransition").attribute("NuclideAtomicNumber").as_int();
            nuclideAtomicMass = dir.child("SpecifySecondTransition").attribute("NuclideAtomicMass").as_int();
            initialLevel = dir.child("SpecifySecondTransition").child("InitialLevel");
            //initialLevelEnergy = initialLevel.attribute("Energy").as_double();
            initialLevelEnergyStr = initialLevel.attribute("Energy").value();
            initialLevelEnergy = stringToDouble(initialLevelEnergyStr);
            specifyTransition = dir.child("SpecifySecondTransition").child("SpecifyTransition");
            specifiedType = specifyTransition.attribute("Type").value();
            //specifiedEnergy = specifyTransition.attribute("TransitionQValue").as_double();
            specifiedEnergyStr = specifyTransition.attribute("TransitionQValue").value();
            specifiedEnergy = stringToDouble(specifiedEnergyStr);

            RecalculateIntensities(nuclideAtomicNumber, nuclideAtomicMass, initialLevelEnergy,
            specifiedType, specifiedEnergy);
        }
	}
	
}


Nuclide LoadDecayData::LoadNuclideData(const string filename)
{	
	std::vector<Level> nuclideLevels;
	pugi::xml_document doc;
    if (!doc.load_file(filename.c_str())) cout << "Error connected to " << filename << " file." << endl; // Exception

    pugi::xml_node nuclide = doc.child("Nuclide");
    int atNumber = nuclide.attribute("AtomicNumber").as_int();
	int atMass = nuclide.attribute("AtomicMass").as_int();
    //double qBeta = nuclide.attribute("QBeta").as_double();
    string qBetaStr = nuclide.attribute("QBeta").value();
    double qBeta = stringToDouble(qBetaStr);
	
    for (pugi::xml_node level = nuclide.first_child(); level; level = level.next_sibling())
    {       
        std::vector<Gamma> gammasFromLvL;
        std::vector<Beta> betasFromLvL;
        std::vector<Neutron> neutronsFromLvL;
        std::vector<Alpha> alphasFromLvL;
        
        for (pugi::xml_node transition = level.first_child(); transition; transition = transition.next_sibling())
		{									
			string type = transition.attribute("Type").value();
            //double transitionQval = transition.attribute("TransitionQValue").as_double();
            string transitionQvalStr = transition.attribute("TransitionQValue").value();
            double transitionQval = stringToDouble(transitionQvalStr);
            //double intensity = transition.attribute("Intensity").as_double();
            string intensityStr = transition.attribute("Intensity").value();
            double intensity = stringToDouble(intensityStr);
            string originString = transition.attribute("Origin").value();
            bool isAddedTransition = false;
            if( originString == "Added" )
                isAddedTransition = true;

            //double finalLvlEnergy = transition.child("TargetLevel").attribute("Energy").as_double();
            string finalLvlEnergyStr = transition.child("TargetLevel").attribute("Energy").value();
            double finalLvlEnergy = stringToDouble(finalLvlEnergyStr);
			int finalLvlAtMass = transition.child("TargetLevel").attribute("AtomicMass").as_int();
			int finalLvlAtNumber = transition.child("TargetLevel").attribute("AtomicNumber").as_int();

			/*if(type == "B+")
			{
				FermiDistribution* fermiDist = new FermiDistribution(atNumber, transitionQval, 1);
				
				pugi::xml_attribute attr = transition.attribute("ElectronConversionCoefficient");
				if(attr = attr.next_attribute())
				{
					//in B+ case eCC is actually EC intensity
					double eCC = transition.attribute("ElectronConversionCoefficient").as_double();
					Transition tempTransition(type, transitionQval, intensity, finalLvlEnergy, finalLvlAtMass, 
					finalLvlAtNumber, fermiDist, eCC);
						
					for (attr; attr; attr = attr.next_attribute())
					{
						double value;
						istringstream( attr.value() ) >> value;
						tempTransition.SetShellElectronConvCoef(attr.name(), value);
					}
					transitionsFromLvL.push_back(tempTransition);
				}
				else transitionsFromLvL.push_back(Transition(type, transitionQval, intensity, finalLvlEnergy, finalLvlAtMass, finalLvlAtNumber, fermiDist));
			} */
			if(type == "B-")
			{
				betasFromLvL.push_back(Beta(type, transitionQval, intensity, finalLvlEnergy, finalLvlAtMass, finalLvlAtNumber));
                if( isAddedTransition )
                    betasFromLvL.back().SetAsAddedTransition();
			}
			else if(type == "G")
			{
				if( !transition.child("ElectronConversionCoefficient").empty() )
				{
					// eCC == ElectronConversionCoefficient
                    //double eCC = transition.child("ElectronConversionCoefficient").attribute("Total").as_double();
                    string eCCStr = transition.child("ElectronConversionCoefficient").attribute("Total").value();
                    double eCC = stringToDouble(eCCStr);

					Gamma gammaTransition(type, transitionQval, intensity, finalLvlEnergy, finalLvlAtMass, 
					finalLvlAtNumber, eCC, atNumber);
					
					for(pugi::xml_attribute attr: transition.child("ElectronConversionCoefficient").attributes())
					{
                        //not sure if this works with Qt; using stringToDouble function may be needed
						double value;
						istringstream( attr.value() ) >> value;
						cout << attr.name() << " " << value << endl;
						gammaTransition.SetShellElectronConvCoef(attr.name(), value);
					}

					gammasFromLvL.push_back(gammaTransition);
                    if( isAddedTransition )
                        gammasFromLvL.back().SetAsAddedTransition();
				}
				else
				{
					gammasFromLvL.push_back(Gamma(type, transitionQval, intensity, finalLvlEnergy, finalLvlAtMass, finalLvlAtNumber));
                    if( isAddedTransition )
                        gammasFromLvL.back().SetAsAddedTransition();
				}
			}
			else if(type == "N")
			{
				neutronsFromLvL.push_back(Neutron(type, transitionQval, intensity, finalLvlEnergy, finalLvlAtMass, finalLvlAtNumber));
                if( isAddedTransition )
                    neutronsFromLvL.back().SetAsAddedTransition();
			}
			else if(type == "A")
			{
				alphasFromLvL.push_back(Alpha(type, transitionQval, intensity, finalLvlEnergy, finalLvlAtMass, finalLvlAtNumber));
                if( isAddedTransition )
                    alphasFromLvL.back().SetAsAddedTransition();
			}	
			else
			{
                cout << "Unknown particle type: " << type << "! Transition QValue: " << transitionQval << ". Data not registered." << endl;
			}		
		}
        //double lvlEnergy = level.attribute("Energy").as_double();
        string lvlEnergyStr = level.attribute("Energy").value();
        double lvlEnergy = stringToDouble(lvlEnergyStr);
        //double lvlSpin = level.attribute("Spin").as_double();
        string lvlSpinStr = level.attribute("Spin").value();
        double lvlSpin = stringToDouble(lvlSpinStr);
		string lvlParity = level.attribute("Parity").value();
        //double lvlHalfLifeTime = level.attribute("HalfLifeTime").as_double();
        string lvlHalfLifeTimeStr = level.attribute("HalfLifeTime").value();
        double lvlHalfLifeTime = stringToDouble(lvlHalfLifeTimeStr);
		string timeUnit = level.attribute("TimeUnit").value();
		double lvlHalfLifeTimeInSeconds = CalculateHalfLifeTimeInSeconds(lvlHalfLifeTime, timeUnit);
        string lvlTypeString = level.attribute("Origin").value();
        bool isPseudoLevel = false;
        if( lvlTypeString == "Added" )
            isPseudoLevel = true;
		
		nuclideLevels.push_back(Level(lvlEnergy, lvlSpin, lvlParity, lvlHalfLifeTimeInSeconds,
		 gammasFromLvL, betasFromLvL, neutronsFromLvL, alphasFromLvL));
        if( isPseudoLevel )
            nuclideLevels.back().setAsPseudoLevel();
    }
    
    cout << "Nuclide data uploaded successfully." << endl; //or not
	
    return Nuclide(atNumber, atMass, qBeta, nuclideLevels);
}

void LoadDecayData::SetPointersToTransitions()
{
	for ( auto it = allNuclides_.begin(); it != allNuclides_.end(); ++it )
	{
		for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
		{
			std::vector<Transition*> pointersToTransitionsFromLvL;
			for ( auto kt = jt->GetBetaTransitions()->begin(); kt != jt->GetBetaTransitions()->end(); ++kt )
			{
				Transition* pointerToBeta = &(*kt);
				pointersToTransitionsFromLvL.push_back(pointerToBeta);
			}
			for ( auto kt = jt->GetGammaTransitions()->begin(); kt != jt->GetGammaTransitions()->end(); ++kt )
			{
				Transition* pointerToGamma = &(*kt);
				pointersToTransitionsFromLvL.push_back(pointerToGamma);
			}
			for ( auto kt = jt->GetNeutronTransitions()->begin(); kt != jt->GetNeutronTransitions()->end(); ++kt )
			{
				Transition* pointerToNeutron = &(*kt);
				pointersToTransitionsFromLvL.push_back(pointerToNeutron);
			}
			for ( auto kt = jt->GetAlphaTransitions()->begin(); kt != jt->GetAlphaTransitions()->end(); ++kt )
			{
				Transition* pointerToAlpha = &(*kt);
				pointersToTransitionsFromLvL.push_back(pointerToAlpha);
			}

			jt->SetTransitions(pointersToTransitionsFromLvL);
            jt->NormalizeTransitionIntensities();
			jt->CalculateTotalProbability();
		}
	}
}

void LoadDecayData::FindPointersToFinalLevels()
{
	for ( auto it = allNuclides_.begin(); it != allNuclides_.end(); ++it )
	{
		for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
		{
			for ( auto kt = jt->GetTransitions()->begin(); kt != jt->GetTransitions()->end(); ++kt )
			{
                (*kt)->SetPointerToFinalLevel( FindPointerToLevel( (*kt)->GetFinalLevelAtomicNumber(), (*kt)->GetFinalLevelAtomicMass(), (*kt)->GetFinalLevelEnergy(), energyLevelUncertainty ) );
			}
		}
	}
}


Level* LoadDecayData::FindPointerToLevel(int atomicNumber, int atomicMass, double energy, double energyLvlUnc)
{
	for ( auto it = allNuclides_.begin(); it != allNuclides_.end(); ++it )
	{
		int atNumber = it->GetAtomicNumber();
		int atMass = it->GetAtomicMass();
		if(atomicNumber == atNumber && atomicMass == atMass)
			for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
			{
				double temp = jt->GetLevelEnergy();
				if( ((temp - energyLvlUnc) <= energy) && ((temp + energyLvlUnc) >= energy) )
					return &(*jt);
            }
	}
	
	// throw Exception
//    cout << "Pointer to level " << energy << " in " << atomicMass << " " << atomicNumber <<
//        " not found with default accuracy of " << energyLevelUncertainty << "keV." << endl;
	
		// additional security
	for ( auto it = allNuclides_.begin(); it != allNuclides_.end(); ++it )
	{
		int atNumber = it->GetAtomicNumber();
		int atMass = it->GetAtomicMass();
		if(atomicNumber == atNumber && atomicMass == atMass)								
            for ( int i = 1; i < 51; i++)
			{
				for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
				{
					double temp = jt->GetLevelEnergy();
					if( ((temp - i) <= energy) && ((temp + i) >= energy) )
                    {
//                        cout << "Pointer to level " << energy << " found with accuracy of " << i << "keV." << endl;
						return &(*jt);
                    }
				}
			}
	}
	
	// throw Exception
    cout << "Pointer to level " << energy << " STILL not found after 50keV threshold!" << endl;
}

void LoadDecayData::SortNuclidesVector()
{
    //Nuclide* motherNuclide = 0L;
    int motherNuclidePosition = 0;
    int nuclideIterator = 0;
    for ( auto it = allNuclides_.begin(); it != allNuclides_.end(); ++it )
    {
        for( auto il = it->GetNuclideLevels()->begin(); il != it->GetNuclideLevels()->end(); ++il)
            if( &(*il) == startLevel_ )
                motherNuclidePosition = nuclideIterator;
        nuclideIterator++;
    }

    int daughterNuclidePosition = 1;
    for( int i = 0; i < allNuclides_.size() - 1; ++i)
    {
        if ( i == 0 )
        {
            if( motherNuclidePosition == 0 )
                continue;
            else
            {
                cout << "Sorting nuclides vector." << endl;

                Nuclide tmpMotherNuclide = allNuclides_.at(motherNuclidePosition);
                Nuclide tmpOtherNuclide = allNuclides_.at(i);
                allNuclides_.at(i) = tmpMotherNuclide;
                allNuclides_.at(motherNuclidePosition) = tmpOtherNuclide;
                motherNuclidePosition = i;

                SetPointersToTransitions();
                FindPointersToFinalLevels();

                if( allNuclides_.at(motherNuclidePosition).GetNuclideLevels()->at(0).GetTransitions()->size() )
                {
                    Level* daughterLevel = allNuclides_.at(motherNuclidePosition).GetNuclideLevels()->at(0).GetTransitions()->at(0)->GetPointerToFinalLevel();
                    nuclideIterator = 0;
                    for ( auto it = allNuclides_.begin(); it != allNuclides_.end(); ++it )
                    {
                        for( auto il = it->GetNuclideLevels()->begin(); il != it->GetNuclideLevels()->end(); ++il)
                            if( &(*il) == daughterLevel )
                                daughterNuclidePosition = nuclideIterator;
                        nuclideIterator++;
                    }
                    continue;
                }
                else
                {
                    cout << "Nuclides not sorted properly!!!" << endl;
                    break;
                }
            }
        }

        if( daughterNuclidePosition == motherNuclidePosition + 1)
            continue;
        else
        {
            cout << "Sorting nuclides vector." << endl;

            Nuclide tmpDaughterNuclide = allNuclides_.at(daughterNuclidePosition);
            Nuclide tmpOtherNuclide = allNuclides_.at(i);
            allNuclides_.at(i) = tmpDaughterNuclide;
            allNuclides_.at(daughterNuclidePosition) = tmpOtherNuclide;
            motherNuclidePosition = i;

            SetPointersToTransitions();
            FindPointersToFinalLevels();

            if( allNuclides_.at(motherNuclidePosition).GetNuclideLevels()->at(0).GetTransitions()->size() )
            {
                Level* daughterLevel = allNuclides_.at(motherNuclidePosition).GetNuclideLevels()->at(0).GetTransitions()->at(0)->GetPointerToFinalLevel();
                nuclideIterator = 0;
                for ( auto it = allNuclides_.begin(); it != allNuclides_.end(); ++it )
                {
                    for( auto il = it->GetNuclideLevels()->begin(); il != it->GetNuclideLevels()->end(); ++il)
                        if( &(*il) == daughterLevel )
                            daughterNuclidePosition = nuclideIterator;
                    nuclideIterator++;
                }
                continue;
            }
            else
            {
                cout << "Nuclides not sorted properly!!!" << endl;
                break;
            }
        }
    }
}

void LoadDecayData::RecalculateIntensities( int atomicNumber, int atomicMass, double lvlEnergy,
string transitionType, double transitionEnergy)
{
	 
	for ( auto it = allNuclides_.begin(); it != allNuclides_.end(); ++it )
	{
		int atNumber = it->GetAtomicNumber();
		int atMass = it->GetAtomicMass();
		if(atomicNumber == atNumber && atomicMass == atMass)
			for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
			{
				double tempLvlE = jt->GetLevelEnergy();
                if( ((tempLvlE - 0.1) <= lvlEnergy) && ((tempLvlE + 0.1) >= lvlEnergy) )
				{
					for ( auto kt = jt->GetTransitions()->begin(); kt != jt->GetTransitions()->end(); ++kt )
					{
						string tempType = (*kt)->GetParticleType();
						double tempE = (*kt)->GetTransitionQValue();
						
						if(tempType != transitionType)
							(*kt)->ChangeIntensity(0.);
							
                        else if( !( ((tempE - 0.1) <= transitionEnergy) && ((tempE + 0.1) >= transitionEnergy) ) )
							(*kt)->ChangeIntensity(0.);
						else
						{
							(*kt)->ChangeIntensity(1.);
							cout << "Changing intensity to 1." << endl;
						}
					}
					jt->CalculateTotalProbability();
				}
			}
	}
}

void LoadDecayData::RecalculatePointersToLevels()
{
    FindPointersToFinalLevels();
}

void LoadDecayData::SetPointerToLevel(Transition* transition, int atomicNumber, int atomicMass, double energy)
{
    cout << "LoadDecayData::SetPointerToLevel" << endl;
    Level* pointerToLevel = FindPointerToLevel(atomicNumber, atomicMass, energy, energyLevelUncertainty);
    transition->SetPointerToFinalLevel(pointerToLevel);
    cout << "pointerToLevel->GetLevelEnergy()" << pointerToLevel->GetLevelEnergy() << endl;
}








