# ClrDumper
ClrDumper can dump .net assemblies from :

1) Native Clr Loaders
2) Managed Assembly (in memory loading Assembly.Load(bytes[]))

For Native loaders
use as:
ClrDumper.exe -nativeclr [PATH_TO_EXE]

For Managed Assemblies
use as:
ClrDumper.exe -asmload [PATH_TO_EXE]

ClrDumper injects HookClr.dll into the processes, please ensure the dll is in the same directory
as ClrDumper.exe

Bypasses all debugger checks, obfuscation!

NOTE: THIS PROGRAM WILL RUN YOUR EXE TO EXTRACT THE ASSEMBLY, USE AT YOUR OWN RISK
