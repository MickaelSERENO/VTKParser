#include <utility>
#include "VTKParser.h"

namespace sereno
{
    const std::regex VTKParser::versionRegex("^# vtk DataFile Version (\\d+)\\.(\\d+)\\s*");
    const std::regex VTKParser::datasetRegex("^DATASET (UNSTRUCTURED_GRID|STRUCTURED_GRID)\\s*");
    const std::regex VTKParser::pointsRegex("^POINTS (\\d+) (\\w+)\\s*");
    const std::regex VTKParser::cellsRegex("^CELLS (\\d+) (\\d+)\\s*");
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
        return VTK_NO_VALUE_FORMAT;
    }

    VTKParser::VTKParser(const std::string& path)
    {
        //Get file statistics
        struct stat st;
        if(stat(path.c_str(), &st) == -1)
        {
            std::cerr << "Could not open " << path.c_str() << std::endl;
            return;
        }

        //Open the file and do a memory mapping on it
#ifdef WIN32
		m_fd = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			              FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY, INVALID_HANDLE_VALUE);
		if(m_fd == INVALID_HANDLE_VALUE)
			return;
		m_file = _fdopen(_open_osfhandle((intptr_t)m_fd, _O_RDONLY), "r");
#else
		m_fileLen = st.st_size;
        m_fd       = open(path.c_str(), O_RDONLY);
        m_mmapData = mmap(NULL, m_fileLen, PROT_READ, MAP_PRIVATE, m_fd, 0);
        if(m_mmapData == MAP_FAILED)
        {
            std::cerr << "Could not map the memory over the file " << path.c_str() << std::endl;
            return;
        }
#endif
    }

    VTKParser::VTKParser(VTKParser&& mvt) : m_type(mvt.m_type)
#ifdef WIN32
		, m_fd(mvt.m_fd), m_file(mvt.m_file)
#else
		, m_fd(mvt.m_fd), m_mmapData(mvt.m_mmapData), m_fileLen(mvt.m_fileLen)
