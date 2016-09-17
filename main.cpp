#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <initializer_list>
#include "json.hpp"
using namespace std;
using conf::json::Json;
using namespace conf::json;
int main(int argc, char** argv)
{
	Json obj;
	Json a = Json("parsed");
	obj = a;
	obj["parsed"] = Json::loadJson(" [{ \"Key\" : \"Value\" , \"Key3\":false },false,[1,3,1.5,3.1]]");
	string str = obj.dump();
	cout << str << endl;
	cout << endl;
	string str3 = obj.ToString();
	cout << str3 << endl;
	cout << endl;

	obj["parsed"][1] = true;
	obj["parsed"][3] = 0.3;
	obj["parsed"][4] = Json::JsonList("a",3,true);
	str3 = obj.dump();
	cout << str3 << endl;
	cout << obj["parsed"][3].getValue<float>() << endl;
	cout << obj["parsed"] << endl;
	cout << obj.hasKey("parsed") << endl;
	cout << obj["parsed"].size() << endl;
	Json b = obj["parsed"].at(6) ;
	Json c = obj["parsed"][1];
	JsonWrapper<json_dict> tmp = c.getJsonDictWrapper();
	system("pause");
	return EXIT_SUCCESS;
}