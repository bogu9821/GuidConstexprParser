#pragma once

#include <string>
#include <charconv>
#include <optional>
#include <ranges>
#include <cstdint>

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

	inline constexpr std::optional<GUID> StringToGuid(const std::string_view t_stringGuid);

	consteval GUID operator"" guid(const char* t_string, size_t t_num)
	{
		return StringToGuid(std::string_view{ t_string,t_num }).value();
	}

	inline constexpr std::optional<GUID> StringToGuid(const std::string_view t_stringGuid)
	{
		if (t_stringGuid.size() != GUID_STRING_SIZE)
		{
			return {};
		}

		struct ParseException : public std::exception
		{
			static void Throw()
			{
				throw ParseException{};
			}
		};

		GUID guid{};

		try
		{
			constexpr auto ParseChars = [](const std::string_view t_str, auto& t_buffer)
			{
				const auto [ptr, ec] = std::from_chars(t_str.data(), std::next(t_str.data(), t_str.size()), t_buffer, 16);

				if (ec != std::errc{})
				{
					ParseException::Throw();
				}
			};

			const auto Data1Begin = t_stringGuid.substr(1, 36);
			const auto Data2Begin = Data1Begin.substr(8 + 1);
			const auto Data3Begin = Data2Begin.substr(4 + 1);

			ParseChars(Data1Begin | std::views::take(8), guid.Data1);
			ParseChars(Data2Begin | std::views::take(4), guid.Data2);
			ParseChars(Data3Begin | std::views::take(4), guid.Data3);

			constexpr auto UnrolledParseArray = []<size_t... Is>(const std::span<unsigned char> t_buffer, const std::string_view t_begin, const std::index_sequence<Is...>)
			{
				((ParseChars(std::string_view{ std::next(t_begin.data(), Is * 2), 2u }, t_buffer[Is])), ...);
			};

			const auto Data4_1Begin = Data3Begin.substr(4 + 1);
			const auto Data4_2Begin = Data4_1Begin.substr(4 + 1);

			UnrolledParseArray(std::span{ guid.Data4, 2u }, Data4_1Begin | std::views::take(4), std::make_index_sequence<2>{});
			UnrolledParseArray(std::span{ std::next(guid.Data4, 2), 6u }, Data4_2Begin, std::make_index_sequence<6>{});
		}
		catch ([[maybe_unused]] const ParseException&)
		{
			return {};
		}


		return guid;
	}


}
