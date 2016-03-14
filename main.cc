///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef _TEST

// ==== Includes ====
#include "Program.h"

// ==== Main Program ====
int main(int argc, char *argv[])
{
	Program program;
	return program.Execute();
}

#else

#include <gtest/gtest.h>

// ==== Main Program ====
int main(int argc, char *argv[]) {
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

	printf("Press enter to quit...");
	getchar();
	return result;
}

#endif
