# rofi-scripts

My scripts for rofi.

## makefile

| command | description |
| - | - |
| make | compile all c files |
| make compile | same as above |
| make clean | remove all c compiles and memcheck outputs |
| make memcheck | check heap usage of all c executables with valgrind |
| make install | copy all c executables to $HOME/.config/rofi |
| make uninstall | rcopy all c executables from $HOME/.config/rofi |

## scripts

| file | dependencies | description |
| - | - | - |
| book.c | | reads all pdf files from a directory and displays, opens them |
| bookmark.c | firefox, sqlite3 | reads all bookmarks from firefox, opens them |
| screen.c | | reads all xrandr scripts from a directory, executes them |
| ssh.c | terminator | reads all information from ssh config, opens connections |
| websearch.c | firefox, curl, uuid | searches for a keyword on startpage |
