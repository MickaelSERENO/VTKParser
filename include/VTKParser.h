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

    /** \brief  Represents one value in the field list */
    struct VTKFieldValue
    {
        std::string    name;            /*!< The field name*/
        VTKValueFormat format;          /*!< The field format value*/
        uint32_t       nbTuples;        /*!< Number of tuples*/
        uint32_t       nbValuePerTuple; /*!< Number of values per tuple*/
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
            void* parseAllCellFieldValues(const VTKFieldValue* fieldData) const;

            /**
             * \brief  Get the dataset type of this VTK object
             * \return   the dataset type
             */
            VTKDatasetType getDatasetType() const {return m_type;}

            /**
             * \brief  Get the dataset unstructured grid point descriptor
             * \return the point descriptor
             */
            VTKPointPositions getUnstructuredGridPointDescriptor() const {return m_unstrGrid.ptsPos;}

			/**
			  * \brief Get the dataset unstructured grid cell type descriptor
			  * \return the cell type descriptor*/
			VTKCellTypes getUnStructuredGridCellTypesDescriptor() const { return m_unstrGrid.cellTypes; }

			/**
			  * \brief Get the dataset unstructured grid cell type descriptor
			  * \return the cells descriptor*/
			VTKCells getUnStructuredGridCellDescriptor() const { return m_unstrGrid.cells; }

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

            /* \brief Parse the structured part of the file
             * \param file the file to read
             * \return false on error, true on success */
            bool parseStructuredGrid(FILE* file);

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

            /** \brief  Read a int value from a VTK buffer
             * \param buf the buffer to convert
             * \return  the int value*/
            static int    readInt(uint8_t* buf);

            /** \brief  Read a float value from a VTK buffer
             * \param buf the buffer to convert
             * \return  the float value */
            static float  readFloat(uint8_t* buf);

            /** \brief  Read a double value from a VTK buffer
             * \param buf the buffer to convert
             * \return  the double value */
            static double readDouble(uint8_t* buf);

            /**
             * \brief  Convert a VTK String to a VTKValueFormat (int, double, etc.)
             * \param str the string to convert
             * \return the value format
             */
            static VTKValueFormat vtkStringToFormat(const std::string& str);

            VTKDatasetType m_type;                 /*!< The dataset type*/
            union
            {
                VTKGrid             m_grid;        /*!< Grid dataset*/
                VTKUnstructuredGrid m_unstrGrid;   /*!< Unstructured grid dataset*/
            };

            VTKData  m_cellData;                   /*!< The cell data value*/
            VTKData  m_ptsData;                    /*!< The point data values*/

            VTKFileFormat         m_fileFormat;    /*!< The file format (BINARY or ASCII)*/

            uint32_t    m_minorVer = 0;            /*!< The minor version used*/
            uint32_t    m_majorVer = 0;            /*!< The major version used*/
            std::string m_header;
#ifdef WIN32
			HANDLE      m_fd   = INVALID_HANDLE_VALUE; /*!< The Windows Handle object*/
			FILE*       m_file = NULL;                 /*!< The VTK file descriptor*/
#else
            int         m_fd       = -1;           /*!< The VTK file descriptor*/
            void*       m_mmapData = MAP_FAILED;   /*!< The memory mapping associated with the opened file*/
			uint32_t    m_fileLen = 0;             /*!< Record of the file length in bytes*/
#endif

            //The regexes
            static const std::regex versionRegex;      /*!< Regex checking the VERSIONing*/
            static const std::regex datasetRegex;      /*!< Regex checking the DATASET name*/
            static const std::regex pointsRegex;       /*!< Regex checking the POINTS     (unstructured grid)*/
            static const std::regex cellsRegex;        /*!< Regex checking the CELLS      (unstructured grid)*/
            static const std::regex cellTypesRegex;    /*!< Regex checking the CELL_TYPES (unstructured grid)*/
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
