report_file=report.xml

cppcheck --xml-version=2 -j 4 --enable=all --inconclusive --force . 2>"${report_file}"
cppcheck-htmlreport --source-dir . --report-dir cppcheck/ <"${report_file}"
