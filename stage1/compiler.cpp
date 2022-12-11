#include <iostream>

#include <token.h>
#include <tokenizer.h>

int main(int, char**) {
    std::cout << "Hello slof!" << std::endl;

    slof::Token test_token(slof::TokenType::Invalid, true);
    std::cout << test_token << std::endl;

    slof::Tokenizer tokenizer(
        "func main(List<string> args) {   \n"
        "    println(\"hello world!\");   \n"
        "}                                \n"
    );

    return 0;
}