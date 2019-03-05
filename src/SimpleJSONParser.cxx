//
//  Created by Norbert Klose on 23.02.2019.
//  Copyright © 2019 Norbert Klose. All rights reserved.
//

#include "SimpleJSONParser.hxx"

namespace libJSON
{
    SimpleJSONParser * SimpleJSONParser::Create(bool verbose)
    {
        return new libJSONImpl::SimpleJSONParser(verbose);
    }
}
