// 3DObject.cpp: implementation of the C3DObject class.
//
//////////////////////////////////////////////////////////////////////
#include "3DObject.h"
#include <iostream>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C3DObject::C3DObject()
{
	m_pModel = NULL;
}

C3DObject::~C3DObject()
{

}
/*
void drawmodel(void)
{
    if (!pmodel) {
        pmodel = glmReadOBJ("data/al.obj");
        if (!pmodel) exit(0);
        glmUnitize(pmodel);
        glmFacetNormals(pmodel);
        glmVertexNormals(pmodel, 90.0);
    }
    
    glmDraw(pmodel, GLM_SMOOTH | GLM_MATERIAL);
}
*/
