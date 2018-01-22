#include "TRestTask.h"
ClassImp(TRestTask);

TRestTask::TRestTask() {
	Initialize();
	fNRequiredArgument = 0;
}

TRestTask::TRestTask(TString MacroFileName) 
{
	Initialize();
	fNRequiredArgument = 0;
	ifstream in(MacroFileName);
	if (!in.is_open()) {
		cout << "Error opening file" << endl;
		exit(1);
	}
	char buffer[256];
	while (!in.eof()) {
		in.getline(buffer, 256);
		string line = buffer;

		if (line.find('(') != -1 && line.find(' ') > 0) {
			//this line in a definition the macro method
			methodname = line.substr(line.find(' '), line.find('(') - line.find(' '));
			SetName(methodname.c_str());


			string args = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
			auto list = Spilt(args, ",");
			argumentname.clear();
			argumenttype.clear();
			for (int i = 0; i < list.size(); i++) 
			{
				auto tmp = Spilt(list[i], " ");
				if (tmp[0] == "TString") 
				{
					argumenttype.push_back(65);
				}
				else if (tmp[0] == "int" || tmp[0] == "Int_t")
				{
					argumenttype.push_back(3);
				}
				else if (tmp[0] == "double" || tmp[0] == "Double_t")
				{
					argumenttype.push_back(8);
				}
				else
				{
					argumenttype.push_back(-1);
				}
				argumentname.push_back(tmp[1]);
				argument.push_back("");
			}

			break;
		}

	}
}


void TRestTask::BeginOfInit()
{
	if (this->ClassName() == (string)"TRestTask")
	{
	}
	else
	{
		int n = GetNumberOfDataMember();
		for (int i = 1; i < n; i++) {
			TStreamerElement* e = GetDataMemberWithID(i);
			SetDataMemberValFromConfig(e);
		}
	}
}

void TRestTask::SetArgumentValue(string name, string value) 
{
	for (int i = 0; i < argumentname.size(); i++) {
		if (name == argumentname[i]) {
			argument[i] = name;
		}
	}
}


TRestTask* TRestTask::GetTask(TString MacroName)
{
	string command = (string)"find $REST_PATH/macros -name *" + (string)MacroName + (string)"* > /tmp/macros.list";
	system(command.c_str());
	FILE *f = fopen("/tmp/macros.list", "r");
	char str[256];
	fscanf(f, "%s\n", str);
	if (feof(f) == 0) 
	{
		cout << "REST ERROR : multi matching of macro \"" << MacroName << "\" found!" << endl;
		fclose(f);
		system("rm /tmp/macros.list");
		return NULL;
	}
	fclose(f);
	system("rm /tmp/macros.list");

	if ((string)str == "")
	{
		return NULL;
	}
	cout << "Found MacroFile "<<(string)str << endl;
	if (gInterpreter->LoadFile(str)!=0)
	{
		return NULL;
	}

	return new TRestTask(str);
}

bool TRestTask::InitTask(vector<string>arg)
{
	argument = arg;
	if (this->ClassName() == (string)"TRestTask") //this class
	{
		string methodname = GetName();
		cmdstr = methodname + "(";
		for (int i = 0; i < argument.size(); i++)
		{
			if (argumenttype[i] == 65) {
				cmdstr += "\""+argument[i]+"\"";
			}
			else
			{
				cmdstr += argument[i];
			}

			if (i < argument.size() - 1)
				cmdstr += ",";
		}
		cmdstr += ")";

		cout << "Command : " <<cmdstr << endl;
	}
	else//call from inherted class
	{
		int n = GetNumberOfDataMember();
		if (argument.size() < fNRequiredArgument) {
			PrintHelp();
			exit(0);
		}
		else
		{
			for (int i = 1; (i < argument.size() + 1 && i < n); i++)
			{
				TStreamerElement* e = GetDataMemberWithID(i);
				SetDataMemberVal(e, argument[i - 1]);
				debug << "data member " << e->GetName() << " has been set to " << GetDataMemberValString(e);
			}
		}
	}

	return true;
}


void TRestTask::RunTask(TRestManager*a)
{
	if (a == NULL&&cmdstr!="") {
		gInterpreter->ProcessLine(cmdstr);
	}
}


void TRestTask::PrintHelp() 
{
	error << this->ClassName() << " Gets invailed number of input argument!" << endl;
	error << "You should give the following argument :" << endl;
	int n = GetNumberOfDataMember();
	for (int i = 1; (i < fNRequiredArgument + 1 && i < n); i++){
		error << GetDataMemberWithID(i)->GetName() << endl;
	}
}