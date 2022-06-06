#ifndef CONTAMINATIONCONTROLLER_H
#define CONTAMINATIONCONTROLLER_H

//#include "contaminationPanel.h"
#include "histogram.h"
#include "contamination.h"


#include <string>
#include <vector>

#include <QWidget>

//class Contamination{
//public:
//    Contamination(Histogram* histVal, std::string filenameVal, int idVal, float norm, float inten) :
//        hist(histVal), filename(filenameVal), id(idVal), normalization(norm), intensity(inten)
//    {}
//    Histogram* hist;
//    std::string filename;
//    int id;
//    float normalization;  /*<Normalization of the specturm:  for experimental like spectra just Number of Counts >*/
//    float intensity;     /*< Intensity <0,1> in the evaluation i.e. percent of the experimental spectrum. To be multiplied by the number of counts
//                        in the experimental spectrum under evaluation when renormalized when evaluating>*/
//};


class ContaminationController{
public:
    explicit ContaminationController();
    ~ContaminationController();
/*    Histogram* getContamination(string name, int id);
    float getNormalization(string name, int id);
    float getIntensity(string name, int id);
    std::vector<Contamination*> getContaminations();
    Histogram* getTotalContamination();
*/
//signals:
//    void UpdateContaminationPanel(bool);
public slots:
    void addContamination(QString expID, QString name, QString id, QString intensity);
    void editContamination(QString expID, QString name, QString id, QString intensity);
    void removeContamination(QString name, QString id);
    void removeAll();
    void SaveAll();
    void SaveAllBayesian();

private:
//    Project *myProject = Project::get();
    std::vector< std::pair<int, Contamination> > contaminations_; // = myProject->contaminations();

};

#endif // CONTAMINATIONCONTROLLER_H
