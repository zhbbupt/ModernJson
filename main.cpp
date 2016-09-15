#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <initializer_list>
#include "json.hpp"
using namespace std;
using conf::json::Json;
int main(int argc, char** argv)
{
	Json obj;
	obj["parsed"] = Json::loadJson("[ { \"Key\" : \"Value\" }, false ]");
	return EXIT_SUCCESS;
}