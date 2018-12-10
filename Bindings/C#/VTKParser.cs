using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;

namespace sereno
{
	public enum VTKValueFormat
	{
		VTK_INT = 0,
		VTK_DOUBLE,
		VTK_FLOAT,
		VTK_NO_VALUE_FORMAT
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct VTKPointPositions
	{
		public UInt32         NbPoints;
		public VTKValueFormat Format;
		public UInt64         Offset;
	}

	/// <summary>
	/// VTK interop class. Contains mapping between C functions and C# callable functions
	/// See C documentation of VTKParser 
	/// </summary>
	public class VTKInterop
	{
		[DllImport("serenoVTKParser", CharSet = CharSet.Ansi)]
		public extern static IntPtr VTKParser_new([MarshalAs(UnmanagedType.LPStr)] string path);
		[DllImport("serenoVTKParser")]
		public extern static bool VTKParser_parse(IntPtr parser);
		[DllImport("serenoVTKParser")]
		public extern static void VTKParser_delete(IntPtr parser);
		[DllImport("serenoVTKParser")]
		public extern static VTKPointPositions VTKParser_getUnstructuredGridPointDescriptor(IntPtr parser);
		[DllImport("serenoVTKParser")]
		public extern static IntPtr VTKParser_parseAllUnstructuredGridPoints(IntPtr parser);
		[DllImport("serenoVTKParser")]
		public unsafe extern static IntPtr* VTKParser_getCellFieldValueDescriptors(IntPtr parser, UInt32* n); 
		[DllImport("serenoVTKParser")]
		public extern static VTKValueFormat VTKParser_getFieldFormat(IntPtr fieldValue);
		[DllImport("serenoVTKParser")]
		public extern static UInt32 VTKParser_getFieldNbTuples(IntPtr fieldValue);
		[DllImport("serenoVTKParser")]
		public extern static UInt32 VTKParser_getFieldNbValuesPerTuple(IntPtr fieldValue);
		[DllImport("serenoVTKParser")]
		public extern static IntPtr VTKParser_getFieldName(IntPtr fieldValue);
		[DllImport("serenoVTKParser")]
		public extern static void VTKParser_free(IntPtr value);
	}

	/// <summary>
	/// VTK value object.
	/// </summary>
	public class VTKValue
	{
		/// <summary>
		/// Pointer to the value.e
		/// </summary>
		public IntPtr         Value;

		/// <summary>
		/// The value format.
		/// </summary>
		public VTKValueFormat Format;

		/// <summary>
		/// The number of values in the array "Value".
		/// </summary>
		public UInt64         NbValues;

		~VTKValue()
		{
			VTKInterop.VTKParser_free(Value);
		}
	}

	/// <summary>
	/// VTK field value.
	/// </summary>
	public class VTKFieldValue
	{
		/// <summary>
		/// The field value name
		/// </summary>
		public String Name;

		/// <summary>
		/// The value format.
		/// </summary>
		public VTKValueFormat Format;

		/// <summary>
		/// Number of tuples contained in this field value
		/// </summary>
		public UInt32 NbTuples;

		/// <summary>
		/// Number of values per tuple
		/// </summary>
		public UInt32 NbValuesPerTuple;
	}

	/// <summary>
	/// VTK Parser class. Permit to parse (a.k.a read) VTK file object
	/// </summary>
	public class VTKParser
	{
		/// <summary>
		/// The parser handler pointer.
		/// </summary>
		private IntPtr m_parser;

		/// <summary>
		/// Constructor. Initialize the Parser without parsing anything
		/// </summary>
		/// <param name="path">The Path.</param>
		VTKParser(String path)
		{
			m_parser = VTKInterop.VTKParser_new("/mnt/Documents/Agulhas_10.vtk");
		}
			
		~VTKParser()
		{
			VTKInterop.VTKParser_delete(m_parser);
		}

		/// <summary>
		/// Parse the VTK file object for getting information (without retrieving values).
		/// </summary>
		bool Parse()
		{
			return VTKInterop.VTKParser_parse(m_parser);
		}

		List<VTKFieldValue> GetCellFieldValueDescriptors()
		{
			List<VTKFieldValue> list = new List<VTKFieldValue>();
			unsafe
			{
				UInt32 nbVal;
				IntPtr* desc = VTKInterop.VTKParser_getCellFieldValueDescriptors(m_parser, &nbVal);
				for(UInt32 i = 0; i < nbVal; i++)
				{
					VTKFieldValue fieldValue = new VTKFieldValue();
					fieldValue.Format           = VTKInterop.VTKParser_getFieldFormat(desc[i]);
					fieldValue.NbTuples         = VTKInterop.VTKParser_getFieldNbTuples(desc[i]);
					fieldValue.NbValuesPerTuple = VTKInterop.VTKParser_getFieldNbValuesPerTuple(desc[i]); 
					fieldValue.Name             = Marshal.PtrToStringAnsi(VTKInterop.VTKParser_getFieldName(desc[i]));
					list.Add(fieldValue);
				}
				VTKInterop.VTKParser_free((IntPtr)desc);
			}
			return list;
		}

		VTKValue ParseAllUnstructuredGridPoints()
		{
			VTKValue          val = new VTKValue();
			VTKPointPositions pos = VTKInterop.VTKParser_getUnstructuredGridPointDescriptor(m_parser);

			val.Value    = VTKInterop.VTKParser_parseAllUnstructuredGridPoints(m_parser);
			val.NbValues = pos.NbPoints*3;
			return val;
		}

		static void Main(string[] argv) 
		{
			VTKParser parser = new VTKParser("/mnt/Documents/Agulhas_10.vtk");
			if(parser.Parse())
				Console.WriteLine("File parsed successfully");
			else
				Console.WriteLine("Error at parsing the file...");
			List<VTKFieldValue> fields = parser.GetCellFieldValueDescriptors();
			foreach(var f in fields)
				Console.WriteLine("Has retrieved field named " + f.Name);
			
			VTKValue val = parser.ParseAllUnstructuredGridPoints();
			Console.WriteLine("Has retrieved " + val.NbValues.ToString() + " values");
		}
	}
}