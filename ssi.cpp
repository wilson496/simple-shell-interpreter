/**
 * Author: Cameron Wilson (V00822184)
 * Date: October 4th, 2019
*/

/**
 * References: 
 * Commenting guidelines: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#main
 * Struct linked list implementation inspired by: https://github.com/kamal-choudhary/singly-linked-list/blob/master/Linked%20List.cpp
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using namespace std;


// Struct to track background processes
struct background_process{ 
    pid_t process_id; 
    string command; 
    struct background_process* next; 
};

void checkBackgroundExecution(background_process *root_background_process, pid_t process_id) {
	/**
     * This function will determine if a background process is currently executing.
     * 
     * This allows the shell to know if any there are remaining processes requiring execution
     * 
     * Input: background_process *root_background_process, pid_t process_id
     * 
     * Output: N/A
     * 
     * */ 

	background_process *current_background_process = root_background_process;
	
	if (root_background_process->process_id == process_id) {
		cout << root_background_process->process_id << " " << root_background_process->command << " has terminated." << endl;
		root_background_process = root_background_process->next;
	} else {
		while (current_background_process == NULL) {
			if (current_background_process->process_id == process_id) {
				cout << current_background_process->process_id << current_background_process->command << " has terminated." << endl;
				current_background_process = current_background_process->next;
			}
		}
	}
}


void handleChangeDirectoryCommand(vector<string> tokenized_user_input){
    
    /**
     * This functions handles the "cd" or change directory command
     * 
     * Will apply the cd command to other arguments provided by the user,
     * or will default to the HOME directory if none are provided.
    */

    string returned_path_string;

    // No path provided or "~"" implies HOME directory requested.
    // Otherwise, change directory to path provided.
    if (tokenized_user_input[1].empty() || tokenized_user_input[1].compare("~") == 0) {
        returned_path_string = getenv("HOME");
    } else {
        returned_path_string = tokenized_user_input[1];
    }
    
    chdir(returned_path_string.c_str());
    
}

char** convertStringArguments(vector<string> tokens) {
    /**
     * Returns a char* array from a vector of string tokens to use with execvp()
     * 
     * Input: vector<string> tokens
     * 
     * Output: char* []
    */

	char **args = new char*[tokens.size() + 1];
	for (int i = 0; i < tokens.size(); i++) {
        char *converted_string = new char[tokens[i].size() + 1]; // char* array used for converted "string"
		strncpy(converted_string, tokens[i].c_str(), tokens[i].size() + 1);
        args[i] = converted_string;
	}
	
    args[tokens.size()] = NULL;
	
    return args;
}

void handleBackgroundListCommand(background_process *current_background_process){
    			
    while (current_background_process != NULL) {
        cout << current_background_process->process_id << ": " << getcwd(NULL, 0) << " " << current_background_process->command << endl;
        current_background_process = current_background_process->next;
    }
}


void handleBackgroundCommand(char **args, string input, background_process root,
                                background_process *tail, int *process_count) {
               
    pid_t process_id = fork();
    
    //If child, execute command
    if (process_id == 0) {
        execvp(args[0], args);
    } else {
        // Parent process
        background_process *process = new background_process;
        process->process_id = process_id;
        process->command = input;
        process->next = NULL;

        // If no processes exist, start at root.
        // Otherwise, add to linked list.
        if (process_count == 0) {
            root = *process;
            tail = process;
            process_count++;
        } else {
            tail->next = process;
            tail = process;
            process_count++;
        }
    }
        
}

int main() {
   
	// Variables to track background processes
	background_process *root_background_process = new background_process;
	background_process *tail = new background_process;
	int process_count = 0;

    while(true) {
        string user_input;
	    vector<string> tokenized_user_input;

        // Prompt the user for input
        cout << "SSI: " << getcwd(NULL, 0) << " > ";
        getline(cin, user_input);		
		
		//Check for child processes
		if (process_count > 0) {
			pid_t pid_terminate = waitpid(-1, NULL, WNOHANG);
			
			if (pid_terminate > 0){
				checkBackgroundExecution(root_background_process, pid_terminate);
			}
		}

        // Check to see if the user entered the "exit" command, and exit the shell accordingly
        if (user_input.compare("exit") == 0
            || user_input.compare("exit") == 10) {
            return EXIT_SUCCESS;
        }		

        // Convert input into vector of strings.
        stringstream inputstream(user_input);
        string input_item;

        while(getline(inputstream, input_item, ' ')) {
            tokenized_user_input.push_back(input_item);
        }

        // Return to the beginning of the while loop if no commands are provided.
        if (tokenized_user_input.size() < 1) {
            continue;
        }
		
        // String to store user command input
		string command_input = tokenized_user_input[0];

        // Background command case (i.e "bg" linux command)
        if (command_input.compare("bg") == 0) {
            char **args = convertStringArguments(tokenized_user_input);
            handleBackgroundCommand(args, user_input, *root_background_process, tail, &process_count);
        } 

		/**
         * bglist case
         * 
         * Background list command to print out background processes currently waiting to be executed.
         * 
         * */ 
		
		else if (command_input.compare("bglist") == 0) {
            handleBackgroundListCommand(root_background_process);
        }		
		
        /**
         * "cd" command case
         * 
         * Change directories to specified directory
        */
		else if (command_input.compare("cd") == 0
            || command_input.compare("cd") == 10) {		
            handleChangeDirectoryCommand(tokenized_user_input);
            continue;
        }

		/**
         * Else case
         * 
         * Create a child process and do either:
         *  1) Execute the process (i.e. if in the child process, process_id = 0)
         *  2) Wait for the child process (i.e. if in the parent process, process_id != 0)
        */
		else {
			pid_t process_id = fork();
			
			if (process_id == 0) {
				char **args = convertStringArguments(tokenized_user_input);
				execvp(args[0], args);
			} else {
                waitpid(process_id, NULL, 0);
            }
            continue;
		}
		
    }
}
