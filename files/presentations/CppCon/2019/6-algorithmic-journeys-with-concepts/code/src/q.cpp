#include <iostream>
#include <algorithm>
#include <memory>
#include <vector>
#include <cassert>
#include <type_traits>
#include <cppcon/cppcon.h>

// TODO: properties for relations

namespace cppcon {

template<totally_ordered T>
const T& min(const T& x, const T& y) {
    if (y < x) {
        return y;
    }
    return x;
}

template<totally_ordered T>
T& min(T& x, T& y) {
    if (y < x) {
        return y;
    }
    return x;
}

template<typename T, weak_strict_ordering<T> R>
const T& min(const T& x, const T& y, R r) {
    if (r(y, x)) {
        return y;
    }
    return x;
}

template<typename T, weak_strict_ordering<T> R>
T& min(T& x, T& y, R r) {
    if (r(y, x)) {
        return y;
    }
    return x;
}

template<totally_ordered T>
const T& max(const T& x, const T& y) {
    if (y < x) {
        return x;
    }
    return y;
}

template<totally_ordered T>
T& max(T& x, T& y) {
    if (y < x) {
        return x;
    }
    return y;
}

template<regular T, relation<T> R>
const T& max(const T& x, const T& y, R r) {
    if (r(y, x)) {
        return x;
    }
    return y;
}

template<regular T, relation<T> R>
T& max(T& x, T& y, R r) {
    if (r(y, x)) {
        return x;
    }
    return y;
}

template<forward_iterator It>
requires(regular<ValueType(It)> && writable<It>)
It unique(It first, It last) {
    if (first == last) { return last; }
    It result = first;
    ++first;
    while (first != last) {
        if (*result == *first) {
           ++first;
        } else {
            ++result;
            *result = *first;
            ++first;
        }
    }
    ++result;
    return result;
}

template<forward_iterator It, relation<ValueType(It)> R>
requires writable<It>
It unique(It first, It last, R r) {
    if (first == last) { return last; }
    It result = first; ++first;
    while (first != last) {
        if (*result == *first) {
           ++first;
        } else {
            ++result;
            *result = *first;
            ++first;
        }
    }
    ++result;
    return result;
}



template<forward_iterator It, additive_monoid N, relation<ValueType(It)> R>
requires readable<It>
N unique_count(It first, It last, N n, R r) {
    if (first == last) { return n; }
    It slow = first;
    ++first;
    while (first != last) {
        if (r(*slow, *first)) {
           ++slow;
           ++first;
        } else {
            ++first;
            ++slow;
            ++n;
        }
    }
    ++n;
    return n;
}


template<forward_iterator It, additive_monoid N>
requires(readable<It>)
N unique_count(It first, It last, N n) {
    return unique_count(first, last, n, [](const auto& x, const auto& y) { return x == y; });
}


template<forward_iterator It, additive_monoid N>
requires readable<It> && regular<ValueType(It)>
std::pair<It, N> find_not(It first, It last, const ValueType(It)& x, N n) {
    while (first != last && *first == x) {
        ++first;
        ++n;
    }
    return {first, n};
}


template<forward_iterator It, relation<ValueType(It)> R, additive_monoid N>
requires(readable<It>) // TODO
std::pair<It, N> find_if_not(It first, It last, const ValueType(It)& x, R r, N n) {
    while (first != last && r(*first, x)) {
        ++first;
        ++n;
    }
    return {first, n};
}


template<forward_iterator It, output_iterator Out>
requires readable<It> && writable<Out> // TODO value type
inline
Out frequencies(It f, It l, Out out) {
  typedef size_t N;
  while (f != l) { 
    It it = f;
    N n = 1;
    ++f;
    auto r = find_not(f, l, *it, n);
    f = r.first;
    *out = { *it, r.second };
    ++out;
  } 
  return out;
}

template<forward_iterator It, output_iterator Out, relation<std::pair<ValueType(It), size_t>> R>
requires readable<It> && writable<Out> // TODO value type
inline
Out frequencies(It f, It l, Out out, R r) {
  typedef size_t N;
  while (f != l) { 
    It start = f;
    N n = 1;
    ++f;
    auto r = find_if_not(f, l, *start, r, n);
    f = r.first;
    *out = { *start, r.second };
    ++out;
  } 
  return out;
}


template<forward_iterator It, output_iterator Out0, output_iterator Out1>
requires readable<It> && writable<Out0> && writable<Out1>
inline
std::pair<Out0, Out1> frequencies(It f, It l, Out0 out0, Out1 out1) {
  typedef size_t N;

  while (f != l) { 
    It start = f;
    N n = 1;
    ++f;
    auto r = find_not(f, l, *start, n);

    *out0 = *start;
    ++out0;

    *out1 = r.second;
    ++out1;

    f = r.first;
  } 
  return {out0, out1};
}

template<forward_iterator It, output_iterator Out0, forward_iterator Out1>
requires readable<It> && writable<Out0> && writable<Out1>
inline
std::pair<Out0, Out1> frequencies(It f, It l, Out0 out0, Out1 out1) {
  typedef ValueType(Out1) N;

  while (f != l) { 
    It it = f;
    N n = 1;
    ++f;
    It r = find_not(f, l, *it, n);

    *out0 = *it;
    ++out0;

    *out1 = r.second;
    ++out1;

    f = r.first;
  } 
  return {out0, out1};
}

template<forward_iterator It,
         output_iterator Out,
         relation<ValueType(It)> P,
         functional_procedure<It, It> F>
requires readable<It> && writable<Out> 
// Codomain(F) == ValueType(It)
Out transform_subgroups(It first, It last, Out out, P pred, F function) {
    if (first == last) { return out; }
    It slow = first;
    It fast = slow;
    ++fast;
    while (fast != last) {
        if (!pred(*slow, *fast)) {
            *out = function(first, fast);
            ++out;
            first = fast;
        }
        ++slow; ++fast;
    }
    *out = function(first, fast);
    return ++out;
}


template<forward_iterator It,
         output_iterator Out,
         relation<ValueType(It)> P,
         functional_procedure<It, difference_type_t<It>> F>
requires readable<It> && writable<Out> 
Out transform_subgroups(It first, It last, Out out, P pred, F function) {
    if (first == last) { return out; }
    It slow = first;
    It fast = slow;
    ++fast;
    size_t n = 0;
    while (fast != last) {
        if (!pred(*slow, *fast)) {
            *out = function(first, n);
            n = 0;
            ++out;
            first = fast;
        }
        ++n;
        ++slow; ++fast;
    }
    *out = function(first, n);
    return ++out;
}


template<forward_iterator It,
         output_iterator Out,
         relation<ValueType(It)> Predicate,
         functional_procedure<It, It> F>
requires readable<It> && writable<It>
Out squash_subgroups(It first, It last, Predicate pred, F function) {
    return transform_subgroups(first, last, first, pred, function);
}

template<forward_iterator It, functional_procedure<It, It> F>
requires readable<It>
void split(It first, It last, const ValueType(It)& value, F f) {
    while (first != last) {
        It middle = std::find(first, last, value);
        f(first, middle);
        first = middle;
        if (first != last) {
            ++first;
        }
    }
}

template<forward_iterator It, output_iterator Out, functional_procedure<It, It> F>
Out transform_split(It first, It last, Out out, const ValueType(It)& value, F f) {
    while (first != last) {
        It middle = std::find(first, last, value);
        *out = f(first, middle);
        ++out;
        first = middle;
        if (first != last) {
            ++first;
        }
    }
    return out;
}

template<forward_iterator It0, forward_iterator It1>
requires readable<It0> && readable<It1>
std::pair<It0, It1> find_missmatch(It0 first0, It0 last0, It1 first1, It1 last1) {
    while (first0 != last0 && first1 != last1 && *first0 == *first1) {
        ++first0;
        ++first1;
    }
    return {first0, first1};
}

template<random_access_iterator It0, random_access_iterator It1>
requires readable<It0> && readable<It1>
std::pair<It0, It1> find_missmatch(It0 first0, It0 last0, It1 first1, It1 last1) {
    if (last0 - first0 < last1 - first1) {
        while (first1 != last1 && *first0 == *first1) {
            ++first0; ++first1;
        }
    } else {
        while (first0 != last0 && *first0 == *first1) {
            ++first0; ++first1;
        }
    }
    return {first0, first1};
}

template<forward_iterator It0, forward_iterator It1>
requires readable<It0> && readable<It1>
It0 find_subrange(It0 first0, It0 last0, It1 first1, It1 last1) {
    while (first0 != last0 &&
           find_missmatch(first0, last0, first1, last1).second != last1) {
        ++first0;
    }
    return first0;
}


template<forward_iterator It0, forward_iterator It1, functional_procedure<It0, It0> F>
requires readable<It0> && readable<It1>
void split(It0 first0, It0 last0, It1 first1, It1 last1, F f) {
    while (first0 != last0) {
        It0 middle = std::search(first0, last0, first1, last1);
        f(first0, middle);
        first0 = middle;
        if (first0 != last0) {
            ++first0;
        }
    }
}


template<forward_iterator It0, output_iterator Out, forward_iterator It1, functional_procedure<It0, It0> F>
requires readable<It0> && readable<It1> && writable<Out>
Out transform_split(It0 first0, It0 last0, Out out, It1 first1, It1 last1, F f) {
    auto step_size = std::distance(first1, last1);
    while (first0 != last0) {
        It0 middle = std::search(first0, last0, first1, last1);
        *out = f(first0, middle);
        ++out;
        first0 = middle;
        if (first0 != last0) {
            std::advance(first0, step_size);
        }
    }
    return out;
}


template<forward_iterator It, unary_predicate<ValueType(It)> P>
requires readable<It> && writable<It>
It remove_if(It first, It last, P pred) {
    first = std::find_if(first, last, pred);
    if (first == last) { return last; }
    It fast = first; ++fast;
    while (fast != last) {
        if (pred(*fast)) {
            ++fast;
        } else {
            *first = std::move(*fast);
            ++first; ++fast;
        }
    }
    return first;
}

template<forward_iterator It, unary_predicate<ValueType(It)> P>
requires readable<It> && writable<It>
It semistable_partition(It first, It last, P pred) {
    first = std::find_if(first, last, pred);
    if (first == last) { return last; }
    It fast = first; ++fast;
    while (fast != last) {
        if (pred(*fast)) {
            ++fast;
        } else {
            std::swap(*first, *fast);
            ++first; ++fast;
        }
    }
    return first;
}

template<forward_iterator It0, forward_iterator It1, binary_predicate<ValueType(It0), ValueType(It1)> P>
requires readable<It0> && readable<It1>
std::pair<It0, It1> find_if(It0 first0, It0 last0, It1 first1, P pred) {
    while (first0 != last0 && !pred(*first0, first1)) {
        ++first0;
        ++first1;
    }
    return {first0, first1};
}


template<forward_iterator It0, forward_iterator It1, binary_predicate<ValueType(It0), ValueType(It1)> P>
requires readable<It0> && writable<It0> && readable<It1> && writable<It1>
std::pair<It0, It1> semistable_partition(It0 first0, It0 last0, It1 first1, P pred) {
    std::pair<It0, It1> r = find_if(first0, last0, first1, pred);
    first0 = r.first; first1 = r.second;
    if (first0 == last0) { return {first0, first1}; }
    It0 fast0 = first0; ++fast0;
    It1 fast1 = first1; ++fast1;
    while (fast0 != last0) {
        if (pred(*fast0, *fast1)) {
            ++fast0; ++fast1;
        } else {
            std::swap(*first0, *fast0);
            ++first0; ++fast0;
            std::swap(*first1, *fast1);
            ++first1; ++fast1;
        }
    }
    return {first0, first1};
}


}  // namespace cppcon


