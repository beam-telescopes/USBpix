#All possible parameter cuts for all sections [section] 
#are listed below. The formatting of a parameter equation is:
#parameter<value; parameter>value or parameter!=value.
#The != operator is equal to the combination of the < and the > operator.
#The cut triggers if the parameter equation is true (example: AnalogHits!=200).
#The section triggers if any cut has triggered (ORed).
#If [BlueChip],[RedChip] and [YellowChip] do not trigger the chip is automatically green.
#If a parameter is mentioned more than once only the last definition
#is taken into account. The [File] area is special and only accepts = operators.
#You can also cut on DCSs and Global register values that you define in Settings.txt to
#be analyzed. These cuts belong to the [...Chip] section, the color is freely selectable.
#Some scans are done more than once with different configurations (HV applied, tuned, ...).
#Thus a Scan identifier[SI] can be used to distinguish these scans. The Scan identifier adds a word
#(like: tuned) to the usual name. For example
#ThresholdScanUntuned, ThresholdScanTuned (the [SI] is Untuned/Tuned). The [SI] can be empty.
#
#There are some scans that are only done in module testing. The cuts for these scans are mentioned
#separately at the end of this file.
#
#For any suggestions: pohl@physik.uni-bonn.de

[File]
version=0.1		(version of the file, PLEASE INCREASE IN CutConf.txt IF ANYTHING CHANGED THERE)

[FailPixel] (normalized to one IC, importand for judging double IC modules)
AnalogHits		(number of hits in the analog test, usually 200)
DigitalHits		(number of hits in the digital test, usually 200)
HitOrEnHits     (number of hits in the hit Or test with disabled hit or, usually 0)
HitOrDisHits    (number of hits in the hit Or test with enabled hit or, usually 10)
LatencyEnHits    (number of hits in the latency test that are enabled for every latency value, usually 100)
LatencyDisHits    (number of hits in the latency test that are disabled for every latency value, usually 0)
CrossTalk[SI]Hits   (number of hits in the cross talk scan, usually 0)
DisabledHits   (number of hits in the disabled pixel scan, usually 0)
BufferActualToTHits  (number off hits in the buffer test for the actual ToT, usually close to 5)
Scurve[SI]Chi2		(chi square of the s curve fit in the threshold scan, take care: failed fits sometimes have a chi2 = -1,0)
Thresh[SI]SigmaOffset	(distance between pixel threshold and mean threshold, distance = ThreshSigmaOffset * sigma of threshold distribution)
Noise[SI]SigmaOffset	(distance between pixel noise and mean noise, distance = NoiseSigmaOffset * sigma of noise distribution)
ESLhits         (number of hits in the check Event Size Limit test, usually either 1 (1/2 of the pixels of first DC) or 0 (others))
ENABLEMaskValue (value of the config for the enable mask of the scan (0/1))
TDACMaskValue (value of the config for the TDAC mask of the scan)
FDACMaskValue (value of the config for the FDAC mask of the scan)

