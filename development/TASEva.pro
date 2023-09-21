#-------------------------------------------------
#
# Project created by QtCreator 2020-01-10T08:49:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport concurrent
#ROOTSYS = /opt/root/root-6.18.04
#GEANTPATH = /home/iuvenis/Geant4/geant4-10-3-1-install
#INCLUDEPATH += $$(ROOTSYS)/include
#INCLUDEPATH += $${GEANT_PATH}/include/Geant4
#QMAKE_RPATHDIR += $$(ROOTSYS)/lib
#QMAKE_RPATHDIR += $${GEANT_PATH}/lib
QMAKE_RPATHDIR += /opt/root/root-6.18.04/lib
QMAKE_RPATHDIR += /home/iuvenis/Geant4/geant4-10-3-1-install/lib
INCLUDEPATH += /opt/root/root-6.18.04/include
INCLUDEPATH += /home/iuvenis/Geant4/geant4-10-3-1-install/include/Geant4
LIBS += -L/opt/root/root-6.18.04/lib -lCore  -lRIO -lNet -lHist \
        -lGraf -lGraf3d -lGpad -lTree \
        -lRint -lPostscript -lMatrix -lPhysics \
        -lz  -lm -ldl -rdynamic
#out -lCint
#        -lGui -lRGL
LIBS += -L/home/iuvenis/Geant4/geant4-10-3-1-install/lib -lG4global -lG4materials -lG4physicslists \
        -lG4processes -lG4clhep -lG4zlib -lG4particles -lG4intercoms -lG4geometry -lG4track \
        -lG4digits_hits -lG4tracking -lG4event -lG4run  -lG4modeling -lG4FR -lG4parmodels \
        -lG4readout -lG4error_propagation -lG4analysis -lG4persistency -lG4interfaces \
        -lG4Tree

#G4graphics_reps -lG4vis_management -lG4VRML -lG4RayTraces -lG4visHepRep -lG4GMocren

TARGET = TASEva
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    deexcitationModelController.cpp \
    mainwindow.cpp \
        project.cpp \
    pseudoLevelsController.cpp \
        qcustomplot.cpp \
        hisdrr.cpp \
        histogram.cpp \
        contaminationController.cpp \
        contamination.cpp \
        pileupController.cpp \
        histogramOutputController.cpp \
        Transition.cc \
        Nuclide.cc \
        Neutron.cc \
        LoadDecayData.cc \
        Level.cc \
        Gamma.cc \
        FermiDistribution.cc \
        Beta.cc \
        Alpha.cc \
        DecayPath.cc \
        pugixml.cc \
        decayPathEditor.cpp \
        rowData.cpp \
        tablecontroller.cpp \
        transitionEditor.cpp \
    decaySpectrum.cpp \
    transitionRespProvider.cpp \
    levelRespProvider.cpp \
    SaveDecayData.cc \
    responsefunction.cpp \
    PeriodicTable.cc \
    analysis2d.cpp \
    binningController.cpp \
    fitController.cpp \
    histogramgraph.cpp \
    manualfitgraph.cpp \
    twoDimFitController.cpp \
    status.cpp \
    calibrateenergy.cpp \
    simplefigure.cpp \
    tableinput.cpp \
    exportFiles.cpp

HEADERS  += mainwindow.h \
    PeriodicTable.hh \
    analysis2d.h \
    binningController.h \
    deexcitationModelController.h \
    fitController.h \
    histogramgraph.h \
    manualfitgraph.h \
    project.h \
    pseudoLevelsController.h \
    qcustomplot.h \
    hisdrr.h \
    histogram.h \
    Exceptions.h \
    contaminationController.h \
    contamination.h \
    pileupController.h \
    histogramOutputController.h \
    Transition.hh \
    Nuclide.hh \
    Neutron.hh \
    LoadDecayData.hh \
    Level.hh \
    Gamma.hh \
    FermiDistribution.hh \
    DeclareHeaders.hh \
    Beta.hh \
    Alpha.hh \
    DecayPath.hh \
    MyTemplates.h \
    pugiconfig.hh \
    pugixml.hh \
    drrblock.h \
    decayPathEditor.h \
    rowData.h \
    tablecontroller.h \
    transitionEditor.h \
    decaySpectrum.h \
    HistIds.h \
    transitionRespProvider.h \
    levelRespProvider.h \
    SaveDecayData.hh \
    responsefunction.h \
    twoDimFitController.h \
    status.h \
    calibrateenergy.h \
    simplefigure.h \
    tableinput.h \
    exportFiles.h

FORMS    += mainwindow.ui \
    analysis2d.ui \
    histogramgraph.ui \
    manualfitgraph.ui \
    pileupController.ui \
    DecayPathEditor.ui \
    transitionEditor.ui \
    status.ui \
    simplefigure.ui \
    tableinput.ui \
    exportFiles.ui

DISTFILES += \
    Decay.xml
