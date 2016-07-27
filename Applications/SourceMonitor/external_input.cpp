
# include "external_input.h"

# include <fstream>
# include <stdexcept>

using namespace online;

external_input::external_input
(std::string const & filename)
{ read(filename); }

external_input::external_input () {}

std::string const & external_input::operator ()
(std::string const & key)
{
	try { return data.at(key); }
	
	catch (std::out_of_range & error)
	{
		throw std::out_of_range ( std::string( error.what() ) + " from online::external_input::operator(), no element \"" + key + "\" exists." );
	}
}

double external_input::operator []
(std::string const & key)
{ return std::stod( operator () (key) ); }

void external_input::read
(std::set <std::string> const & argv)
{
	for (auto arg = argv.begin(); arg != argv.end(); ++arg)
	{
		if ( arg->empty() ) continue;
		
		if ( arg->substr(0,2) != "--" ) continue;
		
		std::string key;
		std::string value;
		
		bool colon = false;
		for (auto i = arg->begin()+2; i != arg->end(); ++i)
		{
			if ( !colon && *i == ':' ) { colon = true; continue; }
			
			if (!colon) key.push_back(*i);
			else      value.push_back(*i);
		}
		
		if ( key.empty() || value.empty() ) continue;
		
		if ( data.count(key) == 0 ) data.insert ( std::make_pair (key, value) );
		else data.at(key) = value;
	}
}

void external_input::read
(std::string const & filename)
{
	std::ifstream reader (filename);
	
	if ( ! reader.is_open() ) throw std::runtime_error ("ERROR: file \"" + filename + "\" cannot be opened for reading.");
	
	std::string line;
	while ( std::getline(reader, line) )
	{
		if ( line.empty() ) continue;
		
		std::string key;
		std::string value;
		
		// trim whitespace and comments, except in quotes.
		// split the key/value parts at a ":" character.
		bool quoted = false;
		bool colon  = false;
		for (auto i = line.begin(); i != line.end(); ++i)
		{
			while ( !quoted && ( *i == ' ' || *i == '\t' ) ) line.erase(i);
			
			if ( !quoted && *i == '#' ) break;
			
			if ( *i == '"' ) { quoted = !quoted; continue; }
			
			if ( !quoted && *i == ':' ) { colon  = true; continue; }
			
			if (!colon) key.push_back(*i);
			else      value.push_back(*i);
		}
		
		if ( key.empty() || value.empty() ) continue;
		
		if ( value == empty_string_code ) value = "";
		if ( key   == empty_string_code ) key   = "";
		
		if ( data.count(key) == 0 ) data.insert ( std::make_pair (key, value) );
		else data.at(key) = value;
	}
	
	reader.close();
}

bool external_input::exists
(std::string const & key) const
{ return data.count(key) != 0; }

std::string const external_input::empty_string_code = "<<__EMPTY__STRING__VALUE__>>";

