### WIP Parsing Expression Grammar (PEG) library as a CPP metaprogramming API.

### Benefits of a metaprogramming API for PEG
Rules define C++ types directly. This makes rule definition, matching, and binding all statically type checked. 
```cpp
 // Each Regex<"..."> is a unique type
using Plus = Regex<"\\+">;
using Number = Regex<"[0-9]+">;

using Addition = Seq<Number, Plus, Number>; 

// Seq matches to a tuple
Matcher<Addition>::Match({.input = "4+3"})->value // -> ReturnType is std::tuple<Number, Plus, Number>

using Minus = Regex<"\\-">;
using Op = Or<Plus, Minus>;

// Or matches to a variant
Matcher<Op>::Match({.input = "-"})->value // -> ReturnType is std::variant<Plus, Minus>

using Ops = Repeated<Op>;

// Repeated matches to a std::vector
Matcher<Ops>::Match({.input = "+-+-+-+"})->value // -> ReturnType is std::vector<std::variant<Plus, Minus>>

using MaybeOp = Conditional<Op, Op>;

// Conditional matches to a std::optional
Matcher<MaybeOp>::Match({.input = "+"})->value // -> ReturnType is std::optional<std::variant<Plus, Minus>>
```



TODOs:
- Some cleanup is still needed on recursive types to avoid verbosity of struct definition.
- Some core implementation is still needed such as `Not` conditionals.
- Bindings could be cleaned up by reducing the verbosity of  `Matcher<T>::ReturnType`.
- Avoid passing Context by value everywhere.
