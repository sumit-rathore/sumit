#include <stdio.h>
#include <stdlib.h>

int *p;
int list[3];

int main()
{
    int i;
    for(i = 0; i < sizeof(list); i++)
    {
        list[i] = i;
    }
    p = list;

    printf("p = %d\n", p);
    printf("&p = %d\n", &p);

    printf("*p = %d\n", *p);
    printf("*(p + 1) = %d\n", *(p+1));

    int x;
    int j = 7;
    int sum = 0;
    for(x=1; x < j; x ++)
    {
        sum = x + j;
    }
    printf("sum = %d\n", sum);

    int num2(int x)
    {
        int ans = 0;
        int temp = x;
        while (x > 1)
        {
            ans = x + temp;
            x--;
        }
        return ans;
    }
               
    int num(int x)
    {
        if (x < 2)
            return x;
        return (num(x-2) + (x-1));
    }

    printf("num(7) = %d\n", num(7));
    printf("num2(7) = %d\n", num2(7));
    return 0;
}
