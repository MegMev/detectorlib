///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestDetectorHitsSmearingProcess.h
///
///             feb 2016:   First concept
///                 Created as part of the conceptualization of existing REST
///                 software.
///                 Javier G. Garza
///_______________________________________________________________________________

#ifndef RestCore_TRestDetectorHitsSmearingProcess
#define RestCore_TRestDetectorHitsSmearingProcess

#include <TRandom3.h>
#include <TRestEventProcess.h>

#include "TRestDetectorGas.h"
#include "TRestDetectorHitsEvent.h"

class TRestDetectorHitsSmearingProcess : public TRestEventProcess {
   private:
    TRestDetectorHitsEvent* fHitsInputEvent;   //!
    TRestDetectorHitsEvent* fHitsOutputEvent;  //!

    TRandom3* fRandom;  //!

    TRestDetectorGas* fGas;  //!

    void InitFromConfigFile() override;
    void Initialize() override;
    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

    Double_t fEnergyRef;         ///< reference energy for the FWHM
    Double_t fResolutionAtERef;  ///< FWHM at Energy of reference

   public:
    any GetInputEvent() const override { return fHitsInputEvent; }
    any GetOutputEvent() const override { return fHitsOutputEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    void PrintMetadata() override {
        BeginPrintProcess();

        metadata << " reference energy (ERef): " << fEnergyRef << endl;
        metadata << " resolution at ERef : " << fResolutionAtERef << endl;

        EndPrintProcess();
    }

    inline TRestMetadata* GetProcessMetadata() const { return nullptr; }

    const char* GetProcessName() const override { return "smearingProcess"; }

    inline Double_t GetEnergyReference() const { return fEnergyRef; }
    inline Double_t GetResolutionReference() const { return fResolutionAtERef; }

    TRestDetectorHitsSmearingProcess();
    TRestDetectorHitsSmearingProcess(const char* configFilename);

    ~TRestDetectorHitsSmearingProcess();

    ClassDefOverride(TRestDetectorHitsSmearingProcess, 2);
};
#endif
