using System.Linq.Expressions;
using System.Runtime.InteropServices;

namespace Core
{
    public static partial class ZsUtils
    {
        [Obsolete]
        public static sbyte[] byteArray2sbyteArray(byte[] _bytes)
        {
            ReadOnlySpan<byte> _org = _bytes;
            ReadOnlySpan<sbyte> _dst = MemoryMarshal.Cast<byte, sbyte>(_org);
            return _dst.ToArray();
        }
        [Obsolete]
        public static sbyte[] string2sbyteArray(string str)
        {
            ushort data = ushort.Parse(str);
            ReadOnlySpan<byte> _org = BitConverter.GetBytes(data);
            ReadOnlySpan<sbyte> _dst = MemoryMarshal.Cast<byte, sbyte>(_org);
            return _dst.ToArray();
        }
        [Obsolete]
        public static byte[] GetBytes(string str)
        {
            System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
            Byte[] bytes = encoding.GetBytes(str);
            return bytes;
        }
        [Obsolete]
        public static sbyte[] GetSBytes(string str)
        {
            ReadOnlySpan<byte> _org = GetBytes(str);
            ReadOnlySpan<sbyte> _dst = MemoryMarshal.Cast<byte, sbyte>(_org);
            return _dst.ToArray();
        }
    }
}
