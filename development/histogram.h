/*
 * File:   Histogram.h
 * Author: aleksandra
 *
 * Created on 5 Sep 2012, 16:40
 * Wrapper for TH1F
 */

#ifndef HISTOGRAM_H
#define	HISTOGRAM_H
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include "Exceptions.h"
#include "binningController.h"
#include "TH1.h"
#include "hisdrr.h"

typedef std::vector<float> vecFloat;

class Histogram
{
public:
    Histogram();
    Histogram(TH1F* hist);
    Histogram( const Histogram* hist );
    /** @param data - vector of floats included data for histogram. It is not
        peaks hights! **/
    Histogram(float xMin, float xMax, vecFloat &data);
    Histogram(string filename, int hisId);
    /** Static method gived instance of empty histogram **/
    static Histogram* GetEmptyHistogram(float xMin, float xMax, int nrOfBins);
    static Histogram *GetEmptyHistogram()
    {
        return GetEmptyHistogram(0.,10*BinningController::getBinningFactor() , 10);
    }

    ~Histogram()
    {
        nrOfCounts_.clear();
        energy_.clear();
    }

    void Clear()
    {
        nrOfCounts_.clear();
        energy_.clear();
    }

    //Getters:
    int GetNrOfBins()
    {
        return nrOfCounts_.size();
    }
    float GetBinValue(unsigned int binNr);
    double GetXMin(){return (*energy_.begin());}
    double GetXMax(){return energy_.back();}
    std::string GetName() {return name_;}
    std::string GetTitle() {return title_;}
    float GetNrOfCounts()
    {
        float totNrOfCounts = 0;
        for(unsigned int i=0; i<nrOfCounts_.size(); ++i)
            totNrOfCounts += nrOfCounts_.at(i);
        return totNrOfCounts;
    }

    float GetMaxValue()
    {
        float maxValue = 0;
        for(unsigned int i=0; i<nrOfCounts_.size(); i++)
        {
            if(nrOfCounts_.at(i) > maxValue)maxValue=nrOfCounts_.at(i);
        }
        return maxValue;
    }

    float GetMaxValue(double xMin, double xMax)
    {
        float maxValue = 0;
        unsigned int minBin = this->FindBin(xMin);
        unsigned int maxBin = this->FindBin(xMax);
        for(unsigned int i=minBin; i<maxBin; ++i)
        {
            if(nrOfCounts_.at(i) > maxValue)maxValue=nrOfCounts_.at(i);
        }
        return maxValue;
    }

    float GetNrOfCounts(double xMin,double xMax)
    {
        float totNrOfCounts = 0;
        unsigned int minBin = this->FindBin(xMin);
        unsigned int maxBin = this->FindBin(xMax);
        for(unsigned int i=minBin; i<maxBin; ++i)
            totNrOfCounts += nrOfCounts_.at(i);
        return totNrOfCounts;
    }

    float GetBinCenter(unsigned int binNr){
//eva        if (binNr>=nrOfCounts_.size())
//Eva            throw Exception("Histogram::GetBinCenter: binNr > maxBin ");
//    cout << "bin center energy "<<  binNr << " "
//            << energy_.at(binNr) << "  "
//            << energy_.at(binNr+1) << endl;
            return (energy_.at(binNr+1) + energy_.at(binNr))/2.;
    }

    vecFloat GetAllData() {return nrOfCounts_;}
    vecFloat GetAllData(float enMin, float enMax);

    std::vector<int> GetAllIntData()
    {
        std::vector<int> allData(nrOfCounts_.size());
        for(unsigned int i=0; i<nrOfCounts_.size(); i++)
            allData.at(i) = static_cast<int> (nrOfCounts_.at(i));
        return allData;
    }
    std::vector<double> GetAllDataD(){
        std::vector<double> tmpvec(nrOfCounts_.begin(),nrOfCounts_.end());
        return tmpvec;}



    vecFloat GetEnergyVector(){return energy_;}
    vector<double> GetEnergyVectorD(){
        std::vector<double> tmpvec(energy_.begin(),energy_.end());
    return tmpvec;
    }

