#ifndef HISTOGRAMOUTPUTCONTROLLER_H
#define HISTOGRAMOUTPUTCONTROLLER_H

#include "hisdrr.h"
#include "histogram.h"

#include <string>
#include <set>

class HistogramOutputController
{
public:
    HistogramOutputController(std::string fileName, int nrOfLevels, int ID);
    HistogramOutputController(std::string fileName, set<int> ids);
    ~HistogramOutputController();
    void saveHistogram(Histogram* hist, int id);
    void makeInputToHisDrrClass();
private:
    HisDrr *output;
    std::string outputName;
    std::string inputToHisDrrName;
    std::set<int> ids;
    int size;
};

#endif // HISTOGRAMOUTPUTCONTROLLER_H
