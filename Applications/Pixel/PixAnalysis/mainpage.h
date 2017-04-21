// Mainpage for Doxygen
 
/** @mainpage package PixAnalysis
 *
 * @section intro Introduction
 *  PixAnalysis is a collection of analysis routines operating
 *  on PixLib::PixScan data as well as histogram/data-handling
 *  classes. It consists of the following components:
 * <ul>
 *  <li>DataContainer: tools for histogram and config reading from
 *                     PixScan data files
 *  <li>Fitting:       general tools for fitting; currently uses
 *                     minuit-fitting via ROOT (in the future, ROOT will be removed)
 *                     and S-curve fitting with the Virzi-method
 *  <li>BocAnalysis: automatic analysis of BOC parameter scans,
 *                   returning a set of recommended BOC parameters
 *  <li>DataAnalysis: automatic analysis of std. tests to derive e.g. average noise
 *                    or threshold dispersion, as well as highlighting problematic data
 * </ul>
 *
 *<hr>
 * @author Joern Grosse-Knetter (Joern.Grosse-Knetter@uni-bonn.de): coordination, DataContainer, Fitting
 * @author Iris Rottlaender (rottlaen@physik.uni-bonn.de): BocAnalysis
 * @author Martin Schmitz (mschmitz@physik.uni-bonn.de): DataAnalysis
 * 
 */
