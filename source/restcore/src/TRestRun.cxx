#include "TRestRun.h"
#include "TRestManager.h"
#include "TRestExternalFileProcess.h"



ClassImp(TRestRun);

TRestRun::TRestRun()
{
	Initialize();
}

TRestRun::TRestRun(string rootfilename)
{
	Initialize();
	OpenInputFile(rootfilename);
}

TRestRun::~TRestRun()
{
	if (fInputFile != NULL) {
		fInputFile->Close(); delete fInputFile;
	}
	//if (fOutputFile != NULL) CloseOutputFile();
}



///////////////////////////////////////////////
/// \brief Set variables by default during initialization.
///
void TRestRun::Initialize()
{
	SetSectionName(this->ClassName());

	fVersion = "2.2";
	time_t  timev; time(&timev);
	fStartTime = (Double_t)timev;
	fEndTime = fStartTime - 1; // So that run length will be -1 if fEndTime is not set

	fRunUser = "";
	fRunNumber = 0;
	fParentRunNumber = 0;
	fRunType = "Null";
	fExperimentName = "Null";
	fRunTag = "Null";


	//fOutputAnalysisTree = NULL;

	fInputFilename = "null";
	fOutputFilename = "default";


	fProcessedEvents = 0;

	fInputFileNames.clear();
	fInputFile = NULL;
	fInputEvent = NULL;
	fInputAnalysisTree = NULL;
	fCurrentEvent = 0;
	fFileProcess = NULL;
	fOutputFileName = "";

	return;
}


void TRestRun::BeginOfInit()
{
	if (fHostmgr == NULL)
	{
		error << "manager not initialized!" << endl;
		exit(0);
	}

	fInputFileName = GetParameter("inputFile");
	fInputFileNames = GetFilesMatchingPattern(fInputFileName);
}


///////////////////////////////////////////////
/// \brief Respond to the input xml element.
///
/// If the declaration of the input element is:
/// 1. addProcess: Add the name and type of this element into the process list, ready 
/// for the method InitThreads() to use. If the field value is not "on", the process will be omitted.
/// 2. TRest***Process: Add this element to config list, ready for the method InitThreads() to use.
///
/// Other types of declarations will be omitted.
///
Int_t TRestRun::ReadConfig(string keydeclare, TiXmlElement* e)
{


	return -1;
}

///////////////////////////////////////////////
/// \brief End of startup
///
/// Things doing in this method:
/// 1. Get some run information from the main config element(fElement).
/// 2. Initialize the processes and threads. Calling the method InitThreads(). TRestThread 
/// objects are not following sequencial startup procedure, as they are not thoroughly resident classes.
/// 3. Initialize input event. If the first process is external, then the input event is set to 
/// the process's output event. If not, the input event is instantiated by calling the method
/// TClass::GetClass()->New(). The address of input event is fixed since now. 
/// 4. Open the input file and find the tree. Then link the input event to the input tree. This is done
/// with in method OpenInputFile().
/// 5. Print some message.
///
void TRestRun::EndOfInit()
{
	//Get some infomation
	fRunUser = GetParameter("user").c_str();
	fRunType = GetParameter("runType", "NotDefined").c_str();
	fRunDescription = GetParameter("runDescription").c_str();
	fInputFilename = GetParameter("inputFile", "default").c_str();
	fOutputFilename = GetParameter("outputFormat", "default.root").c_str();
	fExperimentName = GetParameter("experiment", "preserve").c_str();
	fRunTag = GetParameter("runTag", "preserve").c_str();

	OpenInputFile(0);
	info << this->ClassName() << " : InputFile : " << fInputFileName << endl;
	if (fInputFileNames.size() > 1 && isRootFile((string)fInputFileNames[0]))
	{
		info << "which matches :" << endl;
		for (int i = 0; i < fInputFileNames.size(); i++) {
			info << fInputFileNames[0] << endl;
			info << endl;
		}
	}
	string filename = GetParameter("outputFile", "");
	info << this->ClassName() << " : OutputFile : " << filename << endl;
}




void TRestRun::OpenInputFile(int i)
{
	if (fInputFileNames.size() > i)
	{

		TString Filename = fInputFileNames[i];
		OpenInputFile((string)Filename);
	}

}


