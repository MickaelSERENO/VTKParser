#ifndef  VTKPARSER_INC
#define  VTKPARSER_INC

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif
#include <sys/stat.h>
#include <regex>

#include "VTKParser_C_type.h"
#include "Cells/VTKCell.h"
#include "Cells/VTKWedge.h"

namespace sereno
{
    /* \brief The different VTK File part */
    enum VTKFilePart
    {
        VTK_VERSION,
        VTK_HEADER,
        VTK_FILE_FORMAT,
        VTK_DATASET_STRUCTURE,
        VTK_DATASET_ATTRIBUTES
    };

    /** \brief  Possess FieldValue MetaData */
    struct FieldValueMetaData
    {
        std::string    name;            /*!< The field name*/
        VTKValueFormat format;          /*!< The field format value*/
        uint32_t       nbTuples;        /*!< Number of tuples*/
        uint32_t       nbValuePerTuple; /*!< Number of values per tuple*/
    };

    /** \brief  Value per value equal operator
     * \param l the left operator to test
     * \param r the right operator to test
     * \return   true if they contain the same metadata, false otherwise */
    inline bool operator==(const FieldValueMetaData& l, const FieldValueMetaData& r)
    {
        return l.name            == r.name     && 
               l.format          == r.format   && 
               l.nbTuples        == r.nbTuples &&
               l.nbValuePerTuple == r.nbValuePerTuple;
    }

    /** \brief  Value per value different operator
     * \param l the left operator to test
     * \param r the right operator to test
     * \return   false if they contain the same metadata, true otherwise */
    inline bool operator!=(const FieldValueMetaData& l, const FieldValueMetaData& r)
    {
        return !(l == r);
    }

    /** \brief  Represents one value in the field list */
    struct VTKFieldValue : public FieldValueMetaData
    {
        size_t         offset;          /*!< Offset (in bytes) for reading the field value data*/
    };

    /* \brief The VTK Field structure */
    struct VTKFieldData
    {
        std::string                name;   /*!< The field name*/
        std::vector<VTKFieldValue> values; /*!< The values*/
    };

    /** \brief  A VTK Value */
    struct VTKValue
    {
        /** \brief  Default constructor. No type associated */
        VTKValue() : type(VTK_NO_VALUE_TYPE)
        {}

        /**
         * \brief  Copy constructor
         *
         * \param v the value to copy
         */
        VTKValue(const VTKValue& v) : type(VTK_NO_VALUE_TYPE)
        {
            setType(v.type);
            switch(type)
            {
                case VTK_FIELD_DATA:
                    fieldData = v.fieldData;
                    break;
                default:
                    break;
            }
        }

        ~VTKValue()
        {
            setType(VTK_NO_VALUE_TYPE);
        }

        /**
         * \brief  Switch the type of this value
         *
         * \param t the new type
         */
        void setType(VTKValueType t)
        {
            //Delete
            switch(type)
            {
                case VTK_FIELD_DATA:
                    fieldData.~VTKFieldData();
                    break;
                default:
                    break;
            }
            type = t;
            //Construct
            switch(type)
            {
                case VTK_FIELD_DATA:
                    new(&fieldData) VTKFieldData;
                    break;
                default:
                    break;
            }
        }

        VTKValueType type; /*!< The type of this value */
        union
        {
            VTKFieldData fieldData; /*!< FIELD DATA value*/
        };
    };

    /* \brief The VTK Data (point or cell data)*/
    struct VTKData
    {
        uint32_t              n;      /*!< Number of of data*/
        std::vector<VTKValue> values; /*!< Associated values*/
    };

    /**
     * \brief  Read VTK Value in the source file. If you have used "parsedAll*" function, use readParsedVTKValue instead
     *
     * @tparam T the type to convert (int, float, double, ...)
     * \param val pointer to the value to convert
     * \param format the value format
     *
     * \return   the value with T type
     */
    template <typename T>
    inline T readVTKValue(void* val, VTKValueFormat format)
    {
        uint8_t* v = (uint8_t*)val;
        switch(format)
        {
            case VTK_INT:
            {
                return (v[0] << 24) + (v[1] << 16) + 
                       (v[2] << 8 ) + v[3];
            }
            case VTK_DOUBLE:
            {
                static_assert(sizeof(double) == sizeof(uint64_t), "Double size differt from uint64_t size...");
                union
                {
                    uint64_t _i;
                    double _d;
                }t; 
                t._i = ((uint64_t)v[0] << 56) + ((uint64_t)v[1] << 48) +
                       ((uint64_t)v[2] << 40) + ((uint64_t)v[3] << 32) +
                       ((uint64_t)v[4] << 24) + ((uint64_t)v[5] << 16) +
                       ((uint64_t)v[6] << 8 ) + v[7];
                return t._d;
            }
            case VTK_FLOAT:
            {
                uint32_t t = (v[0] << 24) + (v[1] << 16) + 
                             (v[2] << 8 ) + v[3];
                return *((float*)(&t));
            }
            case VTK_UNSIGNED_CHAR:
            case VTK_CHAR:
                return v[0];
            default:
                return 0;
        }
    }

