#pragma once
#include <deque>
#include <string>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <type_traits>
#include <unordered_map>
namespace conf
{
	namespace json
	{
		class Json;
		typedef std::deque<Json> json_list;
		typedef std::unordered_map<std::string, Json> json_dict;
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
			json_list								*List;
			json_dict								*Dict;
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
		template <typename Container>
		class JsonWrapper
		{
		public:
			Container *object;

		public:
			JsonWrapper(Container *val) : object(val) {}
			JsonWrapper(std::nullptr_t) : object(nullptr) {}

			typename Container::iterator begin() { return object ? object->begin() : typename Container::iterator(); }
			typename Container::iterator end() { return object ? object->end() : typename Container::iterator(); }
			typename Container::const_iterator begin() const { return object ? object->begin() : typename Container::iterator(); }
			typename Container::const_iterator end() const { return object ? object->end() : typename Container::iterator(); }
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

			Json(Json&& other): _json_value(other._json_value), _json_type(other._json_type)
			{
				other._json_type = JSON_TYPE::JSON_NULL;
				other._json_value.Dict = nullptr;
				other._json_value.List = nullptr;
				other._json_value.String = nullptr;
			}

			Json& operator=(Json&& other)
			{
				this->_json_value = other._json_value;
				this->_json_type = other._json_type;
				other._json_value.Dict = nullptr;
				other._json_value.List = nullptr;
				other._json_value.String = nullptr;
				other._json_type = JSON_TYPE::JSON_NULL;
				return *this;
			}

			Json(const Json &other) 
			{
				switch (other._json_type)
				{
				case JSON_TYPE::JSON_DICT:
				{
					this->_json_value.Dict = new std::unordered_map<std::string, Json>(other._json_value.Dict->begin(), other._json_value.Dict->end());
					break;
				}
				case JSON_TYPE::JSON_LIST:
				{
					this->_json_value.List = new std::deque<Json>(other._json_value.List->begin(), other._json_value.List->end());
					break;
				}
				case JSON_TYPE::JSON_STRING:
				{
					this->_json_value.String = new std::string(*other._json_value.String);
					break;
				}
				default:
				{
					this->_json_value = other._json_value;
				}
				}
				this->_json_type = other._json_type;
			}

