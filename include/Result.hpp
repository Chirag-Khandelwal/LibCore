#pragma once

#include "Status.hpp"

namespace core
{

template<typename T, typename E> class Result
{
	Variant<T, Status<E>> data;

public:
	Result(T &&obj) : data(std::move(obj)) {}
	template<typename... Args> Result(E &&err, Args... args)
		: data(Status(err, std::forward<Args>(args)...))
	{}

	inline T &&value() { return std::get<T>(std::move(data)); }
	inline T &valueRef() { return std::get<T>(data); }

	inline Status<E> &&err() { return std::get<Status<E>>(std::move(data)); }
	inline T &errRef() { return std::get<Status<E>>(data); }

	inline bool isErr() const { return std::holds_alternative<Status<E>>(data); }
	inline bool isOk() const { return !isErr(); }
};

} // namespace core