#ifndef VTKWEDGE_INCLUDE
#define VTKWEDGE_INCLUDE

#include "Cells/VTKCell.h"

#ifdef __cplusplus
extern "C"{
    namespace sereno
    {
#endif
        DllExport void      VTKWedge_fillBuffer(void* pts, VTKValueFormat ptsFormat, int32_t* cellPts, void* buffer, VTKValueFormat destFormat);
        DllExport void VTKWedge_fillElementBuffer(int32_t* cellPts, int32_t* buffer);
        DllExport uint32_t  VTKWedge_sizeBuffer(int32_t* cellPts);
        DllExport VTKGLMode VTKWedge_getMode();
        DllExport int32_t   VTKWedge_nbPoints();

        extern const VTKCellVT vtkWedge;
#ifdef __cplusplus
    }
}
#endif

#endif
