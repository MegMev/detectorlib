#include "TRestStringHelper.h"

using namespace std;
TRestStringHelper::TRestStringHelper()
{

}


TRestStringHelper::~TRestStringHelper()
{
}


///////////////////////////////////////////////
/// \brief Returns 1 only if valid mathematical expression keywords (or numbers) are found in the string **in**. If not it returns 0.
///
Int_t TRestStringHelper::isAExpression(string in)
{
	string temp = in;
	string replace[] = { "sqrt","log","exp","gaus" };
	for (int i = 0; i < replace->size(); i++)
	{
		temp = Replace(temp, replace[i], "0", 0);
	}


	if (temp.length() != 0)
	{
		if (temp.find_first_not_of("-0123456789e+*/.,)( ^") == std::string::npos)
		{
			if (temp.find("./") == 0 || temp.find("/") == 0) return 0;
			return 1;
		}
	}

	return 0;
}

///////////////////////////////////////////////
/// \brief Returns 1 only if a valid number is found in the string **in**. If not it returns 0.
///
Int_t TRestStringHelper::isANumber(string in)
{
	return (in.find_first_not_of("-+0123456789.e") == std::string::npos && in.length() != 0);
}

///////////////////////////////////////////////
/// \brief Spilt the input string according to the given separator. Returning a vector of fragments
///
std::vector<string> TRestStringHelper::Spilt(std::string in, string separator)
{
	std::vector<string> result;

	int pos = -1;
	int front = 0;
	while (1)
	{
		pos = in.find(separator.c_str(), pos + 1);
		string sub = in.substr(front, pos - front);
		if (sub != "") {
			result.push_back(sub);
		}
		front = pos + separator.size();
		if (pos == -1)break;
	}

	return result;

}



///////////////////////////////////////////////
/// \brief Returns the input string removing white spaces.
///
string TRestStringHelper::RemoveWhiteSpaces(string in)
{
	string out = in;
	size_t pos = 0;
	while ((pos = out.find(" ", pos)) != string::npos)
	{
		out.erase(pos, 1);
		pos = pos + 1;
	}

	return out;
}

///////////////////////////////////////////////
/// \brief Counts the number of occurences of **substring** inside the input string **in**. 
///
Int_t TRestStringHelper::Count(string in, string substring)
{
	int count = 0;
	size_t nPos = in.find(substring, 0); // First occurrence
	while (nPos != string::npos)
	{
		count++;
		nPos = in.find(substring, nPos + 1);
	}

	return count;
}

///////////////////////////////////////////////
/// \brief Replace every occurences of **thisSring** by **byThisString** inside string **in**.
///
string TRestStringHelper::Replace(string in, string thisString, string byThisString, size_t fromPosition = 0)
{
	string out = in;
	size_t pos = fromPosition;
	while ((pos = out.find(thisString, pos)) != string::npos)
	{
		out.replace(pos, thisString.length(), byThisString);
		pos = pos + 1;
	}

	return out;
}

string TRestStringHelper::ToDateTimeString(time_t time)
{
	tm *tm_ = localtime(&time);                // ��time_t��ʽת��Ϊtm�ṹ��
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	year = tm_->tm_year + 1900;                // ��ʱ�������꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬������ʱ����intΪtm_year����1900��
	month = tm_->tm_mon + 1;                   // ��ʱ�������£�����tm�ṹ����·ݴ洢��ΧΪ0-11��������ʱ����intΪtm_mon����1��
	day = tm_->tm_mday;                        // ��ʱ�������ա�
	hour = tm_->tm_hour;                       // ��ʱ������ʱ��
	minute = tm_->tm_min;                      // ��ʱ�������֡�
	second = tm_->tm_sec;                      // ��ʱ�������롣
	char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// ����ʱ��ĸ���char*������
	sprintf(yearStr, "%d", year);              // �ꡣ
	sprintf(monthStr, "%d", month);            // �¡�
	sprintf(dayStr, "%d", day);                // �ա�
	sprintf(hourStr, "%d", hour);              // ʱ��
	sprintf(minuteStr, "%d", minute);          // �֡�
	if (minuteStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		minuteStr[2] = '\0';
		minuteStr[1] = minuteStr[0];
		minuteStr[0] = '0';
	}
	sprintf(secondStr, "%d", second);          // �롣
	if (secondStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		secondStr[2] = '\0';
		secondStr[1] = secondStr[0];
		secondStr[0] = '0';
	}
	char s[20];                                // ����������ʱ��char*������
	sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// ��������ʱ����ϲ���
	string str(s);                             // ����string����������������ʱ��char*������Ϊ���캯���Ĳ������롣
	return str;                                // ����ת������ʱ����string������
}


///////////////////////////////////////////////
/// \brief Gets a double from a string.
///
Double_t TRestStringHelper::StringToDouble(string in)
{
	if (isANumber(in))
	{
		return stod(in);
	}
	else
	{
		return -1;
	}
}


///////////////////////////////////////////////
/// \brief Gets an integer from a string.
///
Int_t TRestStringHelper::StringToInteger(string in)
{
	return (Int_t)StringToDouble(in);
}


