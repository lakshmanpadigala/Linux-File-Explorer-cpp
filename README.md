
# FILE EXPLORER
File named `main.cpp` has to be complied using `g++ main.cpp` and then run with 
`./a.out` in terminal.
Application starts from the Folder from the point where you started as the ROOT Folder
Using `UP` and `DOWN` arrow keys to navigate between files from the list.
Use `k` for scrolling up and `l` for scrolling down.
Keys `left` and `right` arrows to goto to access previous and following folder visited.
`backspace` key for one folder up.`h` key for root home folder (from where the application has been started).
click `:` for entering into `command mode` 
 
 


## Available Commands

- `copy <source_file(s)> <destination_directory>`
- `move <source_file(s)> <destination_directory>`
- `rename <old_filename> <new_filename>`
- `create_file <file_name> <destination_path>`
- `create_dir <dir_name> <destination_path>`
- `delete_file <file_path>`
- `delete_dir <dir_path>`
- `goto <location>`
- `search <file_name>`

`esc` for Normal Mode `q` for exiting the application.

paths specified can be absolute and relative. In case of Relative `~` symbol 
should be used and that is considered from the location where the application is started.
`.` is used to the refer the present working directory.
If you don't use both the symbols `~ & . ` than total path should be mentioned.  

  