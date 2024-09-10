#include <iostream>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <string>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <ctime>
#include <fcntl.h>
#include <cstring>
using namespace std;


string trim (string input){
    int i=0;
    while(i< input.size() && input[i] == ' '){
        i++;
    }
    if (i< input.size()){
        input=input.substr(i);
    }
    else{
        return "";
    }
    i = input.size()-1;
    while(i>=0 && input[i]==' '){
        i--;
    }
    if(i>=0){
        input=input.substr(0, i+1);
    }
    else{
        return "";
    }
    return input;
} 


char** vec_to_char_array(vector<string>& parts){

    char** result = new char*[parts.size() + 1];
    for (int i=0; i<parts.size(); i++){
        result[i] = (char*) parts[i].c_str();
    }
    result [parts.size()] =NULL;
    return result;
}

vector<string> split (string input, char sep = ' '){
 
    vector<string> partition;
    bool check = false;
    int begn=0;
    //string exp;
    int count=0;
    //string exp1;
    int x =0;
    int insize =input.size();
    for(int i=0; i<insize; i++){
       char quote1= input[i] == '\'';
       char quote2= input[i] == '\"';
        if((quote1 || quote2) && sep == ' ' ){ //echo
            count ++;
            string exp;
            if(count%2 != 0){ 
                begn = i+1;
                check = true;
            }
            else{
                for (int j= begn; j<i; j++){
                    exp.push_back(input[j]);
                }
                check = false;
                partition.push_back(exp);
            }
         }
         else if(check == false){
             if(((insize-1) == i)|| input[i]== sep){
                 string exp1;
                 if(i==insize-1){
                    //int c =0;
                    for (int z = x; z < i + 1; z++){
                         exp1.push_back(input[z]); 
                    }
                 }
                 else{
                     for (int z = x; z < i; z++){
                        exp1.push_back(input[z]);
                     }
                 }
                 partition.push_back(exp1);
                 x=i+1;
             }
         }
    } 
    return partition;
}


int main (){
    dup2(0,1);
    vector<int> backgroundp;
    while (true){
        dup2(1,0);
        //background process
        //vector<int> backgroundp;
        for (int i=0; i<backgroundp.size(); i++){
            if(waitpid(backgroundp[i], 0, WNOHANG) == backgroundp[i]){
                //cout << "Ending Process " << backgroundp[i]<<endl;
                backgroundp.erase(backgroundp.begin() + i);
                i--; 
            }
        }
        //prompt name
        time_t t = time(0);
        string ct = ctime(&t);
        tm *ttm = gmtime(&t);
        ct= asctime(ttm);
        ct.pop_back();
        char* user = getenv("USER");
        cout << user <<" "<< ct << "$ "; //print prompt
        //cout << "My Shell$ "; //print a prompt
        string inputline;
        getline (cin, inputline); //get a line from standard input
        inputline = trim(inputline);
        if (inputline == string("exit")){
            cout << "Bye!! End of shell" << endl;
            break;
        }
        //checking bg process
       // int pid = fork ();
        bool bg = false;
        //inputline = trim(inputline);
        if(inputline[inputline.size()-1]=='&'){
           // cout<< "Bg process found"<< endl;
            bg= true;
            inputline=inputline.substr(0, inputline.size()-1);
            inputline = trim(inputline);
        }
         //echo
        if(inputline.find("echo") != string::npos)
        {
            string spc = "";
            int pos  = inputline.find("'");
            int pos2 = inputline.find("\"");

            if(pos != 0)
            {
                int f = pos + 1;
                int s = inputline.size() - pos - 2;
                spc = inputline.substr(f, s);
                cout << spc << endl;
            }
            else if(pos2 != 0)
            {
                int f = pos2 + 1;
                int s = inputline.size() - pos2 - 2;
                spc = inputline.substr(f, s);
                cout << spc << endl;
            }
            continue;
        }
           
        //cd
        char bufer[1000];
        if(trim(inputline).find("cd")==0){
            string directory;
            directory = trim (split(inputline, ' ')[1]);
            if(directory == "-"){
                cout << bufer << endl;
                chdir(bufer);
                continue;
            }
            else{
                getcwd(bufer, sizeof(bufer));
                chdir(directory.c_str());
                continue;
            }
        }
       
        //piping
        vector<string> piping = split(inputline, '|');
        //cout<<piping.size()<<endl;
       // for loop for piping
       for(int i=0; i<piping.size();i++){
           int fds[2];
           pipe(fds);
           inputline=trim(piping[i]);
           
           int pid = fork ();
           if (pid == 0){ //child process
                //io redirection 
                int position;
                string cmd;
                string file;
                position = inputline.find('>');
                if(position >=1){
                    cmd = trim(inputline.substr(0,position));
                    //inputline=cmd;
                    file = trim(inputline.substr(position+1));
                    inputline=cmd;
                    int fd = open(file.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
                    dup2(fd,1);
                    close(fd);
                }
                position = inputline.find('<');
                if(position >=1){
                    cmd = trim(inputline.substr(0,position));
                    //inputline=cmd;
                    file = trim(inputline.substr(position+1));
                    inputline=cmd;
                    int fd = open(file.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
                    dup2(fd,0);
                    close(fd);
                }
                if(i< piping.size()-1){
                    dup2(fds[1],1);
                    close(fds[1]);
                }
                // preparing the input command for execution
                vector<string> parts = split(inputline);
                char** args = vec_to_char_array(parts);
                //char* args [] = {(char *) inputline.c_str(), NULL};
                execvp (args [0], args);
            }
            else{
                if(!bg)
                    waitpid (pid, 0, 0); //parent waits for child process
                else{
                    backgroundp.push_back(pid);
                }
                dup2(fds[0],0);
                close(fds[1]);
            }
        }
    }
    return 0;
}






