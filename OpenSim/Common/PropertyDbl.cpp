// PropertyDbl.cpp
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
* Copyright (c)  2005, Stanford University. All rights reserved. 
* Use of the OpenSim software in source form is permitted provided that the following
* conditions are met:
* 	1. The software is used only for non-commercial research and education. It may not
*     be used in relation to any commercial activity.
* 	2. The software is not distributed or redistributed.  Software distribution is allowed 
*     only through https://simtk.org/home/opensim.
* 	3. Use of the OpenSim software or derivatives must be acknowledged in all publications,
*      presentations, or documents describing work in which OpenSim or derivatives are used.
* 	4. Credits to developers may not be removed from executables
*     created from modifications of the source.
* 	5. Modifications of source code must retain the above copyright notice, this list of
*     conditions and the following disclaimer. 
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
*  SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR BUSINESS INTERRUPTION) OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Note: This code was originally developed by Realistic Dynamics Inc. 
 * Author: Frank C. Anderson 
 */


//============================================================================
// INCLUDES
//============================================================================
#include "PropertyDbl.h"
#include <cstdio>




using namespace OpenSim;
using namespace std;

//=============================================================================
// CONSTRUCTOR(S)
//=============================================================================
//_____________________________________________________________________________
/**
 * Constructor.
 */
PropertyDbl::
PropertyDbl(const string &aName,double aValue) :
	Property(Property::Dbl,aName)
{
	_value = aValue;
}
//_____________________________________________________________________________
/**
 * Default constructor.
 */
PropertyDbl::
PropertyDbl() :
	Property(Property::Dbl,"DblPropertyName")
{
	_value = 0.0;
}
//_____________________________________________________________________________
/**
 * Copy constructor.
 *
 * @param aProperty Property to be copied.
 */
PropertyDbl::PropertyDbl(const PropertyDbl &aProperty) :
	Property(aProperty)
{
	_value = aProperty.getValueDbl();
}
//_____________________________________________________________________________
/**
 * Construct and return a copy of this property.
 * The property is allocated using the new operator, so the caller is
 * responsible for deleting the returned object.
 *
 * @return Copy of this property.
 */
Property* PropertyDbl::copy() const
{
	Property *property = new PropertyDbl(*this);
	return(property);
}


//=============================================================================
// OPERATORS
//=============================================================================
//-----------------------------------------------------------------------------
// ASSIGNMENT
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Assign this property to another.
 *
 * @param aProperty Property to which to assign this property.
 * @return Reference to this property.
 */
PropertyDbl& PropertyDbl::
operator=(const PropertyDbl &aProperty)
{
	Property::operator =(aProperty);
	_value = aProperty.getValueDbl();
	return(*this);
}


//=============================================================================
// GET AND SET
//=============================================================================
//-----------------------------------------------------------------------------
// TYPE AS STRING
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Get the type of this property as a string.
 *
 * @return Type of the property.
 */
const char* PropertyDbl::
getTypeAsString() const
{
	return("Dbl");
}

//-----------------------------------------------------------------------------
// VALUE
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Set the value of this property.
 *
 * @param aValue Value to which this property is assigned.
 */
void PropertyDbl::
setValue(double aValue)
{
	_value = aValue;
}
//_____________________________________________________________________________
/**
 * Get a reference to the value of this property.  Note that the returned
 * reference can be used to change the value of this property.
 *
 * @return Reference to the value of this property.
 */
double& PropertyDbl::
getValueDbl()
{
	return(_value);
}
//_____________________________________________________________________________
/**
 * Get a constant reference to the value of this property.
 *
 * @return Constant reference to the value of this property.
 */
const double& PropertyDbl::
getValueDbl() const
{
	return(_value);
}
//_____________________________________________________________________________
/**
 * Get a constant String represeting the value of this property.
 *
 * @return Constant String represeting the value of this property.
 */
const string &PropertyDbl::
toString()
{
	char dbl[256];
	sprintf(dbl, "%g", _value);
	_valueString = string(dbl);
	return (_valueString);
}
