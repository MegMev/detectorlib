///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestDetectorHitsNormalizationProcess.h
///
///             feb 2016:   First concept
///                 Created as part of the conceptualization of existing REST
///                 software.
///                 Javier G. Garza
///_______________________________________________________________________________

#ifndef RestCore_TRestDetectorHitsNormalizationProcess
#define RestCore_TRestDetectorHitsNormalizationProcess

#include <TRestDetectorHitsEvent.h>

#include "TRestEventProcess.h"

class TRestDetectorHitsNormalizationProcess : public TRestEventProcess {
   private:
    TRestDetectorHitsEvent* fHitsInputEvent;   //!
    TRestDetectorHitsEvent* fHitsOutputEvent;  //!

    void InitFromConfigFile();
    void Initialize();
    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

    Double_t fFactor;

   public:
    inline any GetInputEvent() const { return fHitsInputEvent; }
    inline any GetOutputEvent() const { return fHitsOutputEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void LoadConfig(std::string configFilename, std::string name = "");

    void PrintMetadata() {
        BeginPrintProcess();

        metadata << " Renormalization factor : " << fFactor << endl;

        EndPrintProcess();
    }

    inline TRestMetadata* GetProcessMetadata() const { return nullptr; }

    inline const char* GetProcessName() const { return "hitsNormalizationProcess"; }

    TRestDetectorHitsNormalizationProcess();
    TRestDetectorHitsNormalizationProcess(const char* configFilename);

    ~TRestDetectorHitsNormalizationProcess();

    ClassDef(TRestDetectorHitsNormalizationProcess, 1);
};
#endif
