// Class automatically generated by Dev-C++ New Class wizard

#include "test_properties.h" // class's header file

// class constructor
PropertiesTest::PropertiesTest()
{
	// insert your code here
}

// class destructor
PropertiesTest::~PropertiesTest()
{
	// insert your code here
}

/**
 * Launch all the tests on the properties
 */
void PropertiesTest::testAll()
{
	cout << "- test AccessMethods" << endl;
  testAccessMethods();
}

/**
 * Test the accessor of the properties
 */
void PropertiesTest::testAccessMethods()
{
	Properties::SetProperty("testAccessMethods", "hop");
	assert(Properties::GetProperty("testAccessMethods").compare("hop") == 0);
}