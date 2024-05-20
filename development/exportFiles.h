#ifndef EXPORTFILES_H
#define EXPORTFILES_H

#include <QWidget>
#include "DeclareHeaders.hh"
namespace Ui {
class ExportFiles;
}

class ExportFiles : public QWidget
{
    Q_OBJECT

public:
    explicit ExportFiles(QWidget *parent = nullptr);
    ~ExportFiles();

    Ui::ExportFiles *uiEF;

    void exportXMLDecayFiles();
    void exportENSDecayFile();
    std::string checkAndPreparePath();
    void exportDecayInfo();
    void exportFeedingBGT();
    void exportGammaEvolution();
//    void saveLevelResp(int histId, QString outputFile);
// set of methods for ENS file outputing

    QString setENSParentQString(Nuclide parent);

public slots:
    void slotExportHIS(bool triggered);
    void slotExportDAT(bool triggered);
    void slotExportASCII(bool triggered);
    void slotExportAll(bool triggered);
    void slotExportResponseFunctions();
    void slotRespFunRaw(int);
    void slotRespFunFit(int);
    void slotDATRespFunRaw(int);
    void slotDATRespFunFit(int);

private:

    std::string path_;
    int respFunNorm_;
    int respFunDATNorm_;
 //----HIS
    bool expFlag_;
    bool simFlag_;
    bool recFlag_;
    bool conFlag_;
    bool respFunFitFlag_;
    bool respFunRawFlag_;
 //---ASCII
    bool decayXMLFlag_;
    bool decayENSFlag_;
    bool decGenInfoFlag_;
    bool feedBGTFlag_;
    bool gammaEvoFlag_;

};

#endif // EXPORTFILES_H
