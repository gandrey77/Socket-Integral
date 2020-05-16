#include<stdio.h>
#include<stdlib.h>
#include<math.h>

int main( int argc, char *argv[] ){


    float range = 0;
    float f_init = 0;
    float f_end = 0;
    double total = 0;
    double point1 = 0;
    double point2 = 0;
    float i = 0;

    range = atof(argv[1]);
    f_init = atof(argv[2]);
    f_end =atof(argv[3]);

    
    for (i = f_init; i + range <= f_end; i+= range ){
        point1 = sqrt(pow(100, 2) - pow(i,2));
        point2 = sqrt(pow(100, 2) - pow((i + range), 2));
       
        total += (point1 + point2)*range/2;
        
    }

    printf("O resultado foi: %f\n", total);
    

}