int main() {
    {
        int x = 3;
        int y = 3;
        assert(&cppcon::min(x, y) == &x);
        assert(&cppcon::max(x, y) == &y);
        std::vector<int> vals;
        cppcon::unique(vals.begin(), vals.end());
    }

    std::vector<int> values = {1, 1, 2, 3, 3, 3, 4, 4, 5};
    auto values_copy = values;
    assert(cppcon::unique(std::begin(values_copy), std::end(values_copy)) - std::begin(values_copy) == 5);
    assert(cppcon::unique_count(std::begin(values), std::end(values), 0) == 5);

    std::vector<std::pair<int, size_t>> values_frequencies;
    cppcon::frequencies(std::begin(values), std::end(values), std::back_inserter(values_frequencies));

    std::vector<std::pair<int, size_t>> expected_frequencies = {{1, 2ul}, {2, 1ul}, {3, 3ul}, {4, 2ul}, {5, 1ul}};
    assert(values_frequencies == expected_frequencies);

    std::vector<int> unique_elements;
    std::vector<size_t> occurences;
    cppcon::frequencies(std::begin(values), std::end(values), std::back_inserter(unique_elements), std::back_inserter(occurences));

    std::vector<int> expected_unique_elements = {1, 2, 3, 4, 5};
    std::vector<size_t> expected_occurences= {2ul, 1ul, 3ul, 2ul, 1ul};
    assert(unique_elements == expected_unique_elements);
    assert(occurences == expected_occurences);

    std::vector<std::pair<int, size_t>> values_frequencies2;
    cppcon::transform_subgroups(
        std::begin(values),
        std::end(values),
        std::back_inserter(values_frequencies2),
        std::equal_to<int>(),
        [](auto first, size_t n) { return std::pair<int, size_t>(*first, n);}
    );

    std::string str = "a_b_c_de_";
    std::string str_new;
    cppcon::split(std::begin(str), std::end(str), '_', [&str_new](auto first, auto last) { std::copy(first, last, std::back_inserter(str_new)); });
    assert(str_new == "abcde");
    std::string str_out;

    cppcon::transform_split(std::begin(str), std::end(str), std::back_inserter(str_out), '_', [](auto first, auto last) { return *first; });
    assert(str_out == "abcd");

    str_out.clear();
    str = "a__b__c__d";
    std::string delimiter = "__";
    cppcon::transform_split(std::begin(str), std::end(str), std::back_inserter(str_out), std::begin(delimiter), std::end(delimiter), [](auto first, auto last) { return *first; });
    assert(str_out == "abcd");

    {
        std::vector<int> values = {1, 2, 3, 4, 5, 6};
        std::vector<int> expected_values = {1, 3, 5};
        values.resize(cppcon::remove_if(values.begin(), values.end(), [](auto x) { return x % 2 == 0; }) - values.begin());
        assert(values == expected_values);
    }
    {
        std::vector<int> values = {1, 2, 3, 4, 5, 6};
        std::vector<int> expected_values = {1, 3, 5, 4, 2, 6};
        auto middle = cppcon::semistable_partition(values.begin(), values.end(), [](auto x) { return x % 2 == 0; });
        assert(std::equal(std::begin(values), middle, values.begin(), values.begin() + 3));
        assert(values == expected_values);
    }
}
