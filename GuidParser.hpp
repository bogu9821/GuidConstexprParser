#pragma once

#include <string>
#include <charconv>
#include <optional>
#include <ranges>
#include <cstdint>
#include <concepts>

#ifndef GUID_DEFINED
#define GUID_DEFINED
struct GUID
{
	std::uint32_t Data1;
	std::uint16_t Data2;
	std::uint16_t Data3;
	std::uint8_t Data4[8];
};
#endif

namespace GuidParser
{
	//{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
	inline constexpr size_t GUID_STRING_SIZE = 38;

	inline constexpr std::optional<GUID> StringToGuid(const std::string_view t_stringGuid) noexcept;

	namespace GuidLiteral
	{
		consteval GUID operator"" _guid(const char* t_string, size_t t_num)
		{
			return StringToGuid(std::string_view{ t_string,t_num }).value();
		}
	}

	namespace Private
	{
		template<std::unsigned_integral T>
		inline constexpr T ParseHexNumber(const std::string_view t_hexData);


		struct ParseFakeException : public std::exception
		{
			static void Throw()
			{
				throw ParseFakeException{};
			}
		};
	}


	inline constexpr std::optional<GUID> StringToGuid(const std::string_view t_stringGuid) noexcept
	{
		if (t_stringGuid.size() != GUID_STRING_SIZE)
		{
			return {};
		}

		GUID guid{};

		try
		{
	
			guid.Data1 = Private::ParseHexNumber<std::uint32_t>(t_stringGuid.substr(1, 8));
			guid.Data2 = Private::ParseHexNumber<std::uint16_t>(t_stringGuid.substr(10, 4));
			guid.Data3 = Private::ParseHexNumber<std::uint16_t>(t_stringGuid.substr(15, 4));


			constexpr auto UnrolledParseArray = []<size_t... Is>(const std::span<unsigned char> t_buffer, const std::string_view t_begin, const std::index_sequence<Is...>)
			{
				((t_buffer[Is] = Private::ParseHexNumber<std::uint8_t>(std::string_view{ std::next(t_begin.data(), Is * 2), 2u })), ...);
			};

	
			UnrolledParseArray(std::span{ guid.Data4, 2u }, t_stringGuid.substr(20, 4), std::make_index_sequence<2>{});
			UnrolledParseArray(std::span{ std::next(guid.Data4, 2), 6u }, t_stringGuid.substr(25, 12), std::make_index_sequence<6>{});
		}
		catch ([[maybe_unused]] const Private::ParseFakeException&)
		{
			return {};
		}


		return guid;
	}

	namespace Private
	{
		template<std::unsigned_integral T>
		constexpr T ParseHexNumber(std::string_view t_hexData)
		{
			if (t_hexData.size() != sizeof(T) * 2)
			{
				ParseFakeException::Throw();
			}

			T number{};

			for (const auto ch : t_hexData)
			{
				number <<= 4;
				switch (ch)
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
					number |= (ch - '0');
					break;
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					number |= (10 + ch - 'a');
					break;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					number |= (10 + ch - 'A');
					break;
				default:
					ParseFakeException::Throw();
				}
			}

			return number;
		}

	}

}
