// Mainpage for Doxygen
 
/** @mainpage package STcontrol
 *
 * @section intro Introduction
 *
 *  STcontrol is the data taking GUI intended for Pixel system test use.  
 *  For user documentation, please consult the <a href="https://uimon.cern.ch/twiki/bin/view/Atlas/STcontrol">Wiki-pages</a>
 * <hr>
 *
 * @author J&ouml;rn Grosse-Knetter (Joern.Grosse-Knetter@uni-bonn.de): coordination, config. related issues, data viewing
 * @author Mauro Donega (mauro.donega@cern.ch): implementation of remote interface (via PixBroker/PixAction)
 * @author Iris Rottl&auml;nder: retired from project
 * @author Jan Schumacher: retired from project
 * @author Jan Valenta: retired from project
 * 
 *<hr>
 * @section installation Installation and compilation instructions
 *
 * This release has been tested in the following environment:
 *<ul>  
 *   <li>CERN SLC 3</li>
 *   <li>root 5.10/00d (current LCG version)</li>
 *   <li>QT 3.3.4</li>
 *   <li>PixLib tag PixLib-2-3 and its <a href="http://sctpixel.home.cern.ch/sctpixel/RodDaq/PixelDoxy/PixLib/html/main.html#installation">dependencies</a></li>, except for TDAQ 01-06-00
 * </ul>
 *
 * To compile you have to check out from the repository the
 * following modules:
 *<ul>  
 *   <li>Application/Pixel/PixLib and its <a href="http://sctpixel.home.cern.ch/sctpixel/RodDaq/PixelDoxy/PixLib/html/main.html#installation">dependencies</a></li>
 *   <li>Application/Pixel/PixAnalysis</li>
 *   <li>Application/Pixel/DBeditor</li>
 *   <li>Application/Pixel/ModuleAnalysis</li>
 *   <li>Application/Pixel/STcontrol</li>
 * </ul>
 *
 * The following symbols have to be defined on top of those
 * described for <a href="http://sctpixel.home.cern.ch/sctpixel/RodDaq/PixelDoxy/PixLib/html/main.html#installation">PixLib</a> in order to
 * compile; their definition is relative to the top directory
 * of the installation SCTPIXEL_ROOT (i.e. where Applications, 
 * RodDaq and VmeInterface are):
 *<ul>
 *   <li>PIX_ANA = $SCTPIXEL_ROOT/Applications/Pixel/PixAnalysis</li>
 *   <li>DBEDT = $SCTPIXEL_ROOT/Applications/Pixel/DBeditor</li>
 *   <li>MODULE_ANALYSIS = $SCTPIXEL_ROOT/Applications/Pixel/ModuleAnalysis</li>
 *</ul>
 * All of these packages, as well as PixLib, use shared libraries. You must thus add them to LD_LIBRARY_PATH too.
 * Also not that DBeditor and ModuleAnalysis must be built with "make shared" in order to get the shared libraries.
 * A simple "make" will just create stand-alone applications.
 * 
 */
