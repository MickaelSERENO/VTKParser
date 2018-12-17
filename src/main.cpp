#include "VTKParser_C.h"
#include <iostream>
#include <vector>

using namespace sereno;

int main(int argc, char* argv[])
{
    if(argc < 1)
    {
        std::cerr << "Not enough parameter.\n";
        return -1;
    }
    HVTKParser parser = VTKParser_new(argv[1]);
    if(!VTKParser_parse(parser))
    {
        std::cerr << "Parsing failed\n";
        return -2;
    }

    void* data = VTKParser_parseAllUnstructuredGridPoints(parser);
    size_t nbFieldValue = 0;
    HVTKFieldValue* fieldValues = VTKParser_getPointFieldValueDescriptors(parser, &nbFieldValue);
    std::vector<void*> datas;
    for(uint32_t i = 0; i < nbFieldValue; i++)
    {
        std::cout << VTKParser_getFieldName(fieldValues[i]) << std::endl;
        datas.push_back(VTKParser_parseAllFieldValues(parser, fieldValues[i]));
    }

    if(VTKParser_getDatasetType(parser) == VTK_UNSTRUCTURED_GRID)
    {
        int32_t* cellsTypes = VTKParser_parseAllUnstructuredGridCellTypes(parser);
        VTKCellTypes cellDescriptor = VTKParser_getUnstructuredGridCellTypesDescriptor(parser);
        int32_t* cellValues = VTKParser_parseAllUnstructuredGridCellsComposition(parser);

        VTKCellConstruction cellCon = VTKParser_getCellConstructionDescriptor(cellDescriptor.nbCells, cellValues, cellsTypes);
        void* cellData = malloc(cellCon.size*4);
        VTKParser_fillUnstructuredGridCellElementBuffer(parser, cellCon.nbCells, cellValues, cellsTypes, (int32_t*)cellData);

        //Free everything
        for(auto& it : datas)
            VTKParser_free(it);
        VTKParser_free(cellsTypes);
        VTKParser_free(data);
        VTKParser_free(cellData);
    }
    VTKParser_free(fieldValues);
    VTKParser_delete(parser);
    return 0;
}
