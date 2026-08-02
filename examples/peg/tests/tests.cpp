#include "examples/peg/peg.h"

using namespace peg;


const std::string kTestCode = "# This is a comment";

int main() {
    Context input{kTestCode};
    auto res = CommentR::Match(input, true);
    if (!res) throw std::runtime_error("Did not match");

}