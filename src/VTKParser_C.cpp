#include "VTKParser_C.h"
#include "VTKParser.h"

namespace sereno
{
    HVTKParser VTKParser_new(const char* filePath)
    {
        return new VTKParser(filePath);
    }

    void VTKParser_delete(HVTKParser ptr)
    {
        delete ptr;
    }

    char VTKParser_parse(HVTKParser parser)
    {
        return parser->parse();
    }

    enum VTKDatasetType VTKParser_getDatasetType(HVTKParser parser)
    {
        return parser->getDatasetType();
    }

    const VTKStructuredPoints* VTKParser_getStructuredPointsDescriptor(HVTKParser parser)
    {
        return &parser->getStructuredPointsDescriptor();
    }

    VTKPointPositions VTKParser_getUnstructuredGridPointDescriptor(HVTKParser parser)
    {
        return parser->getUnstructuredGridPointDescriptor();
    }

    struct VTKCellTypes VTKParser_getUnstructuredGridCellTypesDescriptor(HVTKParser parser)
    {
        return parser->getUnstructuredGridCellTypesDescriptor();
    }

    struct VTKCells VTKParser_getUnstructuredGridCellDescriptor(HVTKParser parser)
    {
        return parser->getUnstructuredGridCellDescriptor();
    }

    HVTKFieldValue* VTKParser_getPointFieldValueDescriptors(HVTKParser parser, size_t* nb)
    {
        std::vector<const VTKFieldValue*> values = parser->getPointFieldValueDescriptors();
        *nb = values.size();
        HVTKFieldValue* res = (HVTKFieldValue*)malloc(sizeof(HVTKFieldValue)*(*nb));
        memcpy(res, values.data(), (*nb)*sizeof(HVTKFieldValue));
        return res;
    }

    HVTKFieldValue* VTKParser_getCellFieldValueDescriptors(HVTKParser parser, size_t* nb)
    {
        std::vector<const VTKFieldValue*> values = parser->getCellFieldValueDescriptors();
        *nb = values.size();
        HVTKFieldValue* res = (HVTKFieldValue*)malloc(sizeof(HVTKFieldValue)*(*nb));
        memcpy(res, values.data(), (*nb)*sizeof(HVTKFieldValue));
        return res;
    }

    void* VTKParser_parseAllUnstructuredGridPoints(HVTKParser parser)
    {
        return parser->parseAllUnstructuredGridPoints();
    }

    int32_t* VTKParser_parseAllUnstructuredGridCellsComposition(HVTKParser parser)
    {
        return parser->parseAllUnstructuredGridCellsComposition();
    }

    int32_t* VTKParser_parseAllUnstructuredGridCellTypes(HVTKParser parser)
    {
        return parser->parseAllUnstructuredGridCellTypes();
    }

    uint32_t VTKParser_getFieldNbTuples(HVTKFieldValue value)
    {
        return value->nbTuples;
    }

    uint32_t VTKParser_getFieldNbValuesPerTuple(HVTKFieldValue value)
    {
        return value->nbValuePerTuple;
    }

    VTKValueFormat VTKParser_getFieldFormat(HVTKFieldValue value)
    {
        return value->format;
    }

    const char* VTKParser_getFieldName(HVTKFieldValue value)
    {
        return value->name.c_str();
    }

    void* VTKParser_parseAllFieldValues(HVTKParser parser, HVTKFieldValue value)
    {
        return parser->parseAllFieldValues(value);
    }

    void VTKParser_free(void* data)
    {
        free(data);
    }

    VTKCellConstruction VTKParser_getCellConstructionDescriptor(uint32_t nbCells, int32_t* cellValues, int32_t* cellTypes)
    {
        return VTKParser::getCellConstructionDescriptor(nbCells, cellValues, cellTypes);
    }

    void VTKParser_fillUnstructuredGridCellBuffer(HVTKParser parser, uint32_t nbCells, void* ptValues, int32_t* cellValues, int32_t* cellTypes, void* buffer, VTKValueFormat destFormat)
    {
        parser->fillUnstructuredGridCellBuffer(nbCells, ptValues, cellValues, cellTypes, buffer, destFormat);
    }

    void VTKParser_fillUnstructuredGridCellElementBuffer(HVTKParser parser, uint32_t nbCells, int32_t* cellValues, int32_t* cellTypes, int32_t* buffer)
    {
        parser->fillUnstructuredGridCellElementBuffer(nbCells, cellValues, cellTypes, buffer);
    }
}
