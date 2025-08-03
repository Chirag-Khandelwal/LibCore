#pragma once

#include "Status.hpp"

namespace core
{

template<typename T, typename E> class Result
{
	Variant<T, E> data;

public:
	Result(E &&err) : data(std::move(err)) {}
	Result(T &&obj) : data(std::move(obj)) {}
	template<typename... Args> Result(E &&err, Args... args)
		: data(Status(err, std::forward<Args>(args)...))
	{}

	inline T &&value() { return std::get<T>(std::move(data)); }
	inline T &valueRef() { return std::get<T>(data); }

	inline E &&err() { return std::get<Status<E>>(std::move(data)); }
	inline E &errRef() { return std::get<E>(data); }

	inline bool isErr() const { return std::holds_alternative<E>(data); }
	inline bool isOk() const { return !isErr(); }
};

} // namespace core