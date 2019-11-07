#pragma once

template<typename T>
inline static void push_unique(T* element, std::vector<T*>& vector);

template<typename T>
inline static void pop(T* element, std::vector<T*>& vector);

template<typename T>
inline void push_unique(T* element, std::vector<T*>& vector)
{
	for (auto it = vector.begin(); it != vector.end(); it++)
	{
		if ((*it) == element)
		{
			return;
		}
	}

	vector.push_back(element);
}

template<typename T>
inline void pop(T* element, std::vector<T*>& vector)
{
	for (auto it = vector.begin(); it != vector.end(); it++)
	{
		if ((*it) == element)
		{
			vector.erase(it);
			return;
		}
	}
}

template<typename T>
inline T clamp(T value, T min, T max)
{
	if (value < min)
	{
		return min;
	}

	if (value > max)
	{
		return max;
	}

	return value;
}
