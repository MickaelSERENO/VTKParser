#include <utility>
#include "VTKParser.h"

namespace sereno
{
    const std::regex VTKParser::versionRegex("^# vtk DataFile Version (\\d+)\\.(\\d+)\\s*");
    const std::regex VTKParser::datasetRegex("^DATASET (UNSTRUCTURED_GRID|STRUCTURED_GRID|STRUCTURED_POINTS)\\s*");
    const std::regex VTKParser::pointsRegex("^POINTS (\\d+) (\\w+)\\s*");
    const std::regex VTKParser::cellsRegex("^CELLS (\\d+) (\\d+)\\s*");
    const std::regex VTKParser::dimensionsRegex("^DIMENSIONS (\\d+) (\\d+) (\\d+)\\s*");
    const std::regex VTKParser::spacingRegex("^SPACING ([+-]?\\d*.?\\d+) ([+-]?\\d*.?\\d+) ([+-]?\\d*.?\\d+)\\s*");
    const std::regex VTKParser::originRegex("^ORIGIN ([+-]?\\d*.?\\d+) ([+-]?\\d*.?\\d+) ([+-]?\\d*.?\\d+)\\s*");
    const std::regex VTKParser::pointDataRegex("^POINT_DATA (\\d+)\\s*");
    const std::regex VTKParser::cellDataRegex("^CELL_DATA (\\d+)\\s*");
    const std::regex VTKParser::informationRegex("^INFORMATION (\\d+)\\s*");
    const std::regex VTKParser::dataRegex("^DATA (\\d+) ((?:[+-]?\\d*.?\\d+)*)\\s*");
    const std::regex VTKParser::nameLocationRegex("^NAME ([a-zA-Z_0-9]+) LOCATION ([a-zA-Z_0-9]+)\\s*");
    const std::regex VTKParser::cellTypesRegex("^CELL_TYPES (\\d+)\\s*");
    const std::regex VTKParser::fieldRegex("^FIELD (\\w+) (\\d+)\\s*");
    const std::regex VTKParser::fieldValueRegex("^(\\w+) (\\d+) (\\d+) (\\w+)\\s*");

    /* \brief Get the next line from a file
     * \param f the file to read
     * \return the next line */
    static std::string getLineFromFile(FILE* f)
    {
        char c[1024];
        std::string ret;
        do
        {
            if(!fgets(c, 1024, f))
                break;
            ret = ret + c;
        }while(ret.back() != '\n');
        return ret;
    }

    VTKValueFormat VTKParser::vtkStringToFormat(const std::string& str)
    {
        if(str == "int")
            return VTK_INT;
        else if(str == "float")
            return VTK_FLOAT;
        else if(str == "double")
            return VTK_DOUBLE;
        else if(str == "unsigned_char")
            return VTK_UNSIGNED_CHAR;
        else if(str == "char")
            return VTK_CHAR;
        return VTK_NO_VALUE_FORMAT;
    }

    VTKParser::VTKParser(const std::string& path) : m_path(path)
    {
        //Open the file and do a memory mapping on it
#ifdef WIN32
        fopen_s(&m_file, path.c_str(), "rb");
#else
		m_file = fopen(path.c_str(), "rb");
#endif
	}

    VTKParser::VTKParser(VTKParser&& mvt) : m_type(mvt.m_type), m_path(std::move(mvt.m_path)), m_file(mvt.m_file)
    {
        switch(mvt.m_type)
        {
            case VTK_STRUCTURED_GRID:
                new(&m_grid) VTKGrid(std::move(mvt.m_grid));
                break;
            default:
                break;
        }

        mvt.m_file = NULL;
        mvt.m_type     = VTK_DATASET_TYPE_NONE;
    }

    VTKParser::~VTKParser()
    {
        closeParser();

        switch(m_type)
        {
            case VTK_STRUCTURED_GRID:
                delete(&m_grid);
                break;
            default:
                break;
        }
    }

