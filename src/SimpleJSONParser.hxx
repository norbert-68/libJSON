//
//  Created by Norbert Klose on 23.02.2019.
//  Copyright © 2019 Norbert Klose. All rights reserved.
//

#ifndef SimpleJSONParser_hxx
#define SimpleJSONParser_hxx

#include <libJSON/libJSON.hxx>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

namespace libJSONImpl
{
    typedef enum
    {
        SPACE,          // (\u0009, \u000a, \u000d, \u0020)+
        NUMBER,         // '-'?('0'|(['1',..,'9']['0',..,'9']*))('.'['0',..,'9'])?(['e','E']['+','-']?['0',..,'9']+)?
        STRING,         // '"' [!'"']* '"'
        L_TRUE,         // true
        L_FALSE,        // false
        L_NULL,         // null
        OPEN_BRACKET,   // "["
        CLOSE_BRACKET,  // "]"
        OPEN_BRACE,     // "{"
        CLOSE_BRACE,    // "}"
        COMMA,          // ','
        COLON,          // ':'
        eof,            // EOF
        JSON,           // value eof
        value,          // object | array | STRING | NUMBER | "true" | "false" | "null"
        object,         // '{' '}' | '{' members '}'
        array,          // '[' ']' | '[' values ']'
        members,        // members ',' member | member
        member,         // STRING ':' value
        values,         // values ',' value | value
        ERROR,
    } JSONTagType;
    
    std::ostream & operator<<(std::ostream & stream, const JSONTagType & tag)
    {
        switch (tag)
        {
            case SPACE:
                return stream << "SPACE";
            case NUMBER:
                return stream << "NUMBER";
            case STRING:
                return stream << "STRING";
            case L_TRUE:
                return stream << "true";
            case L_FALSE:
                return stream << "false";
            case L_NULL:
                return stream << "null";
            case OPEN_BRACKET:
                return stream << "[";
            case CLOSE_BRACKET:
                return stream << "]";
            case OPEN_BRACE:
                return stream << "{";
            case CLOSE_BRACE:
                return stream << "}";
            case COMMA:
                return stream << ",";
            case COLON:
                return stream << ":";
            case eof:
                return stream << "EOF";
            case JSON:
                return stream << "JSON";
            case value:
                return stream << "value";
            case object:
                return stream << "object";
            case array:
                return stream << "array";
            case members:
                return stream << "members";
            case member:
                return stream << "member";
            case values:
                return stream << "values";
            case ERROR:
                return stream << "syntax error";
        }
        return stream;
    }
    
    struct JSONToken
    {
        JSONTagType tag;
        std::string value;
        unsigned line;
        unsigned column;

        JSONToken() : tag(eof), line(0), column(0) {}
        JSONToken(JSONTagType tag) : tag(tag), line(0), column(0) {}

        void clear()
        {
            tag = eof;
            line = 0;
            column = 0;
            if (!value.empty())
                value.clear();
        }
    };
    
    std::ostream & operator<<(std::ostream & stream, const JSONToken & token)
    {
        return stream << "(" << token.line << "," << token.column << ") " << token.tag << ": " << token.value;
    }
    
    struct JSONTokenizer
    {
        std::istream & document;
        unsigned line;
        unsigned column;
        
        JSONTokenizer(std::istream & document) : document(document), line(1), column(1) {}
        
