#include "Cells/VTKWedge.h"

namespace sereno
{
	const VTKCellVT vtkWedge = {VTKWedge_fillBuffer, VTKWedge_sizeBuffer, VTKWedge_getMode, VTKWedge_nbPoints};

	void VTKWedge_fillBuffer(void* pts, VTKValueFormat ptsFormat, int32_t* cellPts, void* buffer)
	{
		//Front triangle
		for (uint32_t i = 0; i < 3; i++)
			VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[i]*3, (3+1)*i);

		//Back triangle
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[3]*3, (1+3)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[5]*3, (1+4)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[4]*3, (1+5)*3);
																	
		//Left rectangle											
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[3]*3, (1+6)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[0]*3, (1+7)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[2]*3, (1+8)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[3]*3, (1+9)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[2]*3, (1+10)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[5]*3, (1+11)*3);
																	
		//Right rectangle											
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1]*3, (1+12)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[4]*3, (1+13)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[2]*3, (1+14)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[4]*3, (1+15)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[5]*3, (1+16)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[2]*3, (1+17)*3);

		//Bottom rectangle
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[3]*3, (1+18)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[0]*3, (1+19)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1]*3, (1+20)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[3]*3, (1+21)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1]*3, (1+22)*3);
		VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[4]*3, (1+23)*3);
	}

	uint32_t VTKWedge_sizeBuffer(int32_t* cellPts)
	{
		return 8*3*3;
	}

	VTKGLMode VTKWedge_getMode()
	{
		return VTK_GL_TRIANGLES;
	}

	int32_t   VTKWedge_nbPoints()
	{
		return 8;
	}
}