    /**
     * \brief  Read VTK Value once parsed (after "parsedAll*" functions)
     *
     * @tparam T the type to convert (int, float, double, ...)
     * \param val pointer to the value to convert
     * \param format the value format
     *
     * \return   the value with T type
     */
    template <typename T>
    inline T readParsedVTKValue(void* val, VTKValueFormat format)
    {
        switch(format)
        {
            case VTK_INT:
                return *reinterpret_cast<uint32_t*>(val);
            case VTK_DOUBLE:
                return *reinterpret_cast<double*>(val);
            case VTK_FLOAT:
                return *reinterpret_cast<float*>(val);
            case VTK_UNSIGNED_CHAR:
            case VTK_CHAR:
                return *reinterpret_cast<uint8_t*>(val);
            default:
                return 0;
        }
    }

    /* \brief VTKParser class. Only support right now STRUCTURED_GRID and BINARY */
    struct DllExport VTKParser
    {
        public:
            /* \brief Constructor
             * \param path the .vtk file path */
            VTKParser(const std::string& path);

            /* \brief Movement Constructor
             * \param mvt the object to move */
            VTKParser(VTKParser&& mvt);

            /* \brief Destructor */
            ~VTKParser();

            /* \brief  Get the file path opened
             * \return  the file path opened */
            const std::string& getPath() const {return m_path;}

            /* \brief Close the VTK Parser */
            void closeParser();

            /* \brief Parse the file. Here, no "real data" is stored : we only get the file structures (fields, etc.)
             * \return true on success, false on faillure */
            bool parse();

            /* \brief Parse all the unstructured grid point.
             * \return a buffer containing the points value. Verify the point type before casting ! Need to be free (using free) */
            void* parseAllUnstructuredGridPoints() const; 

            /**
             * \brief Get the cells values
             * \return data of the cell section (CELLS).
             * These information tells you what points are related to the cells.
             * This has to be combined using parseAllUnstructuredGridCellTypes
             */
            int32_t* parseAllUnstructuredGridCellsComposition() const;

            /**
             * \brief Get the cells types.
             * \return data of the cell_type section (CELL_TYPES).
             * These information tells you how to combine the points given by parseAllUnstructuredGridCellsComposition function*/
            int32_t* parseAllUnstructuredGridCellTypes() const;

            /**
             * \brief  Get the field names present in the point data
             * \return  a list of field names 
             */
            std::vector<std::string> getPointFieldValueNames() const;

            /**
             * \brief Get the field data descriptors for point data
             * \return   a list containing pointer to field data descriptors
             */
            std::vector<const VTKFieldValue*> getPointFieldValueDescriptors() const;

            /**
             * \brief  Get the field names present in the cell data
             * \return   a list of field names
             */
            std::vector<std::string> getCellFieldValueNames() const;

            /**
             * \brief Get the field data descriptors for cell data
             * \return   a list containing pointer to field data descriptors
             */
            std::vector<const VTKFieldValue*> getCellFieldValueDescriptors() const;

            /**
             * \brief  Get the field data internal values for cell data
             *
             * \return a pointer to the field data. Needs to be free (using free). Use fieldData.type to know how to cast this object. Contains nbTuples*nbValuePerTuple*VTKValueFormatInt(format) bytes.
             */
            void* parseAllFieldValues(const VTKFieldValue* fieldData) const;

            /**
             * \brief  Get the dataset type of this VTK object
             * \return   the dataset type
             */
            VTKDatasetType getDatasetType() const {return m_type;}

            /**
             * \brief  Get the structured points descriptor
             * \return   a pointer to the structured points descriptor
             */
            const VTKStructuredPoints& getStructuredPointsDescriptor() const {return m_strPoints;}

            /**
             * \brief  Get the dataset unstructured grid point descriptor
             * \return the point descriptor
             */
            VTKPointPositions getUnstructuredGridPointDescriptor() const {return m_unstrGrid.ptsPos;}

            /**
              * \brief Get the dataset unstructured grid cell type descriptor
              * \return the cell type descriptor*/
            VTKCellTypes getUnstructuredGridCellTypesDescriptor() const { return m_unstrGrid.cellTypes; }

            /**
              * \brief Get the dataset unstructured grid cell type descriptor
              * \return the cells descriptor*/
            VTKCells getUnstructuredGridCellDescriptor() const { return m_unstrGrid.cells; }

            /**
             * \brief  get the rendering unstructured cell buffer
             *
             * \param nbCells the number of cells to get
             * \param ptValues the points values
             * \param cellValues the cell values
             * \param cellTypes the cell types
             * \param buffer the out buffer
             * \param destFormat the destination format. put VTK_NO_VALUE_TYPE if you want the points values format
             */
            void fillUnstructuredGridCellBuffer(uint32_t nbCells, void* ptValues, int32_t* cellValues, int32_t* cellTypes, void* buffer, VTKValueFormat destFormat = VTK_NO_VALUE_FORMAT);

