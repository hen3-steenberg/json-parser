module;
#include <optional>
#include <vector>
#include <string>
#include <string_view>
#include <stack>
#include <iterator>
export module JSON;

namespace HS
{
	namespace JSON
	{
		export enum class json_type
		{
			null,
			boolean,
			number,
			string,
			array,
			object
		};

		struct json_ref
		{
			struct field_ref
			{
				size_t start_index;
				size_t stop_index;
			};
			field_ref field_name;
			json_type type;
			field_ref value;
			size_t level;
			size_t next_sibling_index;
		};

		export struct json_value
		{

		public:

			struct const_iterator
			{
				using iterator_category = std::input_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = json_value;
				using const_pointer = std::vector<json_value>::const_pointer;
				using const_reference = std::vector<json_value>::const_reference;
			private:
				static inline std::vector<json_value> empty{};
				std::vector<json_value> const& json_document;
				size_t next_sibling_index;
				size_t current_index;

				const_iterator()
					: json_document(empty), next_sibling_index(0), current_index(0)
				{}

			public:
				const_iterator(const_iterator const& other)
					: json_document(other.json_document), next_sibling_index(other.next_sibling_index), current_index(other.current_index)
				{}

				const_iterator(json_value const& val)
					: json_document(val.json_document), next_sibling_index(val.next_sibling_index), current_index(val.current_index)
				{}

				static const_iterator end()
				{
					return const_iterator();
				}

				const_reference operator*() const
				{
					return json_document[current_index];
				}

				const_pointer operator->() const
				{
					return &json_document[current_index];
				}

				const_iterator& operator++()
				{
					if (json_document[next_sibling_index].level == json_document[current_index].level)
					{
						current_index = next_sibling_index;
						next_sibling_index = json_document[next_sibling_index].next_sibling_index;
					}
					else
					{
						current_index = 0;
						next_sibling_index = 0;
					}
					return *this;
				}

				const_iterator operator++(int)
				{
					const_iterator tmp = *this;
					++(*this);
					return tmp;
				}

				friend bool operator==(const_iterator const& a, const_iterator const& b)
				{
					return a.current_index == b.current_index && a.next_sibling_index == b.next_sibling_index;
				}

				friend bool operator!=(const_iterator const& a, const_iterator const& b)
				{
					return !(a == b);
				}
			};

			std::string_view field_name;
			std::string_view value;
			json_type type;
			size_t level;

			const_iterator begin() const
			{
				if (json_document[current_index + 1].level > level)
				{
					return const_iterator(json_document[current_index + 1]);
				}
				else return end();
			}

			const_iterator end() const
			{
				return const_iterator::end();
			}

			json_value(std::vector<json_value> const& doc, size_t next_index, size_t index, json_type t, size_t _level, std::string_view name, std::string_view val )
				: json_document(doc), 
				next_sibling_index(next_index), 
				current_index(index), 
				type(t),
				level(_level),
				field_name(name),
				value(val)
			{

			}

		private:
			size_t next_sibling_index;
			size_t current_index;
			std::vector<json_value> const& json_document;


		};

		bool IsWhiteSpace(char c)
		{
			switch (c)
			{
			case ' ':
			case '\n':
			case '\r':
			case '\t':
				return true;
			default:
				return false;
			}
		}

		bool IsObjectStart(char last, char curr, char next)
		{
			return curr == '{';
		}

		bool IsObjectStop(char last, char curr, char next)
		{
			return curr == '}';
		}

		bool IsArrayStart(char last, char curr, char next)
		{
			return curr == '[';
		}

		bool IsArrayStop(char last, char curr, char next)
		{
			return curr == ']';
		}

		bool IsStringStart(char last, char curr, char next)
		{
			return curr == '"';
		}

		bool IsStringStop(char last, char curr, char next)
		{
			return last != '/' && curr == '"';
		}

		bool IsDigit(char c)
		{
			switch (c)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				return true;
			default:
				return false;
			}
		}

		bool IsNumberStart(char last, char curr, char next)
		{
			return IsDigit(curr) || (curr == '-' && IsDigit(next));
		}

		bool IsNumberStop(char last, char curr, char next)
		{
			return IsDigit(curr) && (!(IsDigit(next) || next == 'e' || next == 'E' || next == '+' || next == '-'));
		}

		bool NoStop(char last, char curr, char next)
		{
			return false;
		}

		typedef bool (*StopFunc)(char, char, char);

