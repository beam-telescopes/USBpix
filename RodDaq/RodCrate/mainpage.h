// Mainpage for Doxygen

/** @mainpage package SCTPixelDAQ
 *
 * @section intro Introduction
 *  RodCrate contains the classes for VME modules that reside in the ROD crate.
 *  At present, these are RodModule for the ROD, TimModule for the TIM and
 *  BocCard for the BOC.
 *  RodModule has a number of support classes, all of which begin with the
 *  prefix "Rod" at the start of their name. 
 *
 *  Four support classes hold information passed from the DSP during initialization.
 *  These are MdspMemoryMap, SdspMemoryMap, MdspInternal, SdspInternal. The memory map
 *  classes contain all the addresses for the Master and Slave DSP address space and the
 *  internal classes are probably of no interest to anyone other than DSP programmers.
 *  They will probably never be used by the C++ programs, but classes have been built for them for
 *  completeness.
 
 *  In addition, these classes rely on the VmeInterface class, which is in a
 *  sibling directory to RodCrate and which provide a C++ wrapper to the
 *  ATLAS RCCVmeInterface C library.
 *
 *  There are several sibling directories of header files that also are 
 *  referenced from RodCrate.
 *  CommonWithDsp contains the C headers for definitions needed by both the
 *  ROD Crate Controller and the ROD DSPs. 
 *  CommonForHosts contains C headers for definitions needed by the Linux and
 *  Windows NT host programs.
 *  Wrapper headers exist in RodCrate to hide some of the C syntax in these
 *  files from the C++ code.
 *
 *  A directory RodUtils contains various main programs used to test the
 *  functionality of RodModule, TimModule and BocCard.
 *
 * @author RodModule: Tom Meyer, Iowa State University (meyer@iastate.edu)
 * @author TimModule: John Lane, University College London (jbl@hep.ucl.ac.uk)
 * @author BocCard: John Hill, University of Cambridge (hill@hep.phy.cam.ac.uk)
 *
 *<hr>
 * @section notes  Notes for current release
 * release.notes
 *<hr>
 */
