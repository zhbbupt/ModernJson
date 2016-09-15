#pragma once
#include <deque>
#include <string>
#include <iostream>
#include <stdint.h>
#include <type_traits>
#include <unordered_map>
namespace conf
{
	namespace json
	{
		class Json;
		typedef std::deque<Json> JsonList;
		typedef std::unordered_map<std::string, Json> JsonDict;
		typedef std::string JsonString;
		enum class JSON_TYPE
		{
			JSON_NULL,
			JSON_BOOL,
			JSON_INT,
			JSON_FLOAT,
			JSON_STRING,
			JSON_LIST,
			JSON_DICT
		};
		union JsonValue
		{
			JsonList								*List;
			JsonDict								*Dict;
			JsonString  					    	*String;
			double									Float;
			long									Int;
			bool									Bool;
			JsonValue(double d) : Float(d){}
			JsonValue(long   l) : Int(l){}
			JsonValue(bool   b) : Bool(b){}
			JsonValue(JsonString s) : String(new std::string(s)){}
			JsonValue() : Int(0){}
		};
		class Json
		{
		public:
			Json() :_json_value(), _json_type(JSON_TYPE::JSON_NULL){}
			~Json()
			{
				switch (this->_json_type) {
				case JSON_TYPE::JSON_LIST:
					delete this->_json_value.List;
					break;
				case JSON_TYPE::JSON_DICT:
					delete this->_json_value.Dict;
					break;
				case JSON_TYPE::JSON_STRING:
					delete this->_json_value.String;
					break;
				default:;
				}
			}
			template<typename T>
			Json(T b, typename std::enable_if<std::is_same<T, bool>::value>::type* = 0) : _json_value(b), _json_type(JSON_TYPE::JSON_BOOL){}

			template <typename T>
			Json(T i, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type* = 0) : _json_value((long)i), _json_type(JSON_TYPE::JSON_INT){}

			template <typename T>
			Json(T f, typename std::enable_if<std::is_floating_point<T>::value>::type* = 0) : _json_value((double)f), _json_type(JSON_TYPE::JSON_FLOAT){}

			template <typename T>
			Json(T s, typename std::enable_if<std::is_convertible<T, JsonString>::value>::type* = 0) : _json_value(JsonString(s)), _json_type(JSON_TYPE::JSON_STRING){}

			Json(std::nullptr_t) : _json_value(), _json_type(JSON_TYPE::JSON_NULL){}

			bool setType(JSON_TYPE type) 
			{
				if (type == this->_json_type)
					return true;

				switch (this->_json_type)
				{
				case JSON_TYPE::JSON_DICT:   delete this->_json_value.Dict;    break;
				case JSON_TYPE::JSON_LIST:   delete this->_json_value.List;   break;
				case JSON_TYPE::JSON_STRING: delete this->_json_value.String; break;
				default:;
				}

				switch (type)
				{
				case JSON_TYPE::JSON_NULL:      this->_json_value.Dict = nullptr;				break;
				case JSON_TYPE::JSON_DICT:		this->_json_value.Dict = new std::unordered_map<std::string, Json>();	break;
				case JSON_TYPE::JSON_LIST:		this->_json_value.List = new std::deque<Json>();						break;
				case JSON_TYPE::JSON_STRING:    this->_json_value.String = new JsonString();    break;
				case JSON_TYPE::JSON_FLOAT:		this->_json_value.Float = 0.0;                  break;
				case JSON_TYPE::JSON_INT:		this->_json_value.Int = 0;						break;
				case JSON_TYPE::JSON_BOOL:		this->_json_value.Bool = false;                 break;
				}

				this->_json_type = type;
				return true;
			}

			JSON_TYPE getType()
			{
				return this->_json_type;
			}

			template <typename T>
			typename std::enable_if<std::is_same<T, bool>::value, Json&>::type operator=(T b)
			{
				this->setType(JSON_TYPE::JSON_BOOL); 
				this->_json_value.Bool = b; 
				return *this;
			}

			template <typename T>
			typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, Json&>::type operator=(T i)
			{
				this->setType(JSON_TYPE::JSON_INT);
				this->_json_value.Int = i;
				return *this;
			}

			template <typename T>
			typename std::enable_if<std::is_floating_point<T>::value && !std::is_same<T, bool>::value, Json&>::type operator=(T i)
			{
				this->setType(JSON_TYPE::JSON_FLOAT);
				this->_json_value.Float = i;
				return *this;
			}

