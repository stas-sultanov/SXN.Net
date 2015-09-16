#include "stdafx.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;

// Defines a company Name custom attribute for an assembly manifest

[assembly:AssemblyCompanyAttribute(L"Stas Sultanov")];

// The configuration of the assembly.

#if DEBUG

[assembly:AssemblyConfiguration("Debug")]

#else

[assembly:AssemblyConfiguration("Retail")]

#endif

// The copyright of the assembly.

[assembly:AssemblyCopyright("Copyright © 2015 Stas Sultanov")]

// The culture of the assembly.

[assembly:AssemblyCulture("")]

// The description of the assembly.

[assembly:AssemblyDescriptionAttribute(L"")];

// The product name information.

[assembly:AssemblyProduct("Tcp Net")]

// The title of the assembly.

[assembly:AssemblyTitle("SXN.Net")]

// The trademark of the assembly.

[assembly:AssemblyTrademark("")]

// The version number of the assembly.
// Is managed by the build process.

[assembly:AssemblyVersion("255.255.255.255")]

// Indicate that the assembly is not visible to COM

[assembly:ComVisible(false)]
