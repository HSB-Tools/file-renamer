# file-renamer

for rename file batch

use:
```
file-renamer -p . -f cpp -t txt -y
```

flags:
```
-p : for path
-f : from type file (e.g cpp/cpp)
-t : to type file (e.g txt/txt)
-y : skip confirmation rename file type data
-h : help
```

result:
```
# Without -y flags
❯ fr -p . -f txt -t cpp
Scanning directory: .
file found:
  ./init.txt
  ./parser.txt
  ./tokenizer.txt
  ./builtins.txt
  ./property_service.txt
  ./main.txt
  ./first_stage_init.txt
Will change extensions from '.txt' to '.cpp'
# Confirmation Appear
Do you want to proceed with renaming? (y/N): y
---
Renaming: ./init.txt -> ./init.cpp
Renaming: ./parser.txt -> ./parser.cpp
Renaming: ./tokenizer.txt -> ./tokenizer.cpp
Renaming: ./builtins.txt -> ./builtins.cpp
Renaming: ./property_service.txt -> ./property_service.cpp
Renaming: ./main.txt -> ./main.cpp
Renaming: ./first_stage_init.txt -> ./first_stage_init.cpp
---
Done. Successfully renamed 7 files.

# With -y flags
❯ fr -p . -f cpp -t txt -y
Scanning directory: .
file found:
  ./parser.cpp
  ./tokenizer.cpp
  ./builtins.cpp
  ./main.cpp
  ./first_stage_init.cpp
  ./init.cpp
  ./property_service.cpp
Will change extensions from '.cpp' to '.txt'
# Auto Yes or Skip Confirmation
---
Renaming: ./parser.cpp -> ./parser.txt
Renaming: ./tokenizer.cpp -> ./tokenizer.txt
Renaming: ./builtins.cpp -> ./builtins.txt
Renaming: ./main.cpp -> ./main.txt
Renaming: ./first_stage_init.cpp -> ./first_stage_init.txt
Renaming: ./init.cpp -> ./init.txt
Renaming: ./property_service.cpp -> ./property_service.txt
---
Done. Successfully renamed 7 files.
```
