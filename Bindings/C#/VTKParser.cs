﻿using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;

namespace sereno
{
    /// <summary>
    /// VTK Value Format enumeration, describes what type the binary value is
    /// </summary>
    public enum VTKValueFormat
    {
        VTK_INT = 0,
        VTK_DOUBLE,
        VTK_FLOAT,
        VTK_NO_VALUE_FORMAT
    }

    public enum VTKGLMode
    {
        VTK_GL_TRIANGLES,
        VTK_GL_TRIANGLE_STIP,
        VTK_GL_LINE_STRIP,
        VTK_GL_POINTS,
        VTK_GL_NO_MODE
    }

    /// <summary>
    /// VTK cell construction structure. It contains meta data about celle construction (buffer size, etc.).
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct VTKCellConstruction
    {
        /// <summary>
        /// Buffer size needed to store the data
        /// </summary>
        public UInt32    Size; 

        /// <summary>
        /// The OpenGL Mode needed to render this
        /// </summary>
		public VTKGLMode Mode;

        /// <summary>
        /// The number of cells to consider for this data
        /// </summary>
		public UInt32    NbCell;

        /// <summary>
        /// What is the offset to apply to the cells data for getting to the next cells to parse ?
        /// </summary>
		public UInt32    Next;

        /// <summary>
        /// Was there an error ? (0 == false, 1 == true)
        /// </summary>
		public char      Error;
    }

    /// <summary>
    ///  VTK Point descriptor for Unstructured Grid
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct VTKPointPositions
    {
        /// <summary>
        /// Number of points
        /// </summary>
        public UInt32         NbPoints;

        /// <summary>
        /// Format of points
        /// </summary>
        public VTKValueFormat Format;

        /// <summary>
        /// Offset
        /// </summary>
        public UInt64         Offset;
    }

    /// <summary>
    /// VTK Cell Types descriptor for Unstructured Grid
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct VTKCellTypes
    {
        /// <summary>
        /// Number of Cells
        /// </summary>
        public UInt32 NbCells;

        /// <summary>
        /// Offset in the VTK file
        /// </summary>
        public UInt32 Offset;
    }

    /// <summary>
    /// VTKCells descriptor. Describes how Unstructured Grid Cells are composed
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct VTKCells
    {
        /// <summary>
        /// Number of cells
        /// </summary>
        public UInt32 NbCells;

        /// <summary>
        /// Number of values to parse
        /// </summary>
        public UInt32 WholeSize;

        /// <summary>
        /// Byte offset in the document
        /// </summary>
        public UInt64 Offset;
    }

    /// <summary>
    /// VTK interop class. Contains mapping between C functions and C# callable functions
    /// See C documentation of VTKParser for understanding what are these functions
    /// </summary>
    public class VTKInterop
    {
        //New and delete
        [DllImport("serenoVTKParser", CharSet = CharSet.Ansi)]
        public extern static IntPtr VTKParser_new([MarshalAs(UnmanagedType.LPStr)] string path);
        [DllImport("serenoVTKParser")]
        public extern static void VTKParser_delete(IntPtr parser);

        [DllImport("serenoVTKParser")]
        public extern static byte VTKParser_parse(IntPtr parser);

        //Unstructured grid
        [DllImport("serenoVTKParser")]
        public extern static VTKPointPositions VTKParser_getUnstructuredGridPointDescriptor(IntPtr parser);
        [DllImport("serenoVTKParser")]
        public extern static VTKCellTypes VTKParser_getUnstructuredGridCellTypesDescriptor(IntPtr parser);
        [DllImport("serenoVTKParser")]
        public extern static VTKCells VTKParser_getUnstructuredGridCellDescriptor(IntPtr parser);
        [DllImport("serenoVTKParser")]
        public unsafe extern static IntPtr VTKParser_parseAllUnstructuredGridCellsComposition(IntPtr parser);
        [DllImport("serenoVTKParser")]
        public unsafe extern static IntPtr VTKParser_parseAllUnstructuredGridCellTypes(IntPtr parser);
        [DllImport("serenoVTKParser")]
        public extern static IntPtr VTKParser_parseAllUnstructuredGridPoints(IntPtr parser);
        [DllImport("serenoVTKParser")]
        public unsafe extern static VTKCellConstruction VTKParser_getCellConstructionDescriptor(UInt32 nbCells, Int32* cellValues, Int32* cellTypes);
        [DllImport("serenoVTKParser")]
        public unsafe extern static void VTKParser_fillUnstructuredCellBuffer(IntPtr parser, UInt32 nbCells, IntPtr ptValues, Int32* cellValues, Int32* cellTypes, IntPtr buffer, VTKValueFormat destFormat);

