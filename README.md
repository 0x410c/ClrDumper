# ClrDumper
ClrDumper can dump .net assemblies and scripts from :

- Native Clr Loaders
- Managed Assembly (in memory loading Assembly.Load(bytes[]))
- vbs/js hosting executables
- vbscript or jscript
- poweshell scripts

ClrDumper can also dump scripts at every stage, like eval or Execute

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

For JScript
```
ClrDumper.exe -jscript [PATH_TO_JS]
```

For Powershell
```
ClrDumper.exe -powershell [PATH_TO_JS]
```

For Executables which host vbscript/jscript/powershell
```
ClrDumper.exe -jscript [PATH_TO_EXE]
ClrDumper.exe -vbscript [PATH_TO_EXE]
ClrDumper.exe -powershell [PATH_TO_EXE]
```

ClrDumper injects HookClr.dll into the processes, please ensure the dll is in the same directory
as ClrDumper.exe

Bypasses all debugger checks, obfuscation!

NOTE: THIS PROGRAM WILL RUN YOUR TARGER TO EXTRACT THE ASSEMBLY, USE AT YOUR OWN RISK
