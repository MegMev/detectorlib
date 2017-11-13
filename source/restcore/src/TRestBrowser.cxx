///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestBrowser.cxx
///
///             G4 class description
///
///             sept 2015:   First concept
///                 Created as part of the conceptualization of existing REST 
///                 software.
///                 JuanAn Garcia
///_______________________________________________________________________________


#include "TRestBrowser.h"
using namespace std;

ClassImp(TRestBrowser)
//______________________________________________________________________________
TRestBrowser::TRestBrowser()
{

Initialize();
}

//______________________________________________________________________________
TRestBrowser::~TRestBrowser()
{

frmMain->Cleanup();
//delete frmMain;

}

void TRestBrowser::Initialize(){

isFile=kFALSE;

frmMain = new TGMainFrame(gClient->GetRoot(),300,200);
frmMain->SetCleanup(kDeepCleanup);
frmMain->SetWindowName("Controller");

fCurrentEvent=0;

setButtons( );

frmMain->DontCallClose();
frmMain->MapSubwindows();
//frmMain->Resize();
frmMain->Layout();
frmMain->MapWindow();

fEventViewer=NULL;

}

void TRestBrowser::setButtons( ){


    fVFrame = new TGVerticalFrame(frmMain);
    
    fNEvent= new TGNumberEntry(fVFrame,fCurrentEvent);
    fNEvent->SetIntNumber(fCurrentEvent);
    fVFrame->AddFrame(fNEvent,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
            
    fLoadEvent = new  TGTextButton(fVFrame,"LOAD");///< Load Event button
    fLoadEvent->Connect("Clicked()","TRestBrowser",this,"LoadEventAction()");
    fVFrame->AddFrame(fLoadEvent,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    
    TString icondir(TString::Format("%s/icons/", getenv("ROOTSYS")));
    
    fButNext = new TGPictureButton(fVFrame, gClient->GetPicture(icondir+"GoForward.gif"));
    fButNext->Connect("Clicked()","TRestBrowser",this,"LoadNextEventAction()");
    fVFrame->AddFrame(fButNext,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    
    fButPrev = new TGPictureButton(fVFrame, gClient->GetPicture(icondir+"GoBack.gif"));
    fButPrev->Connect("Clicked()","TRestBrowser",this,"LoadPrevEventAction()");
    fVFrame->AddFrame(fButPrev,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    
    fMenuOpen = new TGPictureButton(fVFrame, gClient->GetPicture(icondir+"bld_open.png"));
    fMenuOpen->Connect("Clicked()","TRestBrowser",this,"LoadFileAction()");
    fVFrame->AddFrame(fMenuOpen,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    
    fExit = new  TGTextButton(fVFrame,"EXIT");///< Exit button
    fExit->Connect("Clicked()","TRestBrowser",this,"ExitAction()");
    fVFrame->AddFrame(fExit,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    
    
frmMain->AddFrame(fVFrame,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

}


void TRestBrowser::LoadEventAction( ){

Int_t eventN = (Int_t)fNEvent->GetNumber();

cout<<"Loading event "<<eventN<<endl;

if(LoadEvent(eventN))fCurrentEvent=eventN;

}

void TRestBrowser::LoadNextEventAction( ){

    Int_t nextEvent = fCurrentEvent+1;

    cout<<" Next event "<<nextEvent<<endl;

    if(LoadEvent(nextEvent))
    {
        fCurrentEvent=nextEvent;
        fNEvent->SetIntNumber(fCurrentEvent);
    }

}

void TRestBrowser::LoadPrevEventAction( ){

Int_t prevEvent = fCurrentEvent-1;

cout<<" Previous event "<<prevEvent<<endl;

if(LoadEvent(prevEvent)){
fCurrentEvent=prevEvent;
fNEvent->SetIntNumber(fCurrentEvent);

}

}

void TRestBrowser::LoadFileAction( ){
 
TGFileInfo fi;
new TGFileDialog (gClient->GetRoot(),gClient->GetRoot() , kFDOpen, &fi);

TString dir = fi.fFilename;

cout<<"Opening "<<dir.Data( )<<endl;

	if(fileExists( dir.Data() )){
	OpenFile(dir);
	fCurrentEvent=0;
	fNEvent->SetIntNumber(fCurrentEvent);
	}

}

Bool_t TRestBrowser::OpenFile( TString fName )
{

    TGeoManager *geometry = NULL;

    string fname = fName.Data();
    if( !fileExists( fname ) ) {
        cout << "WARNING. Input file does not exist" << endl;
        return kFALSE; 
    }
    
    if( fInputFile != NULL ) fInputFile->Close();
        
	OpenInputFile(fName);
	if (fInputFile == NULL) {
		error << "failed to open input file" << endl;
		exit(0);
	}
    TIter nextkey(fInputFile->GetListOfKeys());
    TKey *key;
    while ( (key = (TKey*)nextkey() ) ) {

        string className = key->GetClassName();
	
       if ( className == "TGeoManager" ) 
		   geometry = (TGeoManager *) fInputFile->Get( key->GetName() );
	   if(className=="TRestAnalysisTree")
		   fAnalysisTree= (TRestAnalysisTree *)fInputFile->Get(key->GetName());
    }

    if( fAnalysisTree == NULL && fEventTree == NULL)
    {
        cout << "REST ERROR (OpenFile) : No REST Tree was not found" << endl;
        cout << "Inside file : " << fInputFileName << endl;
        exit(1);
    }
 
	if (fAnalysisTree != NULL) {
		fAnalysisTree->ConnectEventBranches();
		fAnalysisTree->ConnectObservables();
	}

	if (fEventTree != NULL) {
		fEventTree->ConnectEventBranches();
		//init viewer
		string name=fInputEvent->ClassName();
		name += "Viewer";
		TClass *cl = TClass::GetClass(name.c_str());
		if (cl == NULL) {
			warning << "unsupported event type: " << fInputEvent->ClassName() << endl;
		}
		else
		{
			fEventViewer = (TRestEventViewer*)cl->New();
		}
		
	}
    
    if( geometry != NULL )fEventViewer->SetGeometry( geometry );
    
    PrintAllMetadata();
    isFile=kTRUE;
    LoadEventAction( );
    
    return kTRUE; 

}

Bool_t TRestBrowser::LoadEvent( Int_t n ){

if(!isFile){cout<<"No file..."<<endl;return kFALSE;}

if(n<fAnalysisTree->GetEntries()&&n>=0){
        this->GetEntry(n);

}
else{cout<<"Event out of limits"<<endl; return kFALSE;}
   
if (fEventViewer != NULL) {
	fEventViewer->AddEvent(fInputEvent);
}
if(fAnalysisTree!=NULL)
	GetAnalysisTree()->PrintObservables();
   
   
    
return kTRUE;    
}


void TRestBrowser::ExitAction( ){
 
gApplication->Terminate(0);

}

void TRestBrowser::InitFromConfigFile()
{
    cout << __PRETTY_FUNCTION__ << endl;
    
}