        void next(JSONToken & token)
        {
            enum {
                S_START,
                S_SPACE,
                S_NUMBER_1_9,
                S_NUMBER_0,
                S_NUMBER_MINUS,
                S_NUMBER_FRAC,
                S_NUMBER_FRAC_DIGITS,
                S_NUMBER_EXP,
                S_NUMBER_EXP_SIGN,
                S_NUMBER_EXP_DIGITS,
                S_STRING,
                S_ESCAPE,
                S_HEX1,
                S_HEX2,
                S_HEX3,
                S_HEX4,
                S_LITERAL,
                S_STOP
            } state = S_START;
            token.clear();
            token.line = line;
            token.column = column;
            
            while (state != S_STOP && document.good())
            {
                int c = document.peek();
                switch (state)
                {
                    case S_START:
                        if (c == ' ' || c == '\u000a' || c == '\u000d' || c == '\u0009')
                        {
                            token.tag = SPACE;
                            state = S_SPACE;
                        }
                        else if (c == '[')
                        {
                            token.tag = OPEN_BRACKET;
                            state = S_STOP;
                        }
                        else if (c == ']')
                        {
                            token.tag = CLOSE_BRACKET;
                            state = S_STOP;
                        }
                        else if (c == '{')
                        {
                            token.tag = OPEN_BRACE;
                            state = S_STOP;
                        }
                        else if (c == '}')
                        {
                            token.tag = CLOSE_BRACE;
                            state = S_STOP;
                        }
                        else if (c == ',')
                        {
                            token.tag = COMMA;
                            state = S_STOP;
                        }
                        else if (c == ':')
                        {
                            token.tag = COLON;
                            state = S_STOP;
                        }
                        else if (c == '-')
                        {
                            token.tag = NUMBER;
                            state = S_NUMBER_MINUS;
                        }
                        else if (c == '0')
                        {
                            token.tag = NUMBER;
                            state = S_NUMBER_0;
                        }
                        else if (c >= '1' && c <= '9')
                        {
                            token.tag = NUMBER;
                            state = S_NUMBER_1_9;
                        }
                        else if (c == '"')
                        {
                            token.tag = STRING;
                            state = S_STRING;
                        }
                        else if (c == EOF)
                        {
                            state = S_STOP;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_LITERAL;
                        }
                        break; // S_START
                    case S_SPACE:
                        if (! (c == ' ' || c == '\u000a' || c == '\u000d' || c == '\u0009'))
                        {
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_SPACE
                    case S_NUMBER_MINUS:
                        if (c == '0')
                        {
                            state = S_NUMBER_0;
                        }
                        else if (c >= '1' && c <= '9')
                        {
                            state = S_NUMBER_1_9;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_NUMBER_MINUS
                    case S_NUMBER_0:
                        if (c == '.')
                        {
                            state = S_NUMBER_FRAC;
                        }
                        else if (c == 'e' || c == 'E')
                        {
                            state = S_NUMBER_EXP;
                        }
                        else
                        {
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_NUMBER_0
                    case S_NUMBER_1_9:
                        if (c == '.')
                        {
                            state = S_NUMBER_FRAC;
                        }
                        else if (c == 'e' || c == 'E')
                        {
                            state = S_NUMBER_EXP;
                        }
                        else if (! (c >= '0' && c <= '9'))
                        {
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_NUMBER_1_9
                    case S_NUMBER_FRAC:
                        if (c >= '0' && c <= '9')
                        {
                            state = S_NUMBER_FRAC_DIGITS;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_NUMBER_FRAC
                    case S_NUMBER_FRAC_DIGITS:
                        if (c == 'e' || c =='E')
                        {
                            state = S_NUMBER_EXP;
                        }
                        else if (! (c >= '0' && c <= '9'))
                        {
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_NUMBER_FRAC_DIGITS
                    case S_NUMBER_EXP:
                        if (c == '+' || c == '-')
                        {
                            state = S_NUMBER_EXP_SIGN;
                        }
                        else if (c >= '0' && c <= '9')
                        {
                            state = S_NUMBER_EXP_DIGITS;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_NUMBER_EXP
                    case S_NUMBER_EXP_SIGN:
                        if (c >= '0' && c <= '9')
                        {
                            state = S_NUMBER_EXP_DIGITS;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_NUMBER_EXP_SIGN
                    case S_NUMBER_EXP_DIGITS:
                        if (! (c >= '0' && c <= '9'))
                        {
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_NUMBER_EXP_DIGITS
                    case S_LITERAL:
                        if (c == ' ' || c == '\u000a' || c == '\u000d' || c == '\u0009' ||
                            c == '[' || c == ']' ||
                            c == '{' || c == '}' ||
                            c == ',' || c == ':' || c == '.' || c == '"' ||
                            c == '+' || c == '-' ||
                            (c >= '0' && c <= '9'))
                        {
                            state = S_STOP;
                            c = EOF;
                        }
                        if (c == EOF)
                        {
                            if (token.value.compare("true") == 0)
                                token.tag = L_TRUE;
                            else if (token.value.compare("false") == 0)
                                token.tag = L_FALSE;
                            else if (token.value.compare("null") == 0)
                                token.tag = L_NULL;
                        }
                        break; // S_LITERAL
                    case S_STRING:
                        if (c == '"')
                        {
                            state = S_STOP;
                        }
                        else if (c == '\\')
                        {
                            state = S_ESCAPE;
                        }
                        else if (c < '\u0020')
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_STRING
                    case S_ESCAPE:
                        if (c == '"' || c ==  '\\' || c == '/' || c == 'b' ||
                            c == 'n' || c == 'r' || c == 't')
                        {
                            state = S_STRING;
                        }
                        else if (c == 'u')
                        {
                            state = S_HEX1;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_ESCAPE
                    case S_HEX1:
                        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                        {
                            state = S_HEX2;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_HEX1
                    case S_HEX2:
                        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                        {
                            state = S_HEX3;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_HEX2
                    case S_HEX3:
                        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                        {
                            state = S_HEX4;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_HEX3
                    case S_HEX4:
                        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                        {
                            state = S_STRING;
                        }
                        else
                        {
                            token.tag = ERROR;
                            state = S_STOP;
                            c = EOF;
                        }
                        break; // S_HEX4
                    case S_STOP:
                        break;
                }

                if (c != EOF)
                {
                    token.value.append(1, static_cast<char>(c));
                    document.ignore(1);
                    ++column;
                    if (c == '\u000a')
                    {
                        ++line;
                        column = 1;
                    }
                }
                else if (document.eof())
                    break;
                else if (document.fail())
                {
                    token.tag = ERROR;
                    break;
                }
            }
        }
    };

    typedef std::function<void(unsigned line, unsigned column, const std::string & value)> JSONTokenRule;
    
    struct JSONRule : public JSONToken
    {
        std::vector<JSONTagType> rightSide;
      
        JSONRule(const JSONToken & right) : JSONToken(right) {}
        JSONRule(const JSONTagType & tag, const std::vector<JSONTagType> & rightSide) :
            JSONToken(tag),
            rightSide(rightSide) {}
    };
    
    class SimpleJSONParser : public libJSON::SimpleJSONParser
    {
    public:
        
        std::vector<JSONRule> rules;
        std::map<JSONTagType, JSONTokenRule> tokenRules;
        std::size_t maxRuleSize;
        
        SimpleJSONParser(bool verbose) : verbose(verbose)
        {
            tokenRules[SPACE] = std::bind(&SimpleJSONParser::space, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[L_NULL] = std::bind(&SimpleJSONParser::null, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[L_TRUE] = std::bind(&SimpleJSONParser::boolean, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[L_FALSE] = std::bind(&SimpleJSONParser::boolean, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[OPEN_BRACE] = std::bind(&SimpleJSONParser::startObject, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[CLOSE_BRACE] = std::bind(&SimpleJSONParser::endObject, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[OPEN_BRACKET] = std::bind(&SimpleJSONParser::startArray, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[CLOSE_BRACKET] = std::bind(&SimpleJSONParser::endArray, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[STRING] = std::bind(&SimpleJSONParser::string, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[NUMBER] = std::bind(&SimpleJSONParser::number, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[COMMA] = std::bind(&SimpleJSONParser::nextElement, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            tokenRules[COLON] = std::bind(&SimpleJSONParser::memberValue, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

            rules.push_back({value, {object}});
            rules.push_back({value, {array}});
            rules.push_back({value, {STRING}});
            rules.push_back({value, {NUMBER}});
            rules.push_back({value, {L_TRUE}});
            rules.push_back({value, {L_FALSE}});
            rules.push_back({value, {L_NULL}});
            rules.push_back({JSON, {ERROR}});
            rules.push_back({values, {value}});
            rules.push_back({members, {member}});
            rules.push_back({object, {OPEN_BRACE, CLOSE_BRACE}});
            rules.push_back({array, {OPEN_BRACKET, CLOSE_BRACKET}});
            rules.push_back({JSON, {value, eof}});
            rules.push_back({object, {OPEN_BRACE, members, CLOSE_BRACE}});
            rules.push_back({array, {OPEN_BRACKET, values, CLOSE_BRACKET}});
            rules.push_back({member, {STRING, COLON, value}});
            rules.push_back({values, {values, COMMA, value}});
            rules.push_back({members, {members, COMMA, member}});

            maxRuleSize = 3;
        }
        
        virtual SimpleJSONParser & operator<<(const std::string & document)
        {
            std::istringstream stream(document);
            return *this << stream;
        }
        
        virtual SimpleJSONParser & operator<<(std::istream & document)
        {
            if (document.fail() || document.eof())
            {
                error(0, 0, "empty or illegal JSON document");
                return *this;
            }
            
            JSONToken token;
            JSONTokenizer tokenizer(document);
            stack.clear();
            lastStackReduce = -1;
            
            startDocument();
            do
            {
                tokenizer.next(token);

                std::map<JSONTagType, JSONTokenRule>::iterator itokenRule = tokenRules.find(token.tag);
                if (itokenRule != tokenRules.end())
                    itokenRule->second(token.line, token.column, token.value);

                if (token.tag == SPACE)
                    continue;
                else if (token.tag == ERROR)
                    syntaxError(token);
                else
                    stack.push_back(token);
                
                if ( !reduceStack()
                  || (token.tag == eof && (stack.size() != 1 || stack[0].tag != JSON)) )
                    parseError();
            }
            while (token.tag != eof);

            endDocument();
            return *this;
        }

        virtual void boolean(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }
        
        virtual void endArray(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }
        
        virtual void endDocument()
        {
            if (verbose)
                std::clog << std::endl << "END DOCUMENT" << std::endl;
        }
        
        virtual void endObject(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }

        virtual void error(unsigned line, unsigned column, const std::string & text)
        {
            std::cerr << std::endl << "(" << line << "," << column << ") " << text;
        }

        virtual void memberValue(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }

        virtual void nextElement(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }
        
        virtual void null(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }

        virtual void number(unsigned line, unsigned column, const std::string & number)
        {
            if (verbose)
                std::clog << number;
        }
        
        virtual void space(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }
        
        virtual void startDocument()
        {
            if (verbose)
                std::clog << "START DOCUMENT" << std::endl;
        }
        
        virtual void startArray(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }

        virtual void startObject(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }

        virtual void string(unsigned line, unsigned column, const std::string & text)
        {
            if (verbose)
                std::clog << text;
        }

    protected:
        
        void parseError()
        {
            int from = lastStackReduce;
            if (from < 0)
                from = static_cast<int>(stack.size()) - 1;
            if (from >= (maxRuleSize - 1))
                from -= (maxRuleSize - 1);
            if (from >= 0)
            {
                std::ostringstream msg;
                msg << "JSON parse error at";
                for (int i = from; i < std::min(stack.size(), from + maxRuleSize); ++i)
                    msg << " '" << stack[i].value << "' =:: " << stack[i].tag;
                error(stack[from].line, stack[from].column, msg.str());
            }
        }
        
        bool reduceStack()
        {
            bool result = stack.empty();
            while (!stack.empty())
            {
                const JSONRule * candidate = 0;
                int candidateMatch = -1;
                for (const JSONRule & rule : rules)
                {
                    // try to find a matching rule
                    // the rule has higher precedence, if
                    // 1) a prefix from the right side of the rule matches a tail of the stack
                    // 2) a longer prefix has higher precedence
                    // 3) a shorter tail of the stack has higher precedence
                    if (candidate && rule.rightSide.size() < candidate->rightSide.size())
                        continue;
                    for (int i = static_cast<int>(stack.size()) - 1; i >= 0; --i)
                    {
                        bool mayMatch = true;
                        std::size_t mlength = std::min(rule.rightSide.size(), stack.size()-i);
                        for (std::size_t k = 0; k < mlength; ++k)
                            if (rule.rightSide[k] != stack[i+k].tag)
                            {
                                mayMatch = false;
                                break;
                            }
                        if (mayMatch && (candidate == 0 || rule.rightSide.size() > candidate->rightSide.size() || i > candidateMatch))
                        {
                            candidate = &rule;
                            candidateMatch = i;
                            break;
                        }
                    }
                }
                if (candidate)
                {
                    result = true;
                    if (candidate->rightSide.size() <= stack.size() - candidateMatch)
                    {
                        lastStackReduce = candidateMatch;
                        stack[lastStackReduce].tag = candidate->tag;
                        for (std::size_t i = 1; i < candidate->rightSide.size(); ++i)
                        {
                            stack[lastStackReduce].value.append(stack[lastStackReduce+1].value);
                            stack.erase(stack.begin() + (lastStackReduce + 1));
                        }
                        continue;
                    }
                }
                break;
            }
            return result;
        }
        
        void syntaxError(const JSONToken & token)
        {
            std::ostringstream msg;
            msg << "JSON " << token.tag << " at '" << token.value << "'";
            error(token.line, token.column, msg.str());
        }

        std::vector<JSONToken> stack;
        int lastStackReduce;
        bool verbose;
    };
}

#endif // SimpleJSONParser_hxx