#endif
    {
        switch(mvt.m_type)
        {
            case VTK_STRUCTURED_GRID:
                new(&m_grid) VTKGrid(std::move(mvt.m_grid));
                break;
            default:
                break;
        }

#ifdef WIN32
		mvt.m_file = NULL;
		mvt.m_fd   = INVALID_HANDLE_VALUE;
#else
		mvt.m_fd = -1;
        mvt.m_mmapData = MAP_FAILED;
#endif
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
#ifdef WIN32
		if(m_file)
			fclose(m_file);
#else
        if(m_mmapData != MAP_FAILED)
            munmap(m_mmapData, m_fileLen);
        if(m_fd != -1)
            close(m_fd);
#endif
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

    bool VTKParser::parse()
    {
#ifdef WIN32
		if(!m_file)
			return false;
		FILE* f = m_file;
#else
        FILE* f = fdopen(dup(m_fd), "r");
#endif
        fseek(f, 0, SEEK_SET);

        std::smatch match;
        bool pointCellDataParsed = false; //Tells if we parsed something in the Point / Cell data sections. If not, error

        //Version
        std::string line = getLineFromFile(f);
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
        m_header = getLineFromFile(f);
        if(m_header.size() == 0)
        {
            std::cerr << "Not header.\n";
            goto error;
        }

        //Binary or Ascii ?
        line = getLineFromFile(f);
        if(line != "BINARY\n")
        {
            std::cerr << "Do not handle type other than BINARY. Received " << line << std::endl;
            goto error;
        }
        m_fileFormat = VTK_BINARY;

        //Parse dataset information
        GET_VTK_NEXT_LINE(f)
        if(std::regex_match(line, match, datasetRegex))
        {
            if(match[1].str() == "UNSTRUCTURED_GRID")
            {
                m_type = VTK_UNSTRUCTURED_GRID;
                if(!parseUnstructuredGrid(f))
                    goto error;
            }
            else if(match[1].str() == "STRUCTURED_GRID")
                m_type = VTK_STRUCTURED_GRID;              
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

        //Check end of file
        line = getLineFromFile(f);
        if(line.size() == 0)
            return true;

        //Parse point data if exist
        if(std::regex_match(line, match, pointDataRegex))
        {
            pointCellDataParsed = true;
            parseValues(f, m_ptsData);
        }

        //Parse cell data if exist
        if(std::regex_match(line, match, cellDataRegex))
        {
            pointCellDataParsed = true;
            parseValues(f, m_cellData);
        }

        //Error
        if(!pointCellDataParsed)
        {
            std::cerr << "Expecting POINT_DATA token" << std::endl;
            goto error;
        }

#if WIN32
#else
        fclose(f);
#endif
        return true;
	error:
#if WIN32
#else
        fclose(f);
#endif
        return false;
    }

    bool VTKParser::parseUnstructuredGrid(FILE* file)
    {
        std::string line;
        GET_VTK_NEXT_LINE(file)
        std::smatch pointMatch;
        if(std::regex_match(line, pointMatch, pointsRegex))
        {
            try
            {
                //Parsing points 
                m_unstrGrid.ptsPos.nbPoints = std::stoi(pointMatch[1].str());
                m_unstrGrid.ptsPos.format   = vtkStringToFormat(pointMatch[2].str());
                m_unstrGrid.ptsPos.offset   = ftell(file);
                fseek(file, 3*m_unstrGrid.ptsPos.nbPoints*VTKValueFormatInt(m_unstrGrid.ptsPos.format), SEEK_CUR);

                GET_VTK_NEXT_LINE(file)
                if(line != "\n")
                {
                    std::cerr << line << "Unexpected token\n";
                    return false;
                }

                //Parsing points metadata
                GET_VTK_NEXT_LINE(file)
                std::smatch match;
                if(line == "METADATA\n")
                {
                    if(!parseMetadata(file))
                        return false;
                    GET_VTK_NEXT_LINE(file)
                }

                //Parsing cells
                if(std::regex_match(line, match, cellsRegex))
                {
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
                    GET_VTK_NEXT_LINE(file)
                    if(line == "METADATA\n")
                    {
                        if(!parseMetadata(file))
                            return false;
                        GET_VTK_NEXT_LINE(file)
                    }
                }
                else
                {
                    std::cerr << "Expecting CELLS token\n";
                    return false;
                }

                if(std::regex_match(line, match, cellTypesRegex))
                {
                    m_unstrGrid.cellTypes.offset  = ftell(file);
                    m_unstrGrid.cellTypes.nbCells = std::stoi(match[1].str());
                    fseek(file, m_unstrGrid.cellTypes.nbCells*sizeof(int), SEEK_CUR);
                    GET_VTK_NEXT_LINE(file)
                    if(line != "\n")
                    {
                        std::cerr << "Unexpecting token.\n";
                        return false;
                    }
                }
                else
                {
                    std::cerr << "Expecting CELL_TYPES token\n";
                    return false;
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << "Error at dataset block : " << e.what() << std::endl;
                return false;
            }
        }
        else
        {
            std::cerr << "Expecting POINTS token\n";
            return false;
        }
        return true;
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
                            std::cerr << "Error at reading a field value\n";
                            return false;
                        }

                        VTKFieldValue fieldValue;
                        fieldValue.name            = match[1].str();
                        fieldValue.nbTuples        = std::stoi(match[2].str());
                        fieldValue.nbValuePerTuple = std::stoi(match[3].str());
                        fieldValue.format          = vtkStringToFormat(match[4].str());
                        fieldValue.offset          = ftell(file);
                        value.fieldData.values.push_back(fieldValue);

                        fseek(file, fieldValue.nbTuples*fieldValue.nbValuePerTuple*VTKValueFormatInt(fieldValue.format), SEEK_CUR);
                        GET_VTK_NEXT_LINE(file)
                        if(line != "\n")
                        {
                            std::cerr << "Unexpected token\n";
                            return false;
                        }

                        filePos = ftell(file);
                        line = getLineFromFile(file);
                        if(line == "METADATA\n")
                            parseMetadata(file);
                        else
                            fseek(file, filePos, SEEK_SET);

                        std::cerr << "Found : " << fieldValue.name << std::endl;
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
                fseek(file, filePos, SEEK_CUR);
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
		return (int32_t*)getAllBinaryValues(m_unstrGrid.cells.offset, m_unstrGrid.cells.wholeSize*VTKValueFormatInt(VTK_INT), VTK_INT);
	}

	int32_t* VTKParser::parseAllUnstructuredGridCellTypes() const
	{
		return (int32_t*)getAllBinaryValues(m_unstrGrid.cellTypes.offset, m_unstrGrid.cellTypes.nbCells*VTKValueFormatInt(VTK_INT), VTK_INT);
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

    void* VTKParser::parseAllCellFieldValues(const VTKFieldValue* fieldData) const
    {
        return getAllBinaryValues(fieldData->offset, fieldData->nbTuples*fieldData->nbValuePerTuple, fieldData->format);
    }

    void* VTKParser::getAllBinaryValues(size_t offset, uint32_t nbValues, VTKValueFormat format) const
    {
		int sizeFormat = VTKValueFormatInt(format);
#if WIN32
		uint8_t buffer[8];
		fseek(m_file, offset, SEEK_SET);
#else
		uint8_t* buffer = (uint8_t*)m_mmapData;
#endif
        uint8_t* data = (uint8_t*)malloc(sizeFormat*nbValues);
        union
        {
            int     i;
            float   f;
            double  d;
            uint8_t c;
        }val;

        for(uint64_t i = 0; i < nbValues; i++)
        {
#if WIN32
			fread(buffer, 1, sizeFormat, m_file);
#else
			buffer += offset + i * sizeFormat
#endif
            switch(format)
            {
                case VTK_INT:
                    val.i = readInt(buffer);
                    break;
                case VTK_FLOAT:
                    val.f = readFloat(buffer);
                    break;
                case VTK_DOUBLE:
                    val.d = readDouble(buffer);
                    break;
                default:
                    free(data);
                    return NULL;
            }
            memcpy(data+i*sizeFormat, &val.c, sizeFormat);
        }
        return (void*)data;
    }
    
/*----------------------------------------------------------------------------*/
/*---------------------------The read <T> functions---------------------------*/
/*----------------------------------------------------------------------------*/

    int VTKParser::readInt(uint8_t* buf)
    {
        return (buf[3] << 0 ) + (buf[2] << 8 )+
               (buf[1] << 16) + (buf[0] << 24);  
    }

    float VTKParser::readFloat(uint8_t* buf)
    {
        int i = readInt(buf);
        return *(float*)(&i);
    }

    double VTKParser::readDouble(uint8_t* buf)
    {
        int64_t i = ((uint64_t)buf[7] << 0 ) + ((uint64_t)buf[6] << 8 ) +
                    ((uint64_t)buf[5] << 16) + ((uint64_t)buf[4] << 24) +
                    ((uint64_t)buf[3] << 32) + ((uint64_t)buf[2] << 40) +
                    ((uint64_t)buf[1] << 48) + ((uint64_t)buf[0] << 56);
        return *(double*)(&i);
    }
}
