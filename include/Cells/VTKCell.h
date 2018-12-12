#ifndef  VTKCELL_INC
#define  VTKCELL_INC

#include "VTKParser_C_type.h"

#ifdef __cplusplus
extern "C"{
    namespace sereno
    {
#endif
        typedef void     (*VTKCELL_FILLBUFFER)(void* pts, VTKValueFormat ptsFormat, int32_t* cellPts, void* buffer);
        typedef uint32_t (*VTKCELL_SIZEBUFFER)(int32_t* cellPts);
        typedef VTKGLMode(*VTKCELL_GETMODE)();
        typedef int32_t  (*VTKCELL_NBPOINTS)();

        typedef struct VTKCellVT_
        {
            VTKCELL_FILLBUFFER fillBuffer;
            VTKCELL_SIZEBUFFER sizeBuffer;
            VTKCELL_GETMODE    getMode;
            VTKCELL_NBPOINTS   nbPoints;
        }VTKCellVT;

#define VTK_CELL_FILL_PTS_BUFFER(t) \
    for(uint32_t i = 0; i < 3; i++) \
        ((t*)buffer)[i+bufferOffset] = ((t*)pts)[i+ptsOffset];

        inline void VTKCell_fillPtsBuffer(void* pts, VTKValueFormat ptsFormat, void* buffer, int32_t ptsOffset, int32_t bufferOffset)
        {
            switch(ptsFormat)
            {
                case VTK_INT:
                    VTK_CELL_FILL_PTS_BUFFER(int)
                    break;
                case VTK_FLOAT:
                    VTK_CELL_FILL_PTS_BUFFER(float)
                    break;
                case VTK_DOUBLE:
                    VTK_CELL_FILL_PTS_BUFFER(double)
                    break;
                default:
                    break;
            }
        }
#undef VTK_CELL_FILL_PTS_BUFFER
#ifdef __cplusplus
    }
}
#endif

#endif