void TRestRun::OpenInputFile(TString filename, string mode)
{
	CloseFile();
	ReadFileInfo((string)filename);
	if (isRootFile((string)filename))
	{
		debug << "Initializing input file" << endl;


		fInputFile = new TFile(filename, mode.c_str());

		debug << "Finding TRestAnalysisTree.." << endl;
		TRestAnalysisTree * AnalysisTree = (TRestAnalysisTree *)fInputFile->Get("AnalysisTree");
		if (AnalysisTree != NULL)
		{
			fInputAnalysisTree = AnalysisTree;


			debug << "Finding event branch.." << endl;
			TObjArray* branches = fInputAnalysisTree->GetListOfBranches();
			TBranch *br = (TBranch*)branches->At(0);

			if (Count(br->GetName(), "EventBranch") == 0)
			{
				warning << "REST WARNING (OpenInputFile) : No event branch inside file : " << filename << endl;
				cout << "This file may be a pure analysis file" << endl;
			}
			else
			{
				fInputEvent = (TRestEvent*)br->GetAddress();
				debug << "found event branch of event type: " << fInputEvent->ClassName() << endl;
			}

		}

		else
		{
			error << "REST ERROR (OpenInputFile) : AnalysisTree was not found" << endl;
			error << "Inside file : " << filename << endl;
			exit(1);
		}

	}
	else
	{
		if (fFileProcess == NULL)
			info << "Input file is not root file, a TRestExtFileProcess is needed!" << endl;
		fInputFile = NULL;
		fInputAnalysisTree = NULL;
	}




}



void TRestRun::ReadFileInfo(string filename)
{
	//format example:
	//run[aaaa]_cobo[bbbb]_frag[cccc]_[time].graw
	//we are going to match it with inputfile:
	//run00042_cobo1_frag0000.graw

	FileInfo.clear();

	string format = GetParameter("inputFormat", "");
	string name = (string)filename;

	//remove extension
	if (format.find(".") != -1) { format = Spilt(format, ".")[0]; }
	if (name.find(".") != -1) { name = Spilt(name, ".")[0]; }

	//remove path name
	if (format.find("/") != -1) { format = Spilt(format, "/")[Spilt(format, "/").size() - 1]; }
	if (name.find("/") != -1) { name = Spilt(name, "/")[Spilt(name, "/").size() - 1]; }

	vector<string> formatsectionlist = Spilt(format, "_");
	vector<string> inputfilesectionlist = Spilt(name, "_");
	if (formatsectionlist.size() == 0)return;
	debug << "begin matching file names" << endl;
	for (int i = 0; i < formatsectionlist.size(); i++)
	{
		int pos1 = formatsectionlist[i].find("[");
		int pos2 = formatsectionlist[i].find("]");
		if (pos1 == -1 || pos2 == -1) { continue; }

		string prefix = formatsectionlist[i].substr(0, pos1 - 0);
		string FormatName = formatsectionlist[i].substr(pos1 + 1, pos2 - pos1 - 1);
		string FormatValue = "";
		if (prefix == "")
		{
			//如果参数是最后一个，则直接对应inputfile的最后几个参数
			if (i == formatsectionlist.size() - 1 && inputfilesectionlist.size() >= formatsectionlist.size())
			{
				for (int j = 0; j <= inputfilesectionlist.size() - formatsectionlist.size(); j++)
				{
					FormatValue += inputfilesectionlist[j + i] + "_";
				}
				FormatValue = FormatValue.substr(0, FormatValue.size() - 1);
			}
			//如果参数不是最后一个，则直接对应inputfile相应位置参数
			else if (i < formatsectionlist.size() - 1 && inputfilesectionlist.size() >i)
			{
				FormatValue = inputfilesectionlist[i];
			}
			else continue;
		}
		else
		{
			bool find;
			for (int j = 0; j < inputfilesectionlist.size(); j++)
			{
				if (inputfilesectionlist[j].find(prefix) == 0)
				{
					FormatValue = inputfilesectionlist[j].substr(prefix.size(), -1);
					find = true;
					break;
				}
				else if (j == inputfilesectionlist.size() - 1) find = false;
			}
			if (find == false) continue;
		}
		FileInfo[FormatName] = FormatValue;
	}

	debug << "begin collecting file info: " << filename << endl;
	struct stat buf;
	int fd = fileno(fopen(filename.c_str(), "rb"));
	fstat(fd, &buf);

	string datetime = ToDateTimeString(buf.st_mtime);
	FileInfo["Time"] = Spilt(datetime, " ")[1];
	FileInfo["Date"] = Spilt(datetime, " ")[0];
	FileInfo["Size"] = ToString(buf.st_size) + "B";
	FileInfo["Entries"] = ToString(GetTotalEntries());

}



