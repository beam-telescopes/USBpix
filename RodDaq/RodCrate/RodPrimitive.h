// File: RodPrimitive.h

#ifndef SCTPIXELROD_RODPRIMITIVE_H
#define SCTPIXELROD_RODPRIMITIVE_H

/*! 
 * @class RodPrimitive  
 *
 * @brief This is a class for primtives which control ROD execution.
 *
 * This class contains the primitives which are sent as a list to the RODs. They control 
 * almost all aspects of the ROD DSP behavior. New primitives can be defined to perform
 * user-defined actions, but corresponding DSP code must also be written, compiled and
 * loaded into the DSPs before the primitive can be executed.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

#ifdef IBL
#include "iblPrimXface.h"
#endif
//#else
#include "primXface.h"
//#endif

namespace SctPixelRod {

class RodPrimitive
{
public:
	RodPrimitive();                                               // Default constructor
	RodPrimitive( long bodysize, long index, long id, 
                      long revision, long * body);                    // Alternate constructor
	RodPrimitive( const RodPrimitive& );                          // Copy constructor
	~RodPrimitive();                                              // Destructor
	RodPrimitive& operator=( const RodPrimitive&);                //Overload = operator

	//! Accessor function to set length
	void setLength( long len) { m_header.length = len; }
	//! Accessor function to get length
	long getLength() const { return m_header.length; }
	
	//! Accessor function to set index
	void setIndex( long ind) { m_header.index = ind; }
	//! Accessor function to get index
	long getIndex() const { return m_header.index; }
	
	//! Accessor function to set ID
	void setId( long id) { m_header.id = id; }
	//! Accessor function to get ID
	long getId() const { return m_header.id; }
	//! Accessor function to get Revision
	long getRevision() const { return m_header.revision; }
	
	//! Accessor function to set pointer to data body.
	void setBody( long *body, long length) { m_body = body; 
	                                         m_header.length = length + sizeof(m_header)/sizeof(UINT32);}
	//! Accessor function to get pointer to data body.
	long* getBody() const {return m_body; }
	//! Accessor function to get size of data body.
	long getBodySize() const {return (m_header.length - sizeof(m_header)/sizeof(UINT32));}
 
        //! Figure checksum
        long checkSum();

        //! Copy Whole Primitive structure into Buffer
        long copyToBuffer(long unsigned int *buffer);

private:
    //! A primitive header structure 	
        MsgHead m_header;
    //! A pointer to the body of the primitive. Storage is allocated in the calling routine, do
    //! not delete the body from within this class. Watch for memory leaks!!!
	long *m_body;

};                                    // End of RodPrimitive declaration
} // End namespace SctPixelRod

#endif  // SCTPIXELROD_RodPrimitive_H
