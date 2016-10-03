///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestSignalEvent.h
///
///             Event class to store DAQ events either from simulation and acquisition 
///
///             sept 2015:   First concept
///                 Created as part of the conceptualization of existing REST 
///                 software.
///                 JuanAn Garcia
///_______________________________________________________________________________


#ifndef RestDAQ_TRestSignalEvent
#define RestDAQ_TRestSignalEvent

#include <iostream>

#include <TObject.h>
#include <TArrayD.h>
#include <TPad.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TAxis.h>

#include "TRestEvent.h"
#include "TRestSignal.h"

class TRestSignalEvent: public TRestEvent {

    protected:

#ifndef __CINT__
        TPad *fPad;
        Double_t fMinTime;
        Double_t fMaxTime;
        Double_t fMinValue;
        Double_t fMaxValue;
#endif

        std::vector <TRestSignal> fSignal; //Collection of signals that define the event

    private:

        void SetMaxAndMin();

    public:

        Bool_t signalIDExists( Int_t sID ) { if( GetSignalIndex( sID ) == -1 ) return false; return true; }

        void SortSignals( ) { for ( int n = 0; n < GetNumberOfSignals(); n++ ) fSignal[n].Sort(); }

        //Setters
        void AddSignal(TRestSignal s);

        void AddChargeToSignal( Int_t sgnlID, Double_t tm, Double_t chrg );

        //Getters
        Int_t GetNumberOfSignals() { return fSignal.size(); }
        TRestSignal *GetSignal(Int_t n ) { return &fSignal[n]; }
        Int_t GetSignalIndex( Int_t signalID );

        Double_t GetBaseLineAverage( Int_t startBin, Int_t endBin );
        Double_t GetBaseLineSigmaAverage( Int_t startBin, Int_t endBin );
        void SubstractBaselines( Int_t startBin, Int_t endBin );
        Double_t GetIntegral( Int_t startBin = 0, Int_t endBin = 0 );
        Double_t GetIntegralWithThreshold( Int_t from, Int_t to, Int_t startBaseline, Int_t endBaseline, Double_t nSigmas, Int_t nPointsOverThreshold, Double_t minPeakAmplitude );

        Double_t GetMaxValue( );
        Double_t GetMinValue( );
        Double_t GetMinTime( );
        Double_t GetMaxTime( );

        Double_t GetIntegralWithTime( Int_t startTime, Int_t endTime )
        {
            Double_t sum = 0;
            for( int n = 0; n < GetNumberOfSignals(); n++ )
                sum += fSignal[n].GetIntegralWithTime( startTime, endTime );

            return sum;
        }

        // Default
        void Initialize();
        void PrintEvent();

        TPad *DrawEvent();

        //Construtor
        TRestSignalEvent();
        //Destructor
        virtual ~ TRestSignalEvent();

        ClassDef(TRestSignalEvent, 1);     // REST event superclass
};
#endif
