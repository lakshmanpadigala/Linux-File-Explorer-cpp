#include<iostream>
#include<termios.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include<dirent.h>
#include <sys/stat.h>
#include <time.h>
#include<string>
#include <sys/wait.h>
#include<fcntl.h>
#include<pwd.h>

using namespace std;
/*----------------------GLOBAL VARIABLES------------------------------------------*/
int current_row = 1;
int current_col = 1;
int startFileNo = 0;
int endFileNo = 15;
vector<vector<string>> files;
stack<string> front,rear;
string rootpath;
string presentWorkLoc;
struct termios mode;
struct termios oldmode;
bool foundFlag = false;
bool exitFlag = false;


/*-------------FUNCTION DECLARATIONS------------------------------------------------*/


void clear_screen();
void moveTo(int,int);
void resizeScreen(int,int);
void detectAction();
void remove_directory(string,string);
void search_file(string,string);
void startCommandMode();
void getFiles();
const char *FormatBytes(long long , char *);
void print_K_files();
void startNormalMode();


/*--------------FUNCTION DEFINATIONS--------------------------------------------------*/


void clear_screen(){
    std::cout<<"\033[2J\033[1;1H";
    return;
}

void moveTo(int x,int y){
    std::cout<<"\033["<<x<<";"<<y<<"H";
    return;
} 

void getPosition(int *x,int* y){
    printf("\033[6n");
    scanf("\033[%d;%dR", x, y);
}
void resizeScreen(int x,int y){
    std::cout<<"\e[8;"<<x<<";"<<y<<"t";
    return;
}

void detectAction(){
    int c;
    tcgetattr(0,&oldmode);
    tcgetattr(0, &mode);
    mode.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(0, TCSANOW, &mode);
    //std::cout<<"entered non can form";
    while(true){
        c = getchar();
        if(c==27){
            c=getchar();
            if(c==91){
                c=getchar();
                if(c==65){
                    if(current_row > 1){
                        moveTo(--current_row,current_col);
                    }
                    if(current_row==1 && files.size()>15){
                        moveTo(16,1);
                        cout<<"                              ";
                        moveTo(16,1);
                        cout<<"USE K-key FOR SCROLLING UP!";
                        moveTo(current_row,1);
                    }
                }else if(c==66){
                    if(files.size()<15)endFileNo=files.size();
                    else endFileNo=15;
                    if(current_row < endFileNo){
                        moveTo(++current_row,current_col);
                    }if(current_row == endFileNo && files.size()>15){
                        moveTo(16,1);
                        cout<<"USE L-key FOR SCROLLING DOWN!";
                        moveTo(current_row,1);
                    }
                }else if(c==67){
                    if(!front.empty()){
                        rear.push(presentWorkLoc);
                        presentWorkLoc =front.top();
                        front.pop();
                        chdir(presentWorkLoc.c_str());
                        startNormalMode();
                    }else{
                        moveTo(18,1);
                        cout<<"NO FORWARD ACTION!";
                        moveTo(current_row,1);
                    }
                }else if(c==68){
                    if(!rear.empty()){
                        front.push(presentWorkLoc);
                        presentWorkLoc = rear.top();
                        rear.pop();
                        chdir(presentWorkLoc.c_str());
                        startNormalMode();
                    }else{
                        moveTo(18,1);
                        cout<<"NO BACKWARD ACTION!";
                        moveTo(current_row,1);
                    }
                }
            }//k 107//l 108//h 104//enter 10//backspace 127//tab 9
        }else if(c==10){
            struct stat openfile;
            string name =files[startFileNo+current_row-1][0]; 
            //
            char tem[256];
            getcwd(tem,256);
            string address(tem);
            name.insert(0,1,'/');
            //address += "/";
            address = address + name;
            stat(address.c_str(),&openfile);
            moveTo(20,1);
            cout<<"Enter Is Pressed on File!"<<name<<" address:"<<address;
            moveTo(current_row,1);
            if(S_IFDIR & openfile.st_mode){
                moveTo(20,1);
                cout<<"Enter Is Pressed on Directory!"<<name<<" address:"<<address;
                moveTo(current_row,1);
                //address.push_back('/');
                rear.push(presentWorkLoc);
                presentWorkLoc = address;
                int error = chdir(presentWorkLoc.c_str());
                if(error == -1){
                    cout<<"CAN'T OPEN!"<<presentWorkLoc;
                }
                startNormalMode();
            }else{
                //startProcess(address.c_str());
                moveTo(21,1);
                cout<<"Opening:"<<name;
                moveTo(current_row,1);
                pid_t pid = fork();
                //cout<<"here from start process:"<<address<<endl;
                if(pid == 0){
                    execl("/usr/bin/xdg-open","xdg-open",address.c_str(),(char *)0);
                    _exit(127);
                }else{
                    wait(0);
                }
            }
        }else if(c==127){
            //while(front.size())front.pop();
            //while(rear.size())rear.pop();
            //front.push(rootpath);
            if(presentWorkLoc.size()>5){
                rear.push(presentWorkLoc);
                int leng;
                while(true){
                    leng = presentWorkLoc.size();
                    if(presentWorkLoc[leng-1]=='/')break;
                    else presentWorkLoc.pop_back();
                }presentWorkLoc.pop_back();
                cout<<"PrsentWorkingLocation:"<<presentWorkLoc<<endl;
                chdir(presentWorkLoc.c_str());
                startNormalMode();
            }else{
                moveTo(18,1);
                cout<<"ALREADY IN THE HOME FOLDER!";
                moveTo(current_row,1);
            }
            //presentWorkLoc = rootpath;
            //startNormalMode();
        }
        else if((c==107 || c==75) && current_row == 1){
            if(startFileNo>0){
                startFileNo--;
                endFileNo--;
                print_K_files();
                current_row = 1;
                moveTo(current_row,current_col);
            }
        }else if((c==108 || c==76) && current_row == 15){
            if(endFileNo < files.size()){
                startFileNo++;
                endFileNo++;
                print_K_files();
                current_row = 15;//endFileNo - startFileNo;
                moveTo(current_row,current_col);
            }
        }else if(c==72 || c==104){
            rear.push(presentWorkLoc);
            presentWorkLoc = rootpath;
            chdir(presentWorkLoc.c_str());
            startNormalMode();
        }else if(c==58){
            moveTo(20,1);
            cout<<"------------------------------------------------------------COMMAND MODE--------------------------------------------------------------------"<<endl;
            cout<<"|          Available Commands: copy <source_file(s)> <destination_directory> | move <source_file(s)> <destination_directory>               |"<<endl;
            cout<<"|     rename <old_filename> <new_filename> | create_file <file_name> <destination_path> | create_dir <dir_name> <destination_path>         |" <<endl;
            cout<<"|     delete_file <file_path> | delete_dir <dir_path> | goto <location> | search <file_name> | ESC for Normal Mode! | q for exit App.      |"<<endl;
            cout<<"| ~ in relative path is  w.r.t root location from where the application has been started | . Corresponds to Present Directory we are in.   |"<<endl;
            cout<<"------------------------------------------------------------------------------------------------------------------------------------------";
            moveTo(26,1);
            startCommandMode();
            if(exitFlag){
                exitFlag = false;
                return;
            }else{
                startNormalMode();
            }
        }
    }
}

