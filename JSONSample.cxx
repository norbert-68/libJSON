//
//  Created by Norbert Klose on 23.02.2019.
//  Copyright © 2019 Norbert Klose. All rights reserved.
//

#include <libJSON/libJSON.hxx>
#include <iostream>
#include <fstream>
#include <memory>
#include <cstdio>
#include <cstdlib>

int main(int argc, char * args[])
{
    try
    {
        std::unique_ptr<libJSON::SimpleJSONParser> simpleJSONParser(libJSON::SimpleJSONParser::Create(true));
     
        *simpleJSONParser
            //<< "{\n\n}"
            << "{\n  \"id\": -0.1e-01,\n  \"node_id\": \"MDEwOlJlcG9zaXRvcnk2OTE1NTc1OA==\"\n}"
            << "null"
            //<< "true true" // parse error
            << "false"
            //<< "\"\\x0" // syntax error
            //<< "\"\\u00XX" // syntax error
            << "\"\\r\\\\\\n\\u0020\""
            //<< "A0123" // parse error
            //<< "0123A" // parse error
            //<< "123 \n ABC" // syntax error
            //<< "[]{}:,.+-"
            //<< "+123.456e01"  // syntax error
            //<< "-0123.456e01" // syntax error
            //<< "-0.e-01" // syntax error
            << "\"{}\""
        ;
    }
    catch (const std::exception & exception)
    {
        std::cerr << std::endl << exception.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << std::endl << "unknown error" << std::endl;
    }
}
