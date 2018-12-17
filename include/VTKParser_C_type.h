#ifndef  VTKPARSER_C_TYPE_INC
#define  VTKPARSER_C_TYPE_INC

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define DllExport   __declspec( dllexport )
#else
#define DllExport
#endif

#define VTK_ENUM_BODY_STR(name, value, str)     \
    name value,

#define VTK_AS_STRING_CASE_STR(name, value, str)\
    case name: return str;

#define VTK_DEFINE_ENUM(name, list)             \
    typedef enum name                           \
    {                                           \
        list(VTK_ENUM_BODY_STR)                 \
    }name;

#define VTK_AS_DEC_INT(name, list)              \
    static inline int name##Int(name n)         \
    {                                           \
        switch (n)                              \
        {                                       \
            list(VTK_AS_STRING_CASE_STR)        \
            default:                            \
            {                                   \
                return 0;                       \
            }                                   \
        }                                       \
    }


#ifdef __cplusplus
    namespace sereno
    {
#endif

#define ENUM_VTK_VALUE_FORMAT(_)   \
    _(VTK_INT            , = 0, 4) \
    _(VTK_DOUBLE         ,    , 8) \
    _(VTK_FLOAT          ,    , 4) \
    _(VTK_UNSIGNED_CHAR  ,    , 1) \
    _(VTK_CHAR           ,    , 1) \
    _(VTK_NO_VALUE_FORMAT,    , 0)

        VTK_DEFINE_ENUM(VTKValueFormat, ENUM_VTK_VALUE_FORMAT)
        VTK_AS_DEC_INT(VTKValueFormat, ENUM_VTK_VALUE_FORMAT)

#define ENUM_VTK_CELL_TYPE(_)                  \
    _(VTK_CELL_VERTEX              , = 1 , 1)  \
    _(VTK_CELL_POLYVERTEX          , = 2 , -1) \
    _(VTK_CELL_LINE                ,     , 2)  \
    _(VTK_CELL_POLYLINE            ,     , -1) \
    _(VTK_CELL_TRIANGLE            ,     , 3)  \
    _(VTK_CELL_TRIANGLE_STRIP      ,     , -1) \
    _(VTK_CELL_POLYGON             ,     , -1) \
    _(VTK_CELL_PIXEL               ,     , -1) \
    _(VTK_CELL_QUAD                ,     , 4)  \
    _(VTK_CELL_TETRA               ,     , 4)  \
    _(VTK_CELL_VOXEL               ,     , 8)  \
    _(VTK_CELL_HEXAHEDRON          ,     , 8)  \
    _(VTK_CELL_WEDGE               ,     , 6)  \
    _(VTK_CELL_PYRAMID             ,     , 5)  \
    _(VTK_CELL_QUADRATIC_EDGE      , = 21, 3)  \
    _(VTK_CELL_QUADRATIC_TRIANGLE  ,     , 6)  \
    _(VTK_CELL_QUADRATIC_QUAD      ,     , 8)  \
    _(VTK_CELL_QUADRATIC_TETRA     ,     , 10) \
    _(VTK_CELL_QUADRATIC_HEXAHEDRON,     , 20)

        VTK_DEFINE_ENUM(VTKCellType, ENUM_VTK_CELL_TYPE)
        VTK_AS_DEC_INT(VTKCellType, ENUM_VTK_CELL_TYPE)

        /* \brief The VTK file format (ascii / binary) */
        enum VTKFileFormat
        {
            VTK_BINARY,
            VTK_ASCII
        };

        /** \brief  The different VTK value type */
        enum VTKValueType
        {
            VTK_FIELD_DATA,
            VTK_NO_VALUE_TYPE
        };

        /* \brief The VTK Dataset type */
        enum VTKDatasetType
        {
            VTK_STRUCTURED_GRID,
            VTK_UNSTRUCTURED_GRID,
            VTK_STRUCTURED_POINTS,
            VTK_DATASET_TYPE_NONE
        };

        /*\brief The VTK GL Mode. Corresponds to what mode the data created for rendering should use
         * The names corresponds to GLenum mode*/
        enum VTKGLMode
        {
            VTK_GL_TRIANGLES,
            VTK_GL_TRIANGLE_STIP,
            VTK_GL_LINE_STRIP,
            VTK_GL_POINTS,
            VTK_GL_NO_MODE
        };

        /* \brief Points positions */
        struct VTKPointPositions
        {
            uint32_t       nbPoints; /*!< Number of points*/
            VTKValueFormat format;   /*!< Point format*/
            size_t         offset;   /*!< Offset in the file (memory mapping !)*/
        };

        /* \brief VTKCells descriptor */
        struct VTKCells
        {
            uint32_t nbCells;    /*!< The number of cells*/
            uint32_t wholeSize;  /*!< The whole size of the cell fields*/
            size_t   offset;     /*!< Offset in the file (memory mapping !)*/
        };

        /** \brief The VTK Cell types */
        struct VTKCellTypes
        {
            uint32_t nbCells;
            uint32_t offset;
        };

        /** \brief The VTKCellConstruction information*/
        struct VTKCellConstruction
        {
            uint32_t  size;    /*!< The size of the buffer*/
            VTKGLMode mode;    /*!< The rendering mode (corresponds to OpenGL mode)*/
            uint32_t  nbCells; /*!< The number of cell parsed*/
            uint32_t  next;    /*!< Tells the offset to apply for advancing in the cells array*/
            char      error;   /*!< Tells if an error occured (0 == false, 1 == true)*/
        };

        /* \brief VTK Grid structure */
        struct VTKGrid
        {
            uint32_t          size[3];   /*!< The size of the grid*/
            VTKPointPositions ptsPos;    /*!< The point position*/
        };

        /* \brief VTK Unstructured Grid */
        struct VTKUnstructuredGrid
        {
            VTKPointPositions ptsPos;    /*!< The point position*/
            VTKCells          cells;     /*!< The cells*/
            VTKCellTypes      cellTypes; /*!< The cell type values*/
        };

        /** \brief  VTK Structured Point */
        struct VTKStructuredPoints
        {
            uint32_t size[3];    /*!< The dimension of the "grid"*/
            double   spacing[3]; /*!< The spacing between points*/
            double   origin[3];  /*!< The origin (offset) of the points*/
        };

#ifdef __cplusplus
    }
}
#endif

#endif
