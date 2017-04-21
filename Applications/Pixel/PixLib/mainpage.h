// Mainpage for Doxygen

/** @mainpage package PixLib
 *
 * @section intro Introduction
 *  This package contains the PixLib files
 *  PixLib is a collection of classes designed to support applications performing
 *  configuration, calibration, data-taking and tests of Pixel Modules. 
 *
 * @author Paolo Morettini (Paolo.Morettini@ge.infn.it): coordination, PixController, PixModuleGroup
 * @author Guido Gagliardi, (Guido.Gagliardi@ge.infn.it): PixConfDBInterface
 * @author Carlo Schiavi, (Carlo.Schiavi@ge.infn.it): PixFe, PixMCC, PixModule 
 * @author Kendal Reeves, (Reeves@physik.uni-wuppertal.de): PixDcs 
 *
 *<hr>
 * @section releasenotes  Notes for current release
 * release.notes
 *
 *<hr>
 * @section installation Installation and compilation instructions
 *
 * This release has been tested in the following environment:
 *<ul>  
 *   <li>CERN RH 7.3.3</li>
 *   <li>Kernel 2.4.20-28.7</li>
 *   <li>online-00-21-01</li>
 *   <li>dataflow-00-07-00</li>
 *   <li>gcc 3.2</li>
 *   <li>root 3.10/02</li>
 * </ul>
 *
 * To compile you have to check out from the repository the
 * following modules:
 *<ul>  
 *   <li>VmeInterface</li>
 *   <li>RodDaq/RodCrate</li>
 *   <li>RodDaq/CommonWithDsp</li>
 *   <li>RodDaq/CommonForHosts</li>
 *   <li>RodDaq/Dsp/Pixel</li>
 *   <li>Application/Pixel/PixLib</li>
 * </ul>
 *
 * It's assumed that the dataflow setup script is executed
 * at login time, that the symbols TDAQ_INST_PATH, TDAQ_TARGET, 
 * DF_INST_PATH and DF_TARGET are correctly
 * defined, that gcc 3.2 is correctly working, that root is
 * installed (ROOTSYS defined, $ROOTSYS/bin in PATH and
 * $ROOTSYS/lib in LD_LIBRARY_PATH).
 * 
 * The following symbols have to be defined in order to
 * compile; their definition is relative to the top directory
 * of the installation SCTPIXEL_ROOT (i.e. where Applications, 
 * RodDaq and VmeInterface are)
 *<ul>
 *   <li>DAQ_LIB_DIR = $DF_INST_PATH/$DF_TARGET/lib</li>
 *   <li>DAQ_INCL_DIR = $DAQ_INST_PATH/include</li>
 *   <li>VME_INTERFACE = $SCTPIXEL_ROOT/VmeInterface</li>
 *   <li>SCTPIXEL_DAQ_ROOT = $SCTPIXEL_ROOT/RodDaq</li>
 *   <li>ROD_DAQ = $SCTPIXEL_ROOT/RodDaq</li>
 *   <li>PIX_LIB = $SCTPIXEL_ROOT/Applications/Pixel/PixLib</li>
 *   <li>ROD_TYPE = PIXEL_ROD</li>
 *</ul>
 *
 *<hr>
 * @todo 
 */