void TRestRun::ResetEntry()
{
	fCurrentEvent = 0;
	if (fFileProcess != NULL) { fFileProcess->OpenInputExtFiles(fInputFileNames); fFileProcess->InitProcess(); }
}


Int_t TRestRun::GetNextEvent(TRestEvent* targetevt, TRestAnalysisTree* targettree)
{
	if (fFileProcess != NULL)
	{
		extreme << "getting next event from external process" << endl;
		fInputEvent = fFileProcess->ProcessEvent(NULL);
		fCurrentEvent++;
	}
	else
	{
		if (fInputAnalysisTree != NULL)
		{
			if (fInputAnalysisTree->GetEntries() <= fCurrentEvent - 1)fInputEvent = NULL;

			fInputAnalysisTree->GetEntry(fCurrentEvent);
			if (targettree != NULL) {
				for (int n = 0; n < fInputAnalysisTree->GetNumberOfObservables(); n++)
					targettree->SetObservableValue(n, fInputAnalysisTree->GetObservableValue(n));
			}
			fCurrentEvent++;
		}
		else
		{
			warning << "error to get event from input file, missing file process or analysis tree" << endl;
			fInputEvent == NULL;
		}
	}

	if (fInputEvent == NULL)
	{
		return -1;
	}

	fInputEvent->CloneTo(targetevt);


	return 0;
}




TString TRestRun::FormFormat(TString FilenameFormat)
{
	string inString = (string)FilenameFormat;
	string outString = (string)FilenameFormat;

	int pos = 0;
	while (1)
	{
		int pos1 = inString.find("[", pos);
		int pos2 = inString.find("]", pos1);
		if (pos1 == -1 || pos2 == -1)break;

		string targetstr = inString.substr(pos1, pos2 - pos1 + 1);//with []
		string target = inString.substr(pos1 + 1, pos2 - pos1 - 1);//without []

		string replacestr = GetFileInfo(target);
		if (replacestr == target)replacestr = fHostmgr->GetProcessRunner()->GetProcInfo(target);
		if (replacestr == target)replacestr = fHostmgr->GetRunInfo()->Get(target);
		outString = Replace(outString, targetstr, replacestr, 0);

		pos = pos2 + 1;

	}

	return outString;
}

//Merge an input string of file names together. All other files
//will be merged into the first file.
void TRestRun::MergeProcessFile(vector<string> filenames)
{
	string filename = GetParameter("outputFile", "output_[Time].root");
	filename = FormFormat(filename);
	info << "Creating file : " << filename << endl;

	TFileMerger* m = new TFileMerger();
	for (int i = 1; i < filenames.size(); i++)
	{
		m->AddFile(filenames[i].c_str(), false);
	}

	m->OutputFile(filenames[0].c_str(), "UPDATE");

	if (m->Merge())
	{
		rename(filenames[0].c_str(), filename.c_str());
		for (int i = 1; i < filenames.size(); i++)
		{
			remove(filenames[i].c_str());
		}
		fOutputFileName = filename;
	}
	else
	{
		fOutputFileName = "";
	}

	delete m;



	fout << this->ClassName() << " Created: " << filename << endl;

}

void TRestRun::CloseFile()
{
	if (fInputFile != NULL) {
		fInputFile->Close(); delete fInputFile; fInputFile = NULL;
	}
	if (fInputAnalysisTree != NULL) {
		delete fInputAnalysisTree; fInputAnalysisTree = NULL;
	}
}


void TRestRun::SetExtProcess(TRestExternalFileProcess* p)
{
	if (fFileProcess == NULL&&p != NULL) {
		fFileProcess = p;

		if (fFileProcess->OpenInputExtFiles(fInputFileNames) == 0) {
			error << "no files has been loaded by the external process!" << endl;
			exit(0);
		}

		fInputEvent = fFileProcess->GetOutputEvent();
		fInputFile = NULL;
		fInputAnalysisTree = NULL;
		info << "The external file process has been set! Name : " << fFileProcess->GetName() << endl;
	}
	else
	{
		if (fFileProcess != NULL) {
			error << "There can only be one file process!" << endl; exit(0);
		}
		if (p == NULL) {
			warning << "Given file process is null, skipping..." << endl;
		}
	}



}






























