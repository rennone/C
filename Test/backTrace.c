#include <stdio.h>
#include <string.h>

#define NUM_CHAR 128
int skip[NUM_CHAR];

int calc_skip(char *pattern)
{
  int i, m;
  m = strlen(pattern);

  for(i=0; i<NUM_CHAR; i++)
    skip[i] = m;

  for(i=0; i<m; i++)
    skip[pattern[i]] = m-i-1;

  return m;
}

int faster_match(char *text, char *pattern, int n)
{
  int m = calc_skip(pattern) - 1;

  int i=m, j;
  while(i<n)
  {
    j = m;
    while( j >= 0 )
    {
      if( text[i] == pattern[j] )
      {
        if(j==0)
          return i;
        
        i--;j--;
      }
      else
        break;
    }

    i += m-j+1+skip[text[i]];
  }

  return -1;    
}

int main()
{
  char *text    = "abcccc";//"XYZde0XZZkWXYZ";
  char *pattern = "bcccc";//""WXYZ";

  int n = strlen(text);


  int m = calc_skip(pattern);

  int i;
  for(i=0; i<m; i++)
    printf("%d ",skip[pattern[i]]);

  printf("\n%d \n", faster_match(text, pattern, n));
  return 0;
}
