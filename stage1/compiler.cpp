#include <iostream>

#include <token.h>
#include <tokenizer.h>

int main(int, char**) {
    std::cout << "Hello slof!" << std::endl;

    slof::Token test_token(slof::TokenType::Invalid, true);
    std::cout << test_token << std::endl;
    
    return 0;
}