    int FindBin(double energy)
    {
       if(energy > energy_.back())
        {
//mk            cout<<"FindBin "<<energy_.back()<<" "<<energy<<endl;
//mk            throw Exception("Histogram::FindBin: energy >= maxEnergy ");
       energy = energy_.back(); //mk added
       }
        if(energy == energy_.back())
            return energy_.size()-1;
        for(unsigned int i=0; i<energy_.size()-1; ++i)
            if(energy >= energy_.at(i) && energy < energy_.at(i+1))
                return i;
        return 0;
    }

    int FindBin(float energy)
    {
        return FindBin(static_cast<double>(energy));
    }

    Histogram* readHISFile(string filename, int histID);
    Histogram* Convolution(Histogram* hist2);
    /** rebin histogram "group" times
        *example: if group = 5 five bins in old histograms become one
        **/
    void Rebin(float group);

    void Rebin(int group);//simple version

    /** newEnergy = sum_i (calParam.at(i) * oldEnergy^(i))
        example: if calParam has 3 numbers calibrated energy is:
        newEn = calParam.at(0)*oldEn^0 + calParam.at(1)*oldEn^1 + calParam.at(2)*oldEn^2
        **/
    void Recalibrate(std::vector<float> &calParam);
    void Recalibrate(std::vector<double> &calParam);

    void Scale(float c= 1.)
    {
        for(unsigned int i=0; i<nrOfCounts_.size(); ++i)
            nrOfCounts_.at(i) *= c;
    }

    void Scale(double c= 1.)
    {
        cout << "Histogram::Scale nrOfCounts.size:" << nrOfCounts_.size()
             << " scaling factor c: " << c << endl;
        for(unsigned int i=0; i<nrOfCounts_.size(); ++i)
            nrOfCounts_.at(i) *= c;
    }


    void Scale(float c, double xMin, double xMax)
    {
        unsigned int minBin = this->FindBin(xMin);
        unsigned int maxBin = this->FindBin(xMax);
        for(unsigned int i=minBin; i<maxBin; ++i)
            nrOfCounts_.at(i) *= c;
    }
    void ScaleD(double c, double xMin, double xMax)
    {
        unsigned int minBin = this->FindBin(xMin);
        unsigned int maxBin = this->FindBin(xMax);
        for(unsigned int i=minBin; i<maxBin; ++i)
            nrOfCounts_.at(i) *= c;
    }

    double Normalize(double c = 1.)
    {
        cout << "Histogram::Scale factor: " << c << " GetNrOfCounts: " << GetNrOfCounts() << endl;
        this->Scale(c/GetNrOfCounts());
        return c/GetNrOfCounts();
    }

    //void Normalize(double c, double xMin, double xMax) {this->Scale(c/GetNrOfCounts( xMin, xMax), xMin, xMax); }
     double Normalize(double c, double xMin, double xMax)
     {
//         cout<<"void Normalize(double c, double xMin, double xMax) "<<endl;
         cout << "Histogram::Normalize factor: " << c << " GetNrOfCounts: " << GetNrOfCounts()
              << " GEtNrOfCounts("<<xMin<<","<<xMax<<"): " << GetNrOfCounts( xMin, xMax) << endl;
         double ratio =c/GetNrOfCounts( xMin, xMax);
         cout << "RAtio: " << ratio << endl;
         this->Scale(ratio);
         cout << "Histogram::Normalize factor POScale: " << c << " GetNrOfCounts: " << GetNrOfCounts() << endl;
         return c/GetNrOfCounts( xMin, xMax);
     }
    void Fill(float energy, float peakHight)
    {
        int binNumber = FindBin(energy);
        nrOfCounts_.at(binNumber) += peakHight;
    }

    /** Adding method, works only if energy per channel is equal for both histograms**/
    void Add(Histogram* hist, int weight);
    void Add(Histogram* hist, double weight);


//    TH1F* GetRootHistogram(float multiFactor, std::string name="");
 private:
    std::vector<float> CutVector (std::vector<float> data, int min, int max);
    /** obligatory **/
    vecFloat nrOfCounts_;
    /**left edges of each bin
        *  size of energy_ vector is always one higher then nrOfCounts_ vector
        **/
    vecFloat energy_;
    std::string name_;
    std::string title_;
    float FindNewEnergy(double oldEnergy,  std::vector<float> &calParam);
    vecFloat FindEnergyVector(float xMin, float xMax, int nrOfBins);
    vecFloat FindEnergyVector2(float xMin, float calFactor, int nrOfBins);

    static int nrOfhist;
};

#endif