[FailColumn]
AnalogFailPixel		(number of pixel that pass the analog [FailPixel] criterion)
DigitalFailPixel	(number of pixel that pass the digital [FailPixel] criterion)
HitOrEnFailPixel    (number of pixel that pass the hit Or enable [FailPixel] criterions)
HitOrDisFailPixel   (number of pixel that pass the hit Or disable [FailPixel] criterions)
LatencyEnFailPixel    (number of enabled pixel that pass the enable latency [FailPixel] criterion)
LatencyDisFailPixel    (number of disabled pixel that pass the disable latency [FailPixel] criterion)
BufferActualToTFailPixel  (number of pixel that pass the buffer actual ToT [FailPixel] criterion)
BufferAllToTFailPixel  (number of pixel that pass the buffer all ToT [FailPixel] criterion)
CrossTalk[SI]FailPixel  (number of pixel that pass the cross talk scan [FailPixel] criterion)
DisabledFailPixel  (number of pixel that pass the disabled pixel scan [FailPixel] criterion)
PixRegENABLEFailPixel>10  (total number of pixel that fail the pixel register tests for latch ENABLE)
PixRegCAP0FailPixel>10  (total number of pixel that fail the pixel register tests for latch CAP0)
PixRegCAP1FailPixel>10  (total number of pixel that fail the pixel register tests for latch CAP1)
PixRegILEAKFailPixel>10  (total number of pixel that fail the pixel register tests for latch ILEAK)
PixRegTDAC0FailPixel>10  (total number of pixel that fail the pixel register tests for latch TDAC0)
PixRegTDAC1FailPixel>10  (total number of pixel that fail the pixel register tests for latch TDAC1)
PixRegTDAC2FailPixel>10  (total number of pixel that fail the pixel register tests for latch TDAC2)
PixRegTDAC3FailPixel>10  (total number of pixel that fail the pixel register tests for latch TDAC3)
PixRegTDAC4FailPixel>10  (total number of pixel that fail the pixel register tests for latch TDAC4)
PixRegFDAC0FailPixel>10  (total number of pixel that fail the pixel register tests for latch FDAC0)
PixRegFDAC1FailPixel>10  (total number of pixel that fail the pixel register tests for latch FDAC1)
PixRegFDAC2FailPixel>10  (total number of pixel that fail the pixel register tests for latch FDAC2)
PixRegFDAC3FailPixel>10  (total number of pixel that fail the pixel register tests for latch FDAC3)
ENABLEMaskFailPixel (number of pixel that pass the ENABLEMask [FailPixel] criterion)
TDACMaskFailPixel (number of pixel that pass the TDACMask [FailPixel] criterion)
FDACMaskFailPixel (number of pixel that pass the FDACMask [FailPixel] criterion)
HitDcrFailPixel (number of pixel that have a at HitDcs0 a TOT14 or have at HitDcs0 a TOT0 and no TOT 14 hit at HitDcs1 or have at HitDcs1 a TOT0 and no TOT 14 hit at HitDcs2)

