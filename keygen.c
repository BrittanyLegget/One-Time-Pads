/*
Author: Brittany Legget
Date: 3/3/2022
CS 344 Assignment #5 - One-Time-Pads -> Keygen

*/

/*
This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, 
generated using the standard Unix randomization methods. Do not create spaces every five characters, as has been historically done. 
Note that you specifically do not have to do any fancy random number generation: we’re not looking for cryptographically secure random 
number generation. rand() (Links to an external site.) is just fine. The last character keygen outputs should be a newline. Any error 
text must be output to stderr.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char* argv[]) {

	int length;
	//Add 1 character to specified length for the newline character at the end
	char key[length + 1];
	char randChar;

	//seed
	srand(time(NULL));

	/*Array of allowable characters to randomly generate from
	Source: https://edstem.org/us/courses/16718/discussion/1178613 */
	char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	//cast given length from char to int
	length = atoi(argv[1]); 

	/*create key of specified length by looking throughand randomly selecting a character from allowable character array
	Random character from array source: https://stackoverflow.com/questions/17215242/random-element-from-array-in-c */
	for (int i = 0; i < length; i++) {
		randChar = char_set[rand() % 27];
		key[i] = randChar;

	}
	//Add null terminator to end of string
	key[length] = '\n'; 

	printf("%s", key);

	return 0;
}
