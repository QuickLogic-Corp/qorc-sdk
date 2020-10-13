Set objShell = CreateObject("WScript.Shell")
Dim curDir
curDir = objShell.CurrentDirectory

Dim oExcel
Set oExcel = CreateObject("Excel.Application")
Dim oBook
Set oBook = oExcel.Workbooks.Open(curDir & "\FSMstates.xlsx")

Dim filesys
Set filesys = CreateObject("Scripting.FileSystemObject")
If filesys.FileExists(curDir & "\FSMstates.csv") Then
filesys.DeleteFile curDir & "\FSMstates.csv"
End If

oBook.SaveAs curDir & "\FSMstates.csv", 6
oBook.Close False
oExcel.Quit
WScript.Echo "Conversion from FSMstates.xlsx to FSMstates.csv - Done"
