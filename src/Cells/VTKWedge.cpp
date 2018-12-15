#include "Cells/VTKWedge.h"

namespace sereno
{
    const VTKCellVT vtkWedge = {VTKWedge_fillBuffer, VTKWedge_fillElementBuffer, VTKWedge_sizeBuffer, VTKWedge_getMode, VTKWedge_nbPoints};

    void VTKWedge_fillBuffer(void* pts, VTKValueFormat ptsFormat, int32_t* cellPts, void* buffer, VTKValueFormat destFormat)
    {
        //Front triangle
        for (uint32_t i = 0; i < 3; i++)
            VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[i+1]*3, 3*i, destFormat);

        //Back triangle
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+3]*3, 3*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+5]*3, 4*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+4]*3, 5*3, destFormat);
                                                                    
        //Left rectangle                                            
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+3]*3, 6*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+0]*3, 7*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+2]*3, 8*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+3]*3, 9*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+2]*3, 10*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+5]*3, 11*3, destFormat);
                                                                    
        //Right rectangle                                            
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+1]*3, 12*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+4]*3, 13*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+2]*3, 14*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+4]*3, 15*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+5]*3, 16*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+2]*3, 17*3, destFormat);

        //Bottom rectangle
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+3]*3, 18*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+0]*3, 19*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+1]*3, 20*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+3]*3, 21*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+1]*3, 22*3, destFormat);
        VTKCell_fillPtsBuffer(pts, ptsFormat, buffer, cellPts[1+4]*3, 23*3, destFormat);
    }

    void VTKWedge_fillElementBuffer(int32_t* cellPts, int32_t* buffer)
    {
        for(uint32_t i = 0; i < 3; i++)
            buffer[i] = cellPts[i+1];

        //Back triangle
        buffer[3] = cellPts[1+3];
        buffer[4] = cellPts[1+5];
        buffer[5] = cellPts[1+4];
                                                                    
        //Left rectangle                                            
        buffer[6] = cellPts[1+3];
        buffer[7] = cellPts[1+0];
        buffer[8] = cellPts[1+2];
        buffer[9] = cellPts[1+3];
        buffer[10] = cellPts[1+2];
        buffer[11] = cellPts[1+5];
                                                                    
        //Right rectangle                                            
        buffer[12] = cellPts[1+1];
        buffer[13] = cellPts[1+4];
        buffer[14] = cellPts[1+2];
        buffer[15] = cellPts[1+4];
        buffer[16] = cellPts[1+5];
        buffer[17] = cellPts[1+2];

        //Bottom rectangle
        buffer[18] = cellPts[1+3];
        buffer[19] = cellPts[1+0];
        buffer[20] = cellPts[1+1];
        buffer[21] = cellPts[1+3];
        buffer[22] = cellPts[1+1];
        buffer[23] = cellPts[1+4];
    }

    uint32_t VTKWedge_sizeBuffer(int32_t* cellPts)
    {
        return 8*3;
    }

    VTKGLMode VTKWedge_getMode()
    {
        return VTK_GL_TRIANGLES;
    }

    int32_t VTKWedge_nbPoints()
    {
        return 6;
    }
}
