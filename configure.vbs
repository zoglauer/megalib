' This is a basic configuration script for MEGAlib on windows...
' It is far less powerful than it's Linux/Mac version


Dim filesys
Set filesys = CreateObject("Scripting.FileSystemObject")

Dim CurrentPath
CurrentPath = filesys.GetAbsolutePathName(".")


'--------------------------------------------------------------------
' (1) Set up all paths

Set wshShell = CreateObject("WScript.Shell")
Set wshSystemEnv = wshShell.Environment("User")

MEGALIBFolder = wshShell.ExpandEnvironmentStrings("%MEGALIB%")
'Wscript.Echo "MEGALIB: >" & MEGALIBFolder & "<"

If MEGALIBFolder <> "%MEGALIB%" Then
  wshSystemEnv.Remove("MEGALIB")
End If

wshSystemEnv("MEGALIB") = CurrentPath
MEGALIBFolder = wshSystemEnv("MEGALIB")


'--------------------------------------------------------------------
' (2) Check for correct ROOT version


'--------------------------------------------------------------------
' (3) Check for correct Geant4 version


'--------------------------------------------------------------------
' (4) Link (!) include files and solution/project files 

'On Error Resume Next

' Dim filesys, folder, files, NewsFile,sFolder
 





SourceFolder = MEGALIBFolder & "\src"
IncludeFolder = MEGALIBFolder & "\include"

'Wscript.Echo "Include: >" & IncludeFolder & "<"


' Create the include folder
If Not filesys.FolderExists(IncludeFolder) Then
  filesys.CreateFolder(IncludeFolder)
  'Wscript.Echo "Created folder: " & IncludeFolder 
Else
  'Wscript.Echo "Include already exists"  
End If

Set NewFile = filesys.CreateTextFile(MEGALIBFolder + "/Files.txt", True)

' Find all include files (except cosima)
'Wscript.Echo "Name: " & SourceFolder
Set folder = filesys.GetFolder(SourceFolder)
'Wscript.Echo "Name: " & folder.Name 
Call CopyInclude(folder)

NewFile.Close


'--- the end ---
'Wscript.Quit



Sub CopyInclude(Folder)
    For Each Subfolder in Folder.SubFolders
        'Wscript.Echo Subfolder.Path
        Set objFolder = filesys.GetFolder(Subfolder.Path)
        Set colFiles = objFolder.Files
        For Each objFile in colFiles
            If right(objFile.Name, 2) = ".h" And objFile.Name <> "LinkDef.h" Then
                NewFile.WriteLine(objFile.Name)
            End If
        Next
        'Wscript.Echo
        Call CopyInclude(Subfolder)
    Next
End Sub

