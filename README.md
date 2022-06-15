# ClrDumper
ClrDumper can dump .net assemblies and scripts from :

- Native Clr Loaders
- Managed Assembly (in memory loading Assembly.Load(bytes[]))
- vbs execution (for now direct execution)

ClrDumper can also dump scripts at every stage, like eval or Execute
for now vbscript is supported, jscript and powershell is coming soon

For Native loaders
```
ClrDumper.exe -nativeclr [PATH_TO_EXE]
```

For Managed Assemblies
```
ClrDumper.exe -asmload [PATH_TO_EXE]
```

For VbScript
```
ClrDumper.exe -vbscript [PATH_TO_VBS]
```

ClrDumper injects HookClr.dll into the processes, please ensure the dll is in the same directory
as ClrDumper.exe

Bypasses all debugger checks, obfuscation!

NOTE: THIS PROGRAM WILL RUN YOUR TARGER TO EXTRACT THE ASSEMBLY, USE AT YOUR OWN RISK