///////////////////////////////////////////////
/// \brief Gets a 3D-vector from a string. Format should be : (X,Y,Z).
///
TVector3 TRestStringHelper::StringTo3DVector(string in)
{
	TVector3 a;

	size_t startVector = in.find_first_of("(");
	if (startVector == string::npos) return a;

	size_t endVector = in.find_first_of(")");
	if (endVector == string::npos) return a;

	size_t n = count(in.begin(), in.end(), ',');
	if (n != 2) return a;

	size_t firstComma = in.find_first_of(",");
	size_t secondComma = in.find(",", firstComma + 1);

	if (firstComma >= endVector || firstComma <= startVector) return a;
	if (secondComma >= endVector || secondComma <= startVector) return a;

	string X = in.substr(startVector + 1, firstComma - startVector - 1);
	string Y = in.substr(firstComma + 1, secondComma - firstComma - 1);
	string Z = in.substr(secondComma + 1, endVector - secondComma - 1);

	a.SetXYZ(StringToDouble(X), StringToDouble(Y), StringToDouble(Z));

	return a;
}


///////////////////////////////////////////////
/// \brief Gets a 2D-vector from a string.
///
TVector2 TRestStringHelper::StringTo2DVector(string in)
{
	TVector2 a(-1, -1);

	size_t startVector = in.find_first_of("(");
	if (startVector == string::npos) return a;

	size_t endVector = in.find_first_of(")");
	if (endVector == string::npos) return a;

	size_t n = count(in.begin(), in.end(), ',');
	if (n != 1) return a;

	size_t firstComma = in.find_first_of(",");

	if (firstComma >= endVector || firstComma <= startVector) return a;

	string X = in.substr(startVector + 1, firstComma - startVector - 1);
	string Y = in.substr(firstComma + 1, endVector - firstComma - 1);


	a.Set(StringToDouble(X), StringToDouble(Y));

	return a;
}


///////////////////////////////////////////////
/// \brief Returns true if the filename exists.
///
bool TRestStringHelper::fileExists(const std::string& filename)
{
	struct stat buf;
	if (stat(filename.c_str(), &buf) != -1)
	{
		return true;
	}
	return false;
}

///////////////////////////////////////////////
/// \brief Returns true if the **filename** has *.root* extension.
///
bool TRestStringHelper::isRootFile(const std::string& filename)
{
	if (filename.find(".root") == string::npos) return false;

	return true;
}

///////////////////////////////////////////////
/// \brief Returns true if the **path** given by argument is writable 
///
bool TRestStringHelper::isPathWritable(const std::string& path)
{
	int result = 0;
#ifdef WIN32
	result = _access(path.c_str(), 2);
#endif
#ifdef linux
	result = access(path.c_str(), 2);
#endif
	
	if (result == 0) return true;
	else return false;
}

///////////////////////////////////////////////
/// \brief Check if the path is absolute path or not
///
bool TRestStringHelper::isAbsolutePath(const std::string & path)
{
	if (path[0] == '/' ||path[0]=='~'|| path.find(':') != -1) { return true; }
	return false;
}


///////////////////////////////////////////////
/// \brief Windows returns the class name in format of typeid. This method extracts the real class name from that.
///
std::string TRestStringHelper::typeidToClassName(std::string typeidstr)
{
	cout << typeidstr << endl;
	string temp = Replace(typeidstr, "class", "");
	temp = Replace(temp, "*", "");
	temp = Replace(temp, " ", "");
	return temp;

}


///////////////////////////////////////////////
/// \brief Checks if the config file can be openned. It returns OK in case of success, ERROR otherwise.
///
Int_t TRestStringHelper::ChecktheFile(std::string FileName)
{

	ifstream ifs;
	ifs.open(FileName.c_str());

	if (!ifs)
	{
		return -1;
	}
	else ifs.close();

	return 0;
}


///////////////////////////////////////////////
/// \brief Returns a list of files whose name match the pattern string. Key word is "*". e.g. abc00*.root
///
vector <TString> TRestStringHelper::GetFilesMatchingPattern(TString pattern)
{
	std::vector <TString> outputFileNames;

	if (pattern.First("*") >= 0 || pattern.First("?") >= 0)
	{
		char command[256];
		sprintf(command, "find %s > /tmp/RESTTools_fileList.tmp", pattern.Data());

		system(command);

		FILE *fin = fopen("/tmp/RESTTools_fileList.tmp", "r");
		char str[256];
		while (fscanf(fin, "%s\n", str) != EOF)
		{
			TString newFile = str;
			outputFileNames.push_back(newFile);
		}
		fclose(fin);

		system("rm /tmp/RESTTools_fileList.tmp");
	}
	else
	{
		if(fileExists((string)pattern))
			outputFileNames.push_back(pattern);
	}

	return outputFileNames;
}

std::string TRestStringHelper::ToUpper(std::string str)
{
	transform(str.begin(), str.end(), str.begin(), (int(*)(int))toupper);
	return str;
}