///////////////////////////////////////////////
/// \brief Open the root file and import the metadata of this class.
///
/// The class should be recovered to the same condition of the saved one.
///
/// Not implemented.
void TRestRun::ImportMetadata(TString rootFile, TString name, Bool_t store)
{
	if (isRootFile((string)rootFile))
	{
		TFile*f = new TFile(rootFile);
		TRestMetadata* data = (TRestMetadata*)f->Get(name);
		if (data != NULL&&store) {
			fMetadataInfo.push_back(data);
		}
	}
}




//Getters
TString TRestRun::GetDateFormatted(Double_t runTime)
{
	time_t tt = (time_t)runTime;
	struct tm *tm = localtime(&tt);

	char date[256];
	strftime(date, sizeof(date), "%Y-%B-%d", tm);

	return date;
}

TString TRestRun::GetTime(Double_t runTime)
{
	time_t tt = (time_t)runTime;
	struct tm *tm = localtime(&tt);

	char time[256];
	strftime(time, sizeof(time), "%H:%M:%S", tm);

	return time;
}


string TRestRun::Get(string target)
{
	auto a = GetDataMemberWithName(target);
	if (a != NULL)
	{
		return GetDataMemberStr(a);
	}

	return "";
}

//Printers
void TRestRun::PrintInfo()
{

	cout.precision(10);
	cout << endl;
	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
	cout << "TRestRun content" << endl;
	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
	// section name given in the constructor of TRestSpecificMetadata
	cout << "---------------------------------------" << endl;
	cout << "Name : " << GetName() << endl;
	cout << "Title : " << GetTitle() << endl;
	cout << "---------------------------------------" << endl;
	cout << "Parent run number : " << GetParentRunNumber() << endl;
	cout << "Run number : " << GetRunNumber() << endl;
	cout << "Experiment/project : " << GetExperimentName() << endl;
	cout << "Run type : " << GetRunType() << endl;
	cout << "Run tag : " << GetRunTag() << endl;
	cout << "Run user : " << GetRunUser() << endl;
	cout << "Run description : " << GetRunDescription() << endl;
	cout << "Start timestamp : " << GetStartTimestamp() << endl;
	cout << "Date/Time : " << GetDateFormatted(GetStartTimestamp()) << " / " << GetTime(GetStartTimestamp()) << endl;
	cout << "End timestamp : " << GetEndTimestamp() << endl;
	cout << "Date/Time : " << GetDateFormatted(GetEndTimestamp()) << " / " << GetTime(GetEndTimestamp()) << endl;
	//cout << "Input filename : " << fInputFilename << endl;
	//cout << "Output filename : " << fOutputFilename << endl;
	//cout << "Number of initial events : " << GetNumberOfEvents() << endl;
	//cout << "Number of processed events : " << fProcessedEvents << endl;
	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;

	if(fInputAnalysisTree!=NULL)
		fInputAnalysisTree->Show(0);

}


void TRestRun::PrintStartDate()
{
	cout.precision(10);

	cout << "------------------------" << endl;
	cout << "---- Run start date ----" << endl;
	cout << "------------------------" << endl;
	cout << "Unix time : " << fStartTime << endl;
	time_t tt = (time_t)fStartTime;
	struct tm *tm = localtime(&tt);

	char date[20];
	strftime(date, sizeof(date), "%Y-%m-%d", tm);
	cout << "Date : " << date << endl;

	char time[20];
	strftime(time, sizeof(time), "%H:%M:%S", tm);
	cout << "Time : " << time << endl;
	cout << "++++++++++++++++++++++++" << endl;
}

void TRestRun::PrintEndDate()
{
	cout << "----------------------" << endl;
	cout << "---- Run end date ----" << endl;
	cout << "----------------------" << endl;
	cout << "Unix time : " << fEndTime << endl;
	time_t tt = (time_t)fEndTime;
	struct tm *tm = localtime(&tt);

	char date[20];
	strftime(date, sizeof(date), "%Y-%m-%d", tm);
	cout << "Date : " << date << endl;

	char time[20];
	strftime(time, sizeof(time), "%H:%M:%S", tm);
	cout << "Time : " << time << endl;
	cout << "++++++++++++++++++++++++" << endl;
}

TRestMetadata* TRestRun::GetMetadataInfo(string type)
{
	for (int i = 0; i < fMetadataInfo.size(); i++)
	{
		if (fMetadataInfo[i]->ClassName() == type)
		{
			return fMetadataInfo[i];
		}
	}
	return NULL;
}

