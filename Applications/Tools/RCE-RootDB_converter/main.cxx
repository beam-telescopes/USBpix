#include <vector>

#include "FEI4BConfigFile.hh"
#include "FEI4AConfigFile.hh"

#include "PixConfDBInterface/RootDB.h"

# include <iostream>
# include <set>
# include <stdexcept>
# include <string>

#include <ModRCEtoDB.h>

void usage(){
  std::cout << "Usage: " << std::endl;
  std::cout << "RCE-RootDB_converter --RCEtoDB RCE-DIRECTORY ROOT-FILE" << std::endl;
  std::cout << "  RCE-DIRECTORY must exist and have subdirectories named \"configs\", \"masks\", \"tdacs\", and \"fdacs\"." << std::endl;
  std::cout << "  ROOT-FILE must have the extension \".cfg.root\" and must not already exist." << std::endl;
  std::cout << " -- OR -- " << std::endl;
  std::cout << "RCE-RootDB_converter --DBtoRCE ROOT-FILE RCE-FILE [RCE-DIRECTORY]" << std::endl;
  std::cout << "  ROOT-FILE must have the extension \".cfg.root\" and must exist." << std::endl;
}

bool process_command_line_arguments (int argc, char** argv, std::string &RCE_name, std::string &ROOT_name, std::string &dir_name, bool &RCEtoDB)
{
  RCE_name  = "";
  ROOT_name = "";
  dir_name = "rceconf";

  if (argc != 4 && argc!=5)
    {
      usage();
      return true;
    }
  
  if(std::string(argv[1])=="--RCEtoDB"){
    RCEtoDB=true;
    RCE_name  = argv[2];
    ROOT_name = argv[3];
    if ( argc==5 ) {usage(); return true;};
    if ( ! is_cfg_root_file (ROOT_name) ){std::cout << "RCE-RootDB_converter: error: ROOT config file suffix is not \".cfg.root\"."<<std::endl; return true;};
    if ( file_exists        (ROOT_name) ){std::cout << "RCE-RootDB_converter: error: ROOT file \"" + ROOT_name + "\" already exists."<<std::endl; return true;};
    
    if ( ! directory_exists  (RCE_name             ) ) {std::cout << "RCE-RootDB_converter: error: RCE config directory \"" + RCE_name + "\" does not exist."<<std::endl; return true;};
    if ( ! directory_exists  (RCE_name + "/configs") ) {std::cout << "RCE-RootDB_converter: error: RCE config directory \"" + RCE_name + "\" does not possess a \"configs\" subdirectory."<<std::endl; return true;};
    if ( ! directory_exists  (RCE_name + "/masks"  ) ) {std::cout << "RCE-RootDB_converter: error: RCE config directory \"" + RCE_name + "\" does not possess a \"masks\" subdirectory."<<std::endl; return true;};
    if ( ! directory_exists  (RCE_name + "/tdacs"  ) ) {std::cout << "RCE-RootDB_converter: error: RCE config directory \"" + RCE_name + "\" does not possess a \"tdacs\" subdirectory."<<std::endl; return true;};
    if ( ! directory_exists  (RCE_name + "/fdacs"  ) ) {std::cout << "RCE-RootDB_converter: error: RCE config directory \"" + RCE_name + "\" does not possess a \"fdacs\" subdirectory."<<std::endl; return true;};
    if ( ! contains_cfg_file (RCE_name + "/configs") ) {std::cout << "RCE-RootDB_converter: error: RCE config directory \"" + RCE_name + "\" does not possess any *.cfg files in its \"configs\" subdirectory."<<std::endl; return true;};
    return false;
  } else if(std::string(argv[1])=="--DBtoRCE"){
    RCEtoDB=false;
    RCE_name  = argv[3];
    ROOT_name = argv[2];
    if(argc==5) dir_name = argv[4];
    if ( ! is_cfg_root_file (ROOT_name) ){std::cout << "RCE-RootDB_converter: error: ROOT config file suffix is not \".cfg.root\"."<<std::endl; return true;};
    if ( ! file_exists      (ROOT_name) ){std::cout << "RCE-RootDB_converter: error: ROOT file \"" + ROOT_name + "\" does not exist."<<std::endl; return true;};
    return false;
  } else{
    usage();
    return true;
  }
}

void ModDBtoRCE(std::string ROOT_name, std::string RCE_name, std::string dir_name);

int main (int argc, char** argv) try
{
  std::string RCE_name;
  std::string ROOT_name;
  std::string dir_name;
  bool RCEtoDB=true;

  if(process_command_line_arguments (argc, argv, RCE_name, ROOT_name, dir_name, RCEtoDB)) return 0;
	
  if(RCEtoDB)
    {
      FEI4_flavour flavour = get_FEI4_flavour(RCE_name);
      
      start_setting_up_ROOT_database_for_FEI4_of_flavour (flavour, ROOT_name);
      
      read_RCE_config_files_into_ROOT_database (flavour, RCE_name);
    }
  else
    {
      ModDBtoRCE(ROOT_name, RCE_name, dir_name);
    }

  return 0;
}
catch (std::exception & e)            { std::cerr << e.what() << "\n";                    throw; }
catch (PixLib::PixDBException& e)     { std::cerr << e        << "\n"; e.what(std::cout); throw; }
catch (SctPixelRod::BaseException& e) { std::cerr << e        << "\n"; e.what(std::cout); throw; }
catch (...) { std::cerr << "main.exe: error: unexpected exception.\n";                    throw; }

