#include <stdio.h>

int	main ( int argc, char *argv[] )
{
    printf("Content-Type: text/html;charset=us-ascii\n\n");
    printf("<htmL><body>lalala %d</body></html>", 12);
    return 0;
}
