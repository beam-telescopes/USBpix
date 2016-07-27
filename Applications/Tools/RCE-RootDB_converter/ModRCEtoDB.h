#ifndef MODRCETODB_H
#define MODRCETODB_H

enum FEI4_flavour { FEI4A, FEI4B, invalid };

FEI4_flavour get_FEI4_flavour(std::string RCE_directory_name);

void fill_registers_with (ipc::PixelFEI4AConfig const & config);
void fill_registers_with (ipc::PixelFEI4BConfig const & config);

FEI4_flavour get_FEI4_flavour();

void start_setting_up_ROOT_database_for_FEI4_of_flavour (FEI4_flavour flavour, std::string ROOT_filename);

void read_RCE_config_files_into_ROOT_database (FEI4_flavour flavour, std::string RCE_directory_name);

void mount_new_module_of_ROOT_database (FEI4_flavour flavour);   // i don't actually know what a module is in this context. this function adds e.g. a "PixFe_1" and all its children when you've already got a "PixFe_0".

bool is_suffix_file    (std::string const & filename, std::string const & correct_suffix);
bool is_cfg_root_file  (std::string const & filename);
bool is_cfg_file       (std::string const & filename);
bool file_exists       (std::string const & filename);
bool directory_exists  (std::string const & directoryname);
bool contains_cfg_file (std::string const & directoryname);

int  bit_flipped    (unsigned short CLK);
std::vector <int>  make_vector_from_array           ( unsigned int const array[80][336] );
std::vector <bool> make_vector_from_array_using_bit ( unsigned int const array[80][336], long bit );

void set_up_RootDB_inquire_structure();
void set_up_RootDB_field_structure  (FEI4_flavour flavour);

std::set <std::string> entities_in_directory (std::string const & directory_name);

#endif // MODRCETODB_H
