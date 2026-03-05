#include <stdio.h>

int main(int argc,char*argv[]){
    printf("argc = %d\n",argc);
    for(int i=0;i<argc;i++){
        printf("argv[%d]=%s\n",i,argv[i]);
    }

    int a=5;
    printf("a=%d\n",a);
    printf("dir a=%p\n",&a);

    int*b;
    printf("dir b=%p\n",b);
    *b=5;
    printf("dir b=%p\n, valor de b=%d\n",b,*b);

    int*c=malloc(sizeof(int));
    return 0;
}