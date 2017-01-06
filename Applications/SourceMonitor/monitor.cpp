#include <QPushButton>
#include <QLayout>
#include <QTimer>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTextEdit>
#include <QTabWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QToolTip>

#include <stdlib.h>

#include <TCanvas.h>
#include <TVirtualX.h>
#include <TSystem.h>
#include <TFormula.h>
#include <TF1.h>
#include <TH1.h>
#include <TFrame.h>
#include <TTimer.h>

#ifndef _GLIBCXX_USE_NANOSLEEP
#define _GLIBCXX_USE_NANOSLEEP
#endif

#include "monitor.h"

#include <algorithm>
#include <iostream>
#include <thread>

//------------------------------------------------------------------------------

QRootCanvas::QRootCanvas(QWidget *parent) : QWidget(parent, 0), fCanvas(0)
{
   // QRootCanvas constructor.

   // set options needed to properly update the canvas when resizing the widget
   // and to properly handle context menus and mouse move events
   setAttribute(Qt::WA_PaintOnScreen, true);
   setAttribute(Qt::WA_OpaquePaintEvent, true);
   setMinimumSize(300, 200);
   setUpdatesEnabled(kFALSE);
   setMouseTracking(kTRUE);

   // register the QWidget in TVirtualX, giving its native window id
   int wid = gVirtualX->AddWindow((ULong_t)winId(), 600, 400);
   // create the ROOT TCanvas, giving as argument the QWidget registered id
   fCanvas = new TCanvas("Root Canvas", width(), height(), wid);
}

void QRootCanvas::mouseMoveEvent(QMouseEvent *e)
{
   // Handle mouse move events.

   if (fCanvas) {
      if (e->buttons() & Qt::LeftButton) {
         fCanvas->HandleInput(kButton1Motion, e->x(), e->y());
      } else if (e->buttons() & Qt::MidButton) {
         fCanvas->HandleInput(kButton2Motion, e->x(), e->y());
      } else if (e->buttons() & Qt::RightButton) {
         fCanvas->HandleInput(kButton3Motion, e->x(), e->y());
      } else {
         fCanvas->HandleInput(kMouseMotion, e->x(), e->y());
      }
   }
}

void QRootCanvas::mousePressEvent( QMouseEvent *e )
{
   // Handle mouse button press events.

   if (fCanvas) {
      switch (e->button()) {
         case Qt::LeftButton :
            fCanvas->HandleInput(kButton1Down, e->x(), e->y());
            break;
         case Qt::MidButton :
            fCanvas->HandleInput(kButton2Down, e->x(), e->y());
            break;
         case Qt::RightButton :
            //setAttribute(Qt::WA_PaintOnScreen, true);//////////////////
            fCanvas->HandleInput(kButton3Down, e->x(), e->y());
            break;
         default:
            break;
      }
   }
}

void QRootCanvas::mouseReleaseEvent( QMouseEvent *e )
{
   // Handle mouse button release events.

   if (fCanvas) {
      switch (e->button()) {
         case Qt::LeftButton :
            fCanvas->HandleInput(kButton1Up, e->x(), e->y());
            break;
         case Qt::MidButton :
            fCanvas->HandleInput(kButton2Up, e->x(), e->y());
            break;
         case Qt::RightButton :
            //setAttribute(Qt::WA_PaintOnScreen, true);///////////
            fCanvas->HandleInput(kButton3Up, e->x(), e->y());
            break;
         default:
            break;
      }
   }
}

void QRootCanvas::resizeEvent( QResizeEvent * )
{
   // Handle resize events.

   if (fCanvas) {
      fCanvas->Resize();
      fCanvas->Update();
   }
}

void QRootCanvas::paintEvent( QPaintEvent * )
{
   // Handle paint events.

   if (fCanvas) {
      fCanvas->Resize();
      fCanvas->Update();
   }
}

//------------------------------------------------------------------------------

void online::monitor::handle_root_events()
{ gSystem->ProcessEvents(); }

void online::monitor::changeEvent(QEvent * e)
{
   if (e->type() == QEvent ::WindowStateChange) {
      QWindowStateChangeEvent * event = static_cast< QWindowStateChangeEvent * >( e );
      if (( event->oldState() & Qt::WindowMaximized ) ||
          ( event->oldState() & Qt::WindowMinimized ) ||
          ( event->oldState() == Qt::WindowNoState && 
            this->windowState() == Qt::WindowMaximized )) {
         if (canvas->getCanvas()) {
            canvas->getCanvas()->Resize();
            canvas->getCanvas()->Update();
         }
      }
   }
}

using namespace online;

monitor::monitor
(std::set <std::string> const & argv)
: QWidget(0), argv(argv), started(false)
{
	QVBoxLayout * main_layout = new QVBoxLayout(this);
	main_layout->addWidget( main_tabs = new QTabWidget(this) );
	
	initialize_histograms_page();
	
	initialize_parameters_page();
}