    void VTKParser::closeParser()
    {
        if(m_file)
            fclose(m_file);
    }

#define GET_VTK_NEXT_LINE(x) \
    {\
        line = getLineFromFile(x); \
        if(line.size() == 0) \
        {\
            std::cerr << "Unexpected EOF\n";\
            return false;\
        }\
    }


#define VTK_STRINGIFY(a) #a

#pragma GCC diagnostic ignored "-Wshadow"
#define VTK_PARSE_METADATA(_file)        \
    {\
        size_t _off = ftell(_file);      \
        line = getLineFromFile(_file);   \
        if(line == "METADATA\n")         \
        {                                \
            if(!parseMetadata(_file))    \
                return false;            \
        }                                \
        else                             \
            fseek(_file, _off, SEEK_SET);\
    }\

    bool VTKParser::parse()
    {
        fseek(m_file, 0, SEEK_SET);

        bool hasParsedPointData = false;
        bool hasParsedCellData  = false;

        std::smatch match;

        //Version
        std::string line = getLineFromFile(m_file);
        if(std::regex_match(line, match, versionRegex))
        {
            try
            {
                m_majorVer = std::stoi(match[1].str());
                m_minorVer = std::stoi(match[2].str());
            }

            catch(const std::exception& e)
            {
                std::cerr << "Error at parsing the VTK file version. Discarding. error : " <<  e.what() << std::endl;
                goto error;
            }
        }
        else
        {
            std::cerr << "Wrong version VTK format in the VERSION part. " << line << "\n";
            goto error;
        }

        //Header
        m_header = getLineFromFile(m_file);
        if(m_header.size() == 0)
        {
            std::cerr << "Not header.\n";
            goto error;
        }

        //Binary or Ascii ?
        line = getLineFromFile(m_file);
        if(line != "BINARY\n")
        {
            std::cerr << "Do not handle type other than BINARY. Received " << line << std::endl;
            goto error;
        }
        m_fileFormat = VTK_BINARY;

        //Parse dataset information
        GET_VTK_NEXT_LINE(m_file)
        if(std::regex_match(line, match, datasetRegex))
        {
            if(match[1].str() == "UNSTRUCTURED_GRID")
            {
                m_type = VTK_UNSTRUCTURED_GRID;
                if(!parseUnstructuredGrid(m_file))
                    goto error;
            }
            else if(match[1].str() == "STRUCTURED_GRID")
            {
                m_type = VTK_STRUCTURED_GRID;
                if(!parseStructuredGrid(m_file))
                    goto error;
            }
            else if(match[1].str() == "STRUCTURED_POINTS")
            {
                m_type = VTK_STRUCTURED_POINTS;
                if(!parseStructuredPoints(m_file))
                    goto error;
            }
            else
            {
                std::cerr << "Unexpecting vtk dataset structure... Received : " << match[1].str() << "\n";
                goto error;
            }
        }
        else
        {
            std::cerr << "Expecting the DATASET line and got " << line << std::endl;
            goto error;
        }

        for(uint32_t i = 0; i < 2; i++)
        {
            //Check end of file
            line = getLineFromFile(m_file);
            if(line.size() == 0)
                goto success;

            //Parse point data if exist
            if(std::regex_match(line, match, pointDataRegex))
            {
                if(hasParsedPointData)
                {
                    std::cerr << "Already parsed POINT_DATA" << std::endl;
                    goto error;
                }
                hasParsedPointData = true;
                parseValues(m_file, m_ptsData);
            }

            //Parse cell data if exist
            else if(std::regex_match(line, match, cellDataRegex))
            {
                if(hasParsedCellData)
                {
                    std::cerr << "Already parsed CELL_DATA" << std::endl;
                    goto error;
                }
                hasParsedCellData = true;
                parseValues(m_file, m_cellData);
            }
        }

    success:
        return true;
    error:
        return false;
    }

