# include <set>
# include <stdexcept>
# include <string>
#ifndef WIN32
#include <dirent.h>
#else
#include <windows.h>
#endif


std::set <std::string> entities_in_directory (std::string const & directory_name)
{
	auto files = std::set <std::string> ();

#ifdef WIN32
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	TCHAR dname[1000];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, dname, 1000, (directory_name+"/*").c_str(), _TRUNCATE);
	if((hFind = FindFirstFile(dname, &FindFileData)) != INVALID_HANDLE_VALUE) throw std::runtime_error("error: couldn't open directory \"" + directory_name + "\".");
	do{
	  char fname[400];
	  wcstombs_s(&convertedChars, fname, 400, FindFileData.cFileName, _TRUNCATE);
	  files.insert(fname);
	}while(FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
#else
	auto directory = opendir( directory_name.c_str() );
	
	if (directory == NULL) throw std::runtime_error("error: couldn't open directory \"" + directory_name + "\".");
	
	auto file  = readdir(directory);
	
	while (file != NULL)
	{
		files.insert( file->d_name );
		
		file = readdir(directory);
	}
	
	closedir(directory);
#endif	
	return files;
}

