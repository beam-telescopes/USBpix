
// this class reads a file for key-value pairs of parameters.
// these parameters can then be used in an application.

// input file format details:
// a "#" denotes an end-of-line comment.
// key-value pairs appear on one line, separated by a ":".
// only one pair is allowed per line.
// all whitespace will be discarded unless inside double-quotes.
// all double-quotes and unquoted colons will be discarded.
// in order to use the empty string, type "<<__EMPTY__STRING__VALUE__>>".
// all keys and parameters are read off as std::string's.

// for reading in command-line arguments, it's mostly the same.
// make sure to lead all arguments with two dashes ("--").
// also, "#"s have no special meaning.
// only the first colon is discarded.

# ifndef external_input_h
# define external_input_h

# include <set>
# include <string>
# include <unordered_map>

namespace online
{ class external_input; }

class online::external_input
{
		public:
	
	// reads "filename" for key-value pairs to be accesssed later.
	// default version does nothing.
	external_input (std::string const & filename);
	external_input ();
	
	// access a parameter value via its key.
	// throws an error if that's not possible.
	std::string const & operator () (std::string const & key);
	
	// like (), but uses std::stod to convert the value to a number before returning.
	double operator [] (std::string const & key);
	
	// read in a vector of command-line arguments.
	// overwrites any previous values.
	void read (std::set <std::string> const & argv);
	
	// just like the constructor.
	// overwrites prior values.
	void read (std::string const & filename);
	
	// checks if the given key value is assigned a value.
	bool exists (std::string const & key) const;
	
		private:
	
	std::unordered_map <std::string, std::string> data;
	
	static std::string const empty_string_code;
};

# endif