            /**
             * \brief Fill the unstructured grid cell element buffer 
             * \param nbCells the number of cells to use
             * \param cellValues the cell Values
             * \param cellTypes the cell Types
             * \param buffer the buffer to fill*/
            void fillUnstructuredGridCellElementBuffer(uint32_t nbCells, int32_t* cellValues, int32_t* cellTypes, int32_t* buffer);

            /**
             * \brief Get the cell construction descriptor. It the type needed to render the dataset changed, this function returns before having parsed everything
             * \param nbCells    the number of cells to read
             * \param ptValues   the point values (see parseAllUnstructuredGridPoints)
             * \param cellValues the cell values (see parseAllUnstructuredGridCells)
             * \param cellTypes  the cell types (see parseAllUnstructuredGridCellTypes)
             * \return a VTKCellConstruction telling the buffer size and the advancement for the next datasets
             */
            static VTKCellConstruction getCellConstructionDescriptor(uint32_t nbCells, int32_t* cellValues, int32_t* cellTypes);
        private:
            VTKParser(const VTKParser& copy);
            VTKParser& operator=(const VTKParser& copy);

            /* \brief Parse the unstructured part of the file
             * \param file the file to read
             * \return false on error, true on success */
            bool parseUnstructuredGrid(FILE* file);

            /* \brief Parse the structured grid part of the file
             * \param file the file to read
             * \return false on error, true on success */
            bool parseStructuredGrid(FILE* file);

            /* \brief Parse the structured points part of the file
             * \param file the file to read
             * \return false on error, true on success */
            bool parseStructuredPoints(FILE* file);

            /**
             * \brief  Parse values (points / cells values)
             * \param file the file to read
             * \param data the data to update
             * \return true on success, false on failure
             */
            bool parseValues(FILE* file, VTKData& data);
            
            /**
             * \brief  Parse a metadata block
             * \param file the file to read
             * \return false on error, true on success
             */
            bool parseMetadata(FILE* file);

            void* getAllBinaryValues(size_t offset, uint32_t nbValues, VTKValueFormat format) const;

            /**
             * \brief  Convert a VTK String to a VTKValueFormat (int, double, etc.)
             * \param str the string to convert
             * \return the value format
             */
            static VTKValueFormat vtkStringToFormat(const std::string& str);

            VTKDatasetType m_type;                 /*!< The dataset type*/
            union
            {
                VTKStructuredPoints m_strPoints;   /*!< Structured Point*/
                VTKGrid             m_grid;        /*!< Grid dataset*/
                VTKUnstructuredGrid m_unstrGrid;   /*!< Unstructured grid dataset*/
            };

            VTKData  m_cellData;                   /*!< The cell data value*/
            VTKData  m_ptsData;                    /*!< The point data values*/

            VTKFileFormat         m_fileFormat;    /*!< The file format (BINARY or ASCII)*/

            std::string m_path;                    /*!< The dataset path*/

            uint32_t    m_minorVer = 0;            /*!< The minor version used*/
            uint32_t    m_majorVer = 0;            /*!< The major version used*/
            std::string m_header;
            FILE*       m_file = NULL;                 /*!< The VTK file descriptor*/

            //The regexes
            static const std::regex versionRegex;      /*!< Regex checking the VERSIONing*/
            static const std::regex datasetRegex;      /*!< Regex checking the DATASET name*/
            static const std::regex pointsRegex;       /*!< Regex checking the POINTS     (unstructured grid)*/
            static const std::regex cellsRegex;        /*!< Regex checking the CELLS      (unstructured grid)*/
            static const std::regex cellTypesRegex;    /*!< Regex checking the CELL_TYPES (unstructured grid)*/
            static const std::regex dimensionsRegex;   /*!< Regex checking the DIMENSIONS (structured points)*/
            static const std::regex spacingRegex;      /*!< Regex checking the SPACING    (structured points)*/
            static const std::regex originRegex;       /*!< Regex checking the ORIGIN     (structured points)*/
            static const std::regex pointDataRegex;    /*!< Regex checking the POINT_DATA */
            static const std::regex cellDataRegex;     /*!< Regex checking the CELL_DATA */
            static const std::regex informationRegex;  /*!< Regex checking the INFORMATION (METADATA)*/
            static const std::regex dataRegex;         /*!< Regex checking the DATA information (METADATA)*/
            static const std::regex nameLocationRegex; /*!< Regex checking the NAME <word> LOCATION <word> (METADATA)*/
            static const std::regex fieldRegex;        /*!< Regex checking the FIELD*/
            static const std::regex fieldValueRegex;   /*!< Regex checking the FIELD VALUE (field)*/
    };
}

#endif
