WIP Parsing Expression Grammar library as a CPP metaprogramming API.

TODOs:
- Some cleanup is still needed on recursive types to avoid verbosity of struct definition.
- Some core implementation is still needed such as `Not` conditionals.
- Bindings could be cleaned up by reducing the verbosity of  `Matcher<T>::ReturnType`.
- Avoid passing Context by value everywhere.