void remove_directory(string destination_loc){
    if(chdir(destination_loc.c_str())==-1){
        perror("chdir");
        return;
    }
    char temp[256];
    getcwd(temp,256);
    struct stat sobj;
    mode_t mobj;
    DIR* dirobj = opendir(".");
    struct dirent *direntobj;// = readdir(dirobj);
    /*
                    DIR *dirobj;
                    struct dirent *direntobj;
                    if ((dirobj = opendir(wd)) != NULL) {
                        while ((direntobj = readdir(dirobj)) != NULL) {
                            
                        }closedir (dirobj);
                    }else{
                        //cout<<"error happening:"<<endl;
                        perror ("");
                        return;
                    }
                    */
    while((direntobj = readdir(dirobj)) != NULL){
        if(direntobj->d_name[0] != '.' && to_string(direntobj->d_name[0]) != ".."){
            stat(direntobj->d_name,&sobj);
            mobj = sobj.st_mode;
            //cout<<"Deleting.."<<temp<<"/"<<direntobj->d_name<<endl;
                if(mobj & S_IFDIR){
                    string childDir = temp;
                    childDir += '/';
                    childDir += direntobj->d_name;
                    remove_directory(childDir);
                }else{
                    string childFile = temp;
                    childFile += "/";
                    childFile += direntobj->d_name;
                    cout<<"Deleting: "<<childFile <<endl;
                    unlink(childFile.c_str());
                }
            }
    }closedir (dirobj);
    chdir("..");
    if(rmdir(destination_loc.c_str())==-1){
         perror("rmdir");
    }
    cout<<"Deleting Directory:"<<destination_loc<<endl;
    return;
}