[RedChip]
TotalPixelsFail (the total number of different pixels that fail the scans)
TotalColumnsFail(the total number of different columns that fail the scans)
Noise[SI]Mean		(mean noise of the chip)
Noise[SI]Sigma		(noise distribution gaus fit sigma of the chip)
Noise[SI]DistFitQuality	(fit quality in chi2/ndf of the noise distribution fit)
Noise[SI]FailPixel  (number of pixels that the noise [FailPixel] criterion)
Threshold[SI]Mean		(mean threshold of the chip)
Threshold[SI]Sigma	(threshold distribution gaus fit sigma of the chip)
Threshold[SI]DistFitQuality	(fit quality in chi2/ndf of the threshold distribution fit)	
Threshold[SI]FailPixel  (number of pixels that the threshold [FailPixel] criterion)
Scurves[SI]Chi2Mean  (mean chi2 of the chip for the s-curve fits)
Scurves[SI]Chi2Sigma  (chi2 distribution gaus fit sigma of the chip)
AnalogFailPixel		(number of pixel that pass the analog [FailPixel] criterion)
AnalogChighFailPixel     (number of pixel that pass the analog [FailPixel] criterion)
AnalogClowFailPixel     (number of pixel that pass the analog [FailPixel] criterion)
DigitalFailPixel	(number of pixel that pass the digital [FailPixel] criterion)
HitOrEnFailPixel    (number of pixel that pass the hit Or enable [FailPixel] criterions)
HitOrDisFailPixel   (number of pixel that pass the hit Or disable [FailPixel] criterions)
HitOrEnFailColumn         (total number of columns that pass the hit Or enable [FailColumn] criterion)
HitOrDisFailColumn         (total number of columns that pass the hit Or enable [FailColumn] criterion)
LatencyEnFailPixel    (number of enabled pixel that pass the enable latency [FailPixel] criterion)
LatencyDisFailPixel    (number of disabled pixel that pass the disable latency [FailPixel] criterion)
LatencyFailColumn       (total number of columns that pass the hit Or enable/disable [FailColumn] criterion)
BufferActualToTFailPixel  (number of pixel that pass the buffer actual ToT [FailPixel] criterion)
BufferActToTFailColumn  (total number of columns that pass the BufferActualToTFailPixel [FailColumn] criterion)
CrossTalk[SI]FailPixel (number of pixel that pass the cross talk test [FailPixel] criterion)
DisabledFailPixel  (number of pixel that pass the disabled pixel scan [FailPixel] criterion)
DisabledFailColumns  (total number of columns that pass the disabled pixel scan [FailPixel] criterion)
ScurveFitFailPixel	(number of pixel that pass the scurve [FailPixel] criterion)
AnalogFailColumns		(number of double columns that pass the analog [FailColumn] criterion)
DigitalFailColumns		(number of double columns that pass the digital [FailColumn] criterion)
HitOrFailD      (number of pixel that pass the hit Or [FailColumn] criterion)
BufferFailColumns   (number of double columns that pass the buffer test [FailColumn] criterion)
InjDelMinLVL1   (minimum mean LVL1 delay)
InjDelMaxLVL1   (maximum mean LVL1 delay)
InjDelMonotony  (value describing the monotony, usually 0)
SRX_Y           (the service record number for service record Y [0:31] and for the SR request X)
IDDA1			(current consumption on analog 1 channel in mA)
IDDA2			(current consumption on analog 2 channel in mA)
IDDD1			(current consumption on digital 1 channel in mA)
IDDD2			(current consumption on digital 2 channel in mA)
BgAnalog		(voltage of the analog band gap reference mV)
BgDigital		(voltage of the digital band gap reference mV)
PixelRegError	(number of pixels with a pixel register error)
GlobalRegError	(number of global register error)
RXvalley        (distance in bins [1..25] with a delay value without communication errors)
VrefDigMin      (minimum value measured for the digital voltage reference)
VrefDigMax      (maximum value measured for the digital voltage reference)
VrefDigSlope    (slope of the line fit for the Vref digital DAC transfer function, BE AWARE: in uV/DAC)
VrefDigLineFitQuality    (chi2/nfd*1e6 for the line fit of the Vref digital DAC transfer function)
VrefAnMin      (minimum value measured for the analog voltage reference)
VrefAnMax      (maximum value measured for the analog voltage reference)
VrefAnSlope    (slope of the line fit for the Vref analog DAC transfer function, BE AWARE: in uV/DAC)
VrefAnLineFitQuality    (chi2/nfd*1e6 for the line fit of the Vref analog DAC transfer function)
Iref           (best Iref value that can be set, BE AWARE: in nA)
IrefMin      (minimum value measured for the current reference, BE AWARE: in nA)
IrefMax      (maximum value measured for the current reference, BE AWARE: in nA)
IrefSlope    (slope of the line fit for the Iref transfer function, BE AWARE: in nA/DAC)
IrefLineFitQuality    (chi2/nfd*1e12 for the line fit of the Iref transfer function)
PlsrDACmin      (minimum value measured for the PlsrDAC scan, BE AWARE: in mV)
PlsrDACmax      (maximum value measured for the PlsrDAC scan, BE AWARE: in mV)
PlsrDACslope    (slope of the line fit for the PlsrDAC transfer function, BE AWARE: in uV/DAC)
PlsrDACkink    (kink of the PlsrDAC transfer function, BE AWARE: arbitrary unit)
PlsrDAClineFitQuality    (chi2/nfd*1e6 for the line fit of the PlsrDAC transfer function)
IcapSlope    (slope of the line fit for the Icap data, BE AWARE: in nA/DAC)
IcapLineFitQuality    (chi2/nfd*1e12 for the line fit of the Icap data)
OwnDefinedRegisterName (has to be defined in ScanList.txt)
OwnDefinedPixelCalibName (has to be defined in ScanList.txt, BE AWARE: the values read are multiplied by 1000)
HighFreqCurr    (current consumption in the high trigger frequency scan)
SC_CMD      (scan chain CMD, 0: fail, 1: pass, 2: no data)
SC_DOB      (scan chain DOB, 0: fail, 1: pass, 2: no data)
SC_ECL       (scan chain ECL, 0: fail, 1: pass, 2: no data)

[YellowChip]	see [RedChip] parameters

[BlueChip]		see [RedChip] parameters, but due to the fact that blue chip indicate analyzes issues only fit quality cuts and values that should not occure make sense here

#cuts for module scans
[FailPixel]
BumpConNoiDiff      (difference of the noise in e for two threshold scans with and without HV)
BumpConScurveChi2 (chi square of the s curve fit in the threshold scans, take care: failed fits sometimes have a chi2 = -1,0)
[FailColumn]
BumpConFailPixel     (number of pixel that pass the bump connection noise difference [FailPixel] criterion)
[RedChip]
BumpConFailPixel   (number of pixel that pass the bump connection noise difference [FailPixel] criterion)
BumpConFailColumn  (total number of columns that pass the BumpConFailPixel [FailColumn] criterion)
SourceScanEmptyBCID (total number of empty BCID counter values in the source scan)
SourceScanEmptyLV1ID (total number of empty LVL1ID counter values in the source scan)
HitDcrFailPixel (total number of failing pixel in the discriminator scan)
HitDcrFailColumn (total number of failing columns in the discriminator scan)