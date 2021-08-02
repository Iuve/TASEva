#include "histogram.h"
#include "project.h"

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace std;

Histogram::Histogram()
{

}

Histogram::Histogram(TH1F* hist)
{
    if(hist)
    {
        float xMin = hist->GetXaxis()->GetXmin();
        float xMax = hist->GetXaxis()->GetXmax();
        name_ = hist->GetName();
        title_ = hist->GetTitle();
        int nrOfBins = hist-> GetNbinsX();
        energy_ = FindEnergyVector(xMin, xMax, nrOfBins);

        for(int i=0; i<nrOfBins; i++)
            nrOfCounts_.push_back(hist->GetBinContent(i+1));

        //std::cout<<"const "<<++nrOfhist<<std::endl;
    }
//Eva    else
//Eva        throw Exception("Histogram: null argument");
}

Histogram::Histogram( const Histogram* hist )
{
    nrOfCounts_.insert(nrOfCounts_.begin(), hist->nrOfCounts_.begin(), hist->nrOfCounts_.end());
    energy_ .insert(energy_.begin(), hist->energy_.begin(), hist->energy_.end());
    std::vector<double> tmpvec(nrOfCounts_.begin(),nrOfCounts_.end());
    //std::cout<<"const "<<++nrOfhist<<std::endl;

}


Histogram::Histogram(float xMin, float xMax, vecFloat &data)
{
    for(unsigned int i=0; i<data.size(); i++)
    {
        nrOfCounts_.push_back(data.at(i));
    }
    energy_ = FindEnergyVector(xMin, xMax, data.size());
}

Histogram::Histogram(string hisfilename, int hisId)
{
        std::cout << " Histogram::readHISFile :" << hisfilename
                  << " HistID: " << hisId << std::endl;
        if ( hisfilename.substr(0,1) != "/")
        {
//Eva            hisfilename = myProject->getWorkingDir() + hisfilename;
        } else {

        }
       std::string basename = hisfilename.substr (0, hisfilename.size()-4);
//       std::cout << "HIS file basename " << basename << std::endl;
       std::string hisFile = basename + ".his";
       std::string drrFile = basename + ".drr";

       try
       {
       std::vector<unsigned int> hisData;
       HisDrr* inputFile = new HisDrr(drrFile, hisFile);
       inputFile->getHistogram(hisData, hisId);
       for (int i=0; i != hisData.size(); ++i)
            {
           nrOfCounts_.push_back(static_cast<float>(hisData.at(i)));
           energy_.push_back(static_cast<float>(i));
            }
       }
       catch(GenError &r)
       {
/*Eva           std::cout << r.show() << std::endl;
           QMessageBox msgBox;
           msgBox.setText(QString::fromStdString(r.show()));
           msgBox.exec();
*/       }

}

Histogram* Histogram::GetEmptyHistogram(float xMin, float xMax, int nrOfBins)
{
    vecFloat data;
    for(int i=0; i<nrOfBins; i++)
    {
        data.push_back(0);
    }
    return new Histogram(xMin, xMax, data);
}





float Histogram::GetBinValue(unsigned int binNr) {
    if(binNr < nrOfCounts_.size())
        return nrOfCounts_.at(binNr);//first bin has nr 1
    else
    {
        cout<<"Histogram::GetBinValue: binNr > maxBin "<<endl;
//Eva        throw Exception("Histogram::GetBinValue: binNr > maxBin ");
    }
}

vecFloat Histogram::FindEnergyVector(float xMin, float xMax, int nrOfBins)
{
    vecFloat energy;
    for(int i=0; i<nrOfBins; i++)
    {
        float leftEdgeOfBin = xMin + static_cast<float>((xMax-xMin)/nrOfBins * i);
        energy.push_back(leftEdgeOfBin);
    }
    return energy;
}

vecFloat Histogram::FindEnergyVector2(float xMin, float calFactor, int nrOfBins)
{
    vecFloat energy;
    for(int i=0; i<nrOfBins+1; i++)
    {
        float leftEdgeOfBin = xMin + calFactor * i;
        energy.push_back(leftEdgeOfBin);
    }
    return energy;
}

void Histogram::Add(Histogram* hist, int weight)
{
    this->Add(hist, static_cast<double>(weight));
}


