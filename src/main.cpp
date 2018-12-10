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
    HVTKFieldValue* fieldValues = VTKParser_getCellFieldValueDescriptors(parser, &nbFieldValue);
    std::vector<void*> datas;
    for(uint32_t i = 0; i < 2; i++)
        datas.push_back(VTKParser_parseAllCellFieldValues(parser, fieldValues[i]));

    //Free everything
    for(auto& it : datas)
        VTKParser_free(it);
    VTKParser_free(data);
    VTKParser_free(fieldValues);
    VTKParser_delete(parser);
    return 0;
}
