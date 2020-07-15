/* Basic Information of the Program */
//Author: Ziyang Liu
//Created Date: 27/July/2019
//Version: 1.0

//Description:
This program is polynomial solver.
Find_possible_factor function is used to find factor of constant term and leading coefficient.
Substitution function is used to substitute possible roots into equation, and to see whether or not it is zero.
Check_repeated_numbers fiction is used to check if there is same value number recorded and decide to delete one of them or not.
OUTPUT function is used for output the information of results.
The codes written on main function has three sections.
Firstly, it will check arguments, to see if there is no argument or special arguments(--usage or --version), and it will find leading coefficients and constant terms, also record and store other arguments into array.
Secondly, find the factor of leading coefficient and constant term, and substitute the factors into the equation to see if it is zero or not.
Thirdly, check if there is same value numbers, and screen output.

// How to compile:
make

//How to use the code:
./poly.exe ~~~~~
(*enter variables on ~~~~~~~~)

/* CHANGE LOG */
/* Date Changed - Changes */
// 27 July 2019  - Basic File Structure created.
// 28 July 2019  - Function created and organised.
// 28 July 2019  - Arguments are differentiated into arrays.
// 28 July 2019  - Initialize variables and use 'typedef struct' to build global variables.
// 29 July 2019  - Functions related to variables.
// 29 July 2019  - *argv[] converted to int type array.
// 29 July 2019  - Substitution function built.
// 30 July 2019  - Error message found when using 'typedef struct' and remove.
// 30 July 2019  - Error message found when using #include<bigint.h>. 
// 30 July 2019  - Test file running.
// 30 July 2019  - Error found. (23 tests passed, cannot handle the big numbers, and used long long type instead.)
// 31 July 2019  - Negative number argument detected and converted to positive number.
// 31 July 2019  - '--version' and '--usage' command response function building.
// 31 July 2019  - Test Finished with 25 passed, 0 fails.
// 31 July 2019  - Comments for code has been written.
