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
	string str3 = obj.toString();
	cout << str3 << endl;
	cout << endl;

	Json test = Json::loadJson("{\"EXTERNAL_F0_FILENAME\" : \"wavs / filename.F0\"}");
	cout << test << endl;

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
	Json c = obj["parsed"][0];

	//load json file
	Json jf1 = Json::loadJsonFile("test.json");
	cout << jf1 << endl;

	Json jf = Json::loadJsonFile("config_analyse_test.json");
	cout << jf << endl;

	cout << jf["Common parameters"]["SAMPLING_FREQUENCY"] << endl;

	jf.dumpToFile("test_out.json");

	//iterator
	auto tmp = c.getJsonWrapper<json_dict>();

	for (auto& ae : tmp)
	{
		cout << ae.first << ":" << ae.second << endl;
	}
	for (auto& ae : c.getJsonWrapper<json_dict>())
	{
		cout << ae.first << ":" << ae.second << endl;
	}
	for (auto ae = tmp.begin(); ae != tmp.end();ae++)
	{
		cout << (*ae).first << ":" << (*ae).second << endl;
	}

	Json keys = c.getKeys();
	for (int i = 0; i < keys.size();i++)
	{
		cout << keys[i].getValue<string>() << endl;
	}

	for (int i = 0; i < keys.size(); i++)
	{
		cout << keys[i].getValue<string>() << endl;
	}

	auto tmp2 = obj["parsed"][2].getJsonWrapper<json_list>();

	for (auto& ae : tmp2)
	{
		cout << ae << " ";
	}
	cout << endl;
	for (auto& ae : obj["parsed"][2].getJsonWrapper<json_list>())
	{
		cout << ae << " ";
	}
	cout << endl;
	for (auto ae = tmp2.begin(); ae != tmp2.end(); ae++)
	{
		cout << *ae << " ";
	}
	cout << endl;
	system("pause");
	return EXIT_SUCCESS;
}