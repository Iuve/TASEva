#ifndef DEEXCITATIONMODELCONTROLLER_H
#define DEEXCITATIONMODELCONTROLLER_H

#include <string>
#include <vector>

class DeexcitationModelController
{
public:
    DeexcitationModelController();
    ~DeexcitationModelController();

    void createIntensityMethodList();
    void applyDeexcitationModel();

private:
    std::vector<std::string> intensityMethodList_;
};

#endif // DEEXCITATIONMODELCONTROLLER_H
