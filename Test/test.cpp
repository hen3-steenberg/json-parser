#include <iostream>
#include <string>
import JSON;

int main()
{
	std::string json_test = "{ \"empty\" : \"\", \"null\" : \"null\", \"true\" : \"true\", \"false\" : \"false\" , \"number\" : 123, \"string\" : \"string\", \"array\" : [1, 2, 3], \"object\" : { \"f1\" : 1, \"f2\" : \"str\", \"f3\" : { \"arr\" : [1, 2, 3] } } }";
	//std::string json_test = "{ \"array\" : [1, 2, 3] }";
	HS::JSON::json_document doc(json_test);
	for (auto val : doc)
	{
		if (val.type == HS::JSON::json_type::array)
		{
			std::cout << val.field_name << " : [\n";
			for (auto entry : val)
			{
				std::cout << '\t' << entry.value << std::endl;
			}
			std::cout << "]\n";
		}
		else if (val.type == HS::JSON::json_type::object)
		{
			std::cout << val.field_name << " : {\n";
			for (auto entry : val)
			{
				std::cout << '\t' << entry.field_name << " : " << entry.value << std::endl;
			}
			std::cout << "}\n";
		}
		else
		{
			std::cout << val.field_name << " = " << val.value << std::endl;
		}
		
	}

	return 0;
}