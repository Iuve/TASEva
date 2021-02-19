#include "histogramOutputController.h"
#include "Exceptions.h"
//EVA #include "binningController.h"

#include <fstream>
#include <vector>

HistogramOutputController::HistogramOutputController(string fileName, int nrOfLevels, int ID)
{
    outputName = fileName;
    inputToHisDrrName = "hisInputTemp.txt";
    size = 16384;
    ids.insert(ID);
    for(int i = 1; i != nrOfLevels+1; ++i)
    {
        ids.insert(i);
    }
    makeInputToHisDrrClass();
    int dotPossition = fileName.rfind(".");
    string fileName1 =fileName.substr(0,dotPossition);
    cout <<"HHHHHHHHHHHH" << fileName << fileName1 << endl;
    std::string nameHis = fileName1+".his";
    std::string nameDrr = fileName1+".drr";

    try
    {
        output = new HisDrr(nameDrr, nameHis, inputToHisDrrName);
    }
    catch (IOError &err)
    {
        cout << "I/O Error: " << err.show() << endl;
    }
    catch (GenError &err)
    {
        cout << "Error: " << err.show() << endl;
    }
}

HistogramOutputController::~HistogramOutputController()
{
    delete output;
}

void HistogramOutputController::makeInputToHisDrrClass()
{
    std::ofstream inputToHisDrrClass;
    inputToHisDrrClass.open(inputToHisDrrName.c_str());
    inputToHisDrrClass << "#ID     WORDS   CH X    CH Y    TITLE" << endl;
    for(std::set<int>::iterator it = ids.begin(); it != ids.end(); ++it)
        inputToHisDrrClass << *it << "     2       " << size << "   0      " << *it << endl;
    inputToHisDrrClass.close();
}


void HistogramOutputController::saveHistogram(Histogram* hist, int id)
{
    cout<< "histogramOutputController::saveHistogram - POCZATEK" << endl;
    cout<< "histogramOutputController::saveHistogram - 1" << endl;
    cout<< "hist size " << hist->GetNrOfCounts() << endl;
    if(ids.find(id) == ids.end())
    { //   throw Exception("histogramOutputController::saveHistogram : No ids");
        cout << "histogramOutputController::saveHistogram : No ids" << endl;
    }
    vector<int> values = hist->GetAllIntData();
//eva    int binSize = BinningController::getBinningFactor();
    int binSize = 1;
    cout<< "histogramOutputController::saveHistogram - 1" << endl;

    vector<unsigned> unbinValues;
    for(unsigned int i = 0; i != values.size(); ++i)
        for(int binCurrSize = 0; binCurrSize != binSize; ++binCurrSize)
            unbinValues.push_back(values.at(i));

    unbinValues.resize(size);
    cout<< "histogramOutputController::saveHistogram - 2" << endl;
    try
    {
        output->setValue(id, unbinValues);
    }
    catch (IOError &err)
    {
        cout << "I/O Error: " << err.show() << endl;
    }
    catch (GenError &err)
    {
        cout << "Error: " << err.show() << endl;
    }
    cout<< "histogramOutputController::saveHistogram - KONIEC" << endl;

}