void monitor::initialize_histograms_page()
{
	QWidget * histogram_page = new QWidget;
	QHBoxLayout * histogram_page_layout = new QHBoxLayout(histogram_page);
	
	QVBoxLayout * histogram_page_layout_left = new QVBoxLayout;
	histogram_page_layout_left->addWidget( LVL1_button = new QPushButton("LVL1") );
	histogram_page_layout_left->addWidget( LVL2_button = new QPushButton("BCID") );
	histogram_page_layout_left->addWidget(  col_button = new QPushButton("col" ) );
	histogram_page_layout_left->addWidget(  row_button = new QPushButton("row" ) );
	histogram_page_layout_left->addWidget( ToT1_button = new QPushButton("ToT1") );
	histogram_page_layout_left->addWidget( ToT2_button = new QPushButton("ToT2") );
	histogram_page_layout_left->addWidget( total_hits_button    = new QPushButton("total hits") );
	histogram_page_layout_left->addWidget( mean_ToT_button      = new QPushButton("mean ToT") );
	histogram_page_layout_left->addWidget( hitmap_button        = new QPushButton("hitmap") );
	histogram_page_layout_left->addWidget( ToT_map_button       = new QPushButton("ToT map") );
	histogram_page_layout_left->addWidget( bad_pixel_map_button = new QPushButton("bad pixel map") );
	
	QVBoxLayout * histogram_page_layout_right = new QVBoxLayout;
	histogram_page_layout_right->addWidget( canvas = new QRootCanvas(histogram_page) );
	QHBoxLayout * histogram_page_layout_right_buttons = new QHBoxLayout;
	histogram_page_layout_right_buttons->addWidget( next_histogram_button = new QPushButton ("next",  histogram_page) );
	histogram_page_layout_right_buttons->addWidget( reset_button          = new QPushButton ("reset", histogram_page) );
	histogram_page_layout_right_buttons->addWidget( start_button          = new QPushButton ("start", histogram_page) );
	histogram_page_layout_right->addLayout(histogram_page_layout_right_buttons);
	
	histogram_page_layout->addLayout(histogram_page_layout_left );
	histogram_page_layout->addLayout(histogram_page_layout_right);
	histogram_page_layout->setStretchFactor(histogram_page_layout_right, 3);
	
	main_tabs->addTab(histogram_page, "histograms");
	
	QObject::connect( next_histogram_button, SIGNAL( clicked() ), this, SLOT( cycle_to_next_histogram() ) );
	QObject::connect( start_button,          SIGNAL( clicked() ), this, SLOT( open_shop() )               );
	QObject::connect( reset_button,          SIGNAL( clicked() ), this, SLOT( reopen_shop() )             );
}

