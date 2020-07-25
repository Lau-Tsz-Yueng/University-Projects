/* Polynomial solver */
// Programme description is writting in README.txt

// Unit: Further Computer Programming
// Assignment name: EMAT10006
// Date Created: 28 July 2019
// Author: Liu Ziyang
// Version: 1.0 (fixed for test)

// Library and Header files
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

// SIZE for 20 command line arguments
#define SIZE 20

// TRUE and FALSE used for check zero roots
#define FALSE 0
#define TRUE 1

// MAX_SIZE storage for factor of constant term or leading coefficient
#define MAX_SIZE 100

//The order variable is used to count the number of possible roots
int order = 0;

// find_factor function is going to find the possible factor of leading coefficient and factor of constant term
int find_possible_factor(int * array, int term, int count_number){
	if (term < 0){
		term = -term;
	}// check if the term is less than zero, if it is, it will be converted to positive integer for more convenient algorithm
	for (int i = 1, n = 0; i <= term; ++i){//for loop
		if ((term % i) == 0){//check if the number is able to be divided
			array[n] = i;// append this divider into array
			n++;// number of possible root increased by 1
			array[n] = -i;// append the negative number, because negative number could be possible root too
			n++;// number of possible root increased by 1
		}
		count_number = n;// count how many possible roots in this array
	}
	return count_number;// return the value, it will be used for next modules
}

int substitution(int argc, int * array, int * roots, int * root, int count, int count_number){//calculation, substitute possible roots into the equations
	long long sum = 0;//sum for the substituted results of each possible roots
	for (int x = 0; x <= count_number-1; ++x){
		for (int i = 0, n = 1; i <= argc-2; ++i){
			sum += (root[i]) * (pow(array[x], (count-n)));//calculation for each possible roots
			n++;}// each step, the order should be decrease
		if (sum == 0){// check if the substitution results equal to zero
			roots[order] = array[x];// store it in array
			order++;//number of roots increase
		}
		sum = 0;// empty the sum for re-use purpose 
}
	return order;//number of roots should be record
}

void Check_Repeated_Numbers(int order, int * roots){//function used for check repeated numbers in roots array(roots[SIZE])
	for (int i = 0; i < order; i++){
		if ((roots[i] == roots[i-1]) && (roots[i] != 0)){
			roots[i] = roots[i+1];
			order = order - 1;
		}
		/// check repeated factors: ///
		for (int x = order; x != 0; --x){// go through all the numbers in array, check if there is same value number
			if ((roots[i] == roots[x]) && (x != i)){//if there are two numbers that are equally value
				roots[x] = roots[x+1];// remove the roots[x] by replace the roots[x+1](the number next to root[x])
			}
		}
	}
}

void OUTPUT(int * roots, int space_count, int order, int check_zero_root){//Output the information
	for (int i = 0; i < order; i++){
		if (roots[i] != 0){//check if it is not zero
				switch (space_count){//check if there is a space
					case 0://no space detected
						printf("%d", roots[i], stdout);//this is the first number should be output
						space_count++;
						break;
					case 1://space detected
						printf(" ");//enter a space
					    printf("%d", roots[i], stdout);}//output the root

		}
	}
	if ((order == 0) && (check_zero_root == FALSE)){//if there is no number of possible root and no zero root
		printf("No integer roots", stdout);
	}
}
                   /*          //////MAIN LOOP//////         */
int main(int argc, char *argv[])
{
	int root[SIZE] = {0};//possible roots will be stored in this array

	if (argc == 1){//check if there is no arguments
		fprintf(stderr, "./poly.exe --usage\n./poly.exe --version\n./poly.exe COEFFICIENTS...\n");
		return 1;
	}
	for (int i = 1; i <= argc-1; i++){//print version
    	if (strcmp(argv[1], "--version") == 0){
    		fprintf(stdout, "1.0\n");
     		return 0;
     	}
		else if (strcmp(argv[1], "--usage") == 0){//print usage
			fprintf(stdout, "./poly.exe --usage\n./poly.exe --version\n./poly.exe COEFFICIENTS...\n");
			return 0;
		}
}
	// initialize variables //
	int count = 0;	//used to count number of arguments from stdin
	int const_term;	//initialize the constant term variable
	int leading_coef; //initialize the leading coefficient variable
	int check_zero_root = FALSE; //check zero root
	int space_count = 0;// used to check if space existed 

	for (int i = 1, d = 0; i <= argc-1; ++i){
		count += 1;// count number of arguments
		if (i == 1){
			sscanf(argv[1], "%d", &leading_coef);//leading coefficient obtained
	}
		sscanf(argv[i], "%d", &root[d++]);//take all the arguments into root array
	}
	sscanf(argv[count], "%d", &const_term);//constant term obtained
	if (const_term == 0){// if last term equals to zero
		sscanf(argv[count-1],"%d", &const_term); // if the constant term equals to zero, shift left position for one.//
		printf("0", stdout);//zero output first
		space_count++;//space added behind
		check_zero_root = TRUE;// it has zero root
	}
	
	int const_array[MAX_SIZE] = {0};//initialize the factor array of constant term 
	int count_number_2 = 0;//initialize the count number for this const_array
	find_possible_factor(const_array, const_term, count_number_2);//find possible factor of constant term
	count_number_2 = find_possible_factor(const_array, const_term, count_number_2);//record the number of factors

	int leading_array[MAX_SIZE] = {0};//initialize the factor arrat of leading coefficient
	int count_number = 0; //initualize the count number for leading_array
	find_possible_factor(leading_array, leading_coef, count_number);//find possible factor of leading coefficient
	count_number = find_possible_factor(leading_array, leading_coef, count_number);//record the number of factors

	int roots[SIZE] = {0};//initialize the array for possible roots
	substitution(argc, leading_array, roots, root, count, count_number);
	substitution(argc, const_array, roots, root, count, count_number_2);
	//substitute the possible roots into equation check if it equals to zero
	Check_Repeated_Numbers(order, roots);//check if there is same value numbers

	OUTPUT(roots, space_count, order, check_zero_root);//screen output

	printf("\n");//line appended
	return 0;
}
