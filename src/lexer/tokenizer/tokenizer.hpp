
#pragma once

#include <string>
#include <vector>
#include <variant>

#include "token.hpp"

#include "../lexemizer/lexeme.hpp"
#include "../lexemizer/lexemizer.hpp"

#include "keyword_token.hpp"
#include "identifier_token.hpp"
#include "integer_literal_token.hpp"
#include "float_literal_token.hpp"
#include "character_literal_token.hpp"
#include "string_literal_token.hpp"
#include "operator_token.hpp"
#include "punctuation_token.hpp"
#include "whitespace_token.hpp"
#include "comment_token.hpp"

#include "token_variant.hpp"

#include "../../error.hpp"

std::vector<TokenVariant> tokenize(const std::vector<Lexeme>& source) {
    static bool tokenizer_error = false;
    std::vector<TokenVariant> tokens;
    
    for (const Lexeme& lexeme : source) {
        switch (lexeme.type) {
            case LexemeType::Word:
                if (KeywordToken::isKeyword(lexeme)) {
                    tokens.push_back(KeywordToken(lexeme));
                    continue;
                }
                
                tokens.push_back(IdentifierToken(lexeme));
                break;
            case LexemeType::IntegerLiteral:
                tokens.push_back(IntegerLiteralToken(lexeme));
                break;
            case LexemeType::FloatLiteral:
                tokens.push_back(FloatLiteralToken(lexeme));
                break;
            case LexemeType::CharacterLiteral:
                tokens.push_back(CharacterLiteralToken(lexeme));
                break;
            case LexemeType::StringLiteral:
                tokens.push_back(StringLiteralToken(lexeme));
                break;
            case LexemeType::Symbol:
                if (OperatorToken::isOperator(lexeme)) {
                    tokens.push_back(OperatorToken(lexeme));
                    continue;
                }
                if (PunctuationToken::isPunctuation(lexeme)) {
                    tokens.push_back(PunctuationToken(lexeme));
                    continue;
                }
                printError("unknown symbol lexeme", lexeme);
                tokenizer_error = true;
                break;
            case LexemeType::Whitespace:
                tokens.push_back(WhiteSpaceToken(lexeme));
                break;
            case LexemeType::Comment:
                tokens.push_back(CommentToken(lexeme));
                break;
            default:
                printError("unknown lexeme", lexeme);
                tokenizer_error = true;
                break;
        }
    }

    if (tokenizer_error) {
        std::exit(EXIT_FAILURE);
    }

    return tokens;
}