			template <typename T>
			typename std::enable_if<std::is_convertible<T, std::string>::value,Json&>::type operator=(T s)
			{
				this->setType(JSON_TYPE::JSON_STRING); 
				*(this->_json_value.String) = std::string(s); 
				return *this;
			}

			Json& operator[](int32_t index) 
			{
				this->setType(JSON_TYPE::JSON_LIST);
				if (index >= this->_json_value.List->size())
				{
					this->_json_value.List->resize(index + 1);
				}
				return this->_json_value.List->operator[](index);
			}

			Json& operator[](const std::string &key) 
			{
				this->setType(JSON_TYPE::JSON_DICT); 
				return this->_json_value.Dict->operator[](key);
			}

			bool ToBool() 
			{
				if (this->_json_type == JSON_TYPE::JSON_BOOL)
				{
					return this->_json_value.Bool;
				}
				return false;
			}

			std::string ToString()  
			{
				if(this->_json_type == JSON_TYPE::JSON_STRING)
				{
					return std::move(this->_json_escape(*(this->_json_value.String)));
				}
				return std::string("");
			}



		public:

			static Json Make(JSON_TYPE type) 
			{
				Json ret; 
				ret.setType(type);
				return ret;
			}
			static Json JsonList()
			{
				return std::move(Json::Make(JSON_TYPE::JSON_LIST));
			}
			static Json JsonDict()
			{
				return std::move(Json::Make(JSON_TYPE::JSON_DICT));
			}

		private:
			JsonValue  _json_value;
			JSON_TYPE  _json_type;

			std::string _json_escape(const std::string &str)
			{
				std::string output;
				for (int i = 0; i < str.length(); i++)
				{
					switch (str[i]) {
					case '\"': output += "\\\""; break;
					case '\\': output += "\\\\"; break;
					case '\b': output += "\\b";  break;
					case '\f': output += "\\f";  break;
					case '\n': output += "\\n";  break;
					case '\r': output += "\\r";  break;
					case '\t': output += "\\t";  break;
					default: output += str[i]; break;
					}
				}
				return std::move(output);
			}


		public:
			static Json loadJson(const std::string &str)
			{
				int32_t offset = 0;
				//return NULL;
				return std::move(Json::_json_parser->parse_next(str, offset));
			}

		private:
			class JsonParser
			{
			public:
				JsonParser(){};
				~JsonParser(){};
				bool skip_space(const std::string &str, int32_t &offset)
				{
					while (isspace(str[offset]))
					{
						++offset;
					};
					return true;
				}

				Json parse_dict(const std::string &str, int32_t &offset)
				{
					Json jsonDict = Json::Make(JSON_TYPE::JSON_DICT);

					++offset;
					this->skip_space(str, offset);
					if (str[offset] == '}')
					{
						++offset;
						return std::move(jsonDict);
					}

					while (true)
					{
						Json Key = parse_next(str, offset);
						this->skip_space(str, offset);
						if (str[offset] != ':')
						{
							std::cerr << "Error: Object: Expected colon, found '" << str[offset] << "'\n";
							break;
						}
						this->skip_space(str, ++offset);
						Json Value = parse_next(str, offset);
						jsonDict[Key.ToString()] = Value;

						this->skip_space(str, offset);
						if (str[offset] == ',')
						{
							++offset;
							continue;
						}
						else if (str[offset] == '}')
						{
							++offset;
							break;
						}
						else
						{
							std::cerr << "ERROR: Object: Expected comma, found '" << str[offset] << "'\n";
							break;
						}
					}

					return std::move(jsonDict);
				}

				Json parse_list(const std::string &str, int32_t &offset)
				{
					Json jsonList = Json::Make(JSON_TYPE::JSON_LIST);
					int32_t index = 0;

					++offset;
					this->skip_space(str, offset);
					if (str[offset] == ']')
					{
						++offset;
						return std::move(jsonList);
					}

					while (true)
					{
						jsonList[index++] = parse_next(str, offset);
						this->skip_space(str, offset);

						if (str[offset] == ',')
						{
							++offset;
							continue;
						}
						else if (str[offset] == ']')
						{
							++offset;
							break;
						}
						else
						{
							std::cerr << "ERROR: Array: Expected ',' or ']', found '" << str[offset] << "'\n";
							return std::move(Json::Make(JSON_TYPE::JSON_LIST));
						}
					}

					return std::move(jsonList);
				}

