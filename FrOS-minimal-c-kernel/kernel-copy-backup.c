typedef struct kernel
{
    //char name [20];
    int age;
};

int count(int a, int b){
    return '0' + a * b;//'3' + 3 + 3;
}

char * get(){
    return "Hello";
}


void itoa(int num, char *str) {
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        isNegative = 1;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
    }

    // Add '-' if the number is negative
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0';

    // Reverse the string
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

void main(){
   /* char * v = (char*) 0xb8000;
    *v = 'a';
    *v = count(2, 3);  // 6  (2, 8) => @                     //'3' + 3 + 3;
*/

char *v = (char*) 0xb8000; // Pointer to video memory
    char buffer[20];

    itoa(count(2, 3), buffer); // Convert result to string
    for (int i = 0; buffer[i] != '\0'; i++) {
        *(v + i * 2) = buffer[i]; // Write each character to video memory
    }

    itoa(count(2, 8), buffer); // Convert result to string
    for (int i = 0; buffer[i] != '\0'; i++) {
        *(v + 20 + i * 2) = buffer[i]; // Write each character to video memory at a different location
    }



   /*  struct kernel a = {24};
   *v = 24; // arr. up
    *v = a.age; // Why arrow up ? also this *v = 24; 
    */
    // *v = 'a' + a.age; yea




// malloc(); #include <stdlib.h> cannot use
    
    //obj kernel = {24};

  /*  *v = '@';

    int a = 3;
    int b = 4;

    if (b > a){
        *v = 'U';
    }*/
}