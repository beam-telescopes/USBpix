The Settings file contains different sections to set the wafer analysis program settings and options.

The [AnalysisStyle] section defines the type of the data analysis. At the moment it is not used.

The name mapping sections are: [Scans] [DCSGraphAnaValues] [ChipCalibration] [GlobalRegValues] [PixCrtlSetting]
These sections contain a list of scans, DCS names, chip calibration, Global register names, 
PixControler settings and Graph Analysis Value Names that will be analyzed.
It is possible to add any DCS name, chip calibration name, global register name, Graph Analysis Value and PixControler setting. 
You just have to know the name used within the file (cfg or data) and define a name to use in the
Wafer probing program.
Example:
VthinAF=GlobalRegister_Vthin_AltFine (left side: your definition, right side: STControl value name)
You can also cut on this new defined value by writing it into the Cut file (here for example: VthinAF>100)
The scans cannot be defined this way (of cause) because the program has to analyze each scan
in a different way. Only the scans listed in the Scan list file are analyzed and you can not add new ones
without changing the program. To add new scans change the WaferAnalysis class.
As a check all scans (even if they do not require any analyzes) have to be listed in the Scan list file.
Otherwise the program will complain. 
Example:
ThresholdScan=Threshold Scan
The left side of the scan definitions (here: ThresholdScan) should not be changed, otherwise
the scan is not recognized anymore!

The [AddToTotalCount] section:
Defines the pixel based scans (e.g. analog scan) to be added to the total failing pixel/column count or for modules depending
on the scan also to the shorted or not connected pixels count. The name is the scan name
not the STControl scan name. For example:
AnalogTest=1 (enabled)
uncommenting, not mentioning or AnalogTest=0: scan results is not added to the total failing pixel/column count

The [DACscanAnalysisSettings] section:
Here you can define the settings for the DAC scan that are analyzed. The syntax is:
ScanName:X-Axis Titel:Y-Axis Titel:Factor for the Values (Min/Max):Factor for the slope of the line fit:Factor for the X2 of the line fit:BestDACsetting
For example:
"Iref:Iref [PAD]:Current [uA]:1e9:1e9:1e19:2000" tries to find the best DAC setting to reach 2000 nA. The X/Y-axis scaling and the
line fit slope/X2 factors are set in a way that WaferAnalysis shows nice numbers in the order of 1e3.

The [PostProcessing] section:
Defines the post processing that is done after all results are available. These post processing steps are:
- AddAptasicColumn: adds an additional result column "Aptasic" that is green and always change able
- CheckForAbortRun: the data of the chip is checked that the run got not aborted due to abort conditions and adds 
                    an abort flag to the chip results
- TotalCount: activates the total pixel/failing count and add the results to the chip results
- CorrectVrefs: does the GND shift correction to the Vrefs (tunable and fixed)
- CalculateResult: calculates a new results from other results, only / and * as mathematical operators are supported so far
- CompareResults: compares two results and changes the state of one results if they are not the same in a given error range
The post processing steps can be activated via:
PostProcessingName=1

The [CorrectBandGaps] section:
Defines the BGvoltages and currents that are used for the ground shift correction. Syntax:
BandGapVoltageName=NameOfCurrent1:NameOfCurrent2:...
The current are summed up and the plot has a point (BandGapVoltageName, SumOfTheCurrents).
A new result is added with the old result name + "Corr" appendix.  
For example:
BgAn0=IDDA1AP:IDDA2AP:IDDD1AP:IDDD2AP

The [CorrectTunVrefs] section:
Defines the BGvoltages that are used at the moment when the TunVref was measured. Then the mean
difference of the measured BGvoltages to the corrected on is used to correct the tunable Vrefs values.
A new result is added with the old result name + "Corr" appendix.
Example:
VrefDigMin:BgDig1:BgAn1 
(takes the mean difference of the difference BgDig1 to the corrected BgDig and the BgAn1 to the correctd BgAn.
This is then applied to the VrefDigMin)

The [CapacitanceCorrection] section:
Defines values that are used for the formular to calculate the injection capacitance out of the measured slope of
the Icap scan. The value are:
Offset=-1360.
Dividor=1.081
Voltage=1000

The [CompareResults] section:
Defines values that are compared with each other in a certain range and the resulting status if they do not match. 
Syntax: ResultValueName1=ResultValueName2:AllowedDifferenceInPersentOfResult2:StatusIfTheValuesDontMatch. 
Example: PlsrDACslopeST=PlsrDACslope:3:Blue (PlsrDACslopeST is marked blue if it more than 3% off from PlsrDACslope)

The [ZoomDistributionPlots] section:
Defines the names of the results that are plotted in addition to the normal distribution plot with a zoom on the green chips.

The [ExportToXML] section:
Defines the names of the results that are exported to a XML file for the IBL construction data base. The file format is not the final one!
So far its only "proof of concept"

The [CheckPixelShorts] section:
Here you can define the SearchDistance were to search around the analog pixel for cross talk pixel, syntax relative col/relative row, for example -1/-1

The [IVcurveAnalysis] section:
The needed settings for the IV curve analysis are metioned here. For example the operation point of the modules.

[PlotSettings]
The plot settings are mentioned here. The syntax is
settingName=value
You can choose from the following settings:
STcontrolColorPalette=?, sets the color palette of 2d plots according to the STControl palette settings
HistogramPlotRangeInSigma=?, is the plot range of the 3 in 1 plot for the 1d-histogram plot in sigma of the gaus fit
ScatterPlotRangeInSigma=?, is the plot range of the 3 in 1 plot for the scatter plot in sigma of the gaus fit

[ExportOverviewData]
Here the names of the data are listed, that are exported into a text file. The format is:
NameOfTheData:TheNameOfYourChoise. It is case sensitive. For example:
VDDA_AFTER_CFG:ANALOG CURRENT
PlsrDACslope:Plsr DAC slope
status:Status