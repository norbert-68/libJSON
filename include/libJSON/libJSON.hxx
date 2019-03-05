//
//  Created by Norbert Klose on 23.02.2019.
//  Copyright © 2019 Norbert Klose. All rights reserved.
//

#ifndef libJSON_hxx
#define libJSON_hxx

#include <libJSON/config.hxx>
#include <sstream>

namespace libJSON
{
    ///
    /// A simple callback based JSON parser.
    ///
    struct SimpleJSONParser
    {
        static SimpleJSONParser * Create(bool verbose = false);

        virtual SimpleJSONParser & operator<<(const std::string & document) = 0;
        virtual SimpleJSONParser & operator<<(std::istream & document) = 0;
        
        virtual void boolean(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void endArray(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void endDocument() = 0;
        virtual void endObject(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void error(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void memberValue(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void nextElement(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void null(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void number(unsigned line, unsigned column, const std::string & number) = 0;
        virtual void space(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void startArray(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void startDocument() = 0;
        virtual void startObject(unsigned line, unsigned column, const std::string & text) = 0;
        virtual void string(unsigned line, unsigned column, const std::string & text) = 0;
    };
}

#endif // libJSON_hxx
