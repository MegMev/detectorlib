///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestFastHitsToTrackProcess.h
///
///_______________________________________________________________________________


#ifndef RestCore_TRestFastHitsToTrackProcess
#define RestCore_TRestFastHitsToTrackProcess

#include <TRestHitsEvent.h>
#include <TRestTrackEvent.h>
#include <TRestEventProcess.h>

#include <TVector3.h>

class TRestFastHitsToTrackProcess:public TRestEventProcess {
    private:

        TRestHitsEvent *fHitsEvent;
        TRestTrackEvent *fTrackEvent;

        Double_t fCellResolution;
        Double_t fNetSize;
        TVector3 fNetOrigin;
        Int_t fNodes;

        void InitFromConfigFile();

        void Initialize();
        Int_t FindTracks( TRestHits *hits );

    protected:
        //add here the members of your event process
	

    public:
        void InitProcess();
        void BeginOfEventProcess(); 
        TRestEvent *ProcessEvent( TRestEvent *eventInput );
        void EndOfEventProcess(); 
        void EndProcess();
        void LoadDefaultConfig( );

        void LoadConfig( std::string cfgFilename );

        void PrintMetadata() { 

            BeginPrintProcess();

            std::cout << " Cell resolution : " << fCellResolution << " mm " << std::endl;
            std::cout << " Net size : " << fNetSize << " mm " << std::endl;
            std::cout << " Net origin : ( " << fNetOrigin.X() << " , "  << fNetOrigin.Y() << " , "  << fNetOrigin.Z() << " ) mm " << std::endl;
            std::cout << " Number of nodes (per axis) : " << fNodes << std::endl;

            EndPrintProcess();

        }

        TString GetProcessName() { return (TString) "fastHitsToTrack"; }

        //Constructor
        TRestFastHitsToTrackProcess();
        TRestFastHitsToTrackProcess( char *cfgFileName );
        //Destructor
        ~TRestFastHitsToTrackProcess();

        ClassDef(TRestFastHitsToTrackProcess, 1);      // Template for a REST "event process" class inherited from TRestEventProcess
};
#endif

