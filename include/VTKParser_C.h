#ifndef  VTKPARSER_C_INC
#define  VTKPARSER_C_INC

#include <stdlib.h>
#include <stdint.h>
#include "VTKParser_C_type.h"

namespace sereno
{
    struct VTKParser;
    struct VTKFieldValue;

    extern "C"
    {
        typedef VTKParser*           HVTKParser;
        typedef const VTKFieldValue* HVTKFieldValue;

        /**
         * \brief  Create a VTKParser C object
         * \param filePath the file to open
         * \return   the VTKParser C object newly created. Call VTKParser_delete at the end of this object lifetime
         */
        HVTKParser       VTKParser_new(const char* filePath);

        /**
         * \brief  Delete a VTKParser C object
         */
        void             VTKParser_delete(HVTKParser parser);

        /**
         * \brief  Parse the VTK object information
         * \param parser the parser containing the information
         * \return   1 on success, 0 otherwise
         */
        char             VTKParser_parse(HVTKParser parser);

        /**
         * \brief  Get all the cell field value descriptor
         * \param parser the parser containing the information
         * \param nb[out] the number of value in the created array
         * \return   an array of HVTKFieldValue. needs to be freed (free(val)). DO NOT FREE CELL VALUES !
         */
        HVTKFieldValue* VTKParser_getCellFieldValueDescriptors(HVTKParser parser, size_t* nb);

        /**
         * \brief  Get the dataset type of this VTK object
         * \param parser the parser containing the information
         * \return   the dataset type
         */
        enum VTKDatasetType VTKParser_getDatasetFormat(HVTKParser parser);

        /**
         * \brief  Get the dataset unstructured grid point descriptor
         * \param parser the parser containing the information
         * \return the point descriptor
         */
        struct VTKPointPositions VTKParser_getUnstructuredGridPointDescriptor(HVTKParser parser);

        /**
         * \brief  Parse all unstructured grid point. Use getDatasetFormat for knowing the correct format (unstructured, structured, etc.)
         * \param parser the parser containing the information
         * \return   allocated memory containing the point values. Needs to be freed (free(val)). getUnstructuredGridPointFormat for knowing towards which type the result has to be parsed to. Nbvalues : use getUnstructuredGridPointDescriptor function
         */
        void* VTKParser_parseAllUnstructuredGridPoints(HVTKParser parser);

        /**
         * \brief  Get the number of tuples from a VTKFieldValue descriptor object
         * \param value the descriptor
         * \return   the number of tuples contained in this descriptor
         */
        uint32_t VTKParser_getFieldNbTuples(HVTKFieldValue value);

        /**
         * \brief Get the number of value per tuple from a VTKFieldValue descriptor object
         * \param value the descriptor
         * \return the number of value per tuple contained in this descriptor
         */
        uint32_t VTKParser_getFieldNbValuesPerTuple(HVTKFieldValue value);

        /**
         * \brief  Get the value format of a VTKFieldValue descriptor
         * \param value the descriptor
         * \return   the value format
         */
        VTKValueFormat VTKParser_getFieldFormat(HVTKFieldValue value);

        /**
         * \brief  Get the name of a VTKFieldValue descriptor
         * \param value the descriptor
         * \return   the field value name
         */
        const char* VTKParser_getFieldName(HVTKFieldValue value);

        /**
         * \brief  Parse the field value
         * \param parser the parser containing the information
         * \param value the field value descriptor to get data from
         * \return   a pointer to the allocated memory containing the information. Use VTKParser_getCellFieldFormat, VTKParser_getCellFieldNbTuples and VTKParser_getCellFieldNbValuesPerTuple function to get this memory size (format*tuple*nbValuePerTuple). The value needs to be freed (using free)
         */
        void* VTKParser_parseAllCellFieldValues(HVTKParser parser, HVTKFieldValue value);

        /**
         * \brief  Free function calling "free"
         * \param data the data to free
         */
        void VTKParser_free(void* data);
    }
}

#endif
