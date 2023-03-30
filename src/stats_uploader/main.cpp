// (C)2008 S2 Games
// Stats uploading utility for Savage 2

//=============================================================================
// Headers
//=============================================================================
#define _WIN32_WINNT 0x0500
#define _WIN32_WINDOWS 0x0410
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>

using namespace std;
typedef unsigned int uint;
//=============================================================================

/*====================
  main
  ====================*/
int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		cout << "Login/Password required!" << endl;
		return 1;
	}

	_mkdir("archive");

	string sUser(argv[1]);
	string sPass(argv[2]);

	int iAttempts(0);
	string sDeleteMe;

	for (;;)
	{
		Sleep(15000);

		WIN32_FIND_DATA	finddata;

		// Look for a new 
		HANDLE handle(FindFirstFile("*.stats", &finddata));
		if (handle == INVALID_HANDLE_VALUE)
		{
			cout << ".";
			continue;
		}

		cout << endl;

		do
		{
			++iAttempts;

			uint uiReplaySize(0);

			// Check for a replay with the same name
			string sReplayFile(finddata.cFileName);
			sReplayFile = sReplayFile.substr(0, sReplayFile.find_first_of('.'));
			sReplayFile +=  + ".s2r";
			struct _stat stats;
			if (_stat(sReplayFile.c_str(), &stats) == 0)
			{
				uiReplaySize = stats.st_size;

				// Send the replay
				string sCommand("curl -T ");
				sCommand += sReplayFile;
				sCommand += " -u ";
				sCommand += sUser;
				sCommand += ":";
				sCommand += sPass;
				sCommand += " ftp://www.savage2replays.com/replays/";
				sCommand += sReplayFile;

				STARTUPINFO startInfo;
				memset(&startInfo, 0, sizeof(STARTUPINFO));
				PROCESS_INFORMATION	procInfo;
				memset(&procInfo, 0, sizeof(PROCESS_INFORMATION));
				
				char szCommand[2048];
				memset(szCommand, 0, 2048);
				strncpy_s(szCommand, sCommand.c_str(), 2048);

				cout << "Uploading replay: " << sReplayFile.c_str() << endl;
				CreateProcess(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo);
				DWORD dwResult(WaitForSingleObject(procInfo.hProcess, 15 * 60 * 1000));
				if (dwResult != WAIT_OBJECT_0)
				{
					if (dwResult == WAIT_TIMEOUT)
						ExitProcess(TerminateProcess(procInfo.hProcess, -1));
					cout << "Failed sending replay: " << sReplayFile.c_str() << endl;
				}

				DWORD dwExitCode(0);
				bool bGotCode(GetExitCodeProcess(procInfo.hProcess, &dwExitCode) == TRUE);
				if (!bGotCode || (bGotCode && dwExitCode != 0))
				{
					if (iAttempts > 10)
					{
						iAttempts = 0;

						string sNewName("archive/");
						sNewName += sReplayFile;
						rename(sReplayFile.c_str(), sNewName.c_str());
					}
					continue;
				}

				string sNewName("archive/");
				sNewName += sReplayFile;
				rename(sReplayFile.c_str(), sNewName.c_str());
			}
			
			// Send the stats
			ofstream file("temp.stats");
			if (!file.is_open())
			{
				cout << "Failed creating temp.stats" << endl;
				continue;
			}

			// Read request from file
			ifstream fileStats(finddata.cFileName);
			if (!fileStats.is_open())
			{
				if (iAttempts > 10)
				{
					iAttempts = 0;
					string sNewName("archive/");
					sNewName += finddata.cFileName;
					rename(finddata.cFileName, sNewName.c_str());
				}
				continue;
			}

			char szRequest[65536];
			memset(szRequest, 0, 65536);
			fileStats.getline(szRequest, 65536);
			string sURL(szRequest);
			fileStats.read(szRequest, 65535);
			string sRequest(szRequest);

			if (uiReplaySize > 0)
			{
				char szSize[20];
				memset(szSize, 0, 20);
				_itoa_s(uiReplaySize, szSize, 19, 10);
				sRequest += "&file_name=";
				sRequest += sReplayFile;
				sRequest += "&file_size=";
				sRequest += szSize;
			}

			file << sRequest.c_str() << "\0";
			file.close();

			STARTUPINFO startInfo;
			memset(&startInfo, 0, sizeof(STARTUPINFO));
			PROCESS_INFORMATION	procInfo;
			memset(&procInfo, 0, sizeof(PROCESS_INFORMATION));

			string sCommand("curl -d @temp.stats http://masterserver.savage2.s2games.com");
			sCommand += sURL;

			char szCommand[2048];
			memset(szCommand, 0, 2048);
			strncpy_s(szCommand, sCommand.c_str(), 2048);

			cout << "Sending stats" << endl;
			CreateProcess(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo);
			DWORD dwResult(WaitForSingleObject(procInfo.hProcess, 15 * 60 * 1000));
			if (dwResult != WAIT_OBJECT_0)
			{
				if (dwResult == WAIT_TIMEOUT)
					ExitProcess(TerminateProcess(procInfo.hProcess, -1));
				cout << "Failed sending stats" << endl;
			}
 
			string sNewName("archive/");
			sNewName += finddata.cFileName;
			rename("temp.stats", sNewName.c_str());

			DWORD dwExitCode(0);
			bool bGotCode(GetExitCodeProcess(procInfo.hProcess, &dwExitCode) == TRUE);
			if (!bGotCode || (bGotCode && dwExitCode != 0))
			{
				if (iAttempts > 10)
				{
					iAttempts = 0;
					sDeleteMe = finddata.cFileName;
					break;
				}
				continue;
			}

			iAttempts = 0;
			sDeleteMe = finddata.cFileName;
			break;
		}
		while (FindNextFile(handle, &finddata));
		FindClose(handle);

		if (!sDeleteMe.empty())
		{
			string sNewName("archive/");
			sNewName += sDeleteMe;
			_unlink(sDeleteMe.c_str());
			sDeleteMe.clear();
		}
	}

	return 0;
}
