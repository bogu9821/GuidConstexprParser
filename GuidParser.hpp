#pragma once

#include <string_view>
#include <optional>
#include <cstdint>
#include <span>
#include <exception>
#include <array>
#include <iterator>
#include <algorithm>


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

constexpr bool operator==(const GUID& t_left, const GUID& t_right)
{
	//unfortuently memmcpy isn't constexpr yet
	//if we don't want to declare the operator inside the struct
	//we have to check every field

	if (t_left.Data1 != t_right.Data1)
	{
		return false;
	}

	if (t_left.Data2 != t_right.Data2)
	{
		return false;
	}

	if (t_left.Data3 != t_right.Data3)
	{
		return false;
	}

	return std::equal(t_left.Data4, std::next(t_left.Data4, 8), t_right.Data4, std::next(t_right.Data4, 8));
}


namespace GuidParser
{
	//{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
	inline constexpr size_t GUID_STRING_SIZE = 38;

	inline constexpr std::optional<GUID> StringToGuid(const std::string_view t_stringGuid) noexcept;

	template<bool NullTerminated = true>
	inline constexpr auto GuidToString(const GUID& t_guid) noexcept;

	namespace Private
	{
		struct ParseFakeException : public std::exception
		{
			static void Throw()
			{
				throw ParseFakeException{};
			}
		};

		template<typename T>
		inline constexpr T ParseHexNumber(const std::span<const char> t_hexData)
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

	namespace GuidLiteral
	{
		consteval GUID operator"" _guid(const char* t_string, const size_t t_num)
		{
			const auto parsedGuid = StringToGuid(std::string_view{ t_string,t_num });

			if (!parsedGuid.has_value())
			{
				Private::ParseFakeException::Throw();
			}
			else
			{
				return parsedGuid.value();
			}
		}
	}


	inline constexpr std::optional<GUID> StringToGuid(const std::string_view t_stringGuid) noexcept
	{
		if (t_stringGuid.size() != GUID_STRING_SIZE)
		{
			return {};
		}

		if (t_stringGuid.front() != '{' || t_stringGuid.back() != '}')
		{
			return{};
		}

		try
		{
			GUID guid{};
			guid.Data1 = Private::ParseHexNumber<std::uint32_t>(std::span{ std::next(t_stringGuid.data()), 8u });
			guid.Data2 = Private::ParseHexNumber<std::uint16_t>(std::span{ std::next(t_stringGuid.data(), 10), 4u });
			guid.Data3 = Private::ParseHexNumber<std::uint16_t>(std::span{ std::next(t_stringGuid.data(), 15), 4u });

			constexpr auto UnrolledParseArray = []<size_t... Is>(const std::span<unsigned char> t_buffer, const std::span<const char> t_begin, const std::index_sequence<Is...>)
			{
				((t_buffer[Is] = Private::ParseHexNumber<std::uint8_t>(std::span{ std::next(t_begin.data(), Is * 2), 2u })), ...);
			};

			UnrolledParseArray(std::span{ guid.Data4, 2u }, std::span{ std::next(t_stringGuid.data(), 20), 4u }, std::make_index_sequence<2>{});
			UnrolledParseArray(std::span{ std::next(guid.Data4, 2), 6u }, std::span{ std::next(t_stringGuid.data(), 25), 12u }, std::make_index_sequence<6>{});

			return guid;
		}
		catch ([[maybe_unused]] const Private::ParseFakeException&)
		{
			return {};
		}


		return {};
	}

	template<bool NullTerminated>
	inline constexpr auto GuidToString(const GUID& t_guid) noexcept
	{
		constexpr auto size = NullTerminated ? GUID_STRING_SIZE + 1 : GUID_STRING_SIZE;

		std::array<char, size> buffer;

		buffer[0] = '{';
		buffer[9] = '-';
		buffer[14] = '-';
		buffer[19] = '-';
		buffer[24] = '-';
		buffer[GUID_STRING_SIZE - 1] = '}';

		if constexpr (NullTerminated)
		{
			buffer[GUID_STRING_SIZE] = '\0';
		}


		constexpr auto NumberToHexString = [](const auto t_buffer, auto t_integer)
			{
				constexpr const char hexChars[] = "0123456789abcdef";

				for (auto rbegin = std::rbegin(t_buffer); t_integer != 0 && rbegin != std::rend(t_buffer); rbegin++, t_integer >>= 4)
				{
					*rbegin = hexChars[t_integer & 0xf];
				}

			};

		NumberToHexString(std::span{ std::next(buffer.data()), 8u }, t_guid.Data1);
		NumberToHexString(std::span{ std::next(buffer.data(), 10), 4u }, t_guid.Data2);
		NumberToHexString(std::span{ std::next(buffer.data(), 15), 4u }, t_guid.Data3);



		std::uint16_t packedData4_1 =
			(static_cast<std::uint16_t>(t_guid.Data4[0]) << 8) |
			(static_cast<std::uint16_t>(t_guid.Data4[1]));


		std::int64_t packedData4_2 =
			(static_cast<std::int64_t>(t_guid.Data4[2]) << 40) |
			(static_cast<std::int64_t>(t_guid.Data4[3]) << 32) |
			(static_cast<std::int64_t>(t_guid.Data4[4]) << 24) |
			(static_cast<std::int64_t>(t_guid.Data4[5]) << 16) |
			(static_cast<std::int64_t>(t_guid.Data4[6]) << 8) |
			static_cast<std::int64_t>(t_guid.Data4[7]);


		NumberToHexString(std::span{ std::next(buffer.data(), 20), 4u }, packedData4_1);
		NumberToHexString(std::span{ std::next(buffer.data(), 25), 12u }, packedData4_2);


		return buffer;

	}
}