void monitor::initialize_parameters_page()
{
	parameters.push_back( "input"  );
	parameters.push_back( "output" );
	parameters.push_back( "bad pixel file" );
	parameters.push_back( "writing period" );
	parameters.push_back( "max hits per pixel intercept" );
	parameters.push_back( "max hits per pixel slope"     );
	parameters.push_back( "LVL1 name"     );
	parameters.push_back( "LVL1 title"    );
	parameters.push_back( "LVL1 low bin"  );
	parameters.push_back( "LVL1 high bin" );
	parameters.push_back( "LVL1 suppress" );
	parameters.push_back( "BCID name"     );
	parameters.push_back( "BCID title"    );
	parameters.push_back( "BCID suppress" );
	parameters.push_back( "col  name"     );
	parameters.push_back( "col  title"    );
	parameters.push_back( "col  low bin"  );
	parameters.push_back( "col  high bin" );
	parameters.push_back( "col  suppress" );
	parameters.push_back( "row  name"     );
	parameters.push_back( "row  title"    );
	parameters.push_back( "row  low bin"  );
	parameters.push_back( "row  high bin" );
	parameters.push_back( "row  suppress" );
	parameters.push_back( "ToT1 name"     );
	parameters.push_back( "ToT1 title"    );
	parameters.push_back( "ToT1 low bin"  );
	parameters.push_back( "ToT1 high bin" );
	parameters.push_back( "ToT1 suppress" );
	parameters.push_back( "ToT2 name"     );
	parameters.push_back( "ToT2 title"    );
	parameters.push_back( "ToT2 low bin"  );
	parameters.push_back( "ToT2 high bin" );
	parameters.push_back( "ToT2 suppress" );
	parameters.push_back( "total hits name"     );
	parameters.push_back( "total hits title"    );
	parameters.push_back( "total hits suppress" );
	parameters.push_back( "mean ToT name"     );
	parameters.push_back( "mean ToT title"    );
	parameters.push_back( "mean ToT suppress" );
	parameters.push_back( "hitmap name"     );
	parameters.push_back( "hitmap title"    );
	parameters.push_back( "hitmap suppress" );
	parameters.push_back( "ToT map name"     );
	parameters.push_back( "ToT map title"    );
	parameters.push_back( "ToT map suppress" );
	parameters.push_back( "bad pixel map name"     );
	parameters.push_back( "bad pixel map title"    );
	parameters.push_back( "bad pixel map suppress" );
	parameters.push_back( "TH1 fill color" );
	parameters.push_back( "TH1 fill style" );
	parameters.push_back( "TH2 fill options" );
	parameters.push_back( "patient   latency" );
	parameters.push_back( "impatient latency" );
	parameters.push_back( "show time diagnostics" );
	
	defaults.push_back( "INPUT_FILE_HERE" );
	defaults.push_back( "online_monitor_histograms.root" );
	defaults.push_back( "list_of_bad_pixels.txt" );
	defaults.push_back( "1000" );
	defaults.push_back( "2147483647" );
	defaults.push_back( "0" );
	defaults.push_back( "LVL1" );
	defaults.push_back( "LVL1" );
	defaults.push_back( "1" );
	defaults.push_back( "16" );
	defaults.push_back( "0" );
	defaults.push_back( "BCID" );
	defaults.push_back( "BCID" );
	defaults.push_back( "0" );
	defaults.push_back( "col" );
	defaults.push_back( "col" );
	defaults.push_back( "0" );
	defaults.push_back( "79" );
	defaults.push_back( "0" );
	defaults.push_back( "row" );
	defaults.push_back( "row" );
	defaults.push_back( "0" );
	defaults.push_back( "336" );
	defaults.push_back( "0" );
	defaults.push_back( "ToT1" );
	defaults.push_back( "ToT1" );
	defaults.push_back( "0" );
	defaults.push_back( "15" );
	defaults.push_back( "0" );
	defaults.push_back( "ToT2" );
	defaults.push_back( "ToT2" );
	defaults.push_back( "0" );
	defaults.push_back( "15" );
	defaults.push_back( "1" );
	defaults.push_back( "hits_vs_time" );
	defaults.push_back( "hits vs. time" );
	defaults.push_back( "0" );
	defaults.push_back( "ToT_vs_time" );
	defaults.push_back( "ToT vs. time" );
	defaults.push_back( "0" );
	defaults.push_back( "hitmap" );
	defaults.push_back( "hitmap" );
	defaults.push_back( "0" );
	defaults.push_back( "ToT_map" );
	defaults.push_back( "ToT_map" );
	defaults.push_back( "0" );
	defaults.push_back( "bad_pixel_map" );
	defaults.push_back( "bad pixel map" );
	defaults.push_back( "0" );
	defaults.push_back( "600" );
	defaults.push_back( "3003" );
	defaults.push_back( "colz" );
	defaults.push_back( "100" );
	defaults.push_back( "100" );
	defaults.push_back( "0" );
	
	p.read(argv);
	
	for (unsigned int i = 0; i < parameters.size(); ++i)
	{
		QLineEdit * lineedit;
		if ( p.exists( parameters.at(i) ) ) lineedit = new QLineEdit( p( parameters.at(i) ).c_str() );
		else                                lineedit = new QLineEdit(        defaults.at(i).c_str() );
		lineedit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
		
		     if ( parameters.at(i) == "input"  ) lineedit->setToolTip("the source-scan file which this application will read in real time.");
		else if ( parameters.at(i) == "output" ) lineedit->setToolTip("the *.root file which the histograms displayed by this application will periodically be saved to.");
		else if ( parameters.at(i) == "bad pixel file" ) lineedit->setToolTip("a *.txt output file which logs information about any leaky or otherwise faulty pixels detected in the course of operation of this application.");
		else if ( parameters.at(i) == "writing period" ) lineedit->setToolTip("the time, in milliseconds, between updates to the histogram display, and between writes of the histograms to their files.");
		else if ( parameters.at(i) == "max hits per pixel intercept" ) lineedit->setToolTip("pixels are marked as \"bad\" if they acrue more than \"a * BCID + b\" hits; this parameters specifies \"b\".");
		else if ( parameters.at(i) == "max hits per pixel slope"     ) lineedit->setToolTip("pixels are marked as \"bad\" if they acrue more than \"a * BCID + b\" hits; this parameters specifies \"a\".");
		else if ( parameters.at(i) == "LVL1 name"     ) lineedit->setToolTip("the filename of the LVL1 histogram.");
		else if ( parameters.at(i) == "LVL1 title"    ) lineedit->setToolTip("the title of the LVL1 histogram.");
		else if ( parameters.at(i) == "LVL1 low bin"  ) lineedit->setToolTip("the lowest value that LVL1 can take.");
		else if ( parameters.at(i) == "LVL1 high bin" ) lineedit->setToolTip("the highest value that LVL1 can take.");
		else if ( parameters.at(i) == "LVL1 suppress" ) lineedit->setToolTip("if \"1\", does not save LVL1 histogram to the output file.");
		else if ( parameters.at(i) == "BCID name"     ) lineedit->setToolTip("the filename of the bunch-crossing ID histogram.");
		else if ( parameters.at(i) == "BCID title"    ) lineedit->setToolTip("the title of the bunch-crossing ID histogram.");
		else if ( parameters.at(i) == "BCID suppress" ) lineedit->setToolTip("if \"1\", does not save the bunch-crossing ID histogram to the output file.");
		else if ( parameters.at(i) == "col  name"     ) lineedit->setToolTip("the filename of the hits-per-column histogram.");
		else if ( parameters.at(i) == "col  title"    ) lineedit->setToolTip("the title of the hits-per-column histogram.");
		else if ( parameters.at(i) == "col  low bin"  ) lineedit->setToolTip("the lowest column index possible.");
		else if ( parameters.at(i) == "col  high bin" ) lineedit->setToolTip("the highest column index possible.");
		else if ( parameters.at(i) == "col  suppress" ) lineedit->setToolTip("if \"1\", does not save the hits-per-column histogram to the output file.");
		else if ( parameters.at(i) == "row  name"     ) lineedit->setToolTip("the filename of the hits-per-row histogram.");
		else if ( parameters.at(i) == "row  title"    ) lineedit->setToolTip("the title of the hits-per-row histogram.");
		else if ( parameters.at(i) == "row  low bin"  ) lineedit->setToolTip("the lowest row index possible.");
		else if ( parameters.at(i) == "row  high bin" ) lineedit->setToolTip("the highest row index possible.");
		else if ( parameters.at(i) == "row  suppress" ) lineedit->setToolTip("if \"1\", does not save the hits-per-row histogram to the output file.");
		else if ( parameters.at(i) == "ToT1 name"     ) lineedit->setToolTip("the filename of the ToT1 histogram.");
		else if ( parameters.at(i) == "ToT1 title"    ) lineedit->setToolTip("the title of the ToT1 histogram.");
		else if ( parameters.at(i) == "ToT1 low bin"  ) lineedit->setToolTip("the lowest value that ToT1 can take.");
		else if ( parameters.at(i) == "ToT1 high bin" ) lineedit->setToolTip("the highest value that ToT1 can take.");
		else if ( parameters.at(i) == "ToT1 suppress" ) lineedit->setToolTip("if \"1\", does not save ToT1 histogram to the output file.");
		else if ( parameters.at(i) == "ToT2 name"     ) lineedit->setToolTip("the filename of the ToT2 histogram.");
		else if ( parameters.at(i) == "ToT2 title"    ) lineedit->setToolTip("the title of the ToT2 histogram.");
		else if ( parameters.at(i) == "ToT2 low bin"  ) lineedit->setToolTip("the lowest value that ToT2 can take.");
		else if ( parameters.at(i) == "ToT2 high bin" ) lineedit->setToolTip("the highest value that ToT2 can take.");
		else if ( parameters.at(i) == "ToT2 suppress" ) lineedit->setToolTip("if \"1\", does not save ToT2 histogram to the output file.");
		else if ( parameters.at(i) == "total hits name"     ) lineedit->setToolTip("the filename of the hits-integrated-over-time histogram.");
		else if ( parameters.at(i) == "total hits title"    ) lineedit->setToolTip("the title of the hits-integrated-over-time histogram.");
		else if ( parameters.at(i) == "total hits suppress" ) lineedit->setToolTip("if \"1\", does not save hits-integrated-over-time histogram to the output file.");
		else if ( parameters.at(i) == "mean ToT name"     ) lineedit->setToolTip("the filename of the average-ToT-over-time histogram.");
		else if ( parameters.at(i) == "mean ToT title"    ) lineedit->setToolTip("the title of the average-ToT-over-time histogram.");
		else if ( parameters.at(i) == "mean ToT suppress" ) lineedit->setToolTip("if \"1\", does not save average-ToT-over-time histogram to the output file.");
		else if ( parameters.at(i) == "hitmap name"     ) lineedit->setToolTip("the filename of the hits-per-pixel histogram.");
		else if ( parameters.at(i) == "hitmap title"    ) lineedit->setToolTip("the title of the hits-per-pixel histogram.");
		else if ( parameters.at(i) == "hitmap suppress" ) lineedit->setToolTip("if \"1\", does not save hits-per-pixel histogram to the output file.");
		else if ( parameters.at(i) == "ToT map name"     ) lineedit->setToolTip("the filename of the average-ToT-per-pixel histogram.");
		else if ( parameters.at(i) == "ToT map title"    ) lineedit->setToolTip("the title of the average-ToT-per-pixel histogram.");
		else if ( parameters.at(i) == "ToT map suppress" ) lineedit->setToolTip("if \"1\", does not save average-ToT-per-pixel histogram to the output file.");
		else if ( parameters.at(i) == "bad pixel map name"     ) lineedit->setToolTip("the filename of the bad pixel display histogram.");
		else if ( parameters.at(i) == "bad pixel map title"    ) lineedit->setToolTip("the title of the bad pixel display histogram.");
		else if ( parameters.at(i) == "bad pixel map suppress" ) lineedit->setToolTip("if \"1\", does not save bad pixel display histogram to the output file.");
		else if ( parameters.at(i) == "TH1 fill color" ) lineedit->setToolTip("the numeric equivalent of the ROOT fill color for the TH1's.");
		else if ( parameters.at(i) == "TH1 fill style" ) lineedit->setToolTip("the numeric equivalent of the ROOT fill style for the TH1's.");
		else if ( parameters.at(i) == "TH2 fill options" ) lineedit->setToolTip("the string that specifies ROOT TH2 style.");
		else if ( parameters.at(i) == "patient   latency" ) lineedit->setToolTip("how long, in milliseconds, to wait before checking again if you don't recieve input.");
		else if ( parameters.at(i) == "impatient latency" ) lineedit->setToolTip("how long, in patient cycles, to wait before printing an impatient message to the user about how there's no input coming in.");
		else if ( parameters.at(i) == "show time diagnostics" ) lineedit->setToolTip("if \"1\", prints some time-usage information to the console on application termination.");
		
		entry_fields.push_back( lineedit );
	}
	
	QWidget * parameters_page = new QWidget(main_tabs);
	parameter_entry_layout    = new QFormLayout(parameters_page);
	parameter_entry_layout->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
	for (unsigned int i = 0; i < parameters.size(); ++i) parameter_entry_layout->addRow( parameters.at(i).c_str(), entry_fields.at(i) );
	
	for (auto i = parameters.begin(); i != parameters.end(); ++i)
	for (auto j = i->begin(); j != i->end(); ++j)
	while (*j == ' ' || *j == '\t') i->erase(j);
	
	QScrollArea * scroller = new QScrollArea;
	scroller->setWidget(parameters_page);
	scroller->setWidgetResizable(true);
	
	main_tabs->addTab(scroller, "parameters");
}

