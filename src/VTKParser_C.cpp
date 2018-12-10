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

    VTKDatasetType VTKParser_getDatasetFormat(HVTKParser parser)
    {
        return parser->getDatasetType();
    }

    VTKPointPositions VTKParser_getUnstructuredGridPointDescriptor(HVTKParser parser)
    {
        return parser->getUnstructuredGridPointDescriptor();
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

    void* VTKParser_parseAllCellFieldValues(HVTKParser parser, HVTKFieldValue value)
    {
        return parser->parseAllCellFieldValues(value);
    }

    void VTKParser_free(void* data)
    {
        free(data);
    }
}
