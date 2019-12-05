#pragma once

namespace ink::runtime::internal
{
	template<unsigned int N, typename Arg, typename... Args>
	struct get
	{
		using type = typename get<N - 1, Args...>::type;
	};

	template<typename Arg, typename... Args>
	struct get<0, Arg, Args...>
	{
		using type = typename Arg;
	};

	// constant and is_same from http://www.cppreference.com

	template<typename T, T v>
	struct constant {
		static constexpr T value = v;
		typedef T value_type;
		typedef constant type; // using injected-class-name
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; } //since c++14
	};

	template<class T, class U>
	struct is_same : constant<bool, false> {};

	template<class T>
	struct is_same<T, T> : constant<bool, true> {};

	// function_traits from https://functionalcpp.wordpress.com/2013/08/05/function-traits/

	template<class F>
	struct function_traits;

	// function pointer
	template<class R, class... Args>
	struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)>
	{};

	template<class R, class... Args>
	struct function_traits<R(Args...)>
	{
		using return_type = R;

		static constexpr unsigned int arity = sizeof...(Args);

		template <unsigned int N>
		struct argument
		{
			static_assert(N < arity, "error: invalid parameter index.");
			using type = typename get<N, Args...>::type;
		};
	};

	// member function pointer
	template<class C, class R, class... Args>
	struct function_traits<R(C::*)(Args...)> : public function_traits<R(C&, Args...)>
	{};

	// const member function pointer
	template<class C, class R, class... Args>
	struct function_traits<R(C::*)(Args...) const> : public function_traits<R(C&, Args...)>
	{};

	// member object pointer
	template<class C, class R>
	struct function_traits<R(C::*)> : public function_traits<R(C&)>
	{};

	// functor
	template<class F>
	struct function_traits
	{
	private:
		using call_type = function_traits<decltype(&F::operator())>;
	public:
		using return_type = typename call_type::return_type;

		static constexpr unsigned int arity = call_type::arity - 1;

		template <unsigned int N>
		struct argument
		{
			static_assert(N < arity, "error: invalid parameter index.");
			using type = typename call_type::template argument<N + 1>::type;
		};
	};

	// from https://stackoverflow.com/questions/17424477/implementation-c14-make-integer-sequence
	// using aliases for cleaner syntax
	template<class T> using Invoke = typename T::type;

	template<unsigned...> struct seq { using type = seq; };

	template<class S1, class S2> struct concat;

	template<unsigned... I1, unsigned... I2>
	struct concat<seq<I1...>, seq<I2...>>
		: seq<I1..., (sizeof...(I1) + I2)...> {};

	template<class S1, class S2>
	using Concat = Invoke<concat<S1, S2>>;

	template<unsigned N> struct gen_seq;
	template<unsigned N> using GenSeq = Invoke<gen_seq<N>>;

	template<unsigned N>
	struct gen_seq : Concat<GenSeq<N / 2>, GenSeq<N - N / 2>> {};

	template<> struct gen_seq<0> : seq<> {};
	template<> struct gen_seq<1> : seq<0> {};
}