void monitor::open_shop()
{
	if (started) return;
	
	start_timers ();
	
	connect_buttons ();
	
	read_in_parameters ();
	
	define_histogram_filters ();
	
	initialize_histograms_and_data_containers ();
	
	initialize_graphics_utilities ();
	
	if(open_file_streams ()){
	  started = false;
	  return;
	}
	
	prepare_for_looping ();
	
	started = true;
}

void monitor::start_timers()
{
	fRootTimer = new QTimer( this );
	QObject::connect( fRootTimer, SIGNAL(timeout()), this, SLOT(handle_root_events()) );
	fRootTimer->start( 20 );
	time_to_update_histograms = new QTimer(this);
	QObject::connect( time_to_update_histograms, SIGNAL( timeout() ), this, SLOT( update_histograms() ) );
}

void monitor::connect_buttons()
{
	QObject::connect( LVL1_button, SIGNAL( clicked() ), this, SLOT(draw_LVL1()) );
	QObject::connect( LVL2_button, SIGNAL( clicked() ), this, SLOT(draw_LVL2()) );
	QObject::connect(  col_button, SIGNAL( clicked() ), this, SLOT(draw_col() ) );
	QObject::connect(  row_button, SIGNAL( clicked() ), this, SLOT(draw_row() ) );
	QObject::connect( ToT1_button, SIGNAL( clicked() ), this, SLOT(draw_ToT1()) );
	QObject::connect( ToT2_button, SIGNAL( clicked() ), this, SLOT(draw_ToT2()) );
	QObject::connect( total_hits_button,    SIGNAL( clicked() ), this, SLOT(draw_total_hits()   ) );
	QObject::connect( mean_ToT_button,      SIGNAL( clicked() ), this, SLOT(draw_mean_ToT()     ) );
	QObject::connect( hitmap_button,        SIGNAL( clicked() ), this, SLOT(draw_hitmap()       ) );
	QObject::connect( ToT_map_button,       SIGNAL( clicked() ), this, SLOT(draw_ToT_map()      ) );
	QObject::connect( bad_pixel_map_button, SIGNAL( clicked() ), this, SLOT(draw_bad_pixel_map()) );
}

