#ifndef CONTAMINATION_H
#define CONTAMINATION_H

#include "histogram.h"


class Contamination{
public:
    Contamination(Histogram histVal, std::string filenameVal, int idVal, float norm, float inten) :
        hist(histVal), filename(filenameVal), id(idVal), normalization(norm), intensity(inten), freeIntensityFit_(true)
    {}


    Histogram hist;
    std::string filename;
    int id;
    float normalization;  /**<Normalization of the specturm:  for experimental like spectra just
                         Number of Counts i.e. it is Intensity*TotalNUmberOfCounts in Experimental spectrum >*/
    float intensity;     /**< Intensity <0,1> in the evaluation i.e. percent of the experimental spectrum.
                        To be multiplied by the number of counts in the experimental spectrum under evaluation
                        when renormalized when evaluating>*/
    void SetIntensityFitFlag(bool parameter){freeIntensityFit_ = parameter;}
    bool GetIntensityFitFlag(){return freeIntensityFit_;}


private:
    bool freeIntensityFit_;


};


#endif // CONTAMINATION_H
