#ifndef  VTKCELL_INC
#define  VTKCELL_INC

#include "VTKParser_C_type.h"

#ifdef __cplusplus
extern "C"{
    namespace sereno
    {
#endif
        /**
         * \brief  Fill the vertex buffer "buffer"
         *
         * \param pts the points data
         * \param ptsFormat the format of the points data
         * \param cellPts the cells data (numberOfPoints, indiceOfPoints1, etc.)
         * \param buffer the buffer to write
         * \param destFormat the format of the destination buffer
         */
        typedef void     (*VTKCELL_FILLBUFFER)(void* pts, VTKValueFormat ptsFormat, int32_t* cellPts, void* buffer, VTKValueFormat destFormat);

        /**
         * \brief  Fill the element buffer (indices of points)
         *
         * \param cellPts the cell point values
         * \param buffer the buffer to write
         */
        typedef void     (*VTKCELL_FILLELEMENTBUFFER)(int32_t* cellPts, int32_t* buffer);

        /**
         * \brief  Get the size of the buffer needed for this cell (regardless of number of components per points)
         * \param cellPts the cell points values
         * \return   the size of the buffer (regardless the number of components per points and the value format needed)
         */
        typedef uint32_t (*VTKCELL_SIZEBUFFER)(int32_t* cellPts);

        /**
         * \brief  Get the OpenGL mode for this Cell
         * \return  The OpenGL rendering mode 
         */
        typedef VTKGLMode(*VTKCELL_GETMODE)();

        /**
         * \brief  Get the number of points needed for defining this cell
         * \return  The number of points 
         */
        typedef int32_t  (*VTKCELL_NBPOINTS)();

        /** \brief  VTK Cell virtual table. Each parameter is a function pointer to a service on the cell*/
        typedef struct VTKCellVT_
        {
            VTKCELL_FILLBUFFER        fillBuffer;        /*!< Fill the vertex buffer*/
            VTKCELL_FILLELEMENTBUFFER fillElementBuffer; /*!< Fill the element buffer (ID of the points)*/
            VTKCELL_SIZEBUFFER        sizeBuffer;        /*!< Get the size of the buffer */
            VTKCELL_GETMODE           getMode;           /*!< Get the rendering mode of this cell*/
            VTKCELL_NBPOINTS          nbPoints;          /*!< Get the number of points per cell. -1 if no limits*/
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