void monitor::reopen_shop()
{
	if (!started) return;
	
	started = false;
	
	time_to_update_histograms->stop();
	
	read_in_parameters ();
	
	empty_histograms_and_data_containers ();
	
	reader.close();
	writer->Close();
	bad_pixel_file.close();
	
	open_file_streams();
	
	prepare_for_looping ();
	
	started = true;
}

void monitor::empty_histograms_and_data_containers()
{
	for (auto i = all_histograms.begin(); i != all_histograms.end(); ++i)
	if ( map_histograms.count(*i) != 0 ) histograms2D.at(*i).Reset();
	else                                 histograms1D.at(*i).Reset();
	
	for (auto i = raw_event_data.begin(); i != raw_event_data.end(); ++i) i->second.clear();
	
	bad_pixel_list.clear();
}

monitor::~monitor()
{ close_shop(); }

bool monitor::get_line (std::ifstream & reader, std::string & line)
{ return std::getline (reader, line).good(); }

void monitor::read_in_parameters()
{
	std::set <std::string> gui_argv;
	
	for (unsigned int i = 0; i < parameters.size(); ++i)
	{
		auto const & key   = parameters.at(i);
		auto const & value = entry_fields.at(i)->text().toStdString();
		
		gui_argv.insert ( "--" + key + ":" + value );
	}
	
	p.read(gui_argv);
	
	if ( p("input") == "INPUT_FILE_HERE" ) throw std::invalid_argument ("ERROR: you need to set the \"input\" parameter.");
	
	if ( is_not_raw_or_txt_file ( p("input" ) ) ) throw std::invalid_argument ("ERROR: \"input\" is neither a *.raw nor a *.txt file.");
	
	if ( is_not_root_file       ( p("output") ) ) throw std::invalid_argument ("ERROR: \"output\" is not a *.root file.");
	
	if ( is_not_txt_file  ( p("badpixelfile") ) ) throw std::invalid_argument ("ERROR: \"bad pixel file\" is not a *.txt file.");
}

