@echo off
echo �������obj pch idb pdb ncb opt plg res sbr ilk���м��ļ������Ե�......
pause

del  /s *.ncb
del  /s *.pdb
del  /s *.application
del  /s *.exp
del  /s *.ilk
del  /s *.manifest
del  /s *.aps
del  /s *.usr
del  /s *.idb
del  /s *.resources
del  /s *.cache
del  /s *.pch
del  /s *.obj
del  /s *.opt
del  /s *.plg
del  /s *.res
del  /s *.sbr
rd /q /s Debug
rd /q /s Release

echo ����ļ���ɣ�
echo. & pause
