/*************************************************************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
/// This process transforms the hits found inside a TRestDetectorHitsEvent into
/// physical signals inside a TRestDetectorSignalEvent.
///
/// This transformation requires the definition of a TRestDetectorReadout
/// describing the relation between the physical coordinates and the daq id channels
/// of an electronics system that is collecting the data of each physical readout
/// channel.
///
/// For the generation of basic readouts that can be used within this process, please
/// visit the [basic-readouts](https://github.com/rest-for-physics/basic-readouts)
/// repository.
///
/// The readout definition, including cathode and readout plane should be such that
/// defines a volume where the hits coordinates will be found. If not, the hit
/// coordinates will just be ignored.
///
/// Each TRestDetectorReadoutPlane found inside a TRestDetectorReadout definition
/// will be iterated to identify hits contributing to different readout planes.
/// An energy deposit or hit may contribute to more than one readout plane if the
/// hit is found in the volume defined by readout plane and cathode. The volumes
/// might overlap as it is for example the case of segmented mesh micromegas
/// readouts.
///
/// The physical Z-coordinate on the input event will be associated with a
/// physical time using the gas drift velocity given by the user as
/// the metadata parameter "driftVelocity". If the drift velocity is not
/// specified by the user, or it is negative, the process will try to
/// retrieve the drift velocity from the TRestDetectorGas metadata
/// definition that should have been defined inside TRestRun, and therefore
/// available to the TRestRun instance. In the case that the drift velocity is
/// retrieved from TRestDetectorGas two more parameters become relevant, the
/// detector pressure, and the drift field, that play a role on the value of
/// the drift velocity.
///
/// Therefore, relevant parameters (see also the process header) are:
/// * **driftVelocity**: The value of the electrons drift velocity.
/// Typically in mm/us.
/// * **gasPressure**: Gas pressure in bar. Relevant only if TRestDetectorGas
/// is used.
/// * **electricField**: Electric field. Typically in V/cm. Relevant only
/// if TRestDetectorGas is used.
/// * **sampling**: The physical time, even if it is given as a physical time,
/// will be discretized according to the sampling time given.
///
/// \htmlonly <style>div.image img[src="hitsToSignal.png"]{width:800px;}</style> \endhtmlonly
///
/// The following figure shows the results of applying the process to detector
/// hits generated by Monte Carlo artificially. The hit positions are translated
/// to their corresponding physical drift time values. The left figure shows the
/// event at the TRestDetectorHitsEvent form, the right figure shows the result of
/// applying this process.
/// The Z-coordinate on the input event is translated to a physical drift time
/// expressed in microseconds.
///
/// \image html hitsToSignal.png
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2015-October First implementation of TRestDetectorHitsToSignalProcess.
///			     Javier Galan
///
/// 2022-March   Adding documentation.
///              Javier Galan
///
/// \class      TRestDetectorHitsToSignalProcess
/// \author     Javier Galan
///
/// <hr>
///

#include "TRestDetectorHitsToSignalProcess.h"

using namespace std;

ClassImp(TRestDetectorHitsToSignalProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestDetectorHitsToSignalProcess::TRestDetectorHitsToSignalProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Constructor loading data from a config file
///
/// If no configuration path is defined using TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// The default behaviour is that the config file must be specified with
/// full path, absolute or relative.
///
/// \param cfgFileName A const char* giving the path to an RML file.
///
TRestDetectorHitsToSignalProcess::TRestDetectorHitsToSignalProcess(char* cfgFileName) {
    Initialize();

    if (LoadConfigFromFile(cfgFileName) == -1) LoadDefaultConfig();

    if (fReadout == nullptr) fReadout = new TRestDetectorReadout(cfgFileName);
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestDetectorHitsToSignalProcess::~TRestDetectorHitsToSignalProcess() {
    if (fReadout != nullptr) delete fReadout;

    delete fSignalEvent;
}

///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
///
void TRestDetectorHitsToSignalProcess::LoadDefaultConfig() {
    SetName("hitsToSignalProcess-Default");
    SetTitle("Default config");

    cout << "Hits to signal metadata not found. Loading default values" << endl;

    fSampling = 1;
    fElectricField = 1000;
    fGasPressure = 10;
}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestDetectorHitsToSignalProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fReadout = nullptr;
    fGas = nullptr;

    fHitsEvent = nullptr;
    fSignalEvent = new TRestDetectorSignalEvent();
}

///////////////////////////////////////////////
/// \brief Process initialization. This process accesses the information inside
/// TRestGeant4Metadata to identify the geometry volume ids associated to the hits.
///
void TRestDetectorHitsToSignalProcess::InitProcess() {
    fGas = GetMetadata<TRestDetectorGas>();
    if (fGas != nullptr) {
#ifndef USE_Garfield
        ferr << "A TRestDetectorGas definition was found but REST was not linked to Garfield libraries."
             << endl;
        ferr << "Please, remove the TRestDetectorGas definition, and add gas parameters inside the process "
                "TRestDetectorHitsToSignalProcess"
             << endl;
        if (!fGas->GetError()) fGas->SetError("REST was not compiled with Garfield.");
        if (!this->GetError()) this->SetError("Attempt to use TRestDetectorGas without Garfield");
#endif
        if (fGasPressure <= 0) fGasPressure = fGas->GetPressure();
        if (fElectricField <= 0) fElectricField = fGas->GetElectricField();

        fGas->SetPressure(fGasPressure);
        fGas->SetElectricField(fElectricField);

        if (fDriftVelocity <= 0) fDriftVelocity = fGas->GetDriftVelocity();
    } else {
        if (fDriftVelocity < 0) {
            if (!this->GetError()) this->SetError("Drift velocity is negative.");
        }
    }

    fReadout = GetMetadata<TRestDetectorReadout>();

    if (fReadout == nullptr) {
        if (!this->GetError()) this->SetError("The readout was not properly initialized.");
    }
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestDetectorHitsToSignalProcess::ProcessEvent(TRestEvent* evInput) {
    fHitsEvent = (TRestDetectorHitsEvent*)evInput;
    fSignalEvent->SetEventInfo(fHitsEvent);

    if (!fReadout) return nullptr;

    if (GetVerboseLevel() >= REST_Debug) {
        cout << "Number of hits : " << fHitsEvent->GetNumberOfHits() << endl;
        cout << "--------------------------" << endl;
    }

    for (int hit = 0; hit < fHitsEvent->GetNumberOfHits(); hit++) {
        Double_t x = fHitsEvent->GetX(hit);
        Double_t y = fHitsEvent->GetY(hit);
        Double_t z = fHitsEvent->GetZ(hit);
        Double_t t = fHitsEvent->GetTime(hit);

        if (GetVerboseLevel() >= REST_Extreme && hit < 20)
            cout << "Hit : " << hit << " x : " << x << " y : " << y << " z : " << z << " t : " << t << endl;

        Int_t planeId = -1;
        Int_t moduleId = -1;
        Int_t channelId = -1;

        for (int p = 0; p < fReadout->GetNumberOfReadoutPlanes(); p++) {
            Int_t daqId =
                fReadout->GetHitsDaqChannelAtReadoutPlane(TVector3(x, y, z), moduleId, channelId, p);

            TRestDetectorReadoutPlane* plane = fReadout->GetReadoutPlaneWithID(p);

            if (daqId >= 0) {
                Double_t energy = fHitsEvent->GetEnergy(hit);

                Double_t time = plane->GetDistanceTo(x, y, z) / fDriftVelocity + t;

                if (GetVerboseLevel() >= REST_Debug && hit < 20)
                    cout << "Module : " << moduleId << " Channel : " << channelId << " daq ID : " << daqId
                         << endl;

                if (GetVerboseLevel() >= REST_Debug && hit < 20)
                    cout << "Energy : " << energy << " time : " << time << endl;

                if (GetVerboseLevel() >= REST_Extreme && hit < 20)
                    printf(
                        " TRestDetectorHitsToSignalProcess: x %lf y %lf z %lf energy %lf t %lf "
                        "fDriftVelocity %lf fSampling %lf time %lf\n",
                        x, y, z, energy, t, fDriftVelocity, fSampling, time);

                if (GetVerboseLevel() >= REST_Extreme)
                    cout << "Drift velocity : " << fDriftVelocity << " mm/us" << endl;

                time = ((Int_t)(time / fSampling)) * fSampling;  // now time is in unit "us", but dispersed

                fSignalEvent->AddChargeToSignal(daqId, time, energy);

            } else {
                if (GetVerboseLevel() >= REST_Debug)
                    debug << "TRestDetectorHitsToSignalProcess. Readout channel not find for position (" << x
                          << ", " << y << ", " << z << ")!" << endl;
            }
        }
    }

    fSignalEvent->SortSignals();

    if (GetVerboseLevel() >= REST_Debug) {
        cout << "TRestDetectorHitsToSignalProcess : Number of signals added : "
             << fSignalEvent->GetNumberOfSignals() << endl;
        cout << "TRestDetectorHitsToSignalProcess : Total signals integral : " << fSignalEvent->GetIntegral()
             << endl;
    }

    if (fSignalEvent->GetNumberOfSignals() == 0) return nullptr;

    return fSignalEvent;
}

