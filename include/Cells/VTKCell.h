#ifndef  VTKCELL_INC
#define  VTKCELL_INC

#include "VTKParser_C_type.h"

#ifdef __cplusplus
extern "C"{
    namespace sereno
    {
#endif
        typedef void     (*VTKCELL_FILLBUFFER)(void* pts, VTKValueFormat ptsFormat, int32_t* cellPts, void* buffer, VTKValueFormat destFormat);
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

#define VTK_CELL_FILL_PTS_BUFFER(t1)                                        \
    for(uint32_t i = 0; i < 3; i++)                                         \
        switch(destFormat)                                                  \
        {                                                                   \
            case VTK_INT:                                                   \
                ((int*)buffer)[i+bufferOffset] = ((t1*)pts)[i+ptsOffset];   \
                break;                                                      \
            case VTK_FLOAT:                                                 \
                ((float*)buffer)[i+bufferOffset] = ((t1*)pts)[i+ptsOffset]; \
                break;                                                      \
            case VTK_DOUBLE:                                                \
                ((double*)buffer)[i+bufferOffset] = ((t1*)pts)[i+ptsOffset];\
                break;                                                      \
            default:                                                        \
                break;                                                      \
        }                                                                   \

        inline void VTKCell_fillPtsBuffer(void* pts, VTKValueFormat ptsFormat, void* buffer, int32_t ptsOffset, int32_t bufferOffset, VTKValueFormat destFormat)
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