        //Field Value
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
        public VTKParser(String path)
        {
            m_parser = VTKInterop.VTKParser_new(path);
        }
            
        ~VTKParser()
        {
            VTKInterop.VTKParser_delete(m_parser);
        }

        /// <summary>
        /// Parse the VTK file object for getting information (without retrieving values).
        /// </summary>
        public bool Parse()
        {
            return VTKInterop.VTKParser_parse(m_parser) != 0;
        }

        /// <summary>
        /// Get the values of Cell composition for unstructured grid. Use TODO for getting triangle composition of these cells
        /// </summary>
        /// <returns>A VTKValue of the cell composition. Format : VTK_INT</returns>
        public VTKValue ParseAllUnstructuredGridCellsComposition()
        {
            VTKCells desc = VTKInterop.VTKParser_getUnstructuredGridCellDescriptor(m_parser);
            VTKValue res  = new VTKValue();
            unsafe
            { 
                res.Value     = VTKInterop.VTKParser_parseAllUnstructuredGridCellsComposition(m_parser);
            }
            res.NbValues  = desc.WholeSize;
            res.Format    = VTKValueFormat.VTK_INT;

            return res;
        }

        /// <summary>
        /// Get the values of Cell Types for understanding how to merge the points described in ParseAllUnstructuredGridCellsComposition. 
        /// Use FillUnstructuredCellBuffer for getting triangle composition of these cells 
        /// </summary>
        /// <returns>A VTKValue of all the cell types of this dataset. Format : VTK_INT</returns>
        public VTKValue ParseAllUnstructuredGridCellTypesDescriptor()
        {
            VTKCellTypes desc = VTKInterop.VTKParser_getUnstructuredGridCellTypesDescriptor(m_parser);
            VTKValue     res  = new VTKValue();

            res.Format   = VTKValueFormat.VTK_INT;
            unsafe
            { 
                res.Value    = VTKInterop.VTKParser_parseAllUnstructuredGridCellTypes(m_parser);
            }
            res.NbValues = desc.NbCells;

            return res;
        }

        /// <summary>
        /// Get the Field Value descriptors for Cells data (data per cell).
        /// </summary>
        /// <returns>List of all the Field Value available in this dataset</returns>
        public List<VTKFieldValue> GetCellFieldValueDescriptors()
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

        /// <summary>
        /// Parse all the Unstructured Grid Points available in this dataset.   
        /// Use FillUnstructuredCellBuffer for getting triangle composition of these cells 
        /// </summary>
        /// <returns>A VTKValue of these points.</returns>
        public VTKValue ParseAllUnstructuredGridPoints()
        {
            VTKValue          val = new VTKValue();
            VTKPointPositions pos = VTKInterop.VTKParser_getUnstructuredGridPointDescriptor(m_parser);

            val.Value    = VTKInterop.VTKParser_parseAllUnstructuredGridPoints(m_parser);
            val.NbValues = pos.NbPoints*3;
            return val;
        }

		public unsafe void FillUnstructuredCellBuffer(UInt32 nbCells, VTKValue ptValues, VTKValue cellValues, VTKValue cellTypes, IntPtr buffer, VTKValueFormat destFormat = VTKValueFormat.VTK_NO_VALUE_FORMAT)
        {
            unsafe
            {
                VTKInterop.VTKParser_fillUnstructuredCellBuffer(m_parser, nbCells, ptValues.Value, (Int32*)cellValues.Value, (Int32*)cellTypes.Value, buffer, destFormat);
            }
        }

        public static VTKCellConstruction GetCellConstructionDescriptor(UInt32 nbCells, VTKValue cellValues, VTKValue cellTypes)
        {
            unsafe
            { 
                return VTKInterop.VTKParser_getCellConstructionDescriptor(nbCells, (Int32*)cellValues.Value, (Int32*)cellTypes.Value);
            }
        }

        public static Int32 GetFormatSize(VTKValueFormat format)
        {
            switch(format)
            {
                case VTKValueFormat.VTK_INT:
                    return 4;
                case VTKValueFormat.VTK_DOUBLE:
                    return 8;
                case VTKValueFormat.VTK_FLOAT:
                    return 4;
                default:
                    return 0;
            }
        }
    }
}
