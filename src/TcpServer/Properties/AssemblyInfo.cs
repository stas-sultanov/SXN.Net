using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

// The culture of the assembly.

[assembly: AssemblyCulture("")]

// The trademark of the assembly.

[assembly: AssemblyTrademark("")]

// Indicate that the assembly is not visible to COM

[assembly: ComVisible(false)]

#if DEBUG

// The tests assembly.

[assembly: InternalsVisibleTo("SXN.Net.Tests")]

#endif