void search_file(string fileName,string cwd){
    if(chdir(cwd.c_str())==-1){
        //perror("chdir");
        return;
    }
    char temp[256];
    getcwd(temp,256);
    struct stat sobj;
    mode_t mobj;
    DIR* dirobj = opendir(".");
    struct dirent *direntobj;// = readdir(dirobj);
    /*
                    DIR *dirobj;
                    struct dirent *direntobj;
                    if ((dirobj = opendir(wd)) != NULL) {
                        while ((direntobj = readdir(dirobj)) != NULL) {
                            
                        }closedir (dirobj);
                    }else{
                        //cout<<"error happening:"<<endl;
                        perror ("");
                        return;
                    }
                    */
    while((direntobj = readdir(dirobj)) != NULL){
        if(direntobj->d_name[0] != '.' && to_string(direntobj->d_name[0]) != ".."){
            stat(direntobj->d_name,&sobj);
            mobj = sobj.st_mode;
            if(direntobj->d_name == fileName){
                cout<<"TRUE--File Found at:"<<temp<<"/"<<direntobj->d_name<<endl;
                foundFlag = true;
                //cout<<"File Found!"<<endl;
                return;
            }
            if(mobj & S_IFDIR){
                string childDir = temp;
                childDir += '/';
                childDir += direntobj->d_name;
                search_file(fileName,childDir);
            }
        }
    }
    return;
}

