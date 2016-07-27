// File: RodOutList.h

#ifndef SCTPIXELROD_RODOUTLIST_H
#define SCTPIXELROD_RODOUTLIST_H

#include <string>
#ifdef IBL
#include "iblPrimXface.h"
#endif
//#else
#include "primXface.h"
//#endif


namespace SctPixelRod {

/*!
 * @class RodOutList
 *
 * @brief  This is a class for ROD reply buffers.
 *
 * This class contains the reply lists which are responses from a primitive list.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

class RodOutList
{
	// These shouldn't be used so make them private
	RodOutList& operator=( const RodOutList& );         // Overload = operator
	RodOutList( const RodOutList& );                    // Copy constructor
public:
	RodOutList( long length ) ;                         // Constructor
	~RodOutList();                                      // Destructor

	// Accessor function to get length
	long getLength() const { return m_length; }

	// Accessor function to get pointer to data body.
	unsigned long* getBody() const {return m_body; }

	// Accessor function to get number of reply Messages.
	int nMsgs();
	
	// Accessor functions to Message n.
	// Accessor functions to Message Head n.
	ReplyHead* getMsgHead(int n);

	// Accessor functions to Length of Message n.
	int        getMsgLength(int n);

	// Accessor functions to Identifier of Message n.
	int        getMsgId(int n);

	// Accessor functions to MessageBody of Message n.
	void*      getMsgBody(int n);

private:
    //! The length of the reply list. 
	long m_length;
    //! A pointer to an array of UINT32 containing the reply data.
	unsigned long* m_body;

};                                    // End of RodOutList declaration
} // End namespace SctPixelRod

#endif  // SCTPIXELROD_RODOUTLIST_H