				Json parse_string(const std::string &str, int32_t &offset)
				{
					std::string val;
					for (char c = str[++offset]; c != '\"'; c = str[++offset])
					{
						if (c == '\\')
						{
							switch (str[++offset])
							{
							case '\"': val += '\"'; break;
							case '\\': val += '\\'; break;
							case '/': val += '/'; break;
							case 'b': val += '\b'; break;
							case 'f': val += '\f'; break;
							case 'n': val += '\n'; break;
							case 'r': val += '\r'; break;
							case 't': val += '\t'; break;
							case 'u':
							{
								val += "\\u";
								for (int32_t i = 1; i <= 4; ++i)
								{
									c = str[offset + i];
									if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
									{
										val += c;
									}
									else
									{
										std::cerr << "ERROR: String: Expected hex character in unicode escape, found '" << c << "'\n";
										return std::move(Json::Make(JSON_TYPE::JSON_STRING));
									}
								}
								offset += 4;
							} break;
							default: val += '\\'; break;
							}
						}
						else
						{
							val += c;
						}
					}
					++offset;
					Json jsonString = val;
					return std::move(jsonString);
				}

				Json parse_number(const std::string &str, int32_t &offset)
				{
					Json Number;
					std::string val, exp_str;
					char c;
					bool isDouble = false;
					long exp = 0;
					while (true)
					{
						c = str[offset++];
						if ((c == '-') || (c >= '0' && c <= '9'))
						{
							val += c;
						}
						else if (c == '.')
						{
							val += c;
							isDouble = true;
						}
						else
						{
							break;
						}
					}
					if (c == 'E' || c == 'e')
					{
						c = str[offset++];
						if (c == '-')
						{
							++offset;
							exp_str += '-';
						}
						while (true)
						{
							c = str[offset++];
							if (c >= '0' && c <= '9')
								exp_str += c;
							else if (!isspace(c) && c != ',' && c != ']' && c != '}')
							{
								std::cerr << "ERROR: Number: Expected a number for exponent, found '" << c << "'\n";
								return std::move(Json::Make(JSON_TYPE::JSON_NULL));
							}
							else
								break;
						}
						exp = std::stol(exp_str);
					}
					else if (!isspace(c) && c != ',' && c != ']' && c != '}')
					{
						std::cerr << "ERROR: Number: unexpected character '" << c << "'\n";
						return std::move(Json::Make(JSON_TYPE::JSON_NULL));
					}
					--offset;

					if (isDouble)
					{
						Number = std::stod(val) * std::pow(10, exp);
					}
					else
					{
						if (!exp_str.empty())
						{
							Number = std::stol(val) * std::pow(10, exp);
						}
						else
						{
							Number = std::stol(val);
						}
					}
					return std::move(Number);
				}

				Json parse_bool(const std::string &str, int32_t &offset)
				{
					Json jsonBool;
					if (str.substr(offset, 4) == "true")
					{
						jsonBool = true;
					}
					else if (str.substr(offset, 5) == "false")
					{
						jsonBool = false;
					}
					else
					{
						std::cerr << "ERROR: Bool: Expected 'true' or 'false', found '" << str.substr(offset, 5) << "'\n";
						return std::move(Json::Make(JSON_TYPE::JSON_NULL));
					}
					offset += (jsonBool.ToBool() ? 4 : 5);
					return std::move(jsonBool);
				}

				Json parse_null(const std::string &str, int32_t &offset)
				{
					Json jsonNull;
					if (str.substr(offset, 4) != "null")
					{
						std::cerr << "ERROR: Null: Expected 'null', found '" << str.substr(offset, 4) << "'\n";
						return std::move(Json::Make(JSON_TYPE::JSON_NULL));
					}
					offset += 4;
					return std::move(jsonNull);
				}

				Json parse_next(const std::string &str, int32_t &offset)
				{
					char value;
					this->skip_space(str, offset);
					value = str[offset];
					switch (value)
					{
					case '[':  return std::move(this->parse_list(str, offset));
					case '{':  return std::move(this->parse_dict(str, offset));
					case '\"':  return std::move(this->parse_string(str, offset));
					case 't':
					case 'f':  return std::move(this->parse_bool(str, offset));
					case 'n':  return std::move(this->parse_null(str, offset));
					default:
					{
						if ((value <= '9' && value >= '0') || value == '-')
						{
							return std::move(parse_number(str, offset));
						}
					}
					}
					std::cerr << "ERROR: Parse: Unknown starting character '" << value << "'\n";
					return Json();
				}

			};

			static JsonParser* _json_parser;

		};
		
		Json::JsonParser* Json::_json_parser = new Json::JsonParser();
	}
}

