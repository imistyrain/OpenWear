#pragma once
// Rose.h: interface for the CRose class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROSE_H__7F782AB5_290E_4D4F_B43E_169ACE0EA89C__INCLUDED_)
#define AFX_ROSE_H__7F782AB5_290E_4D4F_B43E_169ACE0EA89C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "3DObject.h"

class CGlasses : public C3DObject
{
public:
	CGlasses();
	virtual ~CGlasses();

	void ReadData();

	void Draw();

};

#endif // !defined(AFX_ROSE_H__7F782AB5_290E_4D4F_B43E_169ACE0EA89C__INCLUDED_)

