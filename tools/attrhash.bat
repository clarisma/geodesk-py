"%~dp0gperf" -L C++  -t "--class-name=%1_AttrHash" "--lookup-function-name=lookup" "%1_attr.txt" > "%1_lookup.cxx" 
python "%~dp0attr_names.py" "%1_attr.txt" "%1_attr.cxx"
