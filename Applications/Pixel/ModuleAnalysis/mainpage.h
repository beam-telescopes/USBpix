// Mainpage for Doxygen
 
/** @mainpage package ModuleAnalysis
 *
 * @section intro Introduction
 *  
 * A brief description on how to add code to the ModuleAnalysis package yourself
 * follows below. Please consult also
 * <a href="http://hep1.physik.uni-bonn.de/MainFolder/documents/article/Research/DetectorsAndMicroelectronics/ATLASPixeldetector/ModuleAnalysisFrame">ModuleAnalysis documentation page</a> 
 * for using the package.
 * <br>
 * <ol>
 * <li><a href="#general">General structure</a></li>
 * <li><a href="#extapp">How to call MA from another application</a></li>
 * <li><a href="#newdat">How to read additional data-types</a></li>
 * <li><a href="#newplt">How to add new plotting/analysis features</a>&nbsp;&nbsp;</li>
 * </ol>
 * 
 * <a name="general"><h2>General structure</h2></a>
 * 
 * The program consists of three main parts:
 * <ul>
 * <li> <a href="classTopWin.html">TopWin class</a>: this produces the main GUI panel and controls data
 *      reading and plotting/analysis features. It inherits its basic graphics from the 
 *      TopWinBase class which is auto-generated with QT (from the file TopWinBase.ui);
 * <li> <a href="classDatSet.html">DatSet class</a>: used for data storage into ROOT histograms
 *      of type <a href="http://root.cern.ch/root/html/TH1F.html">TH1F</a> and 
 *      <a href="http://root.cern.ch/root/html/TH2F.html">TH2F</a>;
 * <li> <a href="classRMain.html">RMain class</a>: the main plotting/analysis interface, 
 *      based on <a href="http://root.cern.ch">ROOT</a>.
 * </ul>
 * Other sub-classes are used from within these classes; a full list is available from the
 * links at the top of this page.
 * 
 * <a name="extapp"><h2>How to call MA from another application</h2></a>
 * 
 * <h3>Creating a TopWin object</h3>
 * Recommended call for creating the <a href="classTopWin.html">TopWin</a> main GUI
 * (TopWin *m_MAF should be a member of your control-GUI):
 * <pre>
 * m_MAFwin = new TopWin(NULL,"MAFwin",0,TRUE);
 * </pre>
 * The last argument being true indicates MA that it is controlled by another application; setting
 * this argument to FALSE will enable more user control function not desired for dependent-use of MA
 * and will shut down the entire QT application on exit. To open the panel just call
 * <pre>
 * m_MAFwin->show();
 * </pre>
 * 
 * <h3>Creating module items</h3>
 * 
 * A module item is created via the <a href="classModItem.html">ModItem</a> class; recommended call 
 * (using the m_MAFwin pointer from above):
 * <pre>
 * ModItem *module_item = new ModItem(m_MAFwin->ModuleList, [module name (const char*)]);
 * </pre>
 * 
 * <h3>Adding data items to existing modules</h3>
 * A data item should be a module sub-item and can be created via the <a href="classModItem.html">ModItem</a> class.
 * However, it is recommended to use the <a href="classTopWin.html">TopWin</a>::DataToTree function:
 * <pre>
 * ModItem *data_item = m_MAFwin->DataToTree(module_item,DATA)
 * </pre>
 * where m_MAFwin and module_item are as above, and DATA is the pointer to a <a href="classDatSet.html">DatSet</a>
 * object.<br>
 * Alternatively, you can have MA do all data loading for you, provided you want to read from a
 * TurboDAQ file:
 * <pre>
 * ModItem *data_item = m_MAFwin->LoadData(module_item,filename, label,data_type); 
 * </pre>
 * where <i>filename</i> is the full path of the file, <i>label</i> is the name under which the item
 * will appear in the tree-view of TopWin, and <i>data_type</i> is the type of data,
 * see <a href="FileTypes_8h-source.html">FileTypes.h</a> (items>100) or the list at the top of
 * TopWin.cpp.
 * 
 * <a name="newdat"><h2>How to read additional data-types</h2></a>
 * <h3>Changes to the DatSet class</h3>
 * <p>
 * <u><b>Step 1</b></u><br>
 * The DatSet class can handle different data types as defined in the
 * <a href="FileTypes_8h-source.html">FileTypes.h</a> file. By convention, 
 * values &gt;100 are used as file types (otherwise it's considered a plot type,
 * see <a href="#newplt">below</a>). Define a new name and corresponding index for the new type
 * you want to use in there.
 * </p>
 * <p>
 * <u><b>Step 2</b></u><br>
 * You have to create your own data reading routine, filling Root histograms as explained below,
 * and add it as function to the DatSet class. It should return an integer
 * containing a non-zero error code in case of problems. Having done so, go to the
 * constructor (the DatSet::DatSet(const char *name, const char *path, int type, int* rderr, DatSet* TOTCal, const char* cfgpath) version), look for the switch(type) part and add a case with
 * the type you defined in the beginning. In that case, add your new data reading routine.
 * The constructor argument <i>path</i> will provide an absolute path of the file to be read.
 * It expects <i>rderr</i> be filled with above error code.<br>
 * Data is typically stored in 2D root histograms if the data set contains
 * one value (or one avreage value) per pixel. So far, the private pointer
 * arrays *m_parmap[NPAR] is used for fit-output and the pointer array
 * *m_maps_2D[NRAW] for raw data (eg number of hits). Scan data (only 1D for now)
 * is stored in 1D histograms, one per pixel. A pointer **m_scanhi which points
 * to an array of 46060 TH1F histograms if needed (NULL otherwise) is private
 * DatSet member. Preferrably use one of those pointers to create new histograms which
 * will store the data. <br>
 * <u>1. Example for storing plain data (from DatSet::ReadBinData)</u><br>
 * Create a TH2F histogram and fill element of <i>m_maps_2D</i> array with pointer address:
 * <pre>
 *  sprintf(htit,"rawmap%x",(int)this);
 *  m_maps_2D[HITS_HISTO] = new TH2F(htit,"Map of avg. hit data",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
 * </pre>
 * ...later in the ascii reading loop:
 * <pre>
 *  m_maps_2D[HITS_HISTO]->Fill(xval, yval, meas);
 * </pre>
 * where you might want to use the <i>DatSet::PixXY</i> routine 
 * (see <a href="DataStuff_8h-source.html">DataStuff.h</a>)
 * to convert from chip/column/row to histogram X-Y coordinates.
 * <br>
 * <u>2. Example for storing scan data (from ReadBinData):</u><br>
 * Create array of TH1F histograms and write pointer address into <i>m_scanhi</i>:
 * <pre>
 *  if(scanno>1 && m_scanhi==NULL) m_scanhi = new TH1F*[NCHIP*NPIX];
 * </pre>
 * somewhere in a 0&lt;index&lt;NCHIP*NPIX loop:
 * <pre>
 *  m_scanhi[index] = new TH1F(htit,hname,scanno, scanmin, scanmax);
 * </pre>
 * ...later in the ascii reading loop:
 * <pre>
 *  bin = m_scanhi[PixIndex(chip,col,row)]->Fill(dac,meas);
 * </pre>
 * using the <i>DatSet::PixIndex</i> routine 
 * (see <a href="DataStuff_8h-source.html">DataStuff.h</a>)
 * to convert from chip/column/row to array-index.
 * </p><p>
 * <u><b>Step 3</b></u><br>
 * To allow other routines to access your data stored in 2D histograms
 * you must add your data type to the
 * <i>DatSet::GetMap(int chip, int type)</i> routine, in the switch(type) list,
 * where type will be the data type defined in FileTypes.h.
 * 1D scan histograms are already accessible via the 
 * <i>DatSet::GetScanHi(int chip, int col, int row)</i> function.
 * </p>
 * 
 * <h3>Changes to the TopWin class</h3>
 * 
 * The user action of reading data goes via the <i>TopWin::TopWin</i> 
 * constructor. Add a line like
 * <pre>
 *   m_fwin->AddType("Data type label","Filter (*.ext)",YOURNEWTYPE);
 * </pre>
 * which calls the <i><a href="FileWin_8h-source.html">FileWin</a>::AddType</i> function.
 * It will add an according item in the file reading menu. "Data type label" is what
 * will show up in the type selection combo box of the file window, "Filter" should
 * have the indicated format, ie a description plus the extension in the format *.ext
 * in parentheses, and will be used in the browser (the type Any file (*.*) will be
 * added automatically). <br>
 * Finally you must
 * tell TopWin what type of action you want on the new data set type; see 
 * <a href="#plttype">bottom of next section</a> for details.
 * 
 * <a name="newplt"><h2>How to add new plotting/analysis features</h2></a>
 * 
 * <h3>Changes to the RMain class</h3>
 * <p>
 * Create a new (public) function in the RMain class. One of its arguments
 * must be of DatSet* type to allow hading over of the data. Tyhe DatSet functions
 * <i>DatSet::GetMap(int chip, int type)</i> or
 * <i>DatSet::GetScanHi(int chip, int col, int row)</i> will then allow to access
 * the actual data stored in 2D maps (1 bin per pixel) or 1D scan histograms (1 histo
 * per pixel). For existing data types, the following arguments can be used as <i>type</i>
 * in the <i>DatSet::GetMap</i> function (see <a href="FileTypes_8h-source.html">FileTypes.h</a>):
 * <ul>
 * <li> RAW: (average) hits from binned data
 * <li> TOT_HIT: hits from TOT data
 * <li> TOT: average TOT
 * <li> CLTOT: average calibrated TOT
 * <li> MEAN: threshold from S-curve fits
 * <li> SIG:  noise from S-curve fits
 * <li> TOTA(B/C): parameter A (B/C) from TOT fit
 * <li> CHI: chi<sup>2</sup> from fit
 * </ul>
 * The existing canvas (pointer is stored in RMain::maincan)
 * should be used for plotting. 
 * Call CheckCan() in the beginning to make sure the ROOT-canvas exists and is open.
 * </p>
 * <p>
 * <blink><b>TIP:</b></blink> If you just want simple plots of new data types, not
 * knowing the details of ROOT graphics, you can try
 * to use one of the already defined plotting routines (see 
 * <a href="RootStuff_8h-source.html">RootStuff.h</a>):
 * <ul>
 * <li> Plot2DColour: simple plot of the 2D colour map;
 * <li> PlotMapProjScat: 3-part lay-out of coloured map, projection and scatter plot;
 * <li> PlotMapScanhi: 2D colour map plus 1D scan histogram.
 * </ul>
 * Just add your type to the cases which call one of these functions in TopWin.cpp (see next 
 * sub-section), the data type is retrieved in
 * these plot routines to get the right histogram with the <i>DatSet::GetMap</i> function.
 * </p>
 * 
 * <a name="plttype"><h3>Changes to the TopWin class</h3></a>
 * <p>
 * The various plot/analysis types are defined in the
 * <a href="FileTypes_8h-source.html">FileTypes.h</a> file. By convention, 
 * values above zero and &lt;100 are used as analysis types (otherwise it's considered a file type,
 * see <a href="#newdat">above</a>). Define a new name and corresponding index for the new type
 * you want to use in there.
 * </p><p>
 * The user action of plotting/analysing data goes via the TopWin main panel routine 
 * <i>TopWin::ModuleList_itemselected(ModItem* item)</i> which is called upon
 * double-clicking items in the tree-view. Add your new plot type to the
 * switch(item->GetPID()) list to call above RMain function.<br>
 * Add your new type as case to the switch(type), where the ModItem class, ie a line like
 * <pre>
 *  plotit = new ModItem(subi,"Brief description",did,PLOT-TYPE);
 * </pre>
 * will create an according item in the TopWin tree-view.
 * Last not least, you have to add the new plot type to the corresponding data types. 
 * It is done in the
 * <i>TopWin::DataToTree(ModItem* item, DatSet *did)</i> function.
 * Add your new type as case to the switch(type), where the ModItem class, ie a line like
 * <pre>
 *  plotit = new ModItem(subi,"Brief description",did,PLOT-TYPE);
 * </pre>
 * will create an according item in the TopWin tree-view. Make sure you add it to the
 * correct data-type case!
 * </p>
 * 
 * <hr>
 * <a href="#general">General structure</a>&nbsp;&nbsp;
 * <a href="#newdat">How to read additional data-types</a>&nbsp;&nbsp;
 * <a href="#newplt">How to add new plotting/analysis features</a>&nbsp;&nbsp;
 *
 * <hr>
 * @author J&ouml;rn Grosse-Knetter (Joern.Grosse-Knetter@uni-bonn.de): coordination
 * @author Jens Weingarten (weingart@physik.uni-bonn.de): system test related issues
 * 
 */
