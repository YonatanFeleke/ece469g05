#include <stdio.h>

typedef struct
{
  int int1;
	int int2;
	char char1;
	int int3;
}test_struct;

int main()
{
	test_struct struct1, struct2;
	test_struct *struct3;

	struct1.int1 = 1;
	struct1.int2 = 2;
	struct1.char1 = 'a';
	struct1.int3 = 3;

	printf("struct1 -- int1: %d, int2: %d, char1: %c, int3: %d\n", 	struct1.int1, struct1.int2, struct1.char1, struct1.int3);

	struct2 = struct1;
	printf("struct2 -- int1: %d, int2: %d, char1: %c, int3: %d\n", 	struct2.int1, struct2.int2, struct2.char1, struct2.int3);

	struct1.int1 = 4;

	printf("struct1 -- int1: %d, int2: %d, char1: %c, int3: %d\n", 	struct1.int1, struct1.int2, struct1.char1, struct1.int3);
	printf("struct2 -- int1: %d, int2: %d, char1: %c, int3: %d\n", 	struct2.int1, struct2.int2, struct2.char1, struct2.int3);

	struct3 = &struct1;
	printf("struct3 -- int1: %d, int2: %d, char1: %c, int3: %d\n", 	struct3->int1, struct3->int2, struct3->char1, struct3->int3);


	return(0);

}