			Json& operator=(const Json &other) 
			{
				switch (other._json_type) 
				{
				case JSON_TYPE::JSON_DICT:
				{
					this->_json_value.Dict = new std::unordered_map<std::string, Json>(other._json_value.Dict->begin(), other._json_value.Dict->end());
					break;
				}
				case JSON_TYPE::JSON_LIST:
				{
					this->_json_value.List = new std::deque<Json>(other._json_value.List->begin(), other._json_value.List->end());
					break;
				}
				case JSON_TYPE::JSON_STRING:
				{
					this->_json_value.String = new std::string(*other._json_value.String);
					break;
				}
				default:
				{
					this->_json_value = other._json_value;
				}
				}
				this->_json_type = other._json_type;
				return *this;
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
			typename std::enable_if<std::is_same<T, bool>::value, bool>::type getValue()
			{
				switch (this->_json_type)
				{
				case JSON_TYPE::JSON_BOOL:
				{
					return this->_json_value.Bool;
				}
				case JSON_TYPE::JSON_INT:
				{
					return static_cast<bool>(this->_json_value.Int);
				}
				case JSON_TYPE::JSON_FLOAT:
				{
					return static_cast<bool>(this->_json_value.Float);
				}
				default:
				{
					return false;
				}
				}
			}

			template <typename T>
			typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int32_t>::type getValue()
			{
				switch (this->_json_type)
				{
				case JSON_TYPE::JSON_INT:
				{
					return this->_json_value.Int;
				}
				case JSON_TYPE::JSON_FLOAT:
				{
					return static_cast<int>(this->_json_value.Float);
				}
				case JSON_TYPE::JSON_BOOL:
				{
					return static_cast<int>(this->_json_value.Bool);
				}
				default:
				{
					return 0;
				}
				}
			}

			template <typename T>
			typename std::enable_if<std::is_floating_point<T>::value && !std::is_same<T, bool>::value, float>::type getValue()
			{
				switch (this->_json_type)
				{
				case JSON_TYPE::JSON_FLOAT:
				{
					return this->_json_value.Float;
				}
				case JSON_TYPE::JSON_INT:
				{
					return static_cast<float>(this->_json_value.Int);
				}
				case JSON_TYPE::JSON_BOOL:
				{
					return static_cast<float>(this->_json_value.Bool);
				}
				default:
				{
					return 0;
				}
				}
			}

			template <typename T>
			typename std::enable_if<std::is_convertible<T, std::string>::value, std::string>::type getValue()
			{
				switch (this->_json_type)
				{
				case JSON_TYPE::JSON_STRING:
				{
					return *(this->_json_value.String);
				}
				default:
				{
					return "";
				}
				}
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

			Json& operator[](const int32_t& index) 
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

			Json& at(const std::string &key)
			{
				if (this->_json_type == JSON_TYPE::JSON_DICT)
				{
					if (this->_json_value.Dict->find(key) != this->_json_value.Dict->end())
					{
						return this->_json_value.Dict->operator[](key);
					}
					else
					{
						std::cerr << "Json has no key :" + key << std::endl;
					}
				}
				else
				{
					std::cerr << "Json is not a dict" << std::endl;
				}
				return std::move(Json());
			}

			Json& at(int32_t index) 
			{
				if (this->_json_type == JSON_TYPE::JSON_LIST)
				{
					if (this->_json_value.List->size() > index)
					{
						return this->_json_value.List->operator[](index);
					}
					else
					{
						std::cerr << "Json has no index : " + std::to_string(index) << std::endl;
					}
				}
				else
				{
					std::cerr << "Json is not a list" << std::endl;
				}
				return std::move(Json());
			}

			int size() const
			{
				if (this->_json_type == JSON_TYPE::JSON_LIST)
				{
					return this->_json_value.List->size();
				}
				else if (this->_json_type == JSON_TYPE::JSON_DICT)
				{
					return this->_json_value.Dict->size();
				}
				else
				{
					return -1;
				}
			}

			bool hasKey(const std::string &key) const 
			{
				if (this->_json_type == JSON_TYPE::JSON_DICT)
				{
					return (this->_json_value.Dict->find(key) != this->_json_value.Dict->end());
				}
				return false;
			}

			Json getKeys()
			{
				Json keyList = Json::Make(JSON_TYPE::JSON_LIST);
				if (this->_json_type==JSON_TYPE::JSON_DICT)
				{
					for (auto tmp : (*this->_json_value.Dict))
					{
						keyList.append(tmp.first);
					}
				}
				return std::move(keyList);
			}

			template <typename T>
			bool append(T arg) 
			{
				this->setType(JSON_TYPE::JSON_LIST);
				this->_json_value.List->emplace_back(arg);
				return true;
			}

			template <typename T, typename... U>
			bool append(T arg, U... args) 
			{
				bool OK = false;
				OK = append(arg);
				OK = append(args...);
				return OK;
			}

			std::string toString() const
			{
				switch (this->_json_type)
				{
				case JSON_TYPE::JSON_NULL:
				{
					return "null";
				}
				case JSON_TYPE::JSON_FLOAT:
				{
					return (this->_float_to_string(this->_json_value.Float));
				}
				case JSON_TYPE::JSON_INT:
				{
					return std::to_string(this->_json_value.Int);
				}
				case JSON_TYPE::JSON_BOOL:
				{
					return this->_json_value.Bool ? "true" : "false";
				}
				case JSON_TYPE::JSON_STRING:
				{
					return "\"" + this->_json_escape(*(this->_json_value.String)) + "\"";
				}
				case JSON_TYPE::JSON_DICT:
				{
					std::string s = "{";
					bool skip = true;
					for (auto &p : *(this->_json_value.Dict))
					{
						if (!skip) s += ",";
						s += ("\"" + p.first + "\":");

						s += (p.second.toString());
						skip = false;
					}
					s += "}";
					return s;
				}
				case JSON_TYPE::JSON_LIST:
				{
					std::string s = "[";
					bool skip = true;
					for (auto &p : *(this->_json_value.List))
					{
						if (!skip)
						{
							s += ", ";
						}
						s += p.toString();
						skip = false;
					}
					s += "]";
					return s;
				}
				default:
					return "";
				}
				return "";
			}

			std::string dump(int depth = 0, std::string tab = "  ") const
			{
				std::string pad = "";
				for (int i = 0; i < depth; ++i)
				{
					pad += tab;
				}

				switch (this->_json_type)
				{
				case JSON_TYPE::JSON_NULL:
				{
					return "null";
				}
				case JSON_TYPE::JSON_FLOAT:
				{
					/*std::string str = std::to_string(this->_json_value.Float);
					str = str.erase(str.find_last_not_of('0') + 1, std::string::npos);
					if (str[str.size() - 1] == '.') str += "0";*/
					return (this->_float_to_string(this->_json_value.Float));
				}
				case JSON_TYPE::JSON_INT:
				{
					return std::to_string(this->_json_value.Int);
				}
				case JSON_TYPE::JSON_BOOL:
				{
					return this->_json_value.Bool ? "true" : "false";
				}
				case JSON_TYPE::JSON_STRING:
				{
					return "\"" + this->_json_escape(*(this->_json_value.String)) + "\"";
				}
				case JSON_TYPE::JSON_DICT: 
				{
					std::string s = pad+"{\n";
					bool skip = true;
					for (auto &p : *(this->_json_value.Dict)) 
					{
						if (!skip) s += ",\n";
						s += (tab + pad + "\"" + p.first + "\" : ");
						if (p.second._json_type == JSON_TYPE::JSON_DICT)
						{
							s += "\n";
						}
						else if (p.second._json_type == JSON_TYPE::JSON_LIST)
						{
							for (auto &pl : *(p.second._json_value.List))
							{
								if (pl._json_type == JSON_TYPE::JSON_DICT)
								{
									s += "\n";
									break;
								}
							}
						}

						s += (p.second.dump(depth + 1, tab));
						skip = false;
					}
					s += ("\n" + pad + "}");
					return s;
				}
				case JSON_TYPE::JSON_LIST:
				{
					std::string s = "[";
					bool hasDict = false;
					for (auto &p : *(this->_json_value.List))
					{
						if (p._json_type == JSON_TYPE::JSON_DICT)
						{
							s = pad + "[\n";
							hasDict = true;
							break;
						}
					}
					
					bool skip = true;
					for (auto &p : *(this->_json_value.List)) 
					{
						if (!skip)
						{
							s += ", ";
							if (hasDict == true)
							{
								s += ("\n" + pad + tab);
							}
						}
						s += p.dump(depth + 1, tab);
						skip = false;
					}
					if (hasDict == true)
					{
						s += ("\n" + pad);
					}
					s += "]";
					return s;
				}
				default:
					return "";
				}
				return "";
			}

			bool dumpToFile(const std::string& file_path,const int8_t& mode = std::ios::app) const
			{
				std::fstream json_file;
				json_file.open(file_path, mode);
				if (json_file.good())
				{
					std::string str = this->dump();
					json_file.write(str.c_str(),str.size());
					json_file.close();
					return true;
				}
				else
				{
					return false;
				}
			}

			template <typename T>
			typename std::enable_if<std::is_same<T, json_dict>::value, JsonWrapper<json_dict>>::type getJsonWrapper()
			{
				if (this->_json_type == JSON_TYPE::JSON_DICT)
				{
					return JsonWrapper<T>(this->_json_value.Dict);
				}
				return JsonWrapper<T>(nullptr);
			}

			template <typename T>
			typename std::enable_if<std::is_same<T, json_list>::value, JsonWrapper<json_list>>::type getJsonWrapper()
			{
				if (this->_json_type == JSON_TYPE::JSON_LIST)
				{
					return JsonWrapper<json_list>(this->_json_value.List);
				}
				return JsonWrapper<json_list>(nullptr);
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

			template <typename... T>
			static Json JsonList(T... args) 
			{
				Json arr = Json::Make(JSON_TYPE::JSON_LIST);
				arr.append(args...);
				return std::move(arr);
			}

			static Json JsonDict()
			{
				return std::move(Json::Make(JSON_TYPE::JSON_DICT));
			}

			static Json loadJson(const std::string& str)
			{
				int32_t offset = 0;
				return std::move(Json::_json_parser->parse_next(str, offset));
			}

			static Json loadJsonFile(const std::string& file_path, const std::string& comment_segment = "//")
			{
				std::fstream json_file(file_path);
				std::string line = "";
				int32_t end_pos = -1;
				std::string json_string = "";
				while (std::getline(json_file, line))
				{
					end_pos = line.find(comment_segment);
					json_string += line.substr(0, end_pos);
				}
				return std::move(Json::loadJson(json_string));
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
						std::string tmp_key = Key.getValue<std::string>();
						jsonDict[tmp_key] = Value;

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
							//case '\"': val += '\"'; break;
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
					return std::move(Json(val));
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
					offset += (jsonBool.getValue<bool>() ? 4 : 5);
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

			JsonValue  _json_value;
			JSON_TYPE  _json_type;

			std::string _json_escape(const std::string &str) const
			{
				std::string output;
				for (unsigned i = 0; i < str.length(); i++)
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
			std::string _float_to_string(double f) const
			{
				std::string str = std::to_string(this->_json_value.Float);
				str = str.erase(str.find_last_not_of('0') + 1, std::string::npos);
				if (str[str.size() - 1] == '.') str += "0";
				return str;
			}
		};
		
		Json::JsonParser* Json::_json_parser = new Json::JsonParser();
		std::ostream& operator<<(std::ostream &os, const Json &json)
		{
			os << json.dump();
			return os;
		}
	}
}

