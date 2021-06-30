# [linux指令] 如何刪除(或其他處理)目錄底下所有特定名稱的檔案或是料夾，使用find與-exec
find . -type d -name .vs -exec rm -rfv {} \;
find . -type d -name x64 -exec rm -rfv {} \;
find . -type d -name "[Dd]ebug" -exec rm -rfv {} \;
