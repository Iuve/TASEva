
#ifndef NEW_SDD_H
#define	NEW_SDD_H

#include <string>
#include <vector>

class SaveDecayData
{
public:
    SaveDecayData(std::string path);
    ~SaveDecayData();

    void SaveDecayStructure();  //writes to disk ALL XML nuclide files
    void SaveSpecifiedDecayStructure(Transition* firstTransition, Transition* secondTransition);
    void CreateDecayXML(Transition* firstTransition, Transition* secondTransition);  // create decay.xml (main driver)
    std::vector<std::string> GetXmlFilenames() {return fullFileNames_;}
    std::vector<std::string> GetShortXmlFilenames() {return shortFileNames_;}

private:
	
  std::string path_;
  std::vector<std::string> fullFileNames_;// with full directory
  std::vector<std::string> shortFileNames_; //without directory

};

#endif