    bool VTKParser::parseUnstructuredGrid(FILE* file)
    {
        bool parsedPoints    = false;
        bool parsedCells     = false;
        bool parsedCellTypes = false;

        std::string line;
        std::smatch match;

        try
        {
            for(uint32_t i = 0; i < 3; i++)
            {
                GET_VTK_NEXT_LINE(file)
                if(!parsedPoints && std::regex_match(line, match, pointsRegex))
                {
                    parsedPoints = true;

                    //Parsing points 
                    m_unstrGrid.ptsPos.nbPoints = std::stoi(match[1].str());
                    m_unstrGrid.ptsPos.format   = vtkStringToFormat(match[2].str());
                    m_unstrGrid.ptsPos.offset   = ftell(file);
                    fseek(file, 3*m_unstrGrid.ptsPos.nbPoints*VTKValueFormatInt(m_unstrGrid.ptsPos.format), SEEK_CUR);

                    GET_VTK_NEXT_LINE(file)
                    if(line != "\n")
                    {
                        std::cerr << line << "Unexpected token\n";
                        return false;
                    }

                    //Parsing points metadata
                    VTK_PARSE_METADATA(file)
                }

                //Parsing cells
                else if(!parsedCells && std::regex_match(line, match, cellsRegex))
                {
                    parsedCells = true;
                    m_unstrGrid.cells.nbCells   = std::stoi(match[1].str());
                    m_unstrGrid.cells.wholeSize = std::stoi(match[2].str());
                    m_unstrGrid.cells.offset    = ftell(file);
                    fseek(file, m_unstrGrid.cells.wholeSize*sizeof(int), SEEK_CUR);
                    GET_VTK_NEXT_LINE(file); //"\n"
                    if(line != "\n")
                    {
                        std::cerr << "Unexpected token\n";
                        return false;
                    }

                    VTK_PARSE_METADATA(file)
                }
                else if(!parsedCellTypes && std::regex_match(line, match, cellTypesRegex))
                {
                    parsedCellTypes = true;

                    m_unstrGrid.cellTypes.offset  = ftell(file);
                    m_unstrGrid.cellTypes.nbCells = std::stoi(match[1].str());
                    fseek(file, m_unstrGrid.cellTypes.nbCells*sizeof(int), SEEK_CUR);
                    GET_VTK_NEXT_LINE(file)
                    if(line != "\n")
                    {
                        std::cerr << "Unexpecting token.\n";
                        return false;
                    }
                    VTK_PARSE_METADATA(file)
                }
                else
                {
                    std::cerr << "Expecting a valid unstructured grid token\n";
                    return false;
                }
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error at dataset block : " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool VTKParser::parseStructuredPoints(FILE* file)
    {
        bool parsedDimensions = false;
        bool parsedSpacing    = false;
        bool parsedOrigin     = false;

        std::smatch match;
        std::string line;

        setlocale(LC_ALL, "C");

        try
        {
            for(uint32_t i = 0; i < 3; i++) //Parse three information
            {
                GET_VTK_NEXT_LINE(file)
                if(!parsedDimensions && std::regex_match(line, match, dimensionsRegex))
                {
                    parsedDimensions = true;
                    for(uint32_t j = 0; j < 3; j++)
                        m_strPoints.size[j] = std::stoi(match[j+1].str());
                    VTK_PARSE_METADATA(file)
                }

                else if(!parsedSpacing && std::regex_match(line, match, spacingRegex))
                {
                    parsedSpacing = true;
                    for(uint32_t j = 0; j < 3; j++)
                        m_strPoints.spacing[j] = std::stod(match[j+1].str());
                    VTK_PARSE_METADATA(file)
                }

                else if(!parsedOrigin && std::regex_match(line, match, originRegex))
                {
                    parsedOrigin = true;
                    for(uint32_t j = 0; j < 3; j++)
                    {
                        m_strPoints.origin[j] = std::stod(match[j+1].str());
                        std::cerr << std::stod(match[j+1].str()) << "\n";
                    }
                    VTK_PARSE_METADATA(file)
                }
                else
                {
                    std::cerr << "Expecting a valid structured points token\n";
                    return false;
                }
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error while parsing structured points : " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    bool VTKParser::parseStructuredGrid(FILE* file)
    {
        return false;
    }

    bool VTKParser::parseMetadata(FILE* file)
    {
        std::string line;
        std::smatch match;
        while(true)
        {
            GET_VTK_NEXT_LINE(file)
            if(line == "\n")
                break;

            else if(std::regex_match(line, match, informationRegex))
            {}

            else if(std::regex_match(line, match, nameLocationRegex))
            {}

            else if(std::regex_match(line, match, dataRegex))
            {}

            else
            {
                std::cerr << "Unexpecting token " << line << std::endl;
                return false;
            }
        }
        return true;
    }

    bool VTKParser::parseValues(FILE* file, VTKData& data)
    {
        std::smatch match;
        while(true)
        {
            uint32_t    filePos = ftell(file);
            std::string line    = getLineFromFile(file);
            if(line.size() == 0) //EOF
                return true;

            //Checking for fields
            else if(std::regex_match(line, match, fieldRegex))
            {
                try
                {
                    VTKValue value;
                    value.setType(VTK_FIELD_DATA);
                    value.fieldData.name = match[1].str();

                    //Parse every field arrays
                    uint32_t nbField = std::stoi(match[2].str());
                    for(uint32_t i = 0; i < nbField; i++)
                    {
                        GET_VTK_NEXT_LINE(file)
                        if(!std::regex_match(line, match, fieldValueRegex))
                        {
                            std::cerr << "Error at reading a field value\n" << line;
                            return false;
                        }

                        VTKFieldValue fieldValue;
                        fieldValue.name            = match[1].str();
                        fieldValue.nbTuples        = std::stoi(match[3].str());
                        fieldValue.nbValuePerTuple = std::stoi(match[2].str());
                        fieldValue.format          = vtkStringToFormat(match[4].str());
                        fieldValue.offset          = ftell(file);

                        value.fieldData.values.push_back(fieldValue);

                        fseek(file, fieldValue.nbTuples*fieldValue.nbValuePerTuple*VTKValueFormatInt(fieldValue.format), SEEK_CUR);
                        char endLine;
                        fread(&endLine, 1, 1, file);
                        if(endLine != '\n')
                            fseek(file, -1, SEEK_CUR);

                        VTK_PARSE_METADATA(file)
                    }
                    data.values.push_back(value);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            else
            {
                fseek(file, filePos, SEEK_SET);
                break;
            }
        }
        return true;
    }

#undef GET_VTK_NEXT_LINE


/*----------------------------------------------------------------------------*/
/*-----------------------------Read data methods------------------------------*/
/*----------------------------------------------------------------------------*/
    void* VTKParser::parseAllUnstructuredGridPoints() const
    {
        return getAllBinaryValues(m_unstrGrid.ptsPos.offset, m_unstrGrid.ptsPos.nbPoints*3, m_unstrGrid.ptsPos.format);
    }

    int32_t* VTKParser::parseAllUnstructuredGridCellsComposition() const
    {
        return (int32_t*)getAllBinaryValues(m_unstrGrid.cells.offset, m_unstrGrid.cells.wholeSize, VTK_INT);
    }

    int32_t* VTKParser::parseAllUnstructuredGridCellTypes() const
    {
        return (int32_t*)getAllBinaryValues(m_unstrGrid.cellTypes.offset, m_unstrGrid.cellTypes.nbCells, VTK_INT);
    }

    std::vector<std::string> VTKParser::getPointFieldValueNames() const
    {
        std::vector<std::string> res;
        for(auto& it : m_ptsData.values)
            if(it.type == VTK_FIELD_DATA)
                for(auto& it2 : it.fieldData.values)
                    res.push_back(it2.name);
        return res;
    }

    std::vector<const VTKFieldValue*> VTKParser::getPointFieldValueDescriptors() const
    {
        std::vector<const VTKFieldValue*> res;
        for(auto& it : m_ptsData.values)
            if(it.type == VTK_FIELD_DATA)
                for(auto& it2 : it.fieldData.values)
                    res.push_back(&it2);
        return res;
    }

    std::vector<std::string> VTKParser::getCellFieldValueNames() const
    {
        std::vector<std::string> res;
        for(auto& it : m_cellData.values)
            if(it.type == VTK_FIELD_DATA)
                for(auto& it2 : it.fieldData.values)
                    res.push_back(it2.name);
        return res;
    }

    std::vector<const VTKFieldValue*> VTKParser::getCellFieldValueDescriptors() const
    {
        std::vector<const VTKFieldValue*> res;
        for(auto& it : m_cellData.values)
            if(it.type == VTK_FIELD_DATA)
                for(auto& it2 : it.fieldData.values)
                    res.push_back(&it2);
        return res;
    }

    void* VTKParser::parseAllFieldValues(const VTKFieldValue* fieldData) const
    {
        return getAllBinaryValues(fieldData->offset, fieldData->nbTuples*fieldData->nbValuePerTuple, fieldData->format);
    }

    VTKCellConstruction VTKParser::getCellConstructionDescriptor(uint32_t nbCells, int32_t* cellValues, int32_t* cellTypes)
    {
        VTKCellConstruction con;
        con.mode    = VTK_GL_NO_MODE;
        con.error   = 0;
        con.size    = 0;
        con.nbCells = 0;
        con.next    = 0;

        for(uint32_t i = 0; i < nbCells; i++)
        {
            //Determine which VTKCell to use
            const VTKCellVT* cell = NULL;
            switch (cellTypes[i])
            {
                case VTK_CELL_WEDGE:
                    cell = &vtkWedge;
                    break;
                default:
                    goto error;

            }

            //Check type
            if(con.mode != VTK_GL_NO_MODE && con.mode != cell->getMode())
                return con;
            con.mode = cell->getMode();

            //Check nbPoints (error)
            if (cell->nbPoints() > 0 && cellValues[0] != cell->nbPoints())
                goto error;

            //add size, next and nbCell
            con.size    += cell->sizeBuffer(cellValues);
            con.next    += cellValues[0] + 1;
            con.nbCells++;
            cellValues  += cellValues[0] + 1;
        }

        return con;
    error:
        con.error = 1;
        return con;
    }

    void VTKParser::fillUnstructuredGridCellBuffer(uint32_t nbCells, void* ptValues, int32_t* cellValues, int32_t* cellTypes, void* buffer, VTKValueFormat destFormat)
    {
        if(destFormat == VTK_NO_VALUE_FORMAT)
            destFormat = m_unstrGrid.ptsPos.format;

        uint32_t offset = 0;
        for (uint32_t i = 0; i < nbCells; i++)
        {
            //Determine which VTKCell to use
            const VTKCellVT* cell = NULL;
            switch (cellTypes[i])
            {
            case VTK_CELL_WEDGE:
                cell = &vtkWedge;
                break;
            default:
                return;
            }

            cell->fillBuffer(ptValues, m_unstrGrid.ptsPos.format, cellValues, (uint8_t*)buffer + offset, destFormat);
            offset += cell->sizeBuffer(cellValues)*VTKValueFormatInt(destFormat);
        }
    }

    void* VTKParser::getAllBinaryValues(size_t offset, uint32_t nbValues, VTKValueFormat format) const
    {
        int sizeFormat = VTKValueFormatInt(format);
        uint8_t buffer[8];
        fseek(m_file, offset, SEEK_SET);
        uint8_t* data = (uint8_t*)malloc(sizeFormat*nbValues);

        for(uint64_t i = 0; i < nbValues; i++)
        {
            fread(buffer, sizeFormat, 1, m_file);
            switch(format)
            {
                case VTK_INT:
                {
                    uint32_t val = readVTKValue<uint32_t>(buffer, VTK_INT);
                    memcpy(data+i*sizeFormat, &val, sizeof(val));
                    break;
                }
                case VTK_FLOAT:
                {
                    float val = readVTKValue<float>(buffer, VTK_FLOAT);
                    memcpy(data+i*sizeFormat, &val, sizeof(val));
                    break;
                }
                case VTK_DOUBLE:
                {
                    double val = readVTKValue<double>(buffer, VTK_DOUBLE);
                    memcpy(data+i*sizeFormat, &val, sizeof(val));
                    break;
                }
                case VTK_UNSIGNED_CHAR:
                case VTK_CHAR:
                    data[i] = buffer[0];
                    break;
                default:
                    free(data);
                    return NULL;
            }
        }
        return data;
    }

    void VTKParser::fillUnstructuredGridCellElementBuffer(uint32_t nbCells, int32_t* cellValues, int32_t* cellTypes, int32_t* buffer)
    {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < nbCells; i++)
        {
            //Determine which VTKCell to use
            const VTKCellVT* cell = NULL;
            switch (cellTypes[i])
            {
                case VTK_CELL_WEDGE:
                    cell = &vtkWedge;
                    break;
                default:
                    return;
            }

            cell->fillElementBuffer(cellValues, buffer + offset);
            offset += cell->sizeBuffer(cellValues);
        }
    }
}
