/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
* XML writer helper
*/ 
#ifndef XMLWRITER_H
#define XMLWRITER_H

#include "utils.h"
#include <beastapi/beastapitypes.h>

namespace bex
{
class XMLWriter
{
private:
    static const unsigned int m_tabsize = 2;

public:
    class ScopedTag
    {
    public:
        ScopedTag(XMLWriter& xml, const std::string& name)
            : m_xml(xml)
        {
            m_xml.tag(name);
        }
        ~ScopedTag()
        {
            m_xml.endtag();
        }

    private:
        XMLWriter& m_xml;
    };

    XMLWriter(std::ostream& output)
        :
        m_output(output)
        ,
        m_tab(0)
    {
        m_output << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << std::endl;
    }

    ~XMLWriter()
    {
        while (endtag())
            ;
    }

    void tag(const std::string& name, bool inl = false)
    {
        m_output << tab() << "<" << name << ">";
        if (!inl)
        {
            m_output << std::endl;
        }
        m_tags.push_back(name);
        ++m_tab;
    }

    bool endtag(bool inl = false)
    {
        if (m_tags.empty())
        {
            return false;
        }
        --m_tab;
        if (!inl)
        {
            m_output << tab();
        }
        m_output << "</" << m_tags.back() << ">" << std::endl;
        m_tags.pop_back();
        return true;
    }

    template <class T>
    void data(const std::string& name, const T& value)
    {
        tag(name, true);
        m_output << value;
        endtag(true);
    }

    void data(const std::string& name, bool value)
    {
        tag(name, true);
        m_output << (value ? "true" : "false");
        endtag(true);
    }

private:
    std::ostream& m_output;
    std::vector<std::string> m_tags;
    unsigned int m_tab;

    std::string tab()
    {
        return std::string(m_tabsize * m_tab, ' ');
    }
};

template <>
inline void XMLWriter::data<ILBLinearRGB>(const std::string& name, const ILBLinearRGB& value)
{
    tag(name);
    data("r", value.r);
    data("g", value.g);
    data("b", value.b);
    endtag();
}

template <>
inline void XMLWriter::data<ILBLinearRGBA>(const std::string& name, const ILBLinearRGBA& value)
{
    tag(name);
    data("r", value.r);
    data("g", value.g);
    data("b", value.b);
    data("a", value.a);
    endtag();
}

template <>
inline void XMLWriter::data<ILBVec2>(const std::string& name, const ILBVec2& value)
{
    tag(name);
    data("x", value.x);
    data("y", value.y);
    endtag();
}

class ScopedTag
{
public:
    ScopedTag(XMLWriter& xml, const std::string& name)
        :
        m_xml(xml)
    {
        m_xml.tag(name);
    }
    ~ScopedTag()
    {
        m_xml.endtag();
    }

private:
    XMLWriter& m_xml;
};
}


#endif //XMLWRITER_H
