#pragma once

#include "Converter/Visitor/ConverterVisitorBase.h"

class ConverterVisitorElement
{
public:
    virtual ~ConverterVisitorElement() = default;

    virtual void Accept(ConverterVisitorBase& v) = 0;
};