		std::vector<json_ref> parse_json(std::string const& json)
		{

			std::vector<json_ref> res;

			struct incomplete_ref
			{
				size_t index;
				StopFunc Stop;
			};

			std::stack<incomplete_ref> incomplete_references;

			incomplete_ref current_ref = { 1, IsObjectStop };

			size_t level = 0;
			for (size_t index = 0; index < json.size(); ++index)
			{
				char curr = json[index];
				if (!IsWhiteSpace(curr))
				{
					char prev = index > 0 ? json[index - 1] : ' ';
					char next = (index < json.size() - 1) ? json[index + 1] : ' ';
					if (incomplete_references.size())
					{
						incomplete_ref parent_ref = incomplete_references.top();
						bool IsField = res[parent_ref.index].type == json_type::object;
						bool LookForStart = res[current_ref.index].type == json_type::null;
						bool LookForFieldStop = IsField && LookForStart && !(res[current_ref.index].field_name.stop_index);
						bool LookForFieldStart = LookForFieldStop && !(res[current_ref.index].field_name.start_index);
						if(LookForFieldStop)
							LookForStart = false;

						if (current_ref.Stop(prev, curr, next))
						{
							if (LookForFieldStop)
							{
								res[current_ref.index].field_name.stop_index = index;
								current_ref.Stop = NoStop;
							}
							else
							{
								if (res[current_ref.index].type == json_type::string)
								{
									res[current_ref.index].value.stop_index = index;
								}
								else
								{
									res[current_ref.index].value.stop_index = index + 1;
								}
								res[current_ref.index].next_sibling_index = current_ref.index + 1;
								--level;

								current_ref.index = res.size();
								current_ref.Stop = NoStop;

								json_ref empty;
								empty.field_name = { 0, 0 };
								empty.level = level + 1;
								empty.next_sibling_index = 0;
								empty.type = json_type::null;
								empty.value = { 0, 0 };
								res.push_back(empty);

								
								
							}
						}
						else if (parent_ref.Stop(prev, curr, next))
						{
							res[parent_ref.index].value.stop_index = index + 1;
							--level;

							res[current_ref.index].level = level + 1;
							res[parent_ref.index].next_sibling_index = current_ref.index;
							incomplete_references.pop();
						}
						else if (LookForFieldStart)
						{
							if (IsStringStart(prev, curr, next))
							{
								res[current_ref.index].field_name.start_index = index + 1;
								current_ref.Stop = IsStringStop;
							}
						}
						else if (LookForStart)
						{
							if (IsObjectStart(prev, curr, next))
							{
								res[current_ref.index].value.start_index = index;
								res[current_ref.index].type = json_type::object;
								current_ref.Stop = IsObjectStop;
								++level;

								incomplete_references.push(current_ref);

								current_ref.index = res.size();
								current_ref.Stop = IsObjectStop;

								json_ref empty;
								empty.field_name = { 0, 0 };
								empty.level = level + 1;
								empty.next_sibling_index = 0;
								empty.type = json_type::null;
								empty.value = { 0, 0 };
								res.push_back(empty);
							}
							else if (IsArrayStart(prev, curr, next))
							{
								res[current_ref.index].value.start_index = index;
								res[current_ref.index].type = json_type::array;
								current_ref.Stop = IsArrayStop;
								++level;

								incomplete_references.push(current_ref);

								current_ref.index = res.size();
								current_ref.Stop = IsArrayStop;

								json_ref empty;
								empty.field_name = { 0, 0 };
								empty.level = level + 1;
								empty.next_sibling_index = 0;
								empty.type = json_type::null;
								empty.value = { 0, 0 };
								res.push_back(empty);
							}
							else if (IsStringStart(prev, curr, next))
							{
								res[current_ref.index].value.start_index = index + 1;
								res[current_ref.index].type = json_type::string;
								current_ref.Stop = IsStringStop;
								++level;
							}

							else if (IsNumberStart(prev, curr, next))
							{
								res[current_ref.index].value.start_index = index;
								res[current_ref.index].type = json_type::number;

								if (IsNumberStop(prev, curr, next))
								{
									res[current_ref.index].value.stop_index = index + 1;
									res[current_ref.index].next_sibling_index = current_ref.index + 1;

									current_ref.index = res.size();
									current_ref.Stop = NoStop;

									json_ref empty;
									empty.field_name = { 0, 0 };
									empty.level = level + 1;
									empty.next_sibling_index = 0;
									empty.type = json_type::null;
									empty.value = { 0, 0 };
									res.push_back(empty);
								}
								else
								{
									current_ref.Stop = IsNumberStop;

									++level;
								}

								
							}
						}
					}
					else if(!res.size())
					{
						if (IsObjectStart(prev, curr, next))
						{
							json_ref ref;
							ref.field_name = { 0, 0 };
							ref.level = 0;
							ref.next_sibling_index = 0;
							ref.type = json_type::object;
							ref.value = { index, 0 };
							res.push_back(ref);

							incomplete_references.emplace(incomplete_ref{ 0, IsObjectStop });

							json_ref empty;
							empty.field_name = { 0, 0 };
							empty.level = 1;
							empty.next_sibling_index = 0;
							empty.type = json_type::null;
							empty.value = { 0, 0 };
							res.push_back(empty);
						}
					}
				}
			}
			return res;

		}

		

		export struct json_document
		{
		private:
			
			std::vector<json_value> values;
			std::string raw_value;

			void parse_tokens(std::vector<json_ref> refs)
			{
				std::string_view raw = raw_value;
				values.reserve(refs.size());
				for (const auto& ref : refs)
				{
					std::string_view name = raw.substr(ref.field_name.start_index, ref.field_name.stop_index - ref.field_name.start_index);
					std::string_view value = raw.substr(ref.value.start_index, ref.value.stop_index - ref.value.start_index);

					json_type type = ref.type;
					if (ref.type == json_type::string)
					{
						if (value == "null")
							type = json_type::null;
						else if (value == "true" || value == "false")
							type = json_type::boolean;
					}

					json_value val(values, ref.next_sibling_index, values.size(), ref.type, ref.level, name, value );
					values.push_back(val);
				}
			}
		public:

			json_document(std::string const& str)
				: raw_value(str)
			{
				auto tokens = parse_json(raw_value);
				parse_tokens(tokens);
			}

			json_value::const_iterator end() const
			{
				return json_value::const_iterator::end();
			}

			json_value::const_iterator begin() const
			{
				if (values.size() > 0)
				{
					return values[0].begin();
				}
				else
				{
					return end();
				}
			}
		};
	}

		
}