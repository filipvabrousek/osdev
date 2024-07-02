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

void main(){
    char * v = (char*) 0xb8000;
    *v = 'a';
    *v = count(2, 3);  // 6  (2, 8) => @                     //'3' + 3 + 3;




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