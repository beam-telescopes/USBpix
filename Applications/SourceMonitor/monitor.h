
// read "README.txt".

// QRootCanvas is from <https://root.cern.ch/phpBB3/viewtopic.php?t=19524>, I didn't write it.

# ifndef monitor_h
# define monitor_h

# include "external_input.h"

# include <TCanvas.h>
# include <TFile.h>
# include <TH1D.h>
# include <TH2D.h>

# include <QWidget>

# include <array>
# include <chrono>
# include <fstream>
# include <map>
# include <memory>
# include <set>
# include <string>
# include <vector>

class QFormLayout;
class QLineEdit;
class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QPushButton;
class QTabWidget;
class QTimer;
class TCanvas;

class QRootCanvas : public QWidget
{
   Q_OBJECT

public:
   QRootCanvas( QWidget *parent = 0);
   virtual ~QRootCanvas() {}
   TCanvas* getCanvas() { return fCanvas;}

protected:
   TCanvas        *fCanvas;

   virtual void    mouseMoveEvent( QMouseEvent *e );
   virtual void    mousePressEvent( QMouseEvent *e );
   virtual void    mouseReleaseEvent( QMouseEvent *e );
   virtual void    paintEvent( QPaintEvent *e );
   virtual void    resizeEvent( QResizeEvent *e );
};

//---------------------------------------------------------------

namespace online
{ class monitor; }

class online::monitor : public QWidget
{ Q_OBJECT
		public:
	
	// does absolutely everything on initialization.
	// "argv" are the command-line parameters of the program using this.
	// "parametersfile" is the name of the file this gets its parameters from.
	monitor ( std::set <std::string> const & argv = std::set <std::string> () );
	
		private:
	
	// basic: correspond directly to figures of merit.
	// time : the x-axis changes in time.
	// map  : 2D, displaying the sensor surface.
	std::vector <std::string> all_histograms;
	std::set <std::string>  basic_histograms;
	std::set <std::string>   time_histograms;
	std::set <std::string>    map_histograms;
	
	std::map < std::string, std::vector <int> > raw_event_data;
	
	std::map <std::string, TH1D> histograms1D;
	std::map <std::string, TH2D> histograms2D;
	
	// (col,row)->(timestamp)
	std::map < std::pair<int, int>, int > bad_pixel_list;
	
	external_input p;
	
	std::ifstream           reader;
	std::unique_ptr <TFile> writer;
	std::ofstream           bad_pixel_file;
	
	std::vector<std::string>::iterator histogram_cycler;
	
	// "wait_counter" counts waiting cycles (for error detection),
	// "time_to_udate_histograms" keeps track of the histogram writing timing,
	// "display_helper" lets the hint-to-help-text show,
	// "line" is fed to std::getline in the main loop.
	unsigned int    wait_counter;
	bool            display_helper;
	std::string     line;
	
	int lvl1;
	int lvl2;
	
	std::chrono::microseconds time_spent_waiting;
	std::chrono::microseconds time_spent_writing;
	std::chrono::microseconds time_spent_reading;
	std::chrono::system_clock::time_point start_time;
	std::chrono::system_clock::time_point   end_time;
	
	bool get_line (std::ifstream & reader, std::string & line);
	
	void read_in_parameters ();
	
	bool is_not_raw_or_txt_file (std::string const & filename);
	bool is_not_root_file       (std::string const & filename);
	bool is_not_txt_file        (std::string const & filename);
	
	void define_histogram_filters ();
	
	void initialize_histograms_and_data_containers ();
	
	void initialize_graphics_utilities ();
	
	bool open_file_streams ();
	
	void prepare_for_looping ();
	
	void process_input_line ();
	
	void handle_as_external_trigger (int data);
	
	void skip_next_line ();
	
	void handle_as_data_header (int data);
	
	void handle_as_data_record (int data);
	
	bool is_bad_pixel (int col, int row);
	
	void patiently_bide_time ();
	
	void close_shop ();
	
	void show_time_diagnostics ();
	
	void initialize_histograms_page ();
	
	void initialize_parameters_page ();
	
	void start_timers ();
	
	void connect_buttons ();
	
	void empty_histograms_and_data_containers ();
	
		public slots:
	
	void handle_root_events();
	
	void cycle_to_next_histogram ();
	
	void update_histograms ();
	
	void open_shop();
	
	void reopen_shop ();
	
	void draw_LVL1();
	void draw_LVL2();
	void draw_col ();
	void draw_row ();
	void draw_ToT1();
	void draw_ToT2();
	void draw_total_hits();
	void draw_mean_ToT  ();
	void draw_hitmap ();
	void draw_ToT_map();
	void draw_bad_pixel_map();
	
		public:
	
	virtual ~monitor();
	virtual void changeEvent(QEvent * e);
	
		protected:
	
	QTimer * fRootTimer;
	QTimer * time_to_update_histograms;
	
	QTabWidget * main_tabs;
	
	QRootCanvas * canvas;
	QPushButton * next_histogram_button;
	QPushButton * start_button;
	QPushButton * reset_button;
	
	QPushButton * LVL1_button;
	QPushButton * LVL2_button;
	QPushButton *  col_button;
	QPushButton *  row_button;
	QPushButton * ToT1_button;
	QPushButton * ToT2_button;
	QPushButton * total_hits_button;
	QPushButton * mean_ToT_button;
	QPushButton * hitmap_button;
	QPushButton * ToT_map_button;
	QPushButton * bad_pixel_map_button;
	
	QFormLayout * parameter_entry_layout;
	std::vector <std::string> parameters;
	std::vector <std::string> defaults;
	std::vector <QLineEdit* > entry_fields;
	
	std::set <std::string> argv;
	
	bool started;
};

# endif

