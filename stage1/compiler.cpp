#include <iostream>

#include <token.h>
#include <tokenizer.h>

int main(int, char**) {
    std::cout << "Hello slof!" << std::endl;

    auto tokenization_result = slof::Tokenizer::tokenize("let else be if 100 1.2345");
    if(tokenization_result.is_error()) {
        std::cout << "error occured during tokenization: " << tokenization_result.error_message() << std::endl;
    } else {
        auto& token_stream = tokenization_result.token_stream();
        std::cout << "Token list: " << std::endl;
        while(!token_stream.eos()) {
            std::cout << "   - " << token_stream.consume_unchecked() << std::endl;
        }
    }

    return 0;
}