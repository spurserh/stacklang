
#include <cstdio>

#ifdef FLAG_FOO
const char* message = "Flag on";
#else
const char* message = "Flag off";
#endif//FLAG_FOO


int main()
{
	// Wow
	fprintf(stderr, "Message: %s\n", message);
	return 0;
}