void Histogram::Add(Histogram* hist, double weight)
{
/*    cout << "--->Histogram::Add : POCZATEK" << endl;
    cout << " HISTbase NrOfBins:  " << this->GetNrOfBins() << " HIST to be added: " <<hist->GetNrOfBins() << endl;
    cout << " HISTbase xMIN:  " << this->GetXMin() << " HIST to be added: " <<hist->GetXMin() << endl;
    cout << " HISTbase xMAX:  " << this->GetXMax() << " HIST to be added: " <<hist->GetXMax() << endl;
*/    //first chcek if nr of keV per channel is the same for both histograms
    float calFactor1 = this->GetBinCenter(2) - this->GetBinCenter(1);
    float calFactor2 = hist->GetBinCenter(2) - hist->GetBinCenter(1);

    if(calFactor1 - calFactor2 > 1e-4 || calFactor1 - calFactor2 < -1e-4)
    {
        cout << this->GetXMax() << " " << this->GetXMin() << " " << this->GetNrOfBins()
                << " " << calFactor1
                << " != "
                << hist->GetXMax() << " " << hist->GetXMin() << " " << hist->GetNrOfBins()
                << " " << calFactor2 <<endl;
//Eva        throw WrongCalibrationException("Hisotogram::Add(Histogram*, int): Different calibration!");
    }

    float calFactor = calFactor1;
    float xMin = (this->GetXMin() < hist->GetXMin()) ? this->GetXMin():hist->GetXMin();
    float xMax = (this->GetXMax() > hist->GetXMax()) ? this->GetXMax():hist->GetXMax();
//    std::cout << xMin << " " << xMax << " " << calFactor << endl;
    int nrOfBins = static_cast<int> (xMax - xMin)/calFactor +1;

//    cout << "Histogram::Add FINAL nrOfBins: " <<nrOfBins << endl;
    vecFloat energyNew(nrOfBins);
    for(int i=0; i<nrOfBins; i++)
    {
        energyNew.at(i) = xMin + i*calFactor;
    }

    vecFloat nrOfCountsNew(nrOfBins);
    int firstBin = this->FindBin(this->GetBinCenter(0));
//    cout << "Histogram::Add FINAL firstBin: " << firstBin << endl;
    for(int i=firstBin; i<this->GetNrOfBins(); i++)
    {
        nrOfCountsNew.at(i) +=this->GetBinValue(i);
    }

    firstBin = hist->FindBin(hist->GetBinCenter(0));
    for(int i=firstBin; i<hist->GetNrOfBins(); i++)
    {
        nrOfCountsNew.at(i) +=hist->GetBinValue(i)*weight;
    }

    nrOfCounts_.clear();
    nrOfCounts_ = nrOfCountsNew ;
    energy_.clear();
    energy_ = energyNew;
//    cout << "--->Histogram::Add : KONIEC" << endl;

}

Histogram* Histogram::Convolution(Histogram* hist2)
{
    //time_t czas1 = time(0);
    int n1 = this -> GetNrOfBins();
    int n2 = hist2 -> GetNrOfBins();
//    float max1 = this-> GetXMax();
//    float max2 = hist2 ->GetXMin();
    float norm1 = this->GetNrOfCounts();
    float norm2 = hist2->GetNrOfCounts();
    float calFactor = this->GetBinCenter(2) - this->GetBinCenter(1);

    if(norm1 == 0 || norm2 == 0)
    {
        return GetEmptyHistogram(0, (n1+n2)*calFactor, n1+n2);
        //return GetEmptyHistogram(0, n1+n2, n1+n2);
    }

    Histogram* conv = GetEmptyHistogram(0.0, n1+n2,n1+n2);
    vecFloat energyVector = FindEnergyVector(0, (n1+n2)*calFactor, n1+n2);
    vecFloat valueVector(n1+n2);
    float value;
    for(int i=0; i< n1+n2; i++)
    {
        value = 0;
        for(int j=0; j<=i; j++)
        {
            if(j<n1 && i-j < n2)
                value += this -> GetBinValue(j) *hist2 -> GetBinValue(i-j);
        }
        if(value < 0)
            cout<<" Bin value lower than zero "<<value<<endl;
        valueVector.at(i) = value;
    }
    conv->energy_ = energyVector;
    conv->nrOfCounts_ = valueVector;
    return conv;
}
//mk added read HIS file method from other class



void Histogram::Rebin(float factor)
{
    vecFloat calFactor;
    calFactor.push_back(0);
    calFactor.push_back( 1./factor);
    int oldNrOfBins = this->GetNrOfBins();
    int newNrOfBins = oldNrOfBins/factor;
    double xMin = this->GetXMin();
    double xMax = this->GetXMax();

    Recalibrate(calFactor);

    //change energy vector
    energy_.clear();
    energy_ = FindEnergyVector(xMin,xMax, newNrOfBins);

    nrOfCounts_.resize(energy_.size()+1);

}

void Histogram::Rebin(int group)
{
    double xMin = this->GetXMin();
    vecFloat newCounts;
    int nrOfbinsInGroup = group - 1;
    for(int i = 0; i < this->GetNrOfBins(); i++)
    {
        if(nrOfbinsInGroup == group - 1)
        {
            newCounts.push_back(nrOfCounts_.at(i));
            nrOfbinsInGroup = 0;
        }
        else
        {
            newCounts.back() += nrOfCounts_.at(i);
            nrOfbinsInGroup ++;
        }
    }
    nrOfCounts_.clear();
    nrOfCounts_ .swap(newCounts);
    energy_.clear();
    energy_ = FindEnergyVector2(xMin,group, nrOfCounts_.size());

}