bool monitor::is_not_raw_or_txt_file
(std::string const & filename)
{
	if ( filename.size() < 4 ) return true;
	
	std::string const extension ( filename.substr( filename.size() - 4, 4 ) );
	if (  extension != ".raw" && extension != ".txt" ) return true;
	
	return false;
}

bool monitor::is_not_root_file
(std::string const & filename)
{
	if ( filename.size() < 5 ) return true;
	
	std::string const extension ( filename.substr( filename.size() - 5, 5 ) );
	if (  extension != ".root" ) return true;
	
	return false;
}

bool monitor::is_not_txt_file
(std::string const & filename)
{
	if ( filename.size() < 4 ) return true;
	
	std::string const extension ( filename.substr( filename.size() - 4, 4 ) );
	if ( extension != ".txt" ) return true;
	
	return false;
}

void monitor::define_histogram_filters ()
{
	all_histograms.push_back("hitmap");
	all_histograms.push_back("ToTmap");
	all_histograms.push_back("ToT1");
	all_histograms.push_back("ToT2");
	all_histograms.push_back("meanToT");
	all_histograms.push_back("LVL1");
	all_histograms.push_back("BCID");
	all_histograms.push_back("col" );
	all_histograms.push_back("row" );
	all_histograms.push_back("totalhits");
	all_histograms.push_back("badpixelmap");
	
	basic_histograms.insert("LVL1");
	basic_histograms.insert("BCID");
	basic_histograms.insert("col" );
	basic_histograms.insert("row" );
	basic_histograms.insert("ToT1");
	basic_histograms.insert("ToT2");
	
	time_histograms.insert("BCID");
	time_histograms.insert("totalhits");
	time_histograms.insert("meanToT");
	
	map_histograms.insert("hitmap");
	map_histograms.insert("ToTmap");
	map_histograms.insert("badpixelmap");
}

void monitor::initialize_histograms_and_data_containers ()
{
	for (auto name = basic_histograms.begin(); name != basic_histograms.end(); ++name)
	raw_event_data.insert ( std::make_pair ( *name, std::vector <int> () ) );
	
	for (auto i = all_histograms.begin(); i != all_histograms.end(); ++i)
	{
		const char* name  = p( *i + "name"  ).c_str();
		const char* title = p( *i + "title" ).c_str();
		
		if ( map_histograms.count(*i) != 0 )
		{
			TH2D histogram
			(
				name, title,
				p["colhighbin"] - p["collowbin"] + 1,
				p["collowbin" ],
				p["colhighbin"] + 1,
				p["rowhighbin"] - p["rowlowbin"] + 1,
				p["rowlowbin" ],
				p["rowhighbin"] + 1
			);
			
			histogram.SetOption( p("TH2filloptions").c_str() );
			histogram.SetStats(kFALSE);
			
			histograms2D.insert( std::make_pair( *i, histogram ) );
		}
		
		else
		{
			int low_bin, high_bin;
			
			if ( time_histograms.count(*i) != 0 ) low_bin = high_bin = 0;
			
			else
			{
				low_bin  = p[ *i + "lowbin"  ];
				high_bin = p[ *i + "highbin" ];
			}
			
			TH1D histogram
			(
				name, title,
				high_bin - low_bin + 1,
				low_bin, high_bin + 1
			);
			
			histogram.SetFillColor( p["TH1fillcolor"] );
			histogram.SetFillStyle( p["TH1fillstyle"] );
			histogram.SetStats    ( kFALSE            );
			
			histograms1D.insert( std::make_pair( *i, histogram ) );
		}
	}
}

void monitor::initialize_graphics_utilities ()
{
	histogram_cycler = all_histograms.begin();
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
}

