#ifndef  VTKPARSER_C_TYPE_INC
#define  VTKPARSER_C_TYPE_INC

#include <stdint.h>
#include <stdlib.h>

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


namespace sereno
{

#define ENUM_VTK_VALUE_FORMAT(_)   \
    _(VTK_INT            , = 0, 4) \
    _(VTK_DOUBLE         ,    , 8) \
    _(VTK_FLOAT          ,    , 4) \
    _(VTK_NO_VALUE_FORMAT,    , 0)

    VTK_DEFINE_ENUM(VTKValueFormat, ENUM_VTK_VALUE_FORMAT)
    VTK_AS_DEC_INT (VTKValueFormat, ENUM_VTK_VALUE_FORMAT)

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
		VTK_AS_DEC_INT(VTKCellType,  ENUM_VTK_CELL_TYPE)

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
        VTK_DATASET_TYPE_NONE
    };

    /* \brief Points positions */
    struct VTKPointPositions
    {
        uint32_t       nbPoints; /*!< Number of points*/
        VTKValueFormat format;   /*!< Point format*/
        size_t         offset;   /*!< Offset in the file (memory mapping !)*/
    };

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
}

#endif