void Histogram::Recalibrate(std::vector<double> &calParam)
{
  std::vector<float> calParam_float (calParam.begin(), calParam.end());
  Recalibrate(calParam_float);
}

void Histogram::Recalibrate(std::vector<float> &calParam)
{
    int oldNrOfBins = this->GetNrOfBins();
    double xMinOld = this->GetXMin();
    double xMaxOld = this->GetXMax();
    double binLength = (xMaxOld-xMinOld)/oldNrOfBins;

    Histogram* newSpectrum = GetEmptyHistogram(xMinOld, xMaxOld, oldNrOfBins);

    for(int i=0; i<oldNrOfBins-1; i++)
    {
        /**take low and high edge of bin from old spectrum (enLow and enHigh)
                *  claculate calibrated energy (calEnLow and calEnHigh)
                *  chcek if both edges are in the same channel in new hist
                *  if yes add all counts to one channel
                *  otherwise spread counts with the appropriate weights into several channels
                **/
        double enLow = energy_.at(i);
        double enHigh = energy_.at(i+1);
        double calEnLow = FindNewEnergy(enLow, calParam);
        double calEnHigh = FindNewEnergy(enHigh, calParam);
        if(calEnHigh >= xMaxOld)
            break;

        int calBinLow = newSpectrum->FindBin(calEnLow);
        int calBinHigh = newSpectrum->FindBin(calEnHigh);

        if (calBinLow == calBinHigh)
            newSpectrum->Fill(calEnLow,nrOfCounts_.at(i));
        else
        {
            long double firstBinFraction = (newSpectrum->energy_.at(calBinLow +1) - calEnLow)
                                           /(calEnHigh - calEnLow);
            newSpectrum->Fill(calEnLow, nrOfCounts_.at(i)*firstBinFraction);
            long double lastBinFraction = (calEnHigh - newSpectrum->energy_.at(calBinHigh))
                                          /(calEnHigh - calEnLow);
            newSpectrum->Fill(calEnHigh, nrOfCounts_.at(i)*lastBinFraction);


            int nrOfComplededBins =  calBinHigh - calBinLow - 1;
            if(nrOfComplededBins > 0)
            {
                long double restBinFraction = (1. - firstBinFraction - lastBinFraction)/
                                              (static_cast<double>(nrOfComplededBins));
                for(int k = 1; k<nrOfComplededBins+1; k++)
                    newSpectrum->Fill(calEnLow + k*binLength, nrOfCounts_.at(i)*restBinFraction);
            }
        }
    }
    nrOfCounts_.clear();
    nrOfCounts_ = newSpectrum->GetAllData();
    energy_.clear();
    energy_ = newSpectrum->GetEnergyVector();
    delete newSpectrum;
    //return newSpectrum;

//    //testy 06.08.213 ******************************************************************88
//    oldNrOfBins = this->GetNrOfBins();
//    xMinOld = this->GetXMin();
//    xMaxOld = this->GetXMax();
//    binLength = (xMaxOld-xMinOld)/oldNrOfBins;
//
//    cout<<"Histogram::Recalibrate "<<xMinOld<<" "<<xMaxOld<<" "<<binLength<<endl;
}

float Histogram::FindNewEnergy(double oldEnergy, vector<float> &calParam)
{
    float newEnergy = 0;
    for (unsigned int i = 0; i<calParam.size(); ++i)
        newEnergy += calParam.at(i)*pow(oldEnergy, i);
    if (newEnergy < 0)
        newEnergy = 0;
    return newEnergy;
}

/*
TH1F* Histogram::GetRootHistogram(float multiFactor, std::string name)
{
    if(name_ == "")
    {
        name_ = name;
        title_ = name;
    }
    TH1F* newHis = new TH1F(name_.c_str(), title_.c_str(), GetNrOfBins(), GetXMin(), GetXMax());
    for (int i=0; i<GetNrOfBins(); i++)
        newHis->Fill(energy_.at(i), nrOfCounts_.at(i)*multiFactor);
    return newHis;
}
*/
std::vector<float> Histogram::CutVector (std::vector<float> data, int min, int max)
{

//Eva    if(max >= data.size())
//Eva         throw Exception(":(");
    std::vector<float> cutData;
    cutData.insert (cutData.begin(),data.begin()+ min, data.begin()+ max );
    return cutData;
}

vecFloat Histogram::GetAllData(float enMin, float enMax)
{
    int lowEdgeNumber = this->FindBin(enMin);
    int highEdgeNumber = this->FindBin(enMax);
    vecFloat histValue;
    histValue = CutVector(this->GetAllData(), lowEdgeNumber, highEdgeNumber);
    return histValue;
}

//eva int Histogram::nrOfhist = 0;


void Histogram::AdjustEnergyVectorTo2DBinFactor(int binFactor)
{
    //cout << "\n \n \n energy_.size() = " << energy_.size() << "\n \n \n";
    for(unsigned int i = 0; i < energy_.size(); i++)
    {
        float tmpEnergy = energy_.at(i);
        energy_.at(i) = tmpEnergy * binFactor;
    }

}
