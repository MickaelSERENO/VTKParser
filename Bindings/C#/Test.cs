using sereno;
using System;
using System.Collections.Generic;

public class Test
{
    static void Main(String[] argv)
    {
        if(argv.Length < 1)
        {
            Console.WriteLine("Needs the path to the dataset");
            return;
        }

        VTKParser parser = new VTKParser(argv[0]);
        if(!parser.Parse())
        {
            Console.WriteLine($"Fail to parse the file {argv[0]}");
            return;
        }

        if(parser.GetDatasetType() == VTKDatasetType.VTK_UNSTRUCTURED_GRID)
        {
            Console.WriteLine("Unstructured grid");
            return;
        }
        else if(parser.GetDatasetType() == VTKDatasetType.VTK_STRUCTURED_POINTS)
        {
            Console.WriteLine("Structured points");
            VTKStructuredPoints pts = parser.GetStructuredPointsDescriptor();
            Console.WriteLine($"Structured points : dimensions {pts.Size[0]}, {pts.Size[1]}, {pts.Size[2]}");
            Console.WriteLine($"Structured points : spacing    {pts.Spacing[0]:F4}, {pts.Spacing[1]:F4}, {pts.Spacing[2]:F4}");
            Console.WriteLine($"Structured points : origin     {pts.Origin[0]:F4},  {pts.Origin[1]:F4},  {pts.Origin[2]:F4}");

            List<VTKFieldValue> fieldDesc = parser.GetPointFieldValueDescriptors();
            if(fieldDesc.Count > 0)
            {
                foreach(var f in fieldDesc)
                    Console.WriteLine($"Found {f.Name} with {f.NbTuples} values");
            }
            else
            {
                Console.WriteLine("No value found...");
            }

            return;
        }
    }
}
