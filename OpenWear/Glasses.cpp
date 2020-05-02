#include "Glasses.h"
#include <iostream>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
CGlasses::CGlasses()
{
}


CGlasses::~CGlasses()
{
}

void CGlasses::ReadData()
{
	if (!m_pModel)
	{
		m_pModel = glmReadOBJ("model/glasses.obj");
		if (!m_pModel) exit(0);
		glmUnitize(m_pModel);
		glmFacetNormals(m_pModel);
		glmVertexNormals(m_pModel, 90.0);
	}
}

void CGlasses::Draw()
{
	if (m_pModel)
		glmDraw(m_pModel, GLM_SMOOTH | GLM_MATERIAL);
}