void startCommandMode(){
    //tcsetattr(0, TCSANOW, &oldmode);
    cout<<"CMD:->";
    string rawcmd = "";
    vector<string> command;
    while(true){
        char ch = cin.get();
        if(ch==27){
            break;
        }
        cout<<ch;
        switch(ch){
            case 127:{
                if(rawcmd.size()>0){
                    rawcmd.pop_back();
                    //cout<<"\x1b[0K";
                    int x,y;
                    getPosition(&x,&y);
                    moveTo(x,y-1);
                    cout<<" ";
                    moveTo(x,y-1);
                    //cout<<"\033[(1)D";
                    //cout<<"\033[K";
                }
                break;
            }
            case 10:{
                string token="";
                for(int i=0;i<rawcmd.size();i++){
                    if(rawcmd[i] == ' '){
                        command.push_back(token);
                        token = "";
                    }else{
                        token += rawcmd[i];
                    }
                }if(token.size() > 0)command.push_back(token);
                // for(string s: command){
				// 	cout<<s<<" ";
				// }
                // cout<<endl;
                rawcmd ="";

                if(command.size() < 2 || command[0]==""){
                    cout<<"Invalid Command\nCMD:->";
                    command.clear();
                    break;
                }else if(command[0] == "copy"){
                    if(command.size() < 3){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string destination_loc = command[command.size()-1];
                    if(destination_loc[0] == '~'){
                        destination_loc.erase(0,1);
                        destination_loc=rootpath+destination_loc;
                    } 
                    command.pop_back();
                    for(string s:command){
                        if(s != "copy"){
                            cout<<"copying "<<s<<" from "<<destination_loc<<"/"<<endl;
                            ifstream source(s,ios::binary);
                            ofstream destination(destination_loc+"/"+s,ios::binary);
                            destination<<source.rdbuf();
                            source.close();
                            destination.close();
                        }
                    }
                }else if(command[0] == "move"){
                    if(command.size() < 3){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string destination_loc = command[command.size()-1];
                    if(destination_loc[0] == '~'){
                        destination_loc.erase(0,1);
                        destination_loc=rootpath+destination_loc;
                    }
                    command.pop_back();
                    char te[256];
                    getcwd(te,256);
                    for(string s:command){
                        if(s != "move"){
                            cout<<"Moving: "<<s<<" to "<<destination_loc<<"/"<<endl;
                            ifstream source(s,ios::binary);
                            ofstream destination(destination_loc+"/"+s,ios::binary);
                            destination << source.rdbuf();
                            source.close();
                            destination.close();
                        }
                        string deletefile = te;
                        deletefile += "/" + s;
                        unlink(s.c_str());
                    }
                }else if(command[0] == "rename"){
                    if(command.size() != 3){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string oldName,newName;
                    oldName = command[1];
                    newName = command[2];
                    if(rename(oldName.c_str(),newName.c_str()) == -1)
                        perror("rename");
                }else if(command[0] == "create_file"){
                    if(command.size() != 3){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string destination_loc = command[2];
                    if(destination_loc[0] == '~'){
                        destination_loc.erase(0,1);
                        destination_loc = rootpath + destination_loc;
                    }
                    if(command[2] == "."){
                        char te[256];
                        getcwd(te,256);
                        destination_loc = te;
                    }
                    if(destination_loc[destination_loc.size()-1] != '/')
                        destination_loc += "/";
                    destination_loc += command[1];
                    mode_t filemode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IRGRP;
                    cout<<"Creating File:"<<destination_loc<<endl;
                    creat(destination_loc.c_str(),filemode);
                }else if(command[0] == "create_dir"){
                    if(command.size() != 3){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string destination_loc = command[2];
                    if(destination_loc[0] == '~'){
                        destination_loc.erase(0,1);
                        destination_loc = rootpath + destination_loc;
                    }
                    if(command[2] == "."){
                        char te[256];
                        getcwd(te,256);
                        destination_loc = te;
                    }
                    if(destination_loc[destination_loc.size()-1] != '/')
                        destination_loc += "/";
                    destination_loc += command[1];
                    mode_t dirmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IRGRP | S_IXUSR | S_IXGRP;
                    cout<<"Creating Diectory:"<<destination_loc<<endl;
                    mkdir(destination_loc.c_str(),dirmode);
                }else if(command[0] == "delete_file"){
                    if(command.size() != 2){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string destination_loc = command[1];
                    if(destination_loc[0] == '~'){
                        destination_loc.erase(0,1);
                        destination_loc = rootpath + destination_loc;
                    }
                    cout<<"Deleting:"<<destination_loc<<endl;
                    if(unlink(destination_loc.c_str()) != 0)
                        perror("unlink");
                }else if(command[0] == "delete_dir"){
                    if(command.size() != 2){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string destination_loc = command[1];
                    if(destination_loc[0] == '~'){
                        destination_loc.erase(0,1);
                        destination_loc = rootpath + destination_loc;
                    }
                    remove_directory(destination_loc);
                }else if(command[0] == "goto"){
                    if(command.size() != 2){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string destination_loc = command[1];
                    if(destination_loc[0] == '~'){
                        destination_loc.erase(0,1);
                        destination_loc = rootpath + destination_loc;
                    }
                    //char cwd[256];
                    //getcwd(cwd,256);
                    cout<<"Changing PWD to:"<<destination_loc;
                    presentWorkLoc = destination_loc;
                    chdir(destination_loc.c_str());
                    //some push operation......
                }else if(command[0] == "search"){
                    if(command.size() != 2){
                        cout<<"Invalid Command\nCMD:->";
                        command.clear();
                        break;
                    }
                    string fileName = command[1];
                    char cwd[256];
                    getcwd(cwd,256);
                    //chdir(cwd);
                    search_file(fileName,cwd);
                    if(!foundFlag)cout<<"FALSE--No File Found on given name."<<endl;
                    foundFlag = false;
                }else{
                    cout<<"Invalid Command\nCMD:->";
                    command.clear();
                    break;
                }
                cout<<"\nCMD:->";
                command.clear();
                break;
            }
            case 113:{//q
                moveTo(44,1);
                cout<<"--------------------------------------------------------THANK YOU!----------------------------------------------------"<<endl;
                exitFlag = true;
                return;
            }
            default:{
                rawcmd += ch;
            }
        }
    }
    return;
}

void getFiles(const char* wd){
        DIR *dir;
        struct dirent *ent;
        struct stat fileinfo;
        struct passwd *pws;
        string permission;
        //cout<<wd<<endl;
        moveTo(20,1);
        if ((dir = opendir(wd)) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                vector<string> file;
                permission = "";
                stat(ent->d_name,&fileinfo);
                //if(stat(ent->d_name,&fileinfo)==0){
                    //if(S_ISDIR(fileinfo.st_mode)){
                      //  permission += 'd';// +permissions(ent->d_name);
                    //}else{
                      //  permission += '-';// +permissions(ent->d_name);
                    //}
                    //if(stat(ent->d_name, &fileinfo) == 0){
                        mode_t perm = fileinfo.st_mode;
                        permission += (perm & S_IFDIR) ? 'd' : '-';
                        permission += (perm & S_IRUSR) ? 'r' : '-';
                        permission += (perm & S_IWUSR) ? 'w' : '-';
                        permission += (perm & S_IXUSR) ? 'x' : '-';
                        permission += (perm & S_IRGRP) ? 'r' : '-';
                        permission += (perm & S_IWGRP) ? 'w' : '-';
                        permission += (perm & S_IXGRP) ? 'x' : '-';
                        permission += (perm & S_IROTH) ? 'r' : '-';
                        permission += (perm & S_IWOTH) ? 'w' : '-';
                        permission += (perm & S_IXOTH) ? 'x' : '-';
                    //}else{
                        //permission += strerror(errno);
                    //}
                    file.push_back(ent->d_name);
                    //file.push_back(to_string(fileinfo.st_ino));// = fileinfo.st_ino;
                    file.push_back(to_string(fileinfo.st_size));
                    file.push_back(getpwuid(fileinfo.st_uid)->pw_name);
                    file.push_back(getpwuid(fileinfo.st_gid)->pw_name);
                    file.push_back(permission);
                    file.push_back(ctime(&fileinfo.st_mtime));
                    // for(int i=0;i<6;i++){
                    //     cout<<file[i]<<" ";
                    // }cout<<endl;
                    if(file[0] == "." || file[0] == ".."){
                        //cout<<"insert is happening:";
                        files.insert(files.begin(),file);
                        file.clear();
                    }else{
                        //cout<<"push_back is happening:";
                        files.push_back(file);
                        file.clear();
                    }
                    // std::cout<<permission<<endl;
                    // //std::cout<<fileinfo.st_ino;
                    // std::cout<<ent->d_name<<"\t"<<fileinfo.st_size<<"\t"<<ctime(&fileinfo.st_mtime);
                    //     std::cout<<"\t"<<getpwuid(fileinfo.st_uid)->pw_name<<"\t"<<fileinfo.st_mode<<endl;
                //}
                //printf ("%s\n", ent->d_name);
            }
            closedir (dir);
        }else{
            //cout<<"error happening:"<<endl;
            perror ("");
            return;
        }
        //if(files.size()<15)endFileNo=files.size();
        //cout<<"files Number:"<<files.size()<<endl;
        return;
}

const char *FormatBytes(long long bytes, char *str)
{
    const char *sizes[5] = { "B", "KB", "MB", "GB", "TB" };
 
    int i;
    double dblByte = bytes;
    for (i = 0; i < 5 && bytes >= 1024; i++, bytes /= 1024)
        dblByte = bytes / 1024.0;
 
    sprintf(str, "%.2f", dblByte);
 
    return strcat(strcat(str, " "), sizes[i]);
}

void print_K_files(){
    clear_screen();
    string finame;
    string modtime;
    int endlimit = 15 + startFileNo;
    //startFileNo = 0;
    if(files.size() < 15)endlimit = files.size();
    //else endFileNo = 15;
    for(int i=startFileNo;i<endlimit;i++){
        for(int j=0;j<6;j++){
            if(j==0){
                int size = files[i][j].length();
                finame=files[i][0];
                if(size<20){
                    int diff=20-size;
                    for(int k=0;k<diff;k++)finame += " ";
                }else if(size>20){
                    finame = finame.substr(0,18);
                    finame += "...";
                }
            }
            if(j==5){
                modtime = files[i][j];
                modtime.pop_back();
                //files[i][j].pop_back();
            }
            if(j==1){
                char si[32]="";
                files[i][j] = FormatBytes(stoll(files[i][j]),si);
                if(files[i][j].length()<10){
                    int diff=10-files[i][j].length();
                    for(int k=0;k<diff;k++)files[i][j] += " ";
                }
            }
            if(j==0)std::cout<<finame<<"\t";
            else if(j==5)std::cout<<modtime<<"\t";
            else std::cout<<files[i][j]<<"\t";
        }
        std::cout<<endl;
    }
    // char temp[256];
    // getcwd(temp,256);
    // string address(temp);
    // cout<<address;
    current_row = 1;
    current_col = 1;
}

void startNormalMode(){
    int current_row = 1;
    int current_col = 1;
    files.clear();
    getFiles(presentWorkLoc.c_str());
    startFileNo = 0;
    //cout<<"files Numer:"<<files.size()<<endl;
    print_K_files();
    //moveTo(21,1);
    //cout<<"Normal Mode Started!:"<<presentWorkLoc;
    char temp[256];
    getcwd(temp,256);
    moveTo(17,1);
    cout<<"Present Working Location!"<<temp;
    moveTo(1,1);
    detectAction();
    return;
}


/*--------------------MAIN FUNCTION------------------------------------------------------*/
int main(){
    clear_screen();
    resizeScreen(45,150);
    char temp[256];
    getcwd(temp,256);
    string wd(temp);
    cout<<wd<<endl;
    rootpath = wd;
    presentWorkLoc = wd;
    startNormalMode();
    tcsetattr(0, TCSANOW, &oldmode);
    return 0;
}