bool monitor::open_file_streams ()
{
	reader.open( p("input") );
	if ( ! reader.is_open() ){
	  QString msg;
	  msg  = ("ERROR: couldn't open file \"" + p("input") + "\".\n").c_str();
	  msg += "Fix the problem and press start again.";
	  QMessageBox::critical(this, "no input file", msg);
	  return true;
	}
	
	writer = std::unique_ptr <TFile> ( new TFile( p("output").c_str(), "RECREATE" ) );
	if ( ! writer->IsOpen() ){
	  QString msg;
	  msg  = ("ERROR: couldn't open file \"" + p("output") + "\".\n").c_str();
	  msg += "Fix the problem and press start again.";
	  QMessageBox::critical(this, "can't open output file", msg);
	  return true;
	}
	
	bad_pixel_file.open( p("badpixelfile") );
	if ( ! bad_pixel_file.is_open() ){
	  QString msg;
	  msg  = ("ERROR: couldn't open file \"" + p("badpixelfile") + "\".\n").c_str();
	  msg += "Fix the problem and press start again.";
	  QMessageBox::critical(this, "can't open bad-pixel file", msg);
	  return true;
	}

	bad_pixel_file << "filter intercept: " << p["maxhitsperpixelintercept"] << "\n"
	               << "filter slope    : " << p["maxhitsperpixelslope"    ] << "\n"
	               << "\n";
	return false;
}

void monitor::prepare_for_looping ()
{
	wait_counter    = 0;
	display_helper  = true;
	
	lvl1 = 0;
	lvl2 = 0;
	
	time_spent_waiting = std::chrono::microseconds(0);
	time_spent_writing = std::chrono::microseconds(0);
	time_spent_reading = std::chrono::microseconds(0);
	
	std::cout << "\nreading file \"" << p("input") << "\".\n" << std::flush;
	
	start_time = std::chrono::system_clock::now();
	
	time_to_update_histograms->start( p["writingperiod"] );
}

void monitor::update_histograms ()
{
	while ( get_line(reader, line) ) process_input_line();
	
	auto start = std::chrono::system_clock::now();
	
	for (auto i = all_histograms.begin(); i != all_histograms.end(); ++i)
	
	if ( p[ *i + "suppress" ] ) continue;
	
	else if ( map_histograms.count(*i) != 0 ) histograms2D.at(*i).Write (0, TObject::kOverwrite);
	else                                      histograms1D.at(*i).Write (0, TObject::kOverwrite);
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
	
	auto end = std::chrono::system_clock::now();
	
	time_spent_writing += std::chrono::duration_cast <std::chrono::milliseconds> (end - start);
	
	patiently_bide_time();
}

void monitor::cycle_to_next_histogram ()
{
	if (!started) return;
	
	++histogram_cycler;
	
	if ( histogram_cycler == all_histograms.end() ) histogram_cycler = all_histograms.begin();
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::process_input_line ()
{
	auto start = std::chrono::system_clock::now();
	
	wait_counter = 0;
	
	if ( line.substr(0,2) != "0x" ) return;
	
	int data = std::stoi(line,0,16);
	
	if      ( (data & 0xf80000) == 0xf80000 ) handle_as_external_trigger (data);
	
	else if ( (data & 0xff0000) == 0xe90000 ) handle_as_data_header (data);
	
	else                                      handle_as_data_record (data);
	
	auto end = std::chrono::system_clock::now();
	
	time_spent_reading += std::chrono::duration_cast <std::chrono::milliseconds> (end - start);
}

void monitor::handle_as_external_trigger (int /*data*/)
{
	lvl1 = 0;
	
	++lvl2;
	
	for (auto i = time_histograms.begin(); i != time_histograms.end(); ++i) histograms1D.at(*i).SetBins ( lvl2+1, 0, lvl2+1 );
	
	auto & h1 = histograms1D.at("totalhits");
	h1.Fill ( lvl2, h1.GetBinContent( h1.FindBin(lvl2-1) ) );
	
	auto & h2 = histograms1D.at("meanToT");
	h2.Fill ( lvl2, h2.GetBinContent( h2.FindBin(lvl2-1) ) );
	
	skip_next_line ();
}

void monitor::skip_next_line ()
{
	while (true)
	
	if ( get_line (reader, line) ) break;
	
	else
	{
		reader.clear();
		
		std::this_thread::sleep_for( std::chrono::milliseconds( (unsigned)p["patientlatency"] ) );
	}
}

void monitor::handle_as_data_header (int /*data*/)
{
	++lvl1;
}

void monitor::handle_as_data_record (int data)
{
	int col  = (data & 0xfe0000) >> 17;
	int row  = (data & 0x01ff00) >>  8;
	int tot1 = (data & 0x0000f0) >>  4;
	int tot2 = (data & 0x00000f)      ;
	
	if ( is_bad_pixel (col,row) ) return;
	
	raw_event_data.at("LVL1").push_back (lvl1);
	raw_event_data.at("BCID").push_back (lvl2);
	raw_event_data.at("col" ).push_back (col );
	raw_event_data.at("row" ).push_back (row );
	raw_event_data.at("ToT1").push_back (tot1);
	raw_event_data.at("ToT2").push_back (tot2);
	
	for (auto i = basic_histograms.begin(); i != basic_histograms.end(); ++i) histograms1D.at(*i).Fill( raw_event_data.at(*i).back() );
	
	int    bin     = histograms1D.at("totalhits").FindBin(lvl2);
	int    hits    = histograms1D.at("totalhits").GetBinContent(bin);
	double old_tot = histograms1D.at("meanToT").GetBinContent(bin);
	histograms1D.at("meanToT").SetBinContent ( bin, (old_tot * hits + tot1) / (hits + 1) );
	
	histograms1D.at("totalhits").Fill(lvl2);
	
	bin     = histograms2D.at("hitmap").FindBin (col,row);
	hits    = histograms2D.at("hitmap").GetBinContent(bin);
	old_tot = histograms2D.at("ToTmap").GetBinContent(bin);
	histograms2D.at("ToTmap").SetBinContent ( bin, (old_tot * hits + tot1) / (hits + 1) );
	
	histograms2D.at("hitmap").Fill (col,row);
}

bool monitor::is_bad_pixel (int col, int row)
{
//	auto pixel = std::array <int,2> {{col,row}};
        std::pair<int, int> pixel(col, row);
	
	if ( bad_pixel_list.count(pixel) != 0 ) return true;
	
	int bin  = histograms2D.at("hitmap").FindBin(col,row);
	int hits = histograms2D.at("hitmap").GetBinContent(bin);
	
	if ( hits >= p["maxhitsperpixelintercept"] + lvl2 * p["maxhitsperpixelslope"] )
	{
		histograms2D.at("hitmap").SetBinContent (bin,0);
		histograms2D.at("ToTmap").SetBinContent (bin,0);
		
		histograms2D.at("badpixelmap").SetBinContent (bin,lvl2);
		
		bad_pixel_list.insert ( std::make_pair (pixel,lvl2) );
		
		bad_pixel_file <<      "(" << col << "," << row << ") @ TD " << lvl2 << "\n";
		
		std::cout << "BAD PIXEL (" << col << "," << row << ") @ TD " << lvl2 << "\n";
		
		return true;
	}
	
	else return false;
}

void monitor::patiently_bide_time ()
{
	auto start = std::chrono::system_clock::now();
	
	reader.clear();
	
	++wait_counter;
	if ( wait_counter > p["impatientlatency"] )
	{
		wait_counter = 0;
		std::cout << "waiting for input ...\n" << std::flush;
	}
	
	std::this_thread::sleep_for( std::chrono::milliseconds( (unsigned)p["patientlatency"] ) );
	
	auto end = std::chrono::system_clock::now();
	
	time_spent_waiting += std::chrono::duration_cast <std::chrono::milliseconds> (end - start);
}

void monitor::close_shop ()
{
	end_time = std::chrono::system_clock::now();
	
	std::cout << "exiting ...\n";
	
	if ( p["showtimediagnostics"] ) show_time_diagnostics ();
	
	//if (not started) return;
	if ( reader.is_open() ) reader.close();
	if ( writer!=0 && writer->IsOpen() ) writer->Close();
	
	if ( bad_pixel_file.is_open() ){
	  bad_pixel_file << "\nfinal TD: " << raw_event_data.at("BCID").back() << "\n";
	  bad_pixel_file.close();
	}

	//std::cout << "\n";
}

void monitor::show_time_diagnostics ()
{
	auto waiting = std::chrono::duration_cast <std::chrono::milliseconds> (time_spent_waiting);
	auto writing = std::chrono::duration_cast <std::chrono::milliseconds> (time_spent_writing);
	auto reading = std::chrono::duration_cast <std::chrono::milliseconds> (time_spent_reading);
	
	auto elapsed = std::chrono::duration_cast <std::chrono::milliseconds> (end_time - start_time);
	
	std::cout << "\n"
	          << "time spent reading: " << reading.count() << " milliseconds\n"
	          << "time spent writing: " << writing.count() << " milliseconds\n"
	          << "time spent waiting: " << waiting.count() << " milliseconds\n"
	          << "total time elapsed: " << elapsed.count() << " milliseconds\n";
}

void monitor::draw_LVL1()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "LVL1" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_LVL2()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "BCID" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_col ()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "col" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_row ()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "row" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_ToT1()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "ToT1" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_ToT2()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "ToT2" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_total_hits()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "totalhits" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_mean_ToT  ()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "meanToT" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_hitmap ()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "hitmap" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_ToT_map()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "ToTmap" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}

void monitor::draw_bad_pixel_map()
{
	histogram_cycler = std::find( all_histograms.begin(), all_histograms.end(), "badpixelmap" );
	
	if ( map_histograms.count( *histogram_cycler ) != 0 ) histograms2D.at( *histogram_cycler ).Draw();
	else                                                  histograms1D.at( *histogram_cycler ).Draw();
	
	canvas->getCanvas()->Update();
	canvas->getCanvas()->